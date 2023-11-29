LibriSpeech Benchmark
=====================

This benchmark creates a simple ASR pipeline from the structure of the
LibriSpeech dataset. Simply put there is a set of text files containing lists
of audio files and their transcripts. We transform the audio to Mel scaled
spectrograms and the transcripts to tokens from a SentencePiece tokenizer.

For PyTorch we use the torchaudio LIBRISPEECH dataset and the `MelSpectrogram`
transform with soundfile to load the audio. For tensorflow we use
`tensorflow_text` for the tokenizer and `tensorflow_io` to read the audio.

A more realistic ASR pipeline would load the audio info first in order to
subsequently group the files by length. This can be achieved efficiently in `mlx.data`
using buffered streams.

Getting the data
----------------

You can download the data manually from
[https://www.openslr.org/12](https://www.openslr.org/12) or use the provided
bash script to download the data, the tokenizer and run the benchmarks. The
tokenizer model we use is the Llama SPM model provided by Huggingface.

    bash run_librispeech.sh


Running the benchmarks
----------------------

If you already have the data, you can run each benchmark by simply pointing it
to directory that holds the LibriSpeech dataset. Just make sure the archive is
extracted.

    OMP_NUM_THREADS=1 python mlx_data.py \
        --tokenizer_file /path/to/tokenizer.model \
        /path/to/librispeech/LibriSpeech/dev-clean

You should run the PyTorch and `mlx.data` benchmarks with `OMP_NUM_THREADS=1`
to avoid thread contention from computing the FFT and other operations on CPU.

Dependencies
------------

To automatically download the data you need `wget` and `unzip`. To run the
benchmarks you need the following dependencies:

- `tensorflow`
- `tensorflow_io`
- `tensorflow_text`
- `sentencepiece`
- `torch`
- `torchaudio`
- `intel-numpy` for fast FFT

Since the featurization in `mlx.data` happens in `numpy` we propose you install
`intel-numpy` for a fast FFT implementation.
