Caltech 101 Benchmark
=====================

This benchmark creates a simple image classification data loading pipeline from
a set of directories containing images. Practically, what is implemented in
`torchvision`'s `ImageFolder` dataset.

For PyTorch we use the aforementioned `ImageFolder` dataset while for `tf.data`
we build the pipeline according to [their image classification
tutorial](https://www.tensorflow.org/tutorials/load_data/images).

We apply a minimal set of transforms, namely we resize the original image to
256 pixels across the smallest dimension, center-crop to 224 by 224 pixels and
transform the pixel values to floating point in [0, 1].

Getting the data
----------------

You can download the data manually from
[https://data.caltech.edu/records/mzrjq-6wc02](https://data.caltech.edu/records/mzrjq-6wc02),
or use the provided bash script to download the data, extract them and run the
benchmarks one after the other as follows:

    bash run_caltech.sh


Running the benchmarks
----------------------

If you already have the data, you can run each benchmark by simply pointing it
to directory that holds the Caltech101 dataset. Just make sure that the archive
containing the images is untarred.

    python mlx_data.py /path/to/Caltech101


Dependencies
------------

To automatically download the data you need `wget` and `unzip`. To run the
benchmarks you need `tensorflow` and `PyTorch` with `torchvision` installed.
