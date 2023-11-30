.. _stream:

Stream
======

.. currentmodule:: mlx.data

Using a :class:`Stream` in python should feel like accessing an iterator of
samples. Only the next sample can be fetched and the iteration may be restarted
depending on the underlying source of the data (some trully online sources are
not resettable).

.. code-block:: python

    import mlx.data as dx

    # The samples are never all instantiated
    numbers = dx.stream_python_iterable(lambda: ({"x": i} for i in range(10**10)))

    # Filtering is done with transforms returning an empty sample
    evens = numbers.sample_transform(lambda s: s if s["x"] % 2 == 0 else dict())

    print(next(numbers))
    # prints {'x': array(0)}
    print(next(numbers))
    # prints {'x': array(1)}

    # Streams are pointers to the streams so evens is using numbers under the
    # hood. Since numbers was advanced now evens is advanced as well.
    print(next(evens))
    # prints {'x': array(2)}
    print(next(evens))
    # prints {'x': array(4)}
    print(next(numbers))
    # prints {'x': array(5)}

    # Streams can be reset.
    evens.reset()
    print(next(evens))
    print(next(evens))
    print(next(numbers))
    # prints {'x': array(0)}
    #        {'x': array(2)}
    #        {'x': array(3)}


Factory methods
---------------

We provide the following factory methods to create a stream. When used from
python the most interesting one is probably :func:`stream_python_iterable`. Of
course another good strategy is to start from a :class:`Buffer` that you then
cast to a stream using :meth:`Buffer.to_stream`.

.. autosummary::
   :toctree: _autosummary

   stream_csv_reader
   stream_csv_reader_from_string
   stream_line_reader
   stream_python_iterable

Stream specific API
-------------------

:class:`Stream` has a more powerful API than :class:`Buffer`. It does not allow
for random access, however, it allows for stream composing and prefetching.
Stream composing is when a sample becomes the beginning of a new stream that
can have arbitrary length.

Streams also allow for filtering using the provided functions or a
:meth:`Stream.sample_transform` that returns an empty dictionary (an empty
Sample).

.. autosummary::
   :toctree: _autosummary

   Stream.csv_reader_from_key
   Stream.line_reader_from_key
   Stream.dynamic_batch
   Stream.partition
   Stream.buffered
   Stream.repeat
   Stream.shuffle
   Stream.sliding_window
   Stream.prefetch
