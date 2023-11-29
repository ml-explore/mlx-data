#!/bin/bash

# Get the data
if [ ! -d data ]; then
    mkdir data
    pushd data
    wget "https://www.openslr.org/resources/12/dev-clean.tar.gz" -O dev-clean.tar.gz
    tar -zxf dev-clean.tar.gz && rm dev-clean.tar.gz
    wget "https://huggingface.co/hf-internal-testing/llama-tokenizer/resolve/main/tokenizer.model" -O tokenizer.model
    popd
fi

# Run the benchmarks
OMP_NUM_THREADS=1 python pytorch.py --tokenizer_file data/tokenizer.model data/LibriSpeech/dev-clean 2>/dev/null
echo "============="
python tfds.py --tokenizer_file data/tokenizer.model data/LibriSpeech/dev-clean 2>/dev/null
echo "============="
python mlx_data.py --tokenizer_file data/tokenizer.model data/LibriSpeech/dev-clean
