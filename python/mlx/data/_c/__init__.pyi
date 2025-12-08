"""
mlx data
"""

from __future__ import annotations

import os
import typing

import numpy
import typing_extensions

from . import core

__all__: list[str] = [
    "All",
    "Buffer",
    "BufferIterator",
    "LoadAudioInfo",
    "NumChannels",
    "NumFrames",
    "NumSeconds",
    "Rand",
    "SampleRate",
    "Shortest",
    "Stream",
    "TokenizeMode",
    "buffer_from_vector",
    "core",
    "files_from_tar",
    "stream_csv_reader",
    "stream_csv_reader_from_string",
    "stream_line_reader",
    "stream_python_iterable",
]

class Buffer:
    def __getitem__(self: Buffer, idx: int) -> dict: ...
    def __iter__(self: Buffer) -> BufferIterator: ...
    def __len__(self: Buffer) -> int: ...
    def __repr__(self: Buffer) -> str: ...
    def append(self: Buffer, buffer: Buffer) -> Buffer:
        """
        Append the given buffer to the current buffer.

        Resulting buffer contains elements of the calling buffer,
        followed by elements of the buffer passed as an argument.

        Args:
          buffer: buffer appended to the current buffer.
        """

    def batch(
        self: Buffer,
        batch_size: int | list[int],
        pad: dict[str, float] = {},
        dim: dict[str, int] = {},
    ) -> Buffer:
        """
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
        """

    def concretize(self: Buffer) -> Buffer:
        """
        Make a buffer by concretizing all the elements of the buffer.
                                                  The returned buffer does not have buffer parent dependency.
        """

    def dynamic_batch(
        self: Buffer,
        key: str,
        *,
        size_buffer: Buffer | None = None,
        min_data_size: int = -1,
        max_data_size: int = -1,
        pad: dict[str, float] = {},
        dim: dict[str, int] = {},
        drop_outliers: bool = False,
    ) -> Buffer:
        """
        Dynamic batching returns batches with approximately the same
        number of total elements.

        This is used to minimize padding and waste of computation when
        dealing with samples that can have large variance in sizes.

        For instance if we have a buffer with a key 'tokens' and we
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
          dynbatch_padding = sum(count_padding(s) for s in dset.dynamic_batch("tokens", max_data_size=16*1024))

          # Count the total valid tokens
          valid_tokens = sum(d["length"] for d in dset)

          print("Simple batching: ", naive_padding / (valid_tokens + naive_padding), " of tokens were padding")
          print("Dynamic batching: ", dynbatch_padding / (valid_tokens + dynbatch_padding), " of tokens were padding")

          # prints approximately 40% of tokens were padding in the first case
          # and 5% of tokens in the second case

        Args:
          key (str): Which array's size to use for the dynamic batching
          size_buffer (Buffer): Optional buffer containg sizes of the chosen `key`
            arrays contained in self. These sizes should be provided using the
            same `key` field, and must be integers. If provided, these sizes will
            be used instead of computing array's sizes in the `self` Buffer. This
            approach allows to provide alternative size metrics, or avoid materializing
            the whole samples when computing sizes.
          min_data_size (int): How many elements of the array at
            ``key`` should each batch have, at least. If less or equal to 0 then
            the value is ignored. (default: -1)
          max_data_size (int): How many elements of the array at
            ``key`` should each batch have, at most. If less or equal to 0 then
            batch the whole buffer in which case dynamic batching behaves
            similar to ``batch``. (default: -1)
          pad (dict): The values to use for padding for each key in the samples.
          dim (dict): The dimension to concatenate over.
          drop_outliers (bool): If true then drops samples which are larger than the specified
            ``max_data_size``, if ``max_data_size`` > 0. (default: False)
        """

    def filter_by_shape(
        self: Buffer, key: str, dim: int, low: int = -1, high: int = -1
    ) -> Buffer:
        """
        Filter samples based on the shape of the array.

        Args:
          key (str): The sample key that contains the array we are operating on.
          dim (int): The shape dimension based on which we are filtering.
          low (int): Minimum acceptable size for the dimension (inclusive).
          high (int): Maximum acceptable size for the dimension (inclusive). If
            negative size is given then it is assumed we have no upper limit.
        """

    def filter_by_shape_if(
        self: Buffer, cond: bool, key: str, dim: int, low: int = -1, high: int = -1
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.filter_by_shape`.
        """

    def filter_key(self: Buffer, key: str, remove: bool = False) -> Buffer:
        """
        Transform the samples to either only contain this ``key`` or never
        contain this ``key`` based on the value of ``remove``.

        Args:
          key (str): The key to keep or remove.
          remove (bool): If set to True then remove this key instead of keeping
            it (default: False).
        """

    def filter_key_if(
        self: Buffer, cond: bool, key: str, remove: bool = False
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.filter_key`.
        """

    def image_center_crop(
        self: Buffer, key: str, w: int, h: int, output_key: str = ""
    ) -> Buffer:
        """
        Center crop the image at ``key``.

        Args:
          key (str): The sample key that contains the array we are operating on.
          w (int): The target width.
          h (int): The target height.
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
        """

    def image_center_crop_if(
        self: Buffer, cond: bool, key: str, w: int, h: int, output_key: str = ""
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.image_center_crop`.
        """

    def image_channel_reduction(
        self: Buffer, key: str, preset: str = "default", output_key: str = ""
    ) -> Buffer:
        """
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
        """

    def image_channel_reduction_if(
        self: Buffer,
        cond: bool,
        key: str,
        preset: str = "default",
        output_key: str = "",
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.image_channel_reduction`.
        """

    def image_random_area_crop(
        self: Buffer,
        key: str,
        area_range: tuple[float, float],
        aspect_ratio_range: tuple[float, float],
        num_trial: int = 10,
        output_key: str = "",
    ) -> Buffer:
        """
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
        """

    def image_random_area_crop_if(
        self: Buffer,
        cond: bool,
        key: str,
        area_range: tuple[float, float],
        aspect_ratio_range: tuple[float, float],
        num_trial: int = 10,
        output_key: str = "",
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.image_random_area_crop`.
        """

    def image_random_crop(
        self: Buffer, key: str, w: int, h: int, output_key: str = ""
    ) -> Buffer:
        """
        Extract a random crop of the requested size.

        This operation will fail if the image is smaller than the requested
        width and height.

        Args:
          key (str): The sample key that contains the array we are operating on.
          w (int): The width of the result.
          h (int): The height of the result.
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
        """

    def image_random_crop_if(
        self: Buffer, cond: bool, key: str, w: int, h: int, output_key: str = ""
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.image_random_crop`.
        """

    def image_random_h_flip(
        self: Buffer, key: str, prob: float, output_key: str = ""
    ) -> Buffer:
        """
        Horizontally flip the image ``prob`` percent of the time.

        Args:
          key (str): The sample key that contains the array we are operating on.
          prob (float): The probability to flip an image.
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
        """

    def image_random_h_flip_if(
        self: Buffer, cond: bool, key: str, prob: float, output_key: str = ""
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.image_random_h_flip`.
        """

    def image_resize(
        self: Buffer, key: str, w: int, h: int, output_key: str = ""
    ) -> Buffer:
        """
        Resize the image to the requested size.

        Args:
          key (str): The sample key that contains the array we are operating on.
          w (int): The width of the result.
          h (int): The height of the result.
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
        """

    def image_resize_if(
        self: Buffer, cond: bool, key: str, w: int, h: int, output_key: str = ""
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.image_resize`.
        """

    def image_resize_smallest_side(
        self: Buffer, key: str, size: int, output_key: str = ""
    ) -> Buffer:
        """
        Resize the image such that its smallest side is ``size``.

        This operation combined with a center crop or a random area crop is the
        backbone of many image pipelines.

        Args:
          key (str): The sample key that contains the array we are operating on.
          size (int): The size of the smallest side of the result.
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
        """

    def image_resize_smallest_side_if(
        self: Buffer, cond: bool, key: str, size: int, output_key: str = ""
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.image_resize_smallest_side`.
        """

    def image_rotate(
        self: Buffer, key: str, angle: float, crop: bool = False, output_key: str = ""
    ) -> Buffer:
        """
        Rotate an image around its center point.

        Args:
          key (str): The sample key that contains the array we are operating on.
          angle (float): The angle of rotation in degrees.
          crop (bool): Whether to crop the result to the original image's size.
            (default: False)
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
        """

    def image_rotate_if(
        self: Buffer,
        cond: bool,
        key: str,
        angle: float,
        crop: bool = False,
        output_key: str = "",
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.image_rotate`.
        """

    def key_transform(
        self: Buffer,
        key: str,
        func: typing.Callable[[numpy.ndarray], numpy.ndarray],
        output_key: str = "",
    ) -> Buffer:
        """
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
        """

    def key_transform_if(
        self: Buffer,
        cond: bool,
        key: str,
        func: typing.Callable[[numpy.ndarray], numpy.ndarray],
        output_key: str = "",
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.key_transform`.
        """

    def load_audio(
        self: Buffer,
        key: str,
        prefix: str = "",
        info: bool = False,
        from_memory: bool = False,
        info_type: LoadAudioInfo = LoadAudioInfo.LoadAudioInfo.All,
        sample_rate: int = 0,
        resampling_quality: str = "sinc-fastest",
        info_key: str = "",
        output_key: str = "",
    ) -> Buffer:
        """
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
            of the data in ``output_key``, when ``info_key`` is not provided. (default: False)
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
          info_key (str): The key to store the audio metadata in, if desired. (default: '')
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
        """

    def load_audio_if(
        self: Buffer,
        cond: bool,
        key: str,
        prefix: str = "",
        info: bool = False,
        from_memory: bool = False,
        info_type: LoadAudioInfo = LoadAudioInfo.LoadAudioInfo.All,
        sample_rate: int = 0,
        resampling_quality: str = "sinc-fastest",
        info_key: str = "",
        output_key: str = "",
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.load_audio`.
        """

    def load_file(
        self: Buffer, key: str, prefix: os.PathLike = "", output_key: str = ""
    ) -> Buffer:
        """
        Load the contents of a file.

        It opens the file pointed by ``key`` in binary mode and reads its contents.

        Args:
          key (str): The sample key that contains the array we are operating on.
          prefix (str): The filepath prefix to use when loading the files.
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
        """

    def load_file_if(
        self: Buffer,
        cond: bool,
        key: str,
        prefix: os.PathLike = "",
        output_key: str = "",
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.load_file`.
        """

    def load_image(
        self: Buffer,
        key: str,
        prefix: str = "",
        info: bool = False,
        format: str = "RGB",
        from_memory: bool = False,
        output_key: str = "",
    ) -> Buffer:
        """
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
        """

    def load_image_if(
        self: Buffer,
        cond: bool,
        key: str,
        prefix: str = "",
        info: bool = False,
        format: str = "RGB",
        from_memory: bool = False,
        output_key: str = "",
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.load_image`.
        """

    def load_numpy(
        self: Buffer,
        key: str,
        prefix: str = "",
        from_memory: bool = False,
        output_key: str = "",
    ) -> Buffer:
        """
        Load an array from a .npy file.

        Args:
          key (str): The sample key that contains the array we are operating on.
          prefix (str): The filepath prefix to use when loading the files. (default: '')
          from_memory (bool): If true assume the file contents are in the array
            instead of the file name. (default: False)
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
        """

    def load_numpy_if(
        self: Buffer,
        cond: bool,
        key: str,
        prefix: str = "",
        from_memory: bool = False,
        output_key: str = "",
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.load_numpy`.
        """

    def load_video(
        self: Buffer,
        key: str,
        prefix: str = "",
        info: bool = False,
        from_memory: bool = False,
        output_key: str = "",
    ) -> Buffer:
        """
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
        """

    def load_video_if(
        self: Buffer,
        cond: bool,
        key: str,
        prefix: str = "",
        info: bool = False,
        from_memory: bool = False,
        output_key: str = "",
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.load_video`.
        """

    def ordered_prefetch(self: Buffer, prefetch_size: int, num_thread: int) -> Stream:
        """
        Fetch samples in background threads, while preserving original ordering.

        This operation is the workhorse of data loading. It uses
        ``num_threads`` background threads and fetches
        ``prefetch_size`` samples so that they are ready to be used
        when needed.

        Prefetch can be used both to parallelize operations but also to
        overlap computation with data loading in a background thread.

        If you don't need deterministic ordering, look for :meth:`Stream.prefetch`
        instead, as it may be more efficient.

        .. code-block:: python

          # The final prefetch is parallelizing the whole pipeline and
          # ensures that images are going to be available for training.
          dset = (
            dset
            .load_image("image")
            .image_resize_smallest_side("image", 256)
            .image_center_crop("image", 256, 256)
            .batch(32)
            .ordered_prefetch(8, 8)
          )

        Args:
          num_partitions (int): How many different partitions to split the buffer into.
          partition (int): Which partition to use (0-based).
        """

    def pad(
        self: Buffer,
        key: str,
        dim: int,
        lpad: int,
        rpad: int,
        pad_value: float,
        output_key: str = "",
    ) -> Buffer:
        """
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
        """

    def pad_if(
        self: Buffer,
        cond: bool,
        key: str,
        dim: int,
        lpad: int,
        rpad: int,
        pad_value: float,
        output_key: str = "",
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.pad`.
        """

    def pad_to_multiple(
        self: Buffer,
        key: str,
        dim: int,
        pad_multiple: int,
        pad_value: float,
        output_key: str = "",
    ) -> Buffer:
        """
        Pad the end of an array such that its size is a multiple of ``pad_multiple``.

        Args:
          key (str): The sample key that contains the array we are operating on.
          dim (int): Which dimension of the array to pad.
          pad_multiple (int): The result should be a multiple of ``pad_multiple``.
          pad_value (float): What to pad with.
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
        """

    def pad_to_multiple_if(
        self: Buffer,
        cond: bool,
        key: str,
        dim: int,
        pad_multiple: int,
        pad_value: float,
        output_key: str = "",
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.pad_to_multiple`.
        """

    def pad_to_size(
        self: Buffer,
        key: str,
        dim: int,
        size: int,
        pad_value: float,
        output_key: str = "",
    ) -> Buffer:
        """
        Pad the end of an array such that its size is ``size``.

        Args:
          key (str): The sample key that contains the array we are operating on.
          dim (int): Which dimension of the array to pad.
          size (int): The resulting size of the array at dimension ``dim``.
          pad_value (float): What to pad with.
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
        """

    def pad_to_size_if(
        self: Buffer,
        cond: bool,
        key: str,
        dim: int,
        size: int,
        pad_value: float,
        output_key: str = "",
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.pad_to_size`.
        """

    def partition(self: Buffer, num_partitions: int, partition: int) -> Buffer:
        """
        Equivalent to slicing the buffer with a step equal to
        ``num_partitions`` and starting offset of ``partition``.

        This can be used for distributed settings where different nodes
        should load different parts of a dataset.

        Args:
          num_partitions (int): How many different partitions to split the buffer into.
          partition (int): Which partition to use (0-based).
        """

    def partition_if(
        self: Buffer, cond: bool, num_partitions: int, partition: int
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.partition`.
        """

    def perm(self: Buffer, perm: list[int]) -> Buffer:
        """
        Arbitrarily reorder the buffer with the provided indices.

        This operation actually performs arbitrary indexing of the
        buffer which means it can be used to slice or filter the buffer.

        It should be renamed in the future to avoid confusion.

        Args:
          perm (list of ints): The indices defining the permutation/selection.
        """

    def random_slice(
        self: Buffer,
        ikey: str,
        dims: list[int] | int,
        sizes: list[int] | int,
        output_key: str = "",
    ) -> Buffer:
        """
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
        """

    def random_slice_if(
        self: Buffer,
        cond: bool,
        ikey: str,
        dims: list[int] | int,
        sizes: list[int] | int,
        output_key: str = "",
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.random_slice`.
        """

    def read_from_tar(
        self: Buffer,
        tarkey: str,
        ikey: str,
        okey: str,
        prefix: os.PathLike = "",
        tar_prefix: os.PathLike = "",
        from_key: bool = False,
        file_fetcher: ... = None,
        nested: bool = False,
        num_threads: int = 1,
    ) -> Buffer:
        """
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
        """

    def read_from_tar_if(
        self: Buffer,
        cond: bool,
        tarkey: str,
        ikey: str,
        okey: str,
        prefix: os.PathLike = "",
        tar_prefix: os.PathLike = "",
        from_key: bool = False,
        file_fetcher: ... = None,
        nested: bool = False,
        num_threads: int = 1,
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.read_from_tar`.
        """

    def remove_value(
        self: Buffer, key: str, size_key: str, dim: int, value: float, pad: float = 0
    ) -> Buffer:
        """
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
        """

    def remove_value_if(
        self: Buffer,
        cond: bool,
        key: str,
        size_key: str,
        dim: int,
        value: float,
        pad: float = 0,
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.remove_value`.
        """

    def rename_key(self: Buffer, key: str, output_key: str) -> Buffer:
        """
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
        """

    def rename_key_if(self: Buffer, cond: bool, key: str, output_key: str) -> Buffer:
        """
        Conditional :meth:`Buffer.rename_key`.
        """

    def replace(
        self: Buffer, key: str, old: str, replacement: str, count: int = -1
    ) -> Buffer:
        """
        Replace ``old`` with ``replacement`` in the array at  ``key``.

        Example:

        .. code-block:: python

          # Replace ' ' with '▁' to prepare for SPM tokenization.
          dset = dset.replace("text", " ", "\\u2581")

        Args:
          key (str): The sample key that contains the array we are operating on.
          old (str): The character sequence that we are replacing.
          replacement (str): The character sequence that we are replacing with.
          count (int): Perform at most ``count`` replacements. Ignore if negative.
              Default: ``-1``.
        """

    def replace_bytes(
        self: Buffer, ikey: str, byte_map: list[str], output_key: str = ""
    ) -> Buffer:
        """
        Replace the bytes at ``key`` using the provided ``byte_map``.

        A byte can map to any string. If an array is not a byte type it will be
        reinterpreted as a byte array and remapped.

        Args:
          ikey (str): The sample key that contains the array we are operating on.
          byte_map (list of str): A list of 256 strings that each byte maps to
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
        """

    def replace_bytes_if(
        self: Buffer, cond: bool, ikey: str, byte_map: list[str], output_key: str = ""
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.replace_bytes`.
        """

    def replace_if(
        self: Buffer, cond: bool, key: str, old: str, replacement: str, count: int = -1
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.replace`.
        """

    def resample_audio(
        self: Buffer,
        key: str,
        sample_rate: int,
        input_sample_rate: int = 0,
        info_key: str = "",
        resampling_quality: str = "sinc-fastest",
        output_key: str = "",
    ) -> Buffer:
        """
        Resample audio to the requested rate.

        Either the explicit ``input_sample_rate`` or ``info_key`` must be
        provided. If ``info_key`` is provided, it is assumed to contain
        either a scalar array (the input sampling rate) or an array with
        three elements, the last one being the sampling rate. The format
        follows the metadata information returned by
        :meth:`Buffer.load_audio`.

        The following example resamples previously loaded audio to 16kHz.

        .. code-block:: python

          dset = (
            dset
            .load_audio("audio_file", info_type=LoadAudioInfo.NumSeconds, output_key="audio", info_key="audio_info")
            .resample_audio("audio", 16000, info_key="audio_info")
          )

        Args:
          key (str): The sample key that contains the array we are operating on.
          sample_rate (int): The requested sample frequency in frames per
            second. No operation will be performed if it is a negative value.
          input_sample_rate (int): The input sample frequency in frames per
            second. (Default: 0)
          info_key (str): The key where audio metadata is stored, to infer
          input sample rate. (default: '')
          resampling_quality
            (sinc-fastest|sinc-medium|sinc-best|zero-order-hold|linear): Chooses
            the audio resampling quality if resampling is performed. (default:
            sinc-fastest)
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
        """

    def resample_audio_if(
        self: Buffer,
        cond: bool,
        key: str,
        sample_rate: int,
        input_sample_rate: int = 0,
        info_key: str = "",
        resampling_quality: str = "sinc-fastest",
        output_key: str = "",
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.resample_audio`.
        """

    def sample_transform(self: Buffer, func: typing.Callable[[dict], dict]) -> Buffer:
        """
        Apply the python function ``func`` on whole samples.

        The function should return a dictionary of arrays or values that can be
        cast to arrays (buffers, scalars etc). When used with :class:`Stream`
        it can also be used to skip samples by returning an empty dictionary.

        This transformation is very powerful but it should be used with caution
        given that python is slightly plagued by the global interpreter lock.
        See the `Quick Start <../../quick_start.html#about-the-gil>`_ for more.

        Args:
          func (callable): The function to apply.
        """

    def sample_transform_if(
        self: Buffer, cond: bool, func: typing.Callable[[dict], dict]
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.sample_transform`.
        """

    def save_image(
        self: Buffer, key: str, filename_key: str, prefix: str, filenamePrefix: str = ""
    ) -> Buffer: ...
    def save_image_if(
        self: Buffer,
        cond: bool,
        key: str,
        filename_key: str,
        prefix: str,
        filenamePrefix: str = "",
    ) -> Buffer: ...
    def shape(
        self: Buffer, key: str, output_key: str, dim: int | None = None
    ) -> Buffer:
        """
        Extracts the shape of an array in the sample.

        If a dimension is provided then only the size of that dimension is extracted.

        Args:
          key (str): The sample key that contains the array we are operating on.
          output_key (str): The key to write the output at. It is required on
            this operation as it is very unlikely that we will want to replace
            the original key.
          dim (int, optional): The dimension to report the size for. If not
            provided then the full size of the array is returned. (default: None)
        """

    def shape_if(
        self: Buffer, cond: bool, key: str, output_key: str, dim: int | None = None
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.shape`.
        """

    def shard(self: Buffer, key: str, num_shards: int, output_key: str = "") -> Buffer:
        """
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
        """

    def shard_if(
        self: Buffer, cond: bool, key: str, num_shards: int, output_key: str = ""
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.shard`.
        """

    def shuffle(self: Buffer) -> Buffer:
        """
        Create a random permutation of equal size to the buffer and
        apply it thus shuffling the buffer.
        """

    def shuffle_if(self: Buffer, cond: bool) -> Buffer:
        """
        Conditional :meth:`Buffer.shuffle`.
        """

    def size(self: Buffer) -> int: ...
    def slice(
        self: Buffer,
        ikey: str,
        dims: list[int] | int,
        starts: list[int] | int,
        ends: list[int] | int,
        output_key: str = "",
    ) -> Buffer:
        """
        Slice the array such that the result contains a subarray starting at
        ``starts`` and ending at ``ends``, so [start, end) will be taken, for the axes ``dims``.

        Args:
          key (str): The sample key that contains the array we are operating on.
          dims (int or list of ints): Which dimensions to slice.
          starts (int or list of ints): The starting offsets for the corresponding dimensions (stars positions are included).
          ends (int or list of ints): The ending offsets for the corresponding dimensions (ends positions are excluded).
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
        """

    def slice_if(
        self: Buffer,
        cond: bool,
        ikey: str,
        dims: list[int] | int,
        starts: list[int] | int,
        ends: list[int] | int,
        output_key: str = "",
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.slice`.
        """

    def squeeze(
        self: Buffer, key: str, dim: int | list[int] | None = None, output_key: str = ""
    ) -> Buffer:
        """
        Squeeze singleton dimensions.

        If no dimension is provided squeeze all singleton dimensions.

        Args:
          key (str): The sample key that contains the array we are operating on.
          dim (int or list of ints, optional): The dimensions to squeeze. If
            not provided squeeze all the singleton dimensions. (default: None)
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
        """

    def squeeze_if(
        self: Buffer,
        cond: bool,
        key: str,
        dim: int | list[int] | None = None,
        output_key: str = "",
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.squeeze_if`.
        """

    def to_stream(self: Buffer) -> Stream:
        """
        Make a stream that yields the elements of the buffer.
        """

    def tokenize(
        self: Buffer,
        key: str,
        trie: ...,
        mode: TokenizeMode = TokenizeMode.TokenizeMode.Shortest,
        ignore_unk: bool = False,
        trie_key_scores: list[float] = [],
        output_key: str = "",
    ) -> Buffer:
        """
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
        """

    def tokenize_bpe(
        self: Buffer, key: str, symbols: ..., merges: ..., output_key: str = ""
    ) -> Buffer:
        """
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
        """

    def tokenize_bpe_if(
        self: Buffer,
        cond: bool,
        key: str,
        symbols: ...,
        merges: ...,
        output_key: str = "",
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.tokenize_bpe`.
        """

    def tokenize_if(
        self: Buffer,
        cond: bool,
        key: str,
        trie: ...,
        mode: TokenizeMode = TokenizeMode.TokenizeMode.Shortest,
        ignore_unk: bool = False,
        trie_key_scores: list[float] = [],
        output_key: str = "",
    ) -> Buffer:
        """
        Conditional :meth:`Buffer.tokenize`.
        """

class BufferIterator:
    def __iter__(self) -> BufferIterator: ...
    def __next__(self) -> dict: ...

class LoadAudioInfo:
    """
    Members:

      All

      NumFrames

      NumChannels

      SampleRate

      NumSeconds
    """

    All: typing.ClassVar[LoadAudioInfo]  # value = <LoadAudioInfo.All: 0>
    NumChannels: typing.ClassVar[
        LoadAudioInfo
    ]  # value = <LoadAudioInfo.NumChannels: 2>
    NumFrames: typing.ClassVar[LoadAudioInfo]  # value = <LoadAudioInfo.NumFrames: 1>
    NumSeconds: typing.ClassVar[LoadAudioInfo]  # value = <LoadAudioInfo.NumSeconds: 4>
    SampleRate: typing.ClassVar[LoadAudioInfo]  # value = <LoadAudioInfo.SampleRate: 3>
    __members__: typing.ClassVar[
        dict[str, LoadAudioInfo]
    ]  # value = {'All': <LoadAudioInfo.All: 0>, 'NumFrames': <LoadAudioInfo.NumFrames: 1>, 'NumChannels': <LoadAudioInfo.NumChannels: 2>, 'SampleRate': <LoadAudioInfo.SampleRate: 3>, 'NumSeconds': <LoadAudioInfo.NumSeconds: 4>}
    def __eq__(self, other: typing.Any) -> bool: ...
    def __getstate__(self) -> int: ...
    def __hash__(self) -> int: ...
    def __index__(self) -> int: ...
    def __init__(self, value: int) -> None: ...
    def __int__(self) -> int: ...
    def __ne__(self, other: typing.Any) -> bool: ...
    def __repr__(self) -> str: ...
    def __setstate__(self, state: int) -> None: ...
    def __str__(self) -> str: ...
    @property
    def name(self) -> str: ...
    @property
    def value(self) -> int: ...

class Stream:
    def __call__(self) -> dict: ...
    def __iter__(self) -> Stream: ...
    def __next__(self) -> dict: ...
    def __repr__(self) -> str: ...
    def batch(
        self, batch_size: int, pad: dict[str, float] = {}, dim: dict[str, int] = {}
    ) -> Stream: ...
    def buffered(
        self,
        buffer_size: int,
        on_refill: typing.Callable[[...], ...] | None = None,
        num_threads: int = 1,
    ) -> Stream:
        """
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
        """

    def csv_reader_from_key(
        self,
        key: str,
        sep: str = ",",
        quote: str = '"',
        from_memory: bool = False,
        local_prefix: os.PathLike = "",
        file_fetcher: ... = None,
    ) -> Stream:
        """
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
        """

    def dynamic_batch(
        self,
        buffer_size: int,
        key: str,
        *,
        min_data_size: int = -1,
        max_data_size: int = -1,
        pad: dict[str, float] = {},
        dim: dict[str, int] = {},
        shuffle: bool = False,
        drop_outliers: bool = False,
        max_skipped_samples: int = 1000,
        num_threads: int = 1,
    ) -> Stream:
        """
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
          dynbatch_padding = sum(count_padding(s) for s in dset.to_stream().dynamic_batch(500, "tokens", max_data_size=16*1024))

          # Count the total valid tokens
          valid_tokens = sum(d["length"] for d in dset)

          print("Simple batching: ", naive_padding / (valid_tokens + naive_padding), " of tokens were padding")
          print("Dynamic batching: ", dynbatch_padding / (valid_tokens + dynbatch_padding), " of tokens were padding")

          # prints approximately 40% of tokens were padding in the first case
          # and 5% of tokens in the second case

        Args:
          buffer_size (int): How many samples to consider when computing the dynamic batching
          key (str): Which array's size to use for the dynamic batching
          min_data_size (int): How many elements of the array at
            ``key`` should each batch have, at least. If less or equal to 0 then
            the value is ignored. (default: -1)
          max_data_size (int): How many elements of the array at
            ``key`` should each batch have, at most. If less or equal to 0 then
            batch the whole buffer in which case dynamic batching behaves
            similar to ``batch``. (default: -1)
          pad (dict): The values to use for padding for each key in the samples.
          dim (dict): The dimension to concatenate over.
          shuffle (bool): If true shuffle the batches before returning
            them. Otherwise the larger batch sizes with smaller samples
            will be first and so on. (default: False)
          drop_outliers (bool): If true then drops samples which are larger than the specified
            ``max_data_size``, if ``max_data_size`` > 0. (default: False)
          max_skip_samples (int): When ``min_data_size`` is provided, it may not always be possible
            to pack samples in a way which satisfies the size constraints. In that case, samples may
            be skipped, until there is a working combination satisfying the size constraints.
            ``max_skip_samples`` controls the maximum number of skipped samples. (default: 1000)
          num_threads (int): How many parallel threads to use to fill the buffer. (default: 1)
        """

    def filter_by_shape(
        self, key: str, dim: int, low: int = -1, high: int = -1
    ) -> Stream:
        """
        Filter samples based on the shape of the array.

        Args:
          key (str): The sample key that contains the array we are operating on.
          dim (int): The shape dimension based on which we are filtering.
          low (int): Minimum acceptable size for the dimension (inclusive).
          high (int): Maximum acceptable size for the dimension (inclusive). If
            negative size is given then it is assumed we have no upper limit.
        """

    def filter_by_shape_if(
        self, cond: bool, key: str, dim: int, low: int = -1, high: int = -1
    ) -> Stream:
        """
        Conditional :meth:`Buffer.filter_by_shape`.
        """

    def filter_key(self, key: str, remove: bool = False) -> Stream:
        """
        Transform the samples to either only contain this ``key`` or never
        contain this ``key`` based on the value of ``remove``.

        Args:
          key (str): The key to keep or remove.
          remove (bool): If set to True then remove this key instead of keeping
            it (default: False).
        """

    def filter_key_if(self, cond: bool, key: str, remove: bool = False) -> Stream:
        """
        Conditional :meth:`Buffer.filter_key`.
        """

    def image_center_crop(
        self, key: str, w: int, h: int, output_key: str = ""
    ) -> Stream:
        """
        Center crop the image at ``key``.

        Args:
          key (str): The sample key that contains the array we are operating on.
          w (int): The target width.
          h (int): The target height.
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
        """

    def image_center_crop_if(
        self, cond: bool, key: str, w: int, h: int, output_key: str = ""
    ) -> Stream:
        """
        Conditional :meth:`Buffer.image_center_crop`.
        """

    def image_channel_reduction(
        self, key: str, preset: str = "default", output_key: str = ""
    ) -> Stream:
        """
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
        """

    def image_channel_reduction_if(
        self, cond: bool, key: str, preset: str = "default", output_key: str = ""
    ) -> Stream:
        """
        Conditional :meth:`Buffer.image_channel_reduction`.
        """

    def image_random_area_crop(
        self,
        key: str,
        area_range: tuple[float, float],
        aspect_ratio_range: tuple[float, float],
        num_trial: int = 10,
        output_key: str = "",
    ) -> Stream:
        """
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
        """

    def image_random_area_crop_if(
        self,
        cond: bool,
        key: str,
        area_range: tuple[float, float],
        aspect_ratio_range: tuple[float, float],
        num_trial: int = 10,
        output_key: str = "",
    ) -> Stream:
        """
        Conditional :meth:`Buffer.image_random_area_crop`.
        """

    def image_random_crop(
        self, key: str, w: int, h: int, output_key: str = ""
    ) -> Stream:
        """
        Extract a random crop of the requested size.

        This operation will fail if the image is smaller than the requested
        width and height.

        Args:
          key (str): The sample key that contains the array we are operating on.
          w (int): The width of the result.
          h (int): The height of the result.
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
        """

    def image_random_crop_if(
        self, cond: bool, key: str, w: int, h: int, output_key: str = ""
    ) -> Stream:
        """
        Conditional :meth:`Buffer.image_random_crop`.
        """

    def image_random_h_flip(
        self, key: str, prob: float, output_key: str = ""
    ) -> Stream:
        """
        Horizontally flip the image ``prob`` percent of the time.

        Args:
          key (str): The sample key that contains the array we are operating on.
          prob (float): The probability to flip an image.
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
        """

    def image_random_h_flip_if(
        self, cond: bool, key: str, prob: float, output_key: str = ""
    ) -> Stream:
        """
        Conditional :meth:`Buffer.image_random_h_flip`.
        """

    def image_resize(self, key: str, w: int, h: int, output_key: str = "") -> Stream:
        """
        Resize the image to the requested size.

        Args:
          key (str): The sample key that contains the array we are operating on.
          w (int): The width of the result.
          h (int): The height of the result.
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
        """

    def image_resize_if(
        self, cond: bool, key: str, w: int, h: int, output_key: str = ""
    ) -> Stream:
        """
        Conditional :meth:`Buffer.image_resize`.
        """

    def image_resize_smallest_side(
        self, key: str, size: int, output_key: str = ""
    ) -> Stream:
        """
        Resize the image such that its smallest side is ``size``.

        This operation combined with a center crop or a random area crop is the
        backbone of many image pipelines.

        Args:
          key (str): The sample key that contains the array we are operating on.
          size (int): The size of the smallest side of the result.
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
        """

    def image_resize_smallest_side_if(
        self, cond: bool, key: str, size: int, output_key: str = ""
    ) -> Stream:
        """
        Conditional :meth:`Buffer.image_resize_smallest_side`.
        """

    def image_rotate(
        self, key: str, angle: float, crop: bool = False, output_key: str = ""
    ) -> Stream:
        """
        Rotate an image around its center point.

        Args:
          key (str): The sample key that contains the array we are operating on.
          angle (float): The angle of rotation in degrees.
          crop (bool): Whether to crop the result to the original image's size.
            (default: False)
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
        """

    def image_rotate_if(
        self,
        cond: bool,
        key: str,
        angle: float,
        crop: bool = False,
        output_key: str = "",
    ) -> Stream:
        """
        Conditional :meth:`Buffer.image_rotate`.
        """

    def key_transform(
        self,
        key: str,
        func: typing.Callable[[numpy.ndarray], numpy.ndarray],
        output_key: str = "",
    ) -> Stream:
        """
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
        """

    def key_transform_if(
        self,
        cond: bool,
        key: str,
        func: typing.Callable[[numpy.ndarray], numpy.ndarray],
        output_key: str = "",
    ) -> Stream:
        """
        Conditional :meth:`Buffer.key_transform`.
        """

    def line_reader_from_key(
        self,
        key: str,
        dst_key: str,
        from_memory: bool = False,
        unzip: bool = False,
        local_prefix: os.PathLike = "",
        file_fetcher: ... = None,
    ) -> Stream:
        """
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
        """

    def load_audio(
        self,
        key: str,
        prefix: str = "",
        info: bool = False,
        from_memory: bool = False,
        info_type: LoadAudioInfo = LoadAudioInfo.LoadAudioInfo.All,
        sample_rate: int = 0,
        resampling_quality: str = "sinc-fastest",
        info_key: str = "",
        output_key: str = "",
    ) -> Stream:
        """
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
            of the data in ``output_key``, when ``info_key`` is not provided. (default: False)
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
          info_key (str): The key to store the audio metadata in, if desired. (default: '')
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
        """

    def load_audio_if(
        self,
        cond: bool,
        key: str,
        prefix: str = "",
        info: bool = False,
        from_memory: bool = False,
        info_type: LoadAudioInfo = LoadAudioInfo.LoadAudioInfo.All,
        sample_rate: int = 0,
        resampling_quality: str = "sinc-fastest",
        info_key: str = "",
        output_key: str = "",
    ) -> Stream:
        """
        Conditional :meth:`Buffer.load_audio`.
        """

    def load_file(
        self, key: str, prefix: os.PathLike = "", output_key: str = ""
    ) -> Stream:
        """
        Load the contents of a file.

        It opens the file pointed by ``key`` in binary mode and reads its contents.

        Args:
          key (str): The sample key that contains the array we are operating on.
          prefix (str): The filepath prefix to use when loading the files.
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
        """

    def load_file_if(
        self, cond: bool, key: str, prefix: os.PathLike = "", output_key: str = ""
    ) -> Stream:
        """
        Conditional :meth:`Buffer.load_file`.
        """

    def load_image(
        self,
        key: str,
        prefix: str = "",
        info: bool = False,
        format: str = "RGB",
        from_memory: bool = False,
        output_key: str = "",
    ) -> Stream:
        """
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
        """

    def load_image_if(
        self,
        cond: bool,
        key: str,
        prefix: str = "",
        info: bool = False,
        format: str = "RGB",
        from_memory: bool = False,
        output_key: str = "",
    ) -> Stream:
        """
        Conditional :meth:`Buffer.load_image`.
        """

    def load_numpy(
        self,
        key: str,
        prefix: str = "",
        from_memory: bool = False,
        output_key: str = "",
    ) -> Stream:
        """
        Load an array from a .npy file.

        Args:
          key (str): The sample key that contains the array we are operating on.
          prefix (str): The filepath prefix to use when loading the files. (default: '')
          from_memory (bool): If true assume the file contents are in the array
            instead of the file name. (default: False)
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
        """

    def load_numpy_if(
        self,
        cond: bool,
        key: str,
        prefix: str = "",
        from_memory: bool = False,
        output_key: str = "",
    ) -> Stream:
        """
        Conditional :meth:`Buffer.load_numpy`.
        """

    def load_video(
        self,
        key: str,
        prefix: str = "",
        info: bool = False,
        from_memory: bool = False,
        output_key: str = "",
    ) -> Stream:
        """
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
        """

    def load_video_if(
        self,
        cond: bool,
        key: str,
        prefix: str = "",
        info: bool = False,
        from_memory: bool = False,
        output_key: str = "",
    ) -> Stream:
        """
        Conditional :meth:`Buffer.load_video`.
        """

    def next(self) -> dict: ...
    def pad(
        self,
        key: str,
        dim: int,
        lpad: int,
        rpad: int,
        pad_value: float,
        output_key: str = "",
    ) -> Stream:
        """
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
        """

    def pad_if(
        self,
        cond: bool,
        key: str,
        dim: int,
        lpad: int,
        rpad: int,
        pad_value: float,
        output_key: str = "",
    ) -> Stream:
        """
        Conditional :meth:`Buffer.pad`.
        """

    def pad_to_multiple(
        self,
        key: str,
        dim: int,
        pad_multiple: int,
        pad_value: float,
        output_key: str = "",
    ) -> Stream:
        """
        Pad the end of an array such that its size is a multiple of ``pad_multiple``.

        Args:
          key (str): The sample key that contains the array we are operating on.
          dim (int): Which dimension of the array to pad.
          pad_multiple (int): The result should be a multiple of ``pad_multiple``.
          pad_value (float): What to pad with.
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
        """

    def pad_to_multiple_if(
        self,
        cond: bool,
        key: str,
        dim: int,
        pad_multiple: int,
        pad_value: float,
        output_key: str = "",
    ) -> Stream:
        """
        Conditional :meth:`Buffer.pad_to_multiple`.
        """

    def pad_to_size(
        self, key: str, dim: int, size: int, pad_value: float, output_key: str = ""
    ) -> Stream:
        """
        Pad the end of an array such that its size is ``size``.

        Args:
          key (str): The sample key that contains the array we are operating on.
          dim (int): Which dimension of the array to pad.
          size (int): The resulting size of the array at dimension ``dim``.
          pad_value (float): What to pad with.
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
        """

    def pad_to_size_if(
        self,
        cond: bool,
        key: str,
        dim: int,
        size: int,
        pad_value: float,
        output_key: str = "",
    ) -> Stream:
        """
        Conditional :meth:`Buffer.pad_to_size`.
        """

    def partition(self, num_partitions: int, partition: int) -> Stream:
        """
        For every ``num_partitions`` consecutive samples return the ``partition``-th.

        This can be used for distributed settings where different nodes
        should load different parts of a dataset.

        Args:
          num_partitions (int): How many different partitions to split the stream into.
          partition (int): Which partition to use (0-based).
        """

    def partition_if(self, cond: bool, num_partitions: int, partition: int) -> Stream:
        """
        Conditional :meth:`Stream.partition`.
        """

    def prefetch(self, prefetch_size: int, num_threads: int) -> Stream:
        """
        Fetch samples in background threads.

        This operation is the workhorse of data loading. It uses
        ``num_threads`` background threads and fetches
        ``prefetch_size`` samples so that they are ready to be used
        when needed.

        Prefetch can be used both to parallelize operations but also to
        overlap computation with data loading in a background thread.

        This prefetching order is not deterministic and samples' ordering depends
        on scheduling of the threads. If you need deterministic ordering, look for
        :meth:`Buffer.ordered_prefetch` instead.

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
        """

    def prefetch_if(self, cond: bool, prefetch_size: int, num_threads: int) -> Stream:
        """
        Conditional :meth:`Stream.prefetch`.
        """

    def random_slice(
        self,
        ikey: str,
        dims: list[int] | int,
        sizes: list[int] | int,
        output_key: str = "",
    ) -> Stream:
        """
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
        """

    def random_slice_if(
        self,
        cond: bool,
        ikey: str,
        dims: list[int] | int,
        sizes: list[int] | int,
        output_key: str = "",
    ) -> Stream:
        """
        Conditional :meth:`Buffer.random_slice`.
        """

    def read_from_tar(
        self,
        tarkey: str,
        ikey: str,
        okey: str,
        prefix: os.PathLike = "",
        tar_prefix: os.PathLike = "",
        from_key: bool = False,
        file_fetcher: ... = None,
        nested: bool = False,
        num_threads: int = 1,
    ) -> Stream:
        """
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
        """

    def read_from_tar_if(
        self,
        cond: bool,
        tarkey: str,
        ikey: str,
        okey: str,
        prefix: os.PathLike = "",
        tar_prefix: os.PathLike = "",
        from_key: bool = False,
        file_fetcher: ... = None,
        nested: bool = False,
        num_threads: int = 1,
    ) -> Stream:
        """
        Conditional :meth:`Buffer.read_from_tar`.
        """

    def remove_value(
        self, key: str, size_key: str, dim: int, value: float, pad: float = 0
    ) -> Stream:
        """
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
        """

    def remove_value_if(
        self,
        cond: bool,
        key: str,
        size_key: str,
        dim: int,
        value: float,
        pad: float = 0,
    ) -> Stream:
        """
        Conditional :meth:`Buffer.remove_value`.
        """

    def rename_key(self, key: str, output_key: str) -> Stream:
        """
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
        """

    def rename_key_if(self, cond: bool, key: str, output_key: str) -> Stream:
        """
        Conditional :meth:`Buffer.rename_key`.
        """

    def repeat(self, num_time: int) -> Stream:
        """
        Reset the stream ``num_time`` times before declaring it exhausted.

        Args:
          num_time (int): How many times to repeat the dataset.
        """

    def replace(self, key: str, old: str, replacement: str, count: int = -1) -> Stream:
        """
        Replace ``old`` with ``replacement`` in the array at  ``key``.

        Example:

        .. code-block:: python

          # Replace ' ' with '▁' to prepare for SPM tokenization.
          dset = dset.replace("text", " ", "\\u2581")

        Args:
          key (str): The sample key that contains the array we are operating on.
          old (str): The character sequence that we are replacing.
          replacement (str): The character sequence that we are replacing with.
          count (int): Perform at most ``count`` replacements. Ignore if negative.
              Default: ``-1``.
        """

    def replace_bytes(
        self, ikey: str, byte_map: list[str], output_key: str = ""
    ) -> Stream:
        """
        Replace the bytes at ``key`` using the provided ``byte_map``.

        A byte can map to any string. If an array is not a byte type it will be
        reinterpreted as a byte array and remapped.

        Args:
          ikey (str): The sample key that contains the array we are operating on.
          byte_map (list of str): A list of 256 strings that each byte maps to
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
        """

    def replace_bytes_if(
        self, cond: bool, ikey: str, byte_map: list[str], output_key: str = ""
    ) -> Stream:
        """
        Conditional :meth:`Buffer.replace_bytes`.
        """

    def replace_if(
        self, cond: bool, key: str, old: str, replacement: str, count: int = -1
    ) -> Stream:
        """
        Conditional :meth:`Buffer.replace`.
        """

    def resample_audio(
        self,
        key: str,
        sample_rate: int,
        input_sample_rate: int = 0,
        info_key: str = "",
        resampling_quality: str = "sinc-fastest",
        output_key: str = "",
    ) -> Stream:
        """
        Resample audio to the requested rate.

        Either the explicit ``input_sample_rate`` or ``info_key`` must be
        provided. If ``info_key`` is provided, it is assumed to contain
        either a scalar array (the input sampling rate) or an array with
        three elements, the last one being the sampling rate. The format
        follows the metadata information returned by
        :meth:`Buffer.load_audio`.

        The following example resamples previously loaded audio to 16kHz.

        .. code-block:: python

          dset = (
            dset
            .load_audio("audio_file", info_type=LoadAudioInfo.NumSeconds, output_key="audio", info_key="audio_info")
            .resample_audio("audio", 16000, info_key="audio_info")
          )

        Args:
          key (str): The sample key that contains the array we are operating on.
          sample_rate (int): The requested sample frequency in frames per
            second. No operation will be performed if it is a negative value.
          input_sample_rate (int): The input sample frequency in frames per
            second. (Default: 0)
          info_key (str): The key where audio metadata is stored, to infer
          input sample rate. (default: '')
          resampling_quality
            (sinc-fastest|sinc-medium|sinc-best|zero-order-hold|linear): Chooses
            the audio resampling quality if resampling is performed. (default:
            sinc-fastest)
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
        """

    def resample_audio_if(
        self,
        cond: bool,
        key: str,
        sample_rate: int,
        input_sample_rate: int = 0,
        info_key: str = "",
        resampling_quality: str = "sinc-fastest",
        output_key: str = "",
    ) -> Stream:
        """
        Conditional :meth:`Buffer.resample_audio`.
        """

    def reset(self) -> None:
        """
        Reset the stream so that it can be iterated upon again.
        """

    def sample_transform(self, func: typing.Callable[[dict], dict]) -> Stream:
        """
        Apply the python function ``func`` on whole samples.

        The function should return a dictionary of arrays or values that can be
        cast to arrays (buffers, scalars etc). When used with :class:`Stream`
        it can also be used to skip samples by returning an empty dictionary.

        This transformation is very powerful but it should be used with caution
        given that python is slightly plagued by the global interpreter lock.
        See the `Quick Start <../../quick_start.html#about-the-gil>`_ for more.

        Args:
          func (callable): The function to apply.
        """

    def sample_transform_if(
        self, cond: bool, func: typing.Callable[[dict], dict]
    ) -> Stream:
        """
        Conditional :meth:`Buffer.sample_transform`.
        """

    def save_image(
        self, key: str, filename_key: str, prefix: str, filenamePrefix: str = ""
    ) -> Stream: ...
    def save_image_if(
        self,
        cond: bool,
        key: str,
        filename_key: str,
        prefix: str,
        filenamePrefix: str = "",
    ) -> Stream: ...
    def shape(self, key: str, output_key: str, dim: int | None = None) -> Stream:
        """
        Extracts the shape of an array in the sample.

        If a dimension is provided then only the size of that dimension is extracted.

        Args:
          key (str): The sample key that contains the array we are operating on.
          output_key (str): The key to write the output at. It is required on
            this operation as it is very unlikely that we will want to replace
            the original key.
          dim (int, optional): The dimension to report the size for. If not
            provided then the full size of the array is returned. (default: None)
        """

    def shape_if(
        self, cond: bool, key: str, output_key: str, dim: int | None = None
    ) -> Stream:
        """
        Conditional :meth:`Buffer.shape`.
        """

    def shard(self, key: str, num_shards: int, output_key: str = "") -> Stream:
        """
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
        """

    def shard_if(
        self, cond: bool, key: str, num_shards: int, output_key: str = ""
    ) -> Stream:
        """
        Conditional :meth:`Buffer.shard`.
        """

    def shuffle(self, buffer_size: int) -> Stream:
        """
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
        """

    def shuffle_if(self, cond: bool, buffer_size: int) -> Stream:
        """
        Conditional :meth:`Stream.shuffle`.
        """

    def slice(
        self,
        ikey: str,
        dims: list[int] | int,
        starts: list[int] | int,
        ends: list[int] | int,
        output_key: str = "",
    ) -> Stream:
        """
        Slice the array such that the result contains a subarray starting at
        ``starts`` and ending at ``ends``, so [start, end) will be taken, for the axes ``dims``.

        Args:
          key (str): The sample key that contains the array we are operating on.
          dims (int or list of ints): Which dimensions to slice.
          starts (int or list of ints): The starting offsets for the corresponding dimensions (stars positions are included).
          ends (int or list of ints): The ending offsets for the corresponding dimensions (ends positions are excluded).
          output_key (str): The key to store the result in. If it is an empty
            string then overwrite the input. (default: '')
        """

    def slice_if(
        self,
        cond: bool,
        ikey: str,
        dims: list[int] | int,
        starts: list[int] | int,
        ends: list[int] | int,
        output_key: str = "",
    ) -> Stream:
        """
        Conditional :meth:`Buffer.slice`.
        """

    def sliding_window(
        self, key: str, size: int, stride: int, dim: int = -1, index_key: str = ""
    ) -> Stream:
        """
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
          size (int): The size of the sliding window.
          stride (int): The stride of the sliding window.
          dim (int): Which dimension are we sliding the window over. (default: -1)
          index_key (str): If provided, store the index of the sliding window in
            that key. (default: "")
        """

    def squeeze(
        self, key: str, dim: int | list[int] | None = None, output_key: str = ""
    ) -> Stream:
        """
        Squeeze singleton dimensions.

        If no dimension is provided squeeze all singleton dimensions.

        Args:
          key (str): The sample key that contains the array we are operating on.
          dim (int or list of ints, optional): The dimensions to squeeze. If
            not provided squeeze all the singleton dimensions. (default: None)
          output_key (str): If it is not empty then write the result to this
            key instead of overwriting ``key``. (default: '')
        """

    def squeeze_if(
        self,
        cond: bool,
        key: str,
        dim: int | list[int] | None = None,
        output_key: str = "",
    ) -> Stream:
        """
        Conditional :meth:`Buffer.squeeze_if`.
        """

    def to_buffer(self) -> ...:
        """
        Gather the samples from the stream into a buffer.
        """

    def tokenize(
        self,
        key: str,
        trie: ...,
        mode: TokenizeMode = TokenizeMode.TokenizeMode.Shortest,
        ignore_unk: bool = False,
        trie_key_scores: list[float] = [],
        output_key: str = "",
    ) -> Stream:
        """
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
        """

    def tokenize_bpe(
        self, key: str, symbols: ..., merges: ..., output_key: str = ""
    ) -> Stream:
        """
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
        """

    def tokenize_bpe_if(
        self, cond: bool, key: str, symbols: ..., merges: ..., output_key: str = ""
    ) -> Stream:
        """
        Conditional :meth:`Buffer.tokenize_bpe`.
        """

    def tokenize_if(
        self,
        cond: bool,
        key: str,
        trie: ...,
        mode: TokenizeMode = TokenizeMode.TokenizeMode.Shortest,
        ignore_unk: bool = False,
        trie_key_scores: list[float] = [],
        output_key: str = "",
    ) -> Stream:
        """
        Conditional :meth:`Buffer.tokenize`.
        """

class TokenizeMode:
    """
    Members:

      Shortest

      Rand
    """

    Rand: typing.ClassVar[TokenizeMode]  # value = <TokenizeMode.Rand: 1>
    Shortest: typing.ClassVar[TokenizeMode]  # value = <TokenizeMode.Shortest: 0>
    __members__: typing.ClassVar[
        dict[str, TokenizeMode]
    ]  # value = {'Shortest': <TokenizeMode.Shortest: 0>, 'Rand': <TokenizeMode.Rand: 1>}
    def __eq__(self, other: typing.Any) -> bool: ...
    def __getstate__(self) -> int: ...
    def __hash__(self) -> int: ...
    def __index__(self) -> int: ...
    def __init__(self, value: int) -> None: ...
    def __int__(self) -> int: ...
    def __ne__(self, other: typing.Any) -> bool: ...
    def __repr__(self) -> str: ...
    def __setstate__(self, state: int) -> None: ...
    def __str__(self) -> str: ...
    @property
    def name(self) -> str: ...
    @property
    def value(self) -> int: ...

def buffer_from_vector(data: list) -> Buffer:
    """
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
    """

def files_from_tar(tarfile: str, nested: bool = False, num_threads: int = 1) -> Buffer:
    """
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
    """

def stream_csv_reader(
    file: typing.Any,
    sep: str = ",",
    quote: str = '"',
    *,
    local_prefix: str = "",
    file_fetcher: ... = None,
    file_fetcher_handle: ... = None,
) -> Stream:
    """
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
    """

def stream_csv_reader_from_string(
    content: str, sep: str = ",", quote: str = '"'
) -> Stream:
    """
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
    """

def stream_line_reader(
    file: typing.Any,
    key: str,
    unzip: bool = False,
    *,
    local_prefix: str = "",
    file_fetcher: ... = None,
    file_fetcher_handle: ... = None,
) -> Stream:
    """
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
    """

def stream_python_iterable(iterable_factory: typing.Callable) -> Stream:
    """
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
    """

All: LoadAudioInfo  # value = <LoadAudioInfo.All: 0>
NumChannels: LoadAudioInfo  # value = <LoadAudioInfo.NumChannels: 2>
NumFrames: LoadAudioInfo  # value = <LoadAudioInfo.NumFrames: 1>
NumSeconds: LoadAudioInfo  # value = <LoadAudioInfo.NumSeconds: 4>
Rand: TokenizeMode  # value = <TokenizeMode.Rand: 1>
SampleRate: LoadAudioInfo  # value = <LoadAudioInfo.SampleRate: 3>
Shortest: TokenizeMode  # value = <TokenizeMode.Shortest: 0>
__version__: str = "0.2.0.dev20251207+2f431e9"
