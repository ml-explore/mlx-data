#include <pybind11/functional.h>
#include <pybind11/iostream.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include "mlx/data/Buffer.h"
#include "mlx/data/Stream.h"
#include "mlx/data/stream/Stream.h"

#include "wrap.h"
#include "wrap_dataset.h"

#include <iostream>

namespace py = pybind11;

using namespace mlx::data;

class PyStream : public stream::Stream {
 public:
  PyStream(py::function iterable_factory)
      : iterable_factory_(iterable_factory) {
    py::gil_scoped_acquire gil;
    next_ = iterable_factory_().attr("__iter__")().attr("__next__");
  }
  ~PyStream() {
    py::gil_scoped_acquire gil;

    iterable_factory_.release().dec_ref();
    next_.release().dec_ref();
  }

  virtual void reset() {
    py::gil_scoped_acquire gil;
    next_ = iterable_factory_().attr("__iter__")().attr("__next__");
  }

  virtual Sample next() const {
    Sample sample;
    {
      std::unique_lock lock(mutex_);
      py::gil_scoped_acquire gil;
      try {
        sample = mlx::pybind::to_sample(next_().cast<py::dict>());
      } catch (py::error_already_set& e) {
        if (e.matches(PyExc_StopIteration)) {
          // do nothing
        } else {
          throw;
        }
      } catch (...) {
        throw;
      }
    }
    return sample;
  }

 private:
  py::function iterable_factory_;
  py::function next_;
  mutable std::mutex mutex_;
};

void init_mlx_data_stream(py::module& m) {
  auto py_stream_next = [](Stream& stream) {
    Sample res;
    {
      py::gil_scoped_release release;
      res = stream.next();
    }
    auto pyres = mlx::pybind::to_py_sample(res);
    return pyres;
  };

  auto stream_class =
      py::class_<
          Stream,
          std::unique_ptr<Stream, mlx::pybind::NogilDeleter<Stream>>>(
          m, "Stream")
          .def("__iter__", [](Stream& stream) { return stream; })
          .def(
              "__next__",
              [](Stream& stream) {
                Sample res;
                {
                  py::gil_scoped_release release;
                  res = stream.next();
                }
                if (res.empty()) {
                  throw py::stop_iteration();
                } else {
                  py::dict pyres;
                  std::for_each(
                      res.begin(),
                      res.end(),
                      [&pyres](std::pair<
                               const std::string,
                               std::shared_ptr<mlx::data::Array>>& a) {
                        if (a.second == nullptr) {
                          throw std::runtime_error(
                              "Stream: empty value for " + a.first + "key");
                        }
                        pyres[py::str(a.first)] =
                            mlx::pybind::to_py_array(a.second);
                      });
                  return pyres;
                }
              })
          .def("__call__", py_stream_next)
          .def("__repr__", [](const Stream& s) { return "Stream()"; })
          .def("next", py_stream_next)
          .def(
              "reset",
              &Stream::reset,
              py::call_guard<py::gil_scoped_release>(),
              R"pbcopy(
                Reset the stream so that it can be iterated upon again.
              )pbcopy")
          .def(
              "batch",
              &Stream::batch,
              py::arg("batch_size"),
              py::call_guard<py::gil_scoped_release>(),
              py::arg("pad") = std::unordered_map<std::string, double>(),
              py::arg("dim") = std::unordered_map<std::string, int>())
          .def(
              "csv_reader_from_key",
              &Stream::csv_reader_from_key,
              py::call_guard<py::gil_scoped_release>(),
              py::arg("key"),
              py::arg("sep") = ',',
              py::arg("quote") = '"',
              py::arg("from_memory") = false,
              py::arg("local_prefix") = "",
              py::arg("file_fetcher") = nullptr,
              R"pbcopy(
                Read the csv file pointed to from the array at ``key`` and
                yield the contents as separate samples in the stream.

                This operation is similar to :func:`stream_csv_reader` but
                applied once for every sample in the stream and the samples
                from the resulting stream are returned until exhaustion.

                Args:
                  key (str): The sample key that contains the array we are operating on.
                  sep (str): The field separator in the csv file. (default: ',')
                  quote (str): The quotation character in the csv file. (default: '"')
                  from_memory (bool): Read the csv from the contents of the
                    array rather than treating the array as a filename. (default: False)
                  local_prefix (str): The filepath prefix to use to read the files. (default: '')
                  file_fetcher (mlx.data.core.FileFetcher, optional): A file fetcher to
                    read the csv files possibly from a remote location.
              )pbcopy")
          .def(
              "dynamic_batch",
              &Stream::dynamic_batch,
              py::call_guard<py::gil_scoped_release>(),
              py::arg("buffer_size"),
              py::arg("key"),
              py::arg("max_data_size") = -1,
              py::arg("pad") = std::unordered_map<std::string, double>(),
              py::arg("dim") = std::unordered_map<std::string, int>(),
              py::arg("shuffle") = false,
              py::arg("num_threads") = 1,
              R"pbcopy(
                Dynamic batching returns batches with approximately the same
                number of total elements.

                This is used to minimize padding and waste of computation when
                dealing with samples that can have large variance in sizes.

                For instance if we have a stream with a key 'tokens' and we
                want batches that contain approximately 16k tokens but the
                sample sizes vary from 64 to 1024 we can use dynamic batching
                to group together smaller samples to reduce padding but keep
                the total amount of work approximately constant.

                .. code-block:: python

                  import mlx.data as dx

                  def random_sample():
                      N = int(np.random.rand() * (1024 - 64) + 64)
                      return {"tokens": np.random.rand(N), "length": N}

                  def count_padding(sample):
                      return (sample["tokens"].shape[-1] - sample["length"]).sum()

                  dset = dx.buffer_from_vector([random_sample() for _ in range(10_000)])

                  # Compute the average padding size with naive batching
                  naive_padding = sum(count_padding(s) for s in dset.to_stream().batch(16))

                  # And with dynamic padding. Keep in mind that this also
                  # ensures that the number of tokens in a batch are
                  # approximately constant.
                  dynbatch_padding = sum(count_padding(s) for s in dset.to_stream().dynamic_batch(500, "tokens", 16*1024))

                  # Count the total valid tokens
                  valid_tokens = sum(d["length"] for d in dset)

                  print("Simple batching: ", naive_padding / (valid_tokens + naive_padding), " of tokens were padding")
                  print("Dynamic batching: ", dynbatch_padding / (valid_tokens + dynbatch_padding), " of tokens were padding")

                  # prints approximately 40% of tokens were padding in the first case
                  # and 5% of tokens in the second case

                Args:
                  buffer_size (int): How many buffers to consider when computing the dynamic batching
                  key (str): Which array's size to use for the dynamic batching
                  max_data_size (int): How many elements of the array at
                    ``key`` should each batch have. If less or equal to 0 then
                    batch the whole buffer in which case dynamic batching behaves
                    similar to ``batch``. (default: -1)
                  pad (dict): The values to use for padding for each key in the samples.
                  dim (dict): The dimension to concatenate over.
                  shuffle (bool): If true shuffle the batches before returning
                    them. Otherwise the larger batch sizes with smaller samples
                    will be first and so on. (default: False)
                  num_threads (int): How many parallel threads to use to fill the buffer. (default: 1)
              )pbcopy")
          .def(
              "line_reader_from_key",
              &Stream::line_reader_from_key,
              py::call_guard<py::gil_scoped_release>(),
              py::arg("key"),
              py::arg("dst_key"),
              py::arg("from_memory") = false,
              py::arg("unzip") = false,
              py::arg("local_prefix") = "",
              py::arg("file_fetcher") = nullptr,
              R"pbcopy(
                Read the file pointed to from the array at ``key`` and yield
                the lines as separate samples in the stream in the ``dst_key``.

                This operation is similar to :func:`stream_line_reader` but
                applied once for every sample in the stream and the samples
                from the resulting stream are returned until exhaustion.

                Args:
                  key (str): The sample key that contains the array we are operating on.
                  dst_key (str): The key to put the lines into.
                  from_memory (bool): Read the lines from the contents of the
                    array rather than treating the array as a filename. (default: False)
                  unzip (bool): Treat the file or memory stream as a compressed
                    stream and decompress it on the fly. (default: false)
                  local_prefix (str): The filepath prefix to use to read the files. (default: '')
                  file_fetcher (mlx.data.core.FileFetcher, optional): A file fetcher to
                    read the text files possibly from a remote location.
              )pbcopy")
          .def(
              "shuffle",
              &Stream::shuffle,
              py::call_guard<py::gil_scoped_release>(),
              py::arg("buffer_size"),
              R"pbcopy(
                Shuffle the contents of the stream using a shuffle buffer.

                A buffer of size ``buffer_size`` is filled with samples and
                then a random sample is returned from the buffer and replaced
                with a new one from the stream.

                This can achieve better shuffling than using
                :meth:`Stream.buffered` and then :meth:`Buffer.shuffle` because
                it is not bucketing the stream and a sample is a random sample
                from the first to the current sample of the underlying stream.

                To showcase the difference, the example below shuffles a stream
                of 100 numbers with a buffer of size 10 and measures the
                distance that a number moved from its original location.

                .. code-block:: python

                  import mlx.data as dx

                  numbers = dx.stream_python_iterable(lambda: (dict(x=i) for i in range(100)))
                  buffer_shuffle = numbers.buffered(10, lambda: buff.shuffle())
                  shuffle = numbers.shuffle(10)

                  numbers.reset()
                  print([abs(i-s["x"].item()) for i, s in enumerate(buffer_shuffle)])
                  # All printed numbers above are smaller than 10

                  numbers.reset()
                  print([abs(i-s["x"].item()) for i, s in enumerate(shuffle)])
                  # The numbers can be up to i+10 which means that the first
                  # element could even be yielded last!

                Args:
                  buffer_size (int): How big should the shuffle buffer be.
              )pbcopy")
          .def(
              "shuffle_if",
              &Stream::shuffle_if,
              py::call_guard<py::gil_scoped_release>(),
              py::arg("cond"),
              py::arg("buffer_size"),
              "Conditional :meth:`Stream.shuffle`.")
          .def(
              "partition",
              &Stream::partition,
              py::call_guard<py::gil_scoped_release>(),
              py::arg("num_partitions"),
              py::arg("partition"),
              R"pbcopy(
                For every ``num_partitions`` consecutive samples return the ``partition``-th.

                This can be used for distributed settings where different nodes
                should load different parts of a dataset.

                Args:
                  num_partitions (int): How many different partitions to split the stream into.
                  partition (int): Which partition to use (0-based).
              )pbcopy")
          .def(
              "partition_if",
              &Stream::partition_if,
              py::call_guard<py::gil_scoped_release>(),
              py::arg("cond"),
              py::arg("num_partitions"),
              py::arg("partition"),
              "Conditional :meth:`Stream.partition`.")
          .def(
              "prefetch",
              &Stream::prefetch,
              py::call_guard<py::gil_scoped_release>(),
              py::arg("prefetch_size"),
              py::arg("num_threads"),
              R"pbcopy(
                Fetch samples in background threads.

                This operation is the workhorse of data loading. It uses
                ``num_threads`` background threads and fetches
                ``prefetch_size`` samples so that they are ready to be used
                when needed.

                Prefetch can be used both to parallelize operations but also to
                overlap computation with data loading in a background thread.

                .. code-block:: python

                  # The final prefetch is parallelizing the whole pipeline and
                  # ensures that images are going to be available for training.
                  dset = (
                    dset
                    .load_image("image")
                    .image_resize_smallest_side("image", 256)
                    .image_center_crop("image", 256, 256)
                    .batch(32)
                    .prefetch(8, 8)
                  )

                Args:
                  prefetch_size (int): How many samples to prefetch.
                  num_threads (int): How many background threads to launch.
              )pbcopy")
          .def(
              "prefetch_if",
              &Stream::prefetch_if,
              py::call_guard<py::gil_scoped_release>(),
              py::arg("cond"),
              py::arg("prefetch_size"),
              py::arg("num_threads"),
              "Conditional :meth:`Stream.prefetch`.")
          .def(
              "repeat",
              &Stream::repeat,
              py::call_guard<py::gil_scoped_release>(),
              py::arg("num_time"),
              R"pbcopy(
                Reset the stream ``num_time`` times before declaring it exhausted.

                Args:
                  num_time (int): How many times to repeat the dataset.
              )pbcopy")
          .def(
              "sliding_window",
              &Stream::sliding_window,
              py::call_guard<py::gil_scoped_release>(),
              py::arg("key"),
              py::arg("size"),
              py::arg("stride"),
              py::arg("dim") = -1,
              R"pbcopy(
                Creates sample by sliding a window over the array at ``key``.

                Commonly used in sequence processing pipelines to deal with
                very larger documents.

                .. code-block:: python

                  import mlx.data as dx

                  dset = dx.buffer_from_vector({"x": np.arange(10), "unchanged_keys": 10}).to_stream()

                  for sample in dset.sliding_window("x", 3, 2):
                      print(sample["x"])

                  # prints
                  # [0, 1, 2]
                  # [2, 3, 4]
                  # [4, 5, 6]
                  # [6, 7, 8]
                  # [8, 9]

                Args:
                  key (str): The sample key that contains the array we are operating on.
                  size (int): The size of the sliding window
                  stride (int): The stride of the sliding window
                  dim (int): Which dimension are we sliding the window over. (default: -1)
              )pbcopy")
          .def(
              "to_buffer",
              &Stream::to_buffer,
              py::call_guard<py::gil_scoped_release>(),
              "Gather the samples from the stream into a buffer.")
          .def(
              "buffered",
              [](Stream& stream,
                 int64_t buffer_size,
                 std::optional<std::function<Buffer(const Buffer)>> on_refill,
                 int num_thread) -> Stream {
                // onRefill will be called at construction
                py::gil_scoped_release release;
                if (on_refill) {
                  return stream.buffered(
                      buffer_size, on_refill.value(), num_thread);
                }
                return stream.buffered(
                    buffer_size,
                    [](const Buffer buf) { return buf; },
                    num_thread);
              },
              py::arg("buffer_size"),
              py::arg("on_refill") = std::nullopt,
              py::arg("num_threads") = 1,
              R"pbcopy(
                Gather a buffer of samples, apply a function on the buffer and
                then iterate over the buffer samples.

                This function can be used to implement any logic that requires
                a buffer of samples. For instance it can be used for pseudo
                shuffling by shuffling the buffer or sorting the buffer based
                on sequence lengths to minimize padding and wasted computation.

                .. note::
                   Shuffling the buffer is not the same as a shuffle buffer.
                   In a shuffle buffer of size 1000 the 500th element
                   is a random choice in the range 0-1500 while here it would
                   be a random choice in the range 0-1000. If you need a
                   shuffle buffer use :meth:`Stream.shuffle` .

                The following examples demonstrate the use of ``buffered``.

                .. code-block:: python

                    # Pseudo shuffling
                    dset = dset.buffered(10000, lambda buff: buff.shuffle(), num_threads=8)

                    # Sort by the length of samples in order to minimize padding when batching.
                    # You might also want to check out `dynamic_batch`
                    def sort_by_length(buff):
                        perm = sorted(range(len(buff)), key=len(buff[i]["x"]))
                        return buff.perm(perm)
                    dset = dset.buffered(128 * batch_size, sort_by_length, num_threads=8)

                Args:
                  buffer_size (int): How big should the buffer be.
                  on_refill (callable, optional): The function to apply to the buffer. (default: identity)
                  num_threads (int): How many parallel threads to use when filling the buffer. (default: 1)
              )pbcopy");

  m.def(
      "stream_csv_reader",
      [](py::object file,
         char sep,
         char quote,
         const std::string& local_prefix,
         std::shared_ptr<core::FileFetcher> file_fetcher,
         std::shared_ptr<core::FileFetcherHandle> file_fetcher_handle) {
        if (py::isinstance<py::str>(file)) {
          return stream_csv_reader(
              file.cast<std::string>(), sep, quote, local_prefix, file_fetcher);
        } else {
          auto in = std::make_shared<mlx::pybind::py_istream>(file, 4096);
          return stream_csv_reader(in, sep, quote, file_fetcher_handle);
        }
      },
      py::arg("file"),
      py::arg("sep") = ',',
      py::arg("quote") = '"',
      py::kw_only(),
      py::arg("local_prefix") = "",
      py::arg("file_fetcher") = nullptr,
      py::arg("file_fetcher_handle") = nullptr,
      R"pbcopy(
        Stream samples from a csv file.

        The file can be given as a filename or any python object that has a
        ``read()`` and a ``seek()`` method. Optionally a file fetcher can be
        passed to fetch the file from a remote location.

        In the case that a file object was created from a file fetched by an MLX
        file fetcher, then a handle can be passed (the return value of fetch) to
        ensure that the file is kept on disk for the lifetime of the stream.

        Args:
          file (str or python readable object): The file to read the csv from.
          sep (str): The field separator in the csv file. (default: ',')
          quote (str): The quotation character in the csv file. (default: '"')
          local_prefix (str): The filepath prefix to use to read the files. (default: '')
          file_fetcher (mlx.data.core.FileFetcher, optional): A file fetcher to
            read the csv files possibly from a remote location.
          file_fetcher_handle (mlx.data.core.FileFetcherHandle, optional): A
            handle to ensure that the file is kept on disk if a stream is
            passed instead of a filename.
      )pbcopy");

  m.def(
      "stream_line_reader",
      [](py::object file,
         const std::string& key,
         bool unzip,
         const std::string& local_prefix,
         std::shared_ptr<core::FileFetcher> file_fetcher,
         std::shared_ptr<core::FileFetcherHandle> file_fetcher_handle) {
        if (py::isinstance<py::str>(file)) {
          return stream_line_reader(
              file.cast<std::string>(), key, unzip, local_prefix, file_fetcher);
        } else {
          auto in = std::make_shared<mlx::pybind::py_istream>(file, 4096);
          return stream_line_reader(in, key, unzip, file_fetcher_handle);
        }
      },
      py::arg("file"),
      py::arg("key"),
      py::arg("unzip") = false,
      py::kw_only(),
      py::arg("local_prefix") = "",
      py::arg("file_fetcher") = nullptr,
      py::arg("file_fetcher_handle") = nullptr,
      R"pbcopy(
        Stream lines from a file.

        Similar to :func:`stream_csv_reader`, a file can be a filename or a
        python object with a ``read()`` and a ``seek()``.

        .. note::
           The newline characters are **not** included in the samples.

        Args:
          file (str or python readable object): The file to read the csv from.
          key (str): The destination key for the lines of the file.
          unzip (bool): Treat the file as a compressed stream and decompress it
            on the fly. (default: False)
          local_prefix (str): The filepath prefix to use to read the files. (default: '')
          file_fetcher (mlx.data.core.FileFetcher, optional): A file fetcher to
            read the csv files possibly from a remote location.
          file_fetcher_handle (mlx.data.core.FileFetcherHandle, optional): A
            handle to ensure that the file is kept on disk if a stream is
            passed instead of a filename.
      )pbcopy");

  m.def(
      "stream_csv_reader_from_string",
      &stream_csv_reader_from_string,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("content"),
      py::arg("sep") = ',',
      py::arg("quote") = '"',
      R"pbcopy(
        Stream samples from a csv file provided as a string.

        The same can be achieved with :func:`stream_csv_reader` and using an
        ``io.StringIO`` object as follows:

        .. code-block:: python

          from io import StringIO
          import mlx.data as dx

          # dset1 and dset2 are exactly the same.
          my_csv_string = "... csv content here ..."
          dset1 = dx.stream_csv_reader_from_string(my_csv_string)
          dset2 = dx.stream_csv_reader(StringIO(my_csv_string))

        Args:
          content (str): The string containing the content of a csv file.
          sep (str): The field separator in the csv file. (default: ',')
          quote (str): The quotation character in the csv file. (default: '"')
      )pbcopy");

  m.def(
      "stream_python_iterable",
      [](py::function iterable_factory) {
        auto stream = std::make_shared<PyStream>(iterable_factory);
        return Stream(stream);
      },
      py::arg("iterable_factory"),
      R"pbcopy(
        Stream samples from a python iterable.

        This method allows to make an MLX data stream from any python iterable
        of samples.

        .. code-block:: python

          import mlx.data as dx

          # We cannot make such a buffer as it would require more than 40GB of
          # memory just to hold the integers.
          dset = dx.stream_python_iterable(lambda: (dict(x=i) for i in range(10**10)))
          print(next(dset)) # {'x': 0}
          print(next(dset)) # {'x': 1}
          dset.reset()
          print(next(dset)) # {'x': 0}
          print(next(dset)) # {'x': 1}

          evens = dset.sample_transform(lambda s: s if s["x"] % 2 == 0 else dict())
          print(next(evens)) # {'x': 2}
          print(next(evens)) # {'x': 4}

        .. note::
            This function does not take the iterable directly but instead a
            function that returns an iterable. This allows us to reset the
            stream and restart the iteration.

        Args:
          iterable_factory (callable): A function that returns a python
            iterable object.
      )pbcopy");

  mlx_data_export_dataset(stream_class);
}
