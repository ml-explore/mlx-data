Common operations
=================

.. currentmodule:: mlx.data

Both :class:`Buffer` and :class:`Stream` allow to apply transformations to
samples when they are accessed. These transformations share the same API which
is described below in terms of the ``Buffer`` class (but ``Stream`` is
identical). For the methods specific to :class:`Buffer` or :class:`Stream` see
the corresponding pages.

General sample operations
-------------------------

.. autosummary::
   :toctree: _autosummary

    Buffer.batch
    Buffer.filter_by_shape
    Buffer.filter_key
    Buffer.key_transform
    Buffer.sample_transform
    Buffer.remove_value
    Buffer.rename_key

Image operations
----------------

.. autosummary::
   :toctree: _autosummary

    Buffer.image_center_crop
    Buffer.image_channel_reduction
    Buffer.image_random_area_crop
    Buffer.image_random_crop
    Buffer.image_random_h_flip
    Buffer.image_resize
    Buffer.image_resize_smallest_side
    Buffer.image_rotate


I/O operations
---------------

.. autosummary::
   :toctree: _autosummary

    Buffer.load_audio
    Buffer.load_file
    Buffer.load_numpy
    Buffer.load_image
    Buffer.load_video
    Buffer.read_from_tar

Padding operations
------------------

.. autosummary::
   :toctree: _autosummary

    Buffer.pad
    Buffer.pad_to_multiple
    Buffer.pad_to_size

Shape operations
----------------

.. autosummary::
   :toctree: _autosummary

    Buffer.shape
    Buffer.shard
    Buffer.squeeze

Tokenization
--------------

.. autosummary::
   :toctree: _autosummary

    Buffer.tokenize

Conditional operations
----------------------

A common issue when writing pipelines is configuring them according to the
command line or configuration arguments. This usually results in code with a
lot of redirections that is hard to read and reason about what operations are
actually applied to the data.

For this reason all of the above methods have a conditional variant defined as
follows ``*_if(cond: bool, *args, **kwargs)``. This allows writing pipelines
that read from top to bottom without having to resort to redirection statements
in python.

.. code-block:: python

    # Assuming we have a buffer with image files and labels in dset
    dset = (
        dset
        .load_image("image_file", output_key="image")
        .image_random_crop_if(enable_random_crop, "image", 256, 256)
        .image_random_h_flip_if(flip_prob > 0, "image", flip_prob)
        .key_transform_if(brightness_range > 0, "image",
                          lambda x: ((1 + brightness_range * np.random.rand(x.shape[:2])[..., None]) * x).astype(x.dtype))
    )
