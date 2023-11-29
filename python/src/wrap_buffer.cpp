#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "mlx/data/Buffer.h"

#include "wrap.h"
#include "wrap_dataset.h"

namespace {
class PyBufferIterator {
 public:
  PyBufferIterator(Buffer& buffer)
      : buffer_(buffer), iter_(0), size_(buffer.size()){};
  py::dict next() {
    if (iter_ >= size_) {
      throw py::stop_iteration();
    } else {
      Sample res;
      {
        py::gil_scoped_release release;
        res = buffer_.get(iter_++);
      }
      return mlx::pybind::to_py_sample(res);
    }
  };
  Buffer buffer_;
  int64_t iter_;
  int64_t size_;
};
} // namespace
namespace py = pybind11;
using namespace mlx::data;

void init_mlx_data_buffer(py::module& m) {
  py::class_<PyBufferIterator>(m, "BufferIterator")
      .def(
          "__iter__",
          [](PyBufferIterator& it) -> PyBufferIterator& { return it; })
      .def("__next__", &PyBufferIterator::next);

  auto buffer_class =
      py::class_<
          Buffer,
          std::unique_ptr<Buffer, mlx::pybind::NogilDeleter<Buffer>>>(
          m, "Buffer")
          .def("__iter__", [](Buffer& buf) { return PyBufferIterator(buf); })
          .def("size", &Buffer::size, py::call_guard<py::gil_scoped_release>())
          .def(
              "__len__",
              &Buffer::size,
              py::call_guard<py::gil_scoped_release>())
          .def(
              "__getitem__",
              [](const Buffer& b, int64_t idx) {
                Sample sample;
                {
                  py::gil_scoped_release release;
                  sample = b.get(idx);
                }
                py::dict pysample;
                for (auto& item : sample) {
                  pysample[py::str(item.first)] =
                      mlx::pybind::to_py_array(item.second);
                }
                return pysample;
              },
              py::arg("idx"))
          .def(
              "__repr__",
              [](const Buffer& b) {
                std::stringstream msg;
                msg << "Buffer(size=" << b.size() << ", keys={";
                if (b.size() > 0) {
                  Sample first = b.get(0);
                  int cnt = 0;
                  for (auto& item : first) {
                    if (cnt++ > 0) {
                      msg << ", ";
                    }
                    msg << "'" << item.first << "'";
                  }
                }
                msg << "})";

                return msg.str();
              })
          .def(
              "batch",
              [](const Buffer& b,
                 const std::variant<int64_t, std::vector<int64_t>>& batch_size,
                 const std::unordered_map<std::string, double> pad_values,
                 const std::unordered_map<std::string, int> dims) {
                if (auto int_batch = std::get_if<int64_t>(&batch_size)) {
                  return b.batch(*int_batch, pad_values, dims);
                } else {
                  return b.batch(
                      std::get<std::vector<int64_t>>(batch_size),
                      pad_values,
                      dims);
                }
              },
              py::call_guard<py::gil_scoped_release>(),
              py::arg("batch_size"),
              py::arg("pad") = std::unordered_map<std::string, double>(),
              py::arg("dim") = std::unordered_map<std::string, int>(),
              R"pbdoc(
                Creates batches from ``batch_size`` consecutive samples.

                When two samples have arrays that are not the same shape, the
                batch shape is the smallest shape that contains all samples in
                each dimension. The places that do not have values are filled
                with ``pad`` values.

                When a batch dimension is not provided, the arrays are stacked.
                If it is provided, the arrays are concatenated along that
                dimension.

                The following example showcases the use the ``dim`` argument.

                .. code-block:: python

                  import mlx.data as dx
                  import numpy as np

                  dset = dx.buffer_from_vector([{"x": np.random.randn(10, i+1)} for i in range(10)])

                  print(dset.batch(4)[0]["x"].shape)  # prints (4, 10, 4)
                  print(dset.batch(4)[1]["x"].shape)  # prints (4, 10, 8)

                  print(dset.batch(4, dim=dict(x=0))[0]["x"].shape)  # prints (40, 4)
                  print(dset.batch(4, dim=dict(x=0))[1]["x"].shape)  # prints (40, 8)

                  print(dset.batch(4, dim=dict(x=1))[0]["x"].shape)  # prints (10, 10)
                  print(dset.batch(4, dim=dict(x=1))[1]["x"].shape)  # prints (10, 26)

                Args:
                  batch_size (int): How many samples to gather in a batch.
                  pad (dict): The values to use for padding for each key in the samples.
                  dim (dict): The dimension to concatenate over.
              )pbdoc")
          .def(
              "partition",
              &Buffer::partition,
              py::call_guard<py::gil_scoped_release>(),
              py::arg("num_partitions"),
              py::arg("partition"),
              R"pbcopy(
                Equivalent to slicing the buffer with a step equal to
                ``num_partitions`` and starting offset of ``partition``.

                This can be used for distributed settings where different nodes
                should load different parts of a dataset.

                Args:
                  num_partitions (int): How many different partitions to split the buffer into.
                  partition (int): Which partition to use (0-based).
              )pbcopy")
          .def(
              "partition_if",
              &Buffer::partition_if,
              py::call_guard<py::gil_scoped_release>(),
              py::arg("cond"),
              py::arg("num_partitions"),
              py::arg("partition"),
              "Conditional :meth:`Buffer.partition`.")
          .def(
              "perm",
              &Buffer::perm,
              py::call_guard<py::gil_scoped_release>(),
              py::arg("perm"),
              R"pbcopy(
                Arbitrarily reorder the buffer with the provided indices.

                This operation actually performs arbitrary indexing of the
                buffer which means it can be used to slice or filter the buffer.

                It should be renamed in the future to avoid confusion.

                Args:
                  perm (list of ints): The indices defining the permutation/selection.
              )pbcopy")
          .def(
              "shuffle",
              &Buffer::shuffle,
              py::call_guard<py::gil_scoped_release>(),
              R"pbcopy(
                Create a random permutation of equal size to the buffer and
                apply it thus shuffling the buffer.
              )pbcopy")
          .def(
              "shuffle_if",
              &Buffer::shuffle_if,
              py::call_guard<py::gil_scoped_release>(),
              py::arg("cond"),
              R"pbcopy(
                Conditional :meth:`Buffer.shuffle`.
              )pbcopy")
          .def(
              "to_stream",
              &Buffer::to_stream,
              py::call_guard<py::gil_scoped_release>(),
              "Make a stream that yields the elements of the buffer.");
  mlx_data_export_dataset(buffer_class);

  m.def(
      "buffer_from_vector",
      [](const py::list& data) {
        std::vector<Sample> samples;
        for (auto& element : data) {
          auto sample = mlx::pybind::to_sample(element.cast<py::dict>());
          samples.push_back(std::move(sample));
        }
        return buffer_from_vector(std::move(samples));
      },
      py::arg("data"),
      R"pbcopy(
        Make a buffer from a list of dictionaries.

        This is the main factory method for making buffers to process data
        using MLX Data. For instance the following code makes a buffer of
        filenames and then lazily loads images from these filenames.

        .. code-block:: python

          import mlx.data as dx

          def list_files(root: Path):
              files = list(root.rglob("*.jpg"))
              classes = sorted(set(f.parent.name for f in files))
              classes = dict((v, i) for i, v in enumerate(classes))
              return [
                {"file": str(f.relative_to(root)).encode(), "label": classes[f.parent.name]}
                for f in files
              ]

          root = Path("/path/to/image/dataset")
          dset = (
            dx.buffer_from_vector(list_files(root))
            .load_image("file", prefix=str(root), output_key="image")
          )

        Args:
          data (list of dicts): The list of samples to make a buffer out of.
      )pbcopy");
  m.def(
      "files_from_tar",
      &files_from_tar,
      py::arg("tarfile"),
      py::arg("nested") = false,
      py::arg("num_threads") = 1,
      R"pbcopy(
        Return the list of files contained in a tar archive.

        If ``nested`` is true then the archive is indexed recursively ie
        archives contained in the file are also indexed. Moreover in that case
        the indexing can be parallelized using the argument ``num_threads``

        Args:
          tarfile (str): The path to the tar archive to be indexed.
          nested (bool): Enable recursive indexing of archives in archives.
            (default: False)
          num_threads (int): If nested archives are enabled then index the
            nested archives in parallel using ``num_threads`` threads. (default: 1)
      )pbcopy");
}
