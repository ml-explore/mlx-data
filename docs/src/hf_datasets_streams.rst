HuggingFace Datasets and MLX Streams
====================================

.. currentmodule:: mlx.data

You can load any data using MLX :class:`Buffer`, as detailed in `Buffers and Streams <buffers_streams_samples>`_.

In this tutorial, we will explore how to leverage the popular `Hugging Face Datasets <https://huggingface.co/docs/datasets/index>`_ library
to seamlessly load and process datasets into MLX streams.
Follow along as we guide you through each step, making it easy to utilize datasets for your MLX model training.

Install hugging face datasets:

.. code-block:: bash

    pip install datasets

Download the dataset:

.. code-block:: python

    from datasets import load_dataset

    ds = load_dataset("ylecun/mnist")
    print(ds['train'])

.. code-block:: bash

    Dataset({
        features: ['image', 'label'],
        num_rows: 60000
    })

Start by converting the dataset to numpy arrays
-----------------------------------------------

Sometimes, the dataset is not in the format we can directly work with.
For example, the images in the MNIST dataset are stored as :class:`PIL` images.

.. code-block:: python

    print(type(ds['train']['image']))

.. code-block:: bash

    [
        <PIL.PngImagePlugin.PngImageFile image mode=L size=28x28>,
        <PIL.PngImagePlugin.PngImageFile image mode=L size=28x28>,
        ...
    ]

We can convert them to numpy arrays using the following code:

.. code-block:: python

    import numpy as np

    def huggingface_to_array_of_dict(dataset):    
        return [{"image": np.array(image).copy(), "label": label}
                for label, image in zip(dataset['label'], dataset['image'])]

Buffer
------

Before converting to a :class:`Buffer`, ensure you have a list of dictionaries.
This step will save you a lot of time:

.. code-block:: python

    dicts = huggingface_to_array_of_dict(dataset)

    assert type(dicts) == list
    assert type(dicts[0]) == dict
    assert type(dicts[0]['image']) == np.ndarray
    # assert type(dicts[0]['image'].shape) == ... usually for images you will get a HCW shape
    # but it depends on your use case

We can then convert the list of dictionaries to a :class:`Buffer` using :meth:`buffer_from_vector`:

.. code-block:: python

    import mlx.data as dx

    buffer = dx.buffer_from_vector(dicts)

Streams
-------

Finally you can load the buffer as a :class:`Stream`:

.. code-block:: python

    stream = buffer
        .to_stream()
        .key_transform("image", lambda x: x.astype("float32") / 255)        
        .batch(32)
        .prefetch(prefetch_size=8, num_threads=4)

Take note of the `key_transform`,
the `image` key refers to the `image` key as defined in the original dictionary.

.. code-block:: python

    .key_transform("image", ...)       

You can specify a function for any transformations you want to apply to the inputs,
for example common operations on images are:

- Normalizing the data: ``x / 255``
- Modifying the shape of the data into something your model can understand: ``x.reshape(-1)``


Putting it all together
-----------------------

This loads the dataset, converts it to numpy arrays, and creates a stream of batches:

.. code-block:: python

    import numpy as np
    import mlx.data as dx

    # Convert the content of the dataset into numpy arrays
    def huggingface_to_array_of_dict(dataset):    
        return [{"image": np.array(image).copy(), "label": label}
                for label, image in zip(dataset['label'], dataset['image'])]

    # Convert the Hugging Face dataset to a stream of batches
    def hf_dataset_to_mlx_stream(dataset, shuffle=False):
        numpy_data = huggingface_to_array_of_dict(dataset)

        buffer = dx.buffer_from_vector(numpy_data)
        if shuffle:
            buffer = buffer.shuffle()    

        return (
            buffer
            .to_stream()
            .key_transform("image", lambda x: x.astype("float32") / 255)        
            .batch(32)
            .prefetch(prefetch_size=8, num_threads=4)
        )

And here's how you would use the streams to train one epoch:

.. code-block:: python
    
    import matplotlib.pyplot as plt
    import mlx.core as mx

    train_stream = hf_dataset_to_mlx_stream(ds['train'], shuffle=True)
    test_stream = hf_dataset_to_mlx_stream(ds['test'], shuffle=False)

    train_stream.reset()
    for batch in train_stream:    
        (X, y) = mx.array(batch['image']), mx.array(batch['label'])
            
        print('The image should display a ', y[0].item())
        plt.imshow(X[0])
        break

Don't forget to :meth:`reset` the stream after iterating over it.