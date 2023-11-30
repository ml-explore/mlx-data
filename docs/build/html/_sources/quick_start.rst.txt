Quick Start Guide
=================

MLX Data is a data loading framework agnostic of the array library you are
using or whether it is a machine learning training job or a data pre-processing
job.

You can use MLX Data with PyTorch, Jax or `MLX <https://ml-explore.github.io/mlx/>`_.

The goal of this library is to allow users to leverage multiple threads for
data processing pipelines without the inflexibility of dealing with multiple
processes or having to write in a symbolic language.

.. note::
   In MLX Data pipelines you can use Python to process data, implement logic or cause side effects!


Load some data
--------------

In MLX data all samples are dictionaries of arrays. The library provides
functions to download and iterate over some common datasets but the goal is to
**provide functions that allow the user to load and process on the fly their
own datasets**.

Let's start with the simplest example on MNIST.

.. code-block:: python

    # This is the standard way to import and access mlx.data
    import mlx.data as dx

    # Let's import MNIST loading
    from mlx.data.datasets import load_mnist

    # Loads a buffer with the MNIST images
    mnist_train = load_mnist(train=True)

    # Let's shuffle flatten and batch to prepare for MLP training
    mnist_mlp = (
        mnist_train
        .shuffle()
        .to_stream()
        .key_transform("image", lambda x: x.astype("float32").reshape(-1))
        .batch(32)
        .prefetch(4, 2)
    )

    # Now we can iterate over the batches in normal python
    for batch in mnist_mlp:
        x, y = batch["image"], batch["label"]


MLX Data provides many `operations <python/dataset.html>`_ that transform
samples so you can create arbitrarily complex pipelines.


About the GIL
-------------

Python functions called by MLX data still run under the Global Interpreter
Lock. To avoid serializing your data pipeline, either drop into Numpy or some
other optimized library as quickly as possible or limit the processing time of
the python part of the data pipeline.

We would advise, however, to avoid premature optimization and only try to
reduce GIL overhead if you are certain that it is limiting your data processing
pipeline.

The following are examples where we would use a python function for flexibility
rather than have a specific C++ transformation.

.. code-block:: python

    # Normalizing images in [0, 1]
    dset = dset.key_transform("image", lambda x: x.astype("float32") / 255)

    # Extracting mel spectrogram features
    # A big chunk of the time is spent computing the FFT which is done with the GIL off so...
    from mlx.data.features import mfsc
    dset = dset.key_transform("audio", mfsc(n_filterbank=80, sampling_freq=16000))

    # Filter stream samples based on values (empty dict means drop the sample)
    dset = dset.sample_transform(lambda s: s if s["length"] > 10 else dict())
