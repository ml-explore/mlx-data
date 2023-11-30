# Copyright © 2023 Apple Inc.

import argparse
from functools import partial
from multiprocessing import Pool
from pathlib import Path

import torch
from sentencepiece import SentencePieceProcessor

from utils import Benchmark


def document_iterator(text_file):
    """Extract documents from wikitext raw file."""
    with open(text_file, "rb") as f:
        bufsize = 1024 * 1024 * 16  # 16MiB
        content = b""
        while True:
            new_content = f.read(bufsize)
            if new_content == b"":
                yield content
                return

            content = content + new_content

            i = 0
            k = 0
            while True:
                j = content.find(b"\n \n = ", k)
                if j == -1:
                    content = content[i:]
                    break

                if content[j + 6] != ord("="):
                    yield content[i : j + 1]
                    i = j + 1

                k = j + 1


def tokenize(tokenizer, x):
    return [tokenizer.bos_id()] + tokenizer.encode(x) + [tokenizer.eos_id()]


def flatmap(func, iterable):
    for el in iterable:
        yield from func(el)


def sliding_window(window_size, stride, x):
    for i in range(0, len(x), stride):
        yield x[i : i + window_size]


def pad(x):
    N = max(map(len, x))
    return [xi + [0] * (N - len(xi)) for xi in x]


def batch(batch_size, iterable):
    n = []
    for el in iterable:
        if len(n) >= batch_size:
            yield torch.tensor(pad(n))
            n = []
        else:
            n.append(el)
    yield torch.tensor(pad(n))


def iterate(args, workers):
    tokenizer = SentencePieceProcessor(args.tokenizer_file)
    data_iter = document_iterator(Path(args.data_dir) / "wiki.train.raw")
    with Pool(workers) as p:
        tokenized_data = p.imap_unordered(partial(tokenize, tokenizer), data_iter)
        windowed_data = flatmap(partial(sliding_window, 1025, 1025), tokenized_data)
        batched_data = batch(args.batch_size, windowed_data)

        cnt = 0
        for sample in batched_data:
            cnt += 1
        return cnt


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("data_dir")
    parser.add_argument("--batch_size", type=int, default=32)
    parser.add_argument("--tokenizer_file", default="tokenizer.model")
    args = parser.parse_args()

    benchmark = Benchmark("PyTorch WikiText")
    for i in range(3):
        benchmark.log_run("iterate_workers_4", iterate, args, 4)

    for i in range(3):
        benchmark.log_run("iterate_workers_8", iterate, args, 8)

    for i in range(3):
        benchmark.log_run("iterate_workers_16", iterate, args, 16)
    benchmark.report()
