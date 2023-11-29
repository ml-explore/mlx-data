#!/bin/bash

# Get the data
if [ ! -d data ]; then
    mkdir data
    pushd data
    wget "https://data.caltech.edu/records/mzrjq-6wc02/files/caltech-101.zip?download=1" -O caltech-101.zip
    unzip caltech-101.zip && rm caltech-101.zip
    pushd caltech-101
    tar -zxf 101_ObjectCategories.tar.gz && rm 101_ObjectCategories.tar.gz
    popd
    popd
fi

# Run the benchmarks
python pytorch.py data/caltech-101 2>/dev/null
echo "============="
python tfds.py data/caltech-101 2>/dev/null
echo "============="
python mlx_data.py data/caltech-101
