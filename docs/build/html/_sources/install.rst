Build and Install
=================

Install from PyPI
-----------------

MLX Data can be installed from PyPI. For most Linuxes and Apple silicon Macs
one can simply,

.. code-block::

    pip install mlx-data

This should install the package together with the necessary dependencies to
read audio, images, video and remote content from S3.

Building from source
--------------------

MLX Data consists of a C++ library with shallow python bindings and some Python
helpers. You can choose to build and use only the C++ library or simply pip
install the Python library which transparently builds the required C++ backend.

We plan to provide prebuilt binaries for easy install in the future.

Dependencies
^^^^^^^^^^^^

Handling data requires reading various filetypes over various protocols. This
means that MLX Data has a couple of dependencies e.g. for loading images, audio
and videos or fetching files from an S3 bucket.

The following dependencies are optional but without them some functionality of
MLX data will not be built. Below we enumerate the list of dependencies and
subsequently we provide example shell commands to install them using
``homebrew`` or on Ubuntu using ``apt``.

- ``libsndfile`` to load audio files.
- ``libsamplerate`` for audio samplerate conversion.
- ``ffmpeg`` for loading video files.
- ``libjpegturbo`` to load JPG images.
- ``zlib``, ``bzip2`` and ``liblzma`` to process compressed streams.
- ``aws-sdk`` for accessing S3 buckets.

To install the above on a Mac using homebrew one needs simply to run:

.. code-block:: bash

    brew install libsndfile libsamplerate ffmpeg jpeg-turbo zlib bzip2 xz aws-sdk-cpp

For an Ubuntu machine, on the other hand:

.. code-block:: bash

    sudo apt install libsndfile1-dev libsamplerate0-dev ffmpeg libjpeg-turbo8-dev \
        zlib1g-dev libbz2-dev liblzma-dev

    # We have to build the AWS SDK from source :-(
    # This is *not needed* if installing MLX Data from a prebuilt binary or if
    # you don't care about fetching from S3

    sudo apt install libcurl4-openssl-dev libssl-dev
    git clone --depth 1 --recurse-submodules https://github.com/aws/aws-sdk-cpp.git
    cd aws-sdk-cpp
    mkdir build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_ONLY="s3" -DBUILD_SHARED_LIBS=OFF
    make -j
    sudo make install

Finally before building the python bindings you need to make sure that you
have ``pybind11`` installed and a relatively modern version of ``CMake``. For example,
both be installed with ``pip``,

.. code-block:: bash

    pip install pybind11[global] cmake

Build Python bindings from source
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

After installing the dependencies you care about as described above, you can
directly install MLX data using pip.

.. code-block:: bash

    cd /path/to/mlx/data
    pip install .  # or pip install -e . for an editable install

To enable S3 support you also have to use the corresponding CMAKE option when
building MLX data as follows:

.. code-block:: bash

    CMAKE_ARGS="-DMLX_ENABLE_AWS=ON" pip install .

Building the standalone C++ library
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

MLX data can also be installed as a standalone C++ static library that you can
link your projects against.

.. code-block:: bash

    mkdir build && cd build
    cmake ..
    make -j
    sudo make install

Subsequently, you can use CMake's ``find_package(MLXData)`` which defines
``MLX_DATA_FOUND``, ``MLX_DATA_INCLUDE_DIRS`` and ``MLX_DATA_LIBRARIES`` or
simply link with ``-lmlxdata`` as it is normally installed in
``/usr/local/lib``.
