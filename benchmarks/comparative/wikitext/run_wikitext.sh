#!/bin/bash

# Get the data
if [ ! -d data ]; then
    mkdir data
    pushd data
    wget "https://s3.amazonaws.com/research.metamind.io/wikitext/wikitext-103-raw-v1.zip" -O wikitext-103-raw-v1.zip
    unzip wikitext-103-raw-v1.zip && rm wikitext-103-raw-v1.zip
    wget "https://huggingface.co/hf-internal-testing/llama-tokenizer/resolve/main/tokenizer.model" -O tokenizer.model
    popd
fi

# Run the benchmarks
python pytorch.py --tokenizer_file data/tokenizer.model data/wikitext-103-raw
echo "============="
python tfds.py --tokenizer_file data/tokenizer.model data/wikitext-103-raw 2>/dev/null
echo "============="
python mlx_data.py --tokenizer_file data/tokenizer.model data/wikitext-103-raw
