MLX Data
========

MLX Data is a framework agnostic data loading library brought to you by Apple
machine learning research.

MLX Data can be used to load data for machine learning training or on its own
for data pre-processing. You can use it with PyTorch, Jax or `MLX
<https://ml-explore.github.io/mlx/>`_.

The goal of this library is to allow users to leverage multiple threads for
data processing pipelines without the inflexibility of dealing with multiple
processes or having to write in a symbolic language.

.. note::
   In MLX Data pipelines you can use Python to process data, implement logic or cause side effects!


.. toctree::
   :caption: Install
   :maxdepth: 1

   install

.. toctree::
   :caption: Usage
   :maxdepth: 1

   quick_start
   buffers_streams_samples

.. toctree::
   :caption: Python API Reference
   :maxdepth: 1

   python/dataset
   python/buffer
   python/stream
   python/common_datasets
   python/tokenizing
   python/features
   python/miscellaneous
