.. _buffer:

Buffer
======

.. currentmodule:: mlx.data

As also mentioned in `Buffers, Streams and Samples
<buffers_streams_samples.html>`_ a :class:`Buffer` is an indexable container of
samples. Using a buffer in python should feel very similar to accessing a list
of samples.

.. code-block:: python

    import mlx.data as dx

    numbers = dx.buffer_from_vector([{"x": i} for i in range(10)])
    evens = numbers.key_transform("x", lambda x: 2*x)

    print(evens)
    # prints Buffer(size=10, keys={'x'})

    print(evens[3])
    # prints {'x': array(6)}

    print(len(evens))
    # prints 10

Factory methods
---------------

We provide the following factory methods to create a buffer.

.. autosummary::
   :toctree: _autosummary

   buffer_from_vector
   files_from_tar

Buffer specific API
-------------------

The random access characteristics of a ``Buffer`` allow us to define some
transformations that cannot be implemented or do not make sense for a
:class:`Stream`.

.. autosummary::
   :toctree: _autosummary

    Buffer.partition
    Buffer.perm
    Buffer.shuffle
    Buffer.to_stream
