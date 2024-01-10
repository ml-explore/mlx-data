# Copyright © 2023 Apple Inc.

import argparse
from functools import partial
from pathlib import Path

import tensorflow as tf
import tensorflow_io as tf_io
import tensorflow_text as tf_text
from utils import Benchmark


def files_and_classes(root: Path):
    files = [str(f) for f in root.glob("**/*.jpg")]
    files = [f for f in files if "BACKGROUND" not in f]
    classes = dict(
        map(reversed, enumerate(sorted(set(f.split("/")[-2] for f in files))))
    )
    class_per_file = [classes[f.split("/")[-2]] for f in files]

    return files, class_per_file


def to(dtype, x):
    if isinstance(x, list):
        return [to(dtype, xi) for xi in x]
    return tf.cast(x, dtype=dtype)


to_float32 = partial(to, tf.float32)
to_int32 = partial(to, tf.int32)


def process_sample(tokenizer, prefix):
    def process_sample_impl(sample):
        parts = tf.strings.split(sample, sep=" ", maxsplit=1)
        text = parts[1]
        tokens = tokenizer.tokenize(tf.strings.lower(text))

        name = parts[0]
        name_parts = tf.strings.split(name, sep="-")
        name = tf.strings.join([name, ".flac"])
        filepath = tf.strings.join(
            [prefix, name_parts[0], name_parts[1], name], separator="/"
        )
        audio = tf_io.audio.AudioIOTensor(filepath, dtype=tf.int16).to_tensor()
        audio = tf.squeeze(audio)
        audio = to_float32(audio) / 32768.0
        audio = tf_io.audio.spectrogram(audio, nfft=512, window=400, stride=160)
        audio = tf_io.audio.melscale(audio, rate=16000, mels=128, fmin=0, fmax=8000)
        audio = tf.math.log(tf.maximum(audio, 1e-9))

        return {
            "transcript": tokens,
            "transcript_length": len(tokens),
            "audio": audio,
            "audio_length": len(audio),
        }

    return process_sample_impl


def iterate(args, workers):
    root = Path(args.data_dir)

    # Load the list of lists of files
    filelist = [str(f).encode("ascii") for f in root.glob("**/*.txt")]

    with open(args.tokenizer_file, "rb") as f:
        tokenizer = tf_text.SentencepieceTokenizer(f.read())
        tokenizer.add_bos = True
        tokenizer.add_eos = True
    ds = (
        tf.data.Dataset.from_tensor_slices(filelist)
        .interleave(tf.data.TextLineDataset)
        .shuffle(buffer_size=300)
        .map(process_sample(tokenizer, args.data_dir))
        .padded_batch(
            args.batch_size,
            padded_shapes={
                "transcript": [None],
                "transcript_length": [],
                "audio": [None, 128],
                "audio_length": [],
            },
            padding_values={
                "transcript": 0,
                "transcript_length": 0,
                "audio": 0.0,
                "audio_length": 0,
            },
        )
        .prefetch(workers)
    )

    options = tf.data.Options()
    options.threading.private_threadpool_size = workers

    cnt = 0
    for batch in ds.with_options(options):
        cnt += 1
    return cnt


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("data_dir")
    parser.add_argument("--batch_size", type=int, default=32)
    parser.add_argument("--tokenizer_file", default="tokenizer.model")
    args = parser.parse_args()

    benchmark = Benchmark("TFDS LibriSpeech")
    for i in range(3):
        benchmark.log_run("iterate_no_workers", iterate, args, 1)

    for i in range(3):
        benchmark.log_run("iterate_workers_8", iterate, args, 8)

    for i in range(3):
        benchmark.log_run("iterate_workers_16", iterate, args, 16)
    benchmark.report()
