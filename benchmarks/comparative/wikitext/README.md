WikiText Benchmark
=====================

This benchmark reads the Wikitext103 dataset using a python generator,
tokenizes the text using an SPM model, computes a sliding window of 1,025
tokens and uses a shuffle buffer of 1,000 samples.

Getting the data
----------------

You can download the data manually from
[blog.salesforceairesearch.com/.../](https://blog.salesforceairesearch.com/the-wikitext-long-term-dependency-language-modeling-dataset/)
or use the provided bash script to download the data, the tokenizer and run the
benchmarks. The tokenizer model we use is the Llama SPM model provided by
Huggingface.

    bash run_wikitext.sh


Running the benchmarks
----------------------

If you already have the data, you can run each benchmark by simply pointing it
to directory that holds the wikitext dataset. Just make sure the archive is
extracted.

    OMP_NUM_THREADS=1 python mlx_data.py \
        --tokenizer_file /path/to/tokenizer.model \
        /path/to/wikitext/wikitext-103-raw

Dependencies
------------

To automatically download the data you need `wget` and `unzip`. To run the
TF benchmark you need the following dependencies:

- `tensorflow`
- `tensorflow_io`
- `tensorflow_text`
- `sentencepiece`
