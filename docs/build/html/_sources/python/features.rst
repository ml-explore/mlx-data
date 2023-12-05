Feature extraction
==================

This submodule provides some feature extraction utilities that can be used as
``key_transform`` functions in MLX data pipelines. Even though a C++
implementation would allow for completely circumventing the GIL and better
utilization of multiple threads, we find that an efficient numpy implementation
can often be fast enough while providing signficiantly more flexibility.

.. currentmodule:: mlx.data.features

Audio Features
--------------

.. autosummary::
   :toctree: _autosummary

   WindowType
   FrequencyScale
   mfsc
