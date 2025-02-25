// Copyright © 2023 Apple Inc.

#include <pybind11/functional.h>
#include <pybind11/iostream.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include "mlx/data/Stream.h"

#include "wrap.h"

namespace py = pybind11;

using namespace mlx::data;

namespace {

template <typename T>
std::vector<T> to_vector(std::variant<std::vector<T>, T> x) {
  if (std::holds_alternative<T>(x)) {
    return {std::get<T>(x)};
  }
  return std::move(std::get<std::vector<T>>(x));
}

template <class T, typename P>
void mlx_data_export_dataset(py::class_<T, P>& base) {
  base.def(
      "filter_by_shape",
      &T::filter_by_shape,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("dim"),
      py::arg("low") = -1,
      py::arg("high") = -1,
      R"pbdoc(
        Filter samples based on the shape of the array.

        Args:
          key (str): The sample key that contains the array we are operating on.
          dim (int): The shape dimension based on which we are filtering.
          low (int): Minimum acceptable size for the dimension (inclusive).
          high (int): Maximum acceptable size for the dimension (inclusive). If
            negative size is given then it is assumed we have no upper limit.
      )pbdoc");
  base.def(
      "filter_by_shape_if",
      &T::filter_by_shape_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("dim"),
      py::arg("low") = -1,
      py::arg("high") = -1,
      "Conditional :meth:`Buffer.filter_by_shape`.");

  base.def(
      "filter_key",
      &T::filter_key,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("remove") = false,
      R"pbdoc(
        Transform the samples to either only contain this ``key`` or never
        contain this ``key`` based on the value of ``remove``.

        Args:
          key (str): The key to keep or remove.
          remove (bool): If set to True then remove this key instead of keeping
            it (default: False).
      )pbdoc");
  base.def(
      "filter_key_if",
      &T::filter_key_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("remove") = false,
      "Conditional :meth:`Buffer.filter_key`.");

  base.def(
      "image_center_crop",
      &T::image_center_crop,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("w"),
      py::arg("h"),
      py::arg("output_key") = "",
      R"pbdoc(
        Center crop the image at ``key``.

        Args:
          key (str): The sample key that contains the array we are operating on.
          w (int): The target width.
          h (int): The target height.
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
      )pbdoc");
  base.def(
      "image_center_crop_if",
      &T::image_center_crop_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("w"),
      py::arg("h"),
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.image_center_crop`.");

  base.def(
      "image_channel_reduction",
      &T::image_channel_reduction,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("preset") = "default",
      py::arg("output_key") = "",
      R"pbdoc(
        Reduce an RGB image to gray-scale with various weights for red, green
        and blue.

        .. list-table::
           :header-rows: 1

           * - Preset Name
             - Red weight
             - Green weight
             - Blue weight
           * - default/rec601
             - 0.299
             - 0.587
             - 0.114
           * - rec709
             - 0.2126
             - 0.7152
             - 0.0722
           * - rec2020
             - 0.2627
             - 0.678
             - 0.0593
           * - green
             - 0
             - 1
             - 0

        Args:
          key (str): The sample key that contains the array we are operating on.
          preset (default|rec601|rec709|rec2020|green): The preset defining the reduction weights to gray scale.
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
      )pbdoc");
  base.def(
      "image_channel_reduction_if",
      &T::image_channel_reduction_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("preset") = "default",
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.image_channel_reduction`.");

  base.def(
      "image_random_area_crop",
      &T::image_random_area_crop,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("area_range"),
      py::arg("aspect_ratio_range"),
      py::arg("num_trial") = 10,
      py::arg("output_key") = "",
      R"pbdoc(
        Crop the image randomly such that the result is a portion of the
        original area and within the given aspect ratio range.

        The random crop is found using rejection sampling, namely we sample a
        random width within the range of possible widths, then a random height
        within the range of possible heights. Finally, we check if the area and
        aspect ratio constraints are met before cropping the image.

        If we can't sample a random crop that meets the constraints the
        original image is returned.

        Example:

        .. code-block:: python

          # Extract a random square crop that is from 50% to 100% the original
          # image area
          dset = dset.image_random_area_crop("image", (0.5, 1.0), (1.0, 1.0))

          # Extract a random crop that is 50% to 75% of the original area and
          # from square to 3:2 aspect ratio.
          dset = dset.image_random_area_crop("image", (0.5, 0.75), (1.0, 1.5))

        Args:
          key (str): The sample key that contains the array we are operating on.
          area_range (tuple of floats): A minimum and maximum area portion for the crop.
          aspect_ratio_range (tuple of floats): A minimum and maximum aspect
            ratio for the crop. The aspect ratio is defined as the width
            divided by the height of the image.
          num_trial (int): How many rejection sampling attempts to perform. (default: 10)
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
      )pbdoc");
  base.def(
      "image_random_area_crop_if",
      &T::image_random_area_crop_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("area_range"),
      py::arg("aspect_ratio_range"),
      py::arg("num_trial") = 10,
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.image_random_area_crop`.");

  base.def(
      "image_random_crop",
      &T::image_random_crop,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("w"),
      py::arg("h"),
      py::arg("output_key") = "",
      R"pbdoc(
        Extract a random crop of the requested size.

        This operation will fail if the image is smaller than the requested
        width and height.

        Args:
          key (str): The sample key that contains the array we are operating on.
          w (int): The width of the result.
          h (int): The height of the result.
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
      )pbdoc");
  base.def(
      "image_random_crop_if",
      &T::image_random_crop_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("w"),
      py::arg("h"),
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.image_random_crop`.");

  base.def(
      "image_random_h_flip",
      &T::image_random_h_flip,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("prob"),
      py::arg("output_key") = "",
      R"pbdoc(
        Horizontally flip the image ``prob`` percent of the time.

        Args:
          key (str): The sample key that contains the array we are operating on.
          prob (float): The probability to flip an image.
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
      )pbdoc");
  base.def(
      "image_random_h_flip_if",
      &T::image_random_h_flip_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("prob"),
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.image_random_h_flip`.");

  base.def(
      "image_resize",
      &T::image_resize,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("w"),
      py::arg("h"),
      py::arg("output_key") = "",
      R"pbdoc(
        Resize the image to the requested size.

        Args:
          key (str): The sample key that contains the array we are operating on.
          w (int): The width of the result.
          h (int): The height of the result.
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
      )pbdoc");
  base.def(
      "image_resize_if",
      &T::image_resize_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("w"),
      py::arg("h"),
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.image_resize`.");

  base.def(
      "image_resize_smallest_side",
      &T::image_resize_smallest_side,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("size"),
      py::arg("output_key") = "",
      R"pbdoc(
        Resize the image such that its smallest side is ``size``.

        This operation combined with a center crop or a random area crop is the
        backbone of many image pipelines.

        Args:
          key (str): The sample key that contains the array we are operating on.
          size (int): The size of the smallest side of the result.
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
      )pbdoc");
  base.def(
      "image_resize_smallest_side_if",
      &T::image_resize_smallest_side_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("size"),
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.image_resize_smallest_side`.");

  base.def(
      "image_rotate",
      &T::image_rotate,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("angle"),
      py::arg("crop") = false,
      py::arg("output_key") = "",
      R"pbdoc(
        Rotate an image around its center point.

        Args:
          key (str): The sample key that contains the array we are operating on.
          angle (float): The angle of rotation in degrees.
          crop (bool): Whether to crop the result to the original image's size.
            (default: False)
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
      )pbdoc");
  base.def(
      "image_rotate_if",
      &T::image_rotate_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("angle"),
      py::arg("crop") = false,
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.image_rotate`.");

  base.def(
      "key_transform",
      [](const T& dataset,
         const std::string& ikey,
         std::function<py::array(py::array)> op,
         const std::string& okey) -> T {
        auto op_func = [op](const std::shared_ptr<const mlx::data::Array>& x) {
          py::gil_scoped_acquire gil;
          auto z = mlx::pybind::to_py_array(x);
          return mlx::pybind::to_array(op(z));
        };
        return dataset.key_transform(ikey, op_func, okey);
      },
      py::arg("key"),
      py::arg("func"),
      py::arg("output_key") = "",
      R"pbdoc(
        Apply the python function ``func`` on the arrays in the selected ``key``.

        The function should return a value that can be cast to an array ie
        something implementing the buffer protocol.

        An example use of the transformation is shown below:

        .. code-block:: python

          from mlx.data.datasets import load_mnist

          mnist = (
              load_mnist()
              .key_transform("image", lambda x: x.astype("float32") / 255)
          )

        Args:
          key (str): The sample key that contains the array we are operating on.
          func (callable): The function to apply.
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
      )pbdoc");
  base.def(
      "key_transform_if",
      [](const T& dataset,
         bool cond,
         const std::string& ikey,
         std::function<py::array(py::array)> op,
         const std::string& okey) -> T {
        auto op_func = [op](const std::shared_ptr<const mlx::data::Array>& x) {
          py::gil_scoped_acquire gil;
          auto z = mlx::pybind::to_py_array(x);
          return mlx::pybind::to_array(op(z));
        };
        return dataset.key_transform_if(cond, ikey, op_func, okey);
      },
      py::arg("cond"),
      py::arg("key"),
      py::arg("func"),
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.key_transform`.");

  base.def(
      "sample_transform",
      [](const T& dataset, std::function<py::dict(py::dict)> op) -> T {
        auto op_func = [op](const Sample& sample) {
          py::gil_scoped_acquire gil;
          return mlx::pybind::to_sample(op(mlx::pybind::to_py_sample(sample)));
        };
        return dataset.sample_transform(op_func);
      },
      py::arg("func"),
      R"pbdoc(
        Apply the python function ``func`` on whole samples.

        The function should return a dictionary of arrays or values that can be
        cast to arrays (buffers, scalars etc). When used with :class:`Stream`
        it can also be used to skip samples by returning an empty dictionary.

        This transformation is very powerful but it should be used with caution
        given that python is slightly plagued by the global interpreter lock.
        See the `Quick Start <../../quick_start.html#about-the-gil>`_ for more.

        Args:
          func (callable): The function to apply.
      )pbdoc");
  base.def(
      "sample_transform_if",
      [](const T& dataset,
         bool cond,
         std::function<py::dict(py::dict)> op) -> T {
        auto op_func = [op](const Sample& sample) {
          py::gil_scoped_acquire gil;
          return mlx::pybind::to_sample(op(mlx::pybind::to_py_sample(sample)));
        };
        return dataset.sample_transform_if(cond, op_func);
      },
      py::arg("cond"),
      py::arg("func"),
      "Conditional :meth:`Buffer.sample_transform`.");

  base.def(
      "load_audio",
      [](T& dataset,
         const std::string& key,
         const std::string& prefix,
         bool info,
         bool from_memory,
         LoadAudioInfo info_type,
         int sample_rate,
         const std::string& resampling_quality,
         const std::string& output_key) -> T {
        static const std::unordered_map<std::string, LoadAudioResamplingQuality>
            e_resampling_quality = {
                {"sinc-best", LoadAudioResamplingQuality::SincBest},
                {"sinc-medium", LoadAudioResamplingQuality::SincMedium},
                {"sinc-fastest", LoadAudioResamplingQuality::SincFastest},
                {"zero-order-hold", LoadAudioResamplingQuality::ZeroOrderHold},
                {"linear", LoadAudioResamplingQuality::Linear}};

        auto it = e_resampling_quality.find(resampling_quality);
        if (it == e_resampling_quality.end()) {
          throw std::runtime_error(
              "invalid resampling quality (expected {sinc-best, sinc-medium, sinc-fastest, zero-order-hold, linear})");
        }
        return dataset.load_audio(
            key,
            prefix,
            info,
            from_memory,
            info_type,
            sample_rate,
            it->second,
            output_key);
      },
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("prefix") = "",
      py::arg("info") = false,
      py::arg("from_memory") = false,
      py::arg("info_type") = LoadAudioInfo::All,
      py::arg("sample_rate") = 0,
      py::arg("resampling_quality") = "sinc-fastest",
      py::arg("output_key") = "",
      R"pbcopy(
        Load an audio file.

        Decodes audio from an audio file on disk or in memory. It can also load
        the audio info instead. If a sample rate is provided it resamples the
        audio to the requested rate.

        If ``info_type`` is set to ``LoadAudioInfo.All`` then the result will
        contain the number of frames, the number of channels and the sampling
        rate of the audio file.

        It can also be set to ``LoadAudioInfo.NumFrames``,
        ``LoadAudioInfo.NumChannels``, ``LoadAudioInfo.SampleRate`` and
        ``LoadAudioInfo.NumSeconds`` to load the corresponding information.

        The following example filters from the ``Stream`` all audio files that
        are less than 10 seconds long.

        .. code-block:: python

          dset = (
            dset
            .load_audio("audio_file", info=True, info_type=LoadAudioInfo.NumSeconds, output_key="audio_info")
            .sample_transform(lambda s: s if s["audio_info"] >= 10 else dict())
          )

        Args:
          key (str): The sample key that contains the array we are operating on.
          prefix (str): The filepath prefix to use when loading the audio files.
          info (bool): If set to True load the audio file information instead
            of the data. (default: False)
          from_memory (bool): If true assume the file contents are in the array
            instead of the file name. (default: False)
          info_type (LoadAudioInfo): If ``info`` is True then load this type of
            audio metadata.
          sample_rate (int): The requested sample frequency in frames per
            second. If it is set to 0 then no resampling is performed. (default: 0)
          resampling_quality
            (sinc-fastest|sinc-medium|sinc-best|zero-order-hold|linear): Chooses
            the audio resampling quality if resampling is performed. (default:
            sinc-fastest)
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
      )pbcopy");
  base.def(
      "load_audio_if",
      [](T& dataset,
         bool cond,
         const std::string& key,
         const std::string& prefix,
         bool info,
         bool from_memory,
         LoadAudioInfo info_type,
         int sample_rate,
         const std::string& resampling_quality,
         const std::string& output_key) -> T {
        static const std::unordered_map<std::string, LoadAudioResamplingQuality>
            e_resampling_quality = {
                {"sinc-best", LoadAudioResamplingQuality::SincBest},
                {"sinc-medium", LoadAudioResamplingQuality::SincMedium},
                {"sinc-fastest", LoadAudioResamplingQuality::SincFastest},
                {"zero-order-hold", LoadAudioResamplingQuality::ZeroOrderHold},
                {"linear", LoadAudioResamplingQuality::Linear}};

        auto it = e_resampling_quality.find(resampling_quality);
        if (it == e_resampling_quality.end()) {
          throw std::runtime_error(
              "invalid resampling quality (expected {sinc-best, sinc-medium, sinc-fastest, zero-order-hold, linear})");
        }
        return dataset.load_audio_if(
            cond,
            key,
            prefix,
            info,
            from_memory,
            info_type,
            sample_rate,
            it->second,
            output_key);
      },
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("prefix") = "",
      py::arg("info") = false,
      py::arg("from_memory") = false,
      py::arg("info_type") = LoadAudioInfo::All,
      py::arg("sample_rate") = 0,
      py::arg("resampling_quality") = "sinc-fastest",
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.load_audio`.");

  base.def(
      "load_file",
      &T::load_file,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("prefix") = "",
      py::arg("output_key") = "",
      R"pbcopy(
        Load the contents of a file.

        It opens the file pointed by ``key`` in binary mode and reads its contents.

        Args:
          key (str): The sample key that contains the array we are operating on.
          prefix (str): The filepath prefix to use when loading the files.
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
      )pbcopy");
  base.def(
      "load_file_if",
      &T::load_file_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("prefix") = "",
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.load_file`.");

  base.def(
      "load_image",
      &T::load_image,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("prefix") = "",
      py::arg("info") = false,
      py::arg("format") = "RGB",
      py::arg("from_memory") = false,
      py::arg("output_key") = "",
      R"pbcopy(
        Load an image file.

        Loads an image from an image file on disk or in memory. It can also
        load the image info instead.

        .. note::
           The format is ignored for now.

        Args:
          key (str): The sample key that contains the array we are operating on.
          prefix (str): The filepath prefix to use when loading the files. (default: '')
          info (bool): If True load the image width and height instead of the
            image data. (default: False)
          format (str): Currently ignored but in the future it should decide
            whether to load the alpha channel or map the channels to some other
            space (e.g. YCbCr) (default: RGB).
          from_memory (bool): If true assume the file contents are in the array
            instead of the file name. (default: False)
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
      )pbcopy");
  base.def(
      "load_image_if",
      &T::load_image_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("prefix") = "",
      py::arg("info") = false,
      py::arg("format") = "RGB",
      py::arg("from_memory") = false,
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.load_image`.");

  base.def(
      "load_numpy",
      &T::load_numpy,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("prefix") = "",
      py::arg("from_memory") = false,
      py::arg("output_key") = "",
      R"pbcopy(
        Load an array from a .npy file.

        Args:
          key (str): The sample key that contains the array we are operating on.
          prefix (str): The filepath prefix to use when loading the files. (default: '')
          from_memory (bool): If true assume the file contents are in the array
            instead of the file name. (default: False)
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
      )pbcopy");
  base.def(
      "load_numpy_if",
      &T::load_numpy_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("prefix") = "",
      py::arg("from_memory") = false,
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.load_numpy`.");

  base.def(
      "load_video",
      &T::load_video,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("prefix") = "",
      py::arg("info") = false,
      py::arg("from_memory") = false,
      py::arg("output_key") = "",
      R"pbcopy(
        Load a video file.

        Decodes a video file to memory from a file or from memory. If ``info``
        is true then it, instead, reads the information of the video, namely
        width, height and number of frames.

        Args:
          key (str): The sample key that contains the array we are operating on.
          prefix (str): The filepath prefix to use when loading the files. (default: '')
          info (bool): If True load the video width, height and frames instead
            of the video data. (default: False)
          from_memory (bool): If true assume the file contents are in the array
            instead of the file name. (default: False)
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
      )pbcopy");
  base.def(
      "load_video_if",
      &T::load_video_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("prefix") = "",
      py::arg("info") = false,
      py::arg("from_memory") = false,
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.load_video`.");

  base.def(
      "pad",
      &T::pad,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("dim"),
      py::arg("lpad"),
      py::arg("rpad"),
      py::arg("pad_value"),
      py::arg("output_key") = "",
      R"pbcopy(
        Pad the array at ``key``.

        The following example inserts a space character at the beginning of the
        array at key 'text'.

        .. code-block:: python

          dset = dset.pad("text", 0, 1, 0, ord(" "))

        Args:
          key (str): The sample key that contains the array we are operating on.
          dim (int): Which dimension of the array to pad.
          lpad (int): How many positions to pad on the left (beginning) of the array.
          rpad (int): How many positions to pad on the right (end) of the array.
          pad_value (float): What to pad with.
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
      )pbcopy");
  base.def(
      "pad_if",
      &T::pad_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("dim"),
      py::arg("lpad"),
      py::arg("rpad"),
      py::arg("pad_value"),
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.pad`.");

  base.def(
      "pad_to_multiple",
      &T::pad_to_multiple,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("dim"),
      py::arg("pad_multiple"),
      py::arg("pad_value"),
      py::arg("output_key") = "",
      R"pbcopy(
        Pad the end of an array such that its size is a multiple of ``pad_multiple``.

        Args:
          key (str): The sample key that contains the array we are operating on.
          dim (int): Which dimension of the array to pad.
          pad_multiple (int): The result should be a multiple of ``pad_multiple``.
          pad_value (float): What to pad with.
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
      )pbcopy");
  base.def(
      "pad_to_multiple_if",
      &T::pad_to_multiple_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("dim"),
      py::arg("pad_multiple"),
      py::arg("pad_value"),
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.pad_to_multiple`.");

  base.def(
      "pad_to_size",
      py::overload_cast<
          const std::string&,
          int,
          int64_t,
          double,
          const std::string&>(&T::pad_to_size, py::const_),
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("dim"),
      py::arg("size"),
      py::arg("pad_value"),
      py::arg("output_key") = "",
      R"pbcopy(
        Pad the end of an array such that its size is ``size``.

        Args:
          key (str): The sample key that contains the array we are operating on.
          dim (int): Which dimension of the array to pad.
          size (int): The resulting size of the array at dimension ``dim``.
          pad_value (float): What to pad with.
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
      )pbcopy");
  base.def(
      "pad_to_size_if",
      py::overload_cast<
          bool,
          const std::string&,
          int,
          int64_t,
          double,
          const std::string&>(&T::pad_to_size_if, py::const_),
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("dim"),
      py::arg("size"),
      py::arg("pad_value"),
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.pad_to_size`.");

  base.def(
      "random_slice",
      [](const T& dataset,
         const std::string& ikey,
         std::variant<std::vector<int>, int> dims,
         std::variant<std::vector<int64_t>, int64_t> sizes,
         const std::string& okey) {
        std::vector<int> dims_ = to_vector(dims);
        std::vector<int64_t> sizes_ = to_vector(sizes);
        return dataset.random_slice(ikey, dims_, sizes_, okey);
      },
      py::call_guard<py::gil_scoped_release>(),
      py::arg("ikey"),
      py::arg("dims"),
      py::arg("sizes"),
      py::arg("output_key") = "",
      R"pbcopy(
        Take a random slice of from the array such that the result contains a a
        random subarray of size ``sizes`` for the axes ``dims``.

        If a dimension is smaller than the given size then the whole dimension
        is taken.

        Args:
          key (str): The sample key that contains the array we are operating on.
          dims (int or list of ints): Which dimensions to slice.
          sizes (int or list of ints): The size of the corresponding dimensions.
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
      )pbcopy");
  base.def(
      "random_slice_if",
      [](const T& dataset,
         bool cond,
         const std::string& ikey,
         std::variant<std::vector<int>, int> dims,
         std::variant<std::vector<int64_t>, int64_t> sizes,
         const std::string& okey) {
        std::vector<int> dims_ = to_vector(dims);
        std::vector<int64_t> sizes_ = to_vector(sizes);
        return dataset.random_slice_if(cond, ikey, dims_, sizes_, okey);
      },
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("ikey"),
      py::arg("dims"),
      py::arg("sizes"),
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.random_slice`.");

  base.def(
      "read_from_tar",
      &T::read_from_tar,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("tarkey"),
      py::arg("ikey"),
      py::arg("okey"),
      py::arg("prefix") = "",
      py::arg("tar_prefix") = "",
      py::arg("from_key") = false,
      py::arg("file_fetcher") = nullptr,
      py::arg("nested") = false,
      py::arg("num_threads") = 1,
      R"pbcopy(
        Read data from tarfiles.

        This function reads whole files from one or many tarfiles. It is
        commonly used to read the data in memory before decoding them with
        ``load_image`` or ``load_video``.

        ``tarkey`` can refer to a filename or a sample key that defines the tar
        file name to load from. This function first indexes the whole tar so it
        is most efficient when reading many files from each tar archive.

        When reading nested tar archives (ie tar archives that contain tar
        archives), we can parallelize the indexing process using the
        ``num_threads`` argument.

        Args:
          tarkey (str): The path to the tar file or the sample key containing
            the path to the tarfile based on the value of ``from_key``.
          ikey (str): The sample key containing the file name to read from the
            tar archive.
          okey (str): The sample key to write the data to.
          prefix (str): The filepath prefix to use when **loading the files
            from the tar archive**. (default: '')
          tar_prefix (str): The filepath prefix to use for the tar archive.
            (default: '')
          from_key (bool): If True treat the sample value at ``tarkey`` as a
            filename, otherwise treat ``tarkey`` as a filename. (default: False)
          file_fetcher (mlx.data.core.FileFetcher, optional): A file fetcher to
            read the tar files possibly from a remote location.
          nested (bool): If True then process nested tar files as folder and
            expand them inline. (default: False)
          num_threads (int): When ``nested`` is True use that many parallel
            threads to index the nested archives. (default: 1)
      )pbcopy");
  base.def(
      "read_from_tar_if",
      &T::read_from_tar_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("tarkey"),
      py::arg("ikey"),
      py::arg("okey"),
      py::arg("prefix") = "",
      py::arg("tar_prefix") = "",
      py::arg("from_key") = false,
      py::arg("file_fetcher") = nullptr,
      py::arg("nested") = false,
      py::arg("num_threads") = 1,
      "Conditional :meth:`Buffer.read_from_tar`.");

  base.def(
      "remove_value",
      &T::remove_value,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("size_key"),
      py::arg("dim"),
      py::arg("value"),
      py::arg("pad") = 0,
      R"pbdoc(
        Remove instances of a certain value from an array and shift the whole
        array to the left.

        The size of the array remains unchanged and the end is replaced with
        pad values. Moreover, the length array is updated to match the number
        of values present.

        Args:
          key (str): The sample key that contains the array we are operating on.
          size_key (str): The sample key that contains the array with the valid
            sizes of the array at ``key``.
          dim (int): The dimension the sizes correspond to and the one to be
            filtered.
          value (double): The value to look for and remove.
          pad (double): The pad value to use.
      )pbdoc");
  base.def(
      "remove_value_if",
      &T::remove_value_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("size_key"),
      py::arg("dim"),
      py::arg("value"),
      py::arg("pad") = 0,
      "Conditional :meth:`Buffer.remove_value`.");

  base.def(
      "replace",
      &T::replace,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("old"),
      py::arg("replacement"),
      py::arg("count") = -1,
      R"pbdoc(
        Replace ``old`` with ``replacement`` in the array at  ``key``.

        Example:

        .. code-block:: python

          # Replace ' ' with '▁' to prepare for SPM tokenization.
          dset = dset.replace("text", " ", "\u2581")

        Args:
          key (str): The sample key that contains the array we are operating on.
          old (str): The character sequence that we are replacing.
          replacement (str): The character sequence that we are replacing with.
          count (int): Perform at most ``count`` replacements. Ignore if negative.
              Default: ``-1``.
      )pbdoc");
  base.def(
      "replace_if",
      &T::replace_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("old"),
      py::arg("replacement"),
      py::arg("count") = -1,
      "Conditional :meth:`Buffer.replace`.");

  base.def(
      "replace_bytes",
      &T::replace_bytes,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("ikey"),
      py::arg("byte_map"),
      py::arg("output_key") = "",
      R"pbdoc(
        Replace the bytes at ``key`` using the provided ``byte_map``.

        A byte can map to any string. If an array is not a byte type it will be
        reinterpreted as a byte array and remapped.

        Args:
          ikey (str): The sample key that contains the array we are operating on.
          byte_map (list of str): A list of 256 strings that each byte maps to
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
      )pbdoc");
  base.def(
      "replace_bytes_if",
      &T::replace_bytes_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("ikey"),
      py::arg("byte_map"),
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.replace_bytes`.");

  base.def(
      "rename_key",
      &T::rename_key,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("output_key"),
      R"pbdoc(
        Rename a sample key.

        This is equivalent to

        .. code-block:: python

          def rename_key(s):
            s[output_key] = s[key]
            del s[key]
            return s

          dset = dset.sample_transform(rename_key)

        but more efficient and with better error reporting.

        Args:
          key (str): The key to rename.
          output_key (str): The value to set ``key`` to.
      )pbdoc");
  base.def(
      "rename_key_if",
      &T::rename_key_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("output_key"),
      "Conditional :meth:`Buffer.rename_key`.");

  base.def(
      "save_image",
      &T::save_image,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("filename_key"),
      py::arg("prefix"),
      py::arg("filenamePrefix") = "");
  base.def(
      "save_image_if",
      &T::save_image_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("filename_key"),
      py::arg("prefix"),
      py::arg("filenamePrefix") = "");

  base.def(
      "shape",
      [](const T& dset,
         const std::string& ikey,
         const std::string& okey,
         std::optional<int> dim) {
        if (dim.has_value()) {
          return dset.shape(ikey, *dim, okey);
        } else {
          return dset.shape(ikey, okey);
        }
      },
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("output_key"),
      py::arg("dim") = nullptr,
      R"pbdoc(
        Extracts the shape of an array in the sample.

        If a dimension is provided then only the size of that dimension is extracted.

        Args:
          key (str): The sample key that contains the array we are operating on.
          output_key (str): The key to write the output at. It is required on
            this operation as it is very unlikely that we will want to replace
            the original key.
          dim (int, optional): The dimension to report the size for. If not
            provided then the full size of the array is returned. (default: None)
      )pbdoc");
  base.def(
      "shape_if",
      [](const T& dset,
         bool cond,
         const std::string& ikey,
         const std::string& okey,
         std::optional<int> dim) {
        if (!cond) {
          return dset;
        }

        if (dim.has_value()) {
          return dset.shape(ikey, *dim, okey);
        } else {
          return dset.shape(ikey, okey);
        }
      },
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("output_key"),
      py::arg("dim") = nullptr,
      "Conditional :meth:`Buffer.shape`.");

  base.def(
      "shard",
      &T::shard,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("num_shards"),
      py::arg("output_key") = "",
      R"pbcopy(
        Split the first dimension in ``num_shards``.

        This operation performs the following numpy style reshape:

        .. code-block:: python

          def shard(x):
            shape = x.shape
            return x.reshape(num_shards, -1, *shape[1:])

        Args:
          key (str): The sample key that contains the array we are operating on.
          num_shards (int): The size of the first dimension of the reshaped array.
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
      )pbcopy");
  base.def(
      "shard_if",
      &T::shard_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("num_shards"),
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.shard`.");

  base.def(
      "squeeze",
      [](const T& dset,
         const std::string& key,
         std::optional<std::variant<int, std::vector<int>>> dim,
         const std::string& output_key) {
        if (dim.has_value()) {
          auto dim_value = *dim;
          if (const int* dim_int = std::get_if<int>(&dim_value)) {
            return dset.squeeze(key, *dim_int, output_key);
          } else {
            return dset.squeeze(key, std::get<1>(dim_value), output_key);
          }
        } else {
          return dset.squeeze(key, output_key);
        }
      },
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("dim") = nullptr,
      py::arg("output_key") = "",
      R"pbdoc(
        Squeeze singleton dimensions.

        If no dimension is provided squeeze all singleton dimensions.

        Args:
          key (str): The sample key that contains the array we are operating on.
          dim (int or list of ints, optional): The dimensions to squeeze. If
            not provided squeeze all the singleton dimensions. (default: None)
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
      )pbdoc");
  base.def(
      "squeeze_if",
      [](const T& dset,
         bool cond,
         const std::string& key,
         std::optional<std::variant<int, std::vector<int>>> dim,
         const std::string& output_key) {
        if (!cond) {
          return dset;
        }

        if (dim.has_value()) {
          auto dim_value = *dim;
          if (const int* dim_int = std::get_if<int>(&dim_value)) {
            return dset.squeeze(key, *dim_int, output_key);
          } else {
            return dset.squeeze(key, std::get<1>(dim_value), output_key);
          }
        } else {
          return dset.squeeze(key, output_key);
        }
      },
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("dim") = nullptr,
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.squeeze_if`.");

  base.def(
      "slice",
      [](const T& dataset,
         const std::string& ikey,
         std::variant<std::vector<int>, int> dims,
         std::variant<std::vector<int64_t>, int64_t> starts,
         std::variant<std::vector<int64_t>, int64_t> ends,
         const std::string& okey) {
        std::vector<int> dims_ = to_vector(dims);
        std::vector<int64_t> starts_ = to_vector(starts);
        std::vector<int64_t> ends_ = to_vector(ends);
        return dataset.slice(ikey, dims_, starts_, ends_, okey);
      },
      py::call_guard<py::gil_scoped_release>(),
      py::arg("ikey"),
      py::arg("dims"),
      py::arg("starts"),
      py::arg("ends"),
      py::arg("output_key") = "",
      R"pbcopy(
        Slice the array such that the result contains a subarray starting at
        ``starts`` and ending at ``ends``, so [start, end) will be taken, for the axes ``dims``.

        Args:
          key (str): The sample key that contains the array we are operating on.
          dims (int or list of ints): Which dimensions to slice.
          starts (int or list of ints): The starting offsets for the corresponding dimensions (stars positions are included).
          ends (int or list of ints): The ending offsets for the corresponding dimensions (ends positions are excluded).
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
      )pbcopy");
  base.def(
      "slice_if",
      [](const T& dataset,
         bool cond,
         const std::string& ikey,
         std::variant<std::vector<int>, int> dims,
         std::variant<std::vector<int64_t>, int64_t> starts,
         std::variant<std::vector<int64_t>, int64_t> ends,
         const std::string& okey) {
        std::vector<int> dims_ = to_vector(dims);
        std::vector<int64_t> starts_ = to_vector(starts);
        std::vector<int64_t> ends_ = to_vector(ends);
        return dataset.slice_if(cond, ikey, dims_, starts_, ends_, okey);
      },
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("ikey"),
      py::arg("dims"),
      py::arg("starts"),
      py::arg("ends"),
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.slice`.");

  base.def(
      "tokenize",
      &T::tokenize,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("trie"),
      py::arg("mode") = TokenizeMode::shortest,
      py::arg("ignore_unk") = false,
      py::arg("trie_key_scores") = std::vector<double>({}),
      py::arg("output_key") = "",
      R"pbcopy(
        Tokenize the contents of the array at ``key``.

        This operation uses a :class:`mlx.data.core.CharTrie` to tokenize the
        contents of the array. The tokenizer computes a graph of Trie nodes
        that matches the content of the array at ``key``. Subsequently, it
        either samples a path along the graph (if mode is
        ``mlx.data.core.TokenizeMode.rand``) or finds the shortest weighted
        path using the ``trie_key_scores`` for weights.

        If ``trie_key_scores`` are not provided, then each has the same weight
        of 1 and the result is the smallest number of tokens that can represent
        the content.

        Args:
          key (str): The sample key that contains the array we are operating on.
          trie (mlx.data.core.CharTrie): The trie to use for the tokenization.
          mode (mlx.data.core.TokenizeMode): The tokenizer mode to use.
            Shortest or random as described above. (default: mlx.data.core.TokenizeMode.shortest)
          ignore_unk (bool): If True then ignore content that cannot be
            represented. Otherwise throw an exception. (default: False)
          trie_key_scores (list of float): The weights of each node in the
            trie. (default: [] which means each node gets a weight of 1)
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
      )pbcopy");
  base.def(
      "tokenize_if",
      &T::tokenize_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("trie"),
      py::arg("mode") = TokenizeMode::shortest,
      py::arg("ignore_unk") = false,
      py::arg("trie_key_scores") = std::vector<double>({}),
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.tokenize`.");

  base.def(
      "tokenize_bpe",
      &T::tokenize_bpe,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("key"),
      py::arg("symbols"),
      py::arg("merges"),
      py::arg("output_key") = "",
      R"pbcopy(
        Tokenize the the contents of the array at ``key`` using the BPE merging
        algorithm.

        For instance this can be used to match the tokenization of the
        Sentencepiece tokenizers.

        Args:
          key (str): The sample key that contains the array we are operating on.
          symbols (mlx.data.core.CharTrie): A trie containing the basic symbols
            to use for the tokenization.
          merges (mlx.data.core.BPEMerges): A datastructure containing the
            merges of the basic symbols in order of priority.
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
      )pbcopy");
  base.def(
      "tokenize_bpe_if",
      &T::tokenize_bpe_if,
      py::call_guard<py::gil_scoped_release>(),
      py::arg("cond"),
      py::arg("key"),
      py::arg("symbols"),
      py::arg("merges"),
      py::arg("output_key") = "",
      "Conditional :meth:`Buffer.tokenize_bpe`.");
}
} // namespace
