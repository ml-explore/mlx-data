# Copyright © 2023 Apple Inc.

import argparse
from pathlib import Path

from mlx.data.features import mfsc
from mlx.data.tokenizer_helpers import read_trie_from_spm
from utils import Benchmark

import mlx.data as dx


def to_audio_and_transcript(sample):
    # Split the line
    file_part, transcript = bytes(sample["line"]).split(b" ", 1)

    # Extract the audio path
    parts = file_part.split(b"-")
    parts[-1] = file_part + b".flac"
    audio_path = b"/".join(parts)

    # Prepare the transcript
    transcript = transcript.lower()

    return {"audio": audio_path, "transcript": transcript}


def iterate(args, workers):
    root = Path(args.data_dir)

    # Load the list of lists of files
    filelist = [{"file": str(f).encode("ascii")} for f in root.glob("**/*.txt")]

    # Load the tokenizer
    trie, _ = read_trie_from_spm(args.tokenizer_file)

    dset = (
        dx.buffer_from_vector(filelist)
        .shuffle()
        .to_stream()
        .line_reader_from_key("file", "line")
        # Transform the lines into an audio path and the transcript
        .sample_transform(to_audio_and_transcript)
        # Load the audio and extract features
        .load_audio("audio", prefix=args.data_dir)
        .squeeze("audio")
        .key_transform("audio", mfsc(128, 16000))
        .shape("audio", "audio_length", 0)
        # Tokenize the transcript
        .tokenize("transcript", trie)
        .pad("transcript", 0, 1, 0, trie.search("<s>").id)
        .pad("transcript", 0, 0, 1, trie.search("</s>").id)
        .shape("transcript", "transcript_length", 0)
        # Batch and prefetch
        .batch(args.batch_size)
        .prefetch(workers, workers)
    )

    cnt = 0
    for sample in dset:
        cnt += 1
    return cnt


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("data_dir")
    parser.add_argument("--batch_size", type=int, default=32)
    parser.add_argument("--tokenizer_file", default="tokenizer.model")
    args = parser.parse_args()

    benchmark = Benchmark("MLX Librispeech")
    for i in range(3):
        benchmark.log_run("iterate_no_workers", iterate, args, 1)

    for i in range(3):
        benchmark.log_run("iterate_workers_8", iterate, args, 8)

    for i in range(3):
        benchmark.log_run("iterate_workers_16", iterate, args, 16)
    benchmark.report()
