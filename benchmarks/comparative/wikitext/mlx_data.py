# Copyright © 2023 Apple Inc.

import argparse
from functools import partial
from pathlib import Path

from mlx.data.tokenizer_helpers import read_trie_from_spm
from utils import Benchmark

import mlx.data as dx


def document_iterator(text_file):
    """Extract documents from wikitext raw file."""
    with open(text_file, "rb") as f:
        bufsize = 1024 * 1024 * 16  # 16MiB
        content = b""
        while True:
            new_content = f.read(bufsize)
            if new_content == b"":
                yield {"document": content}
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
                    yield {"document": content[i : j + 1]}
                    i = j + 1

                k = j + 1


def iterate(args, workers):
    root = Path(args.data_dir)

    # Make the document iterator factory
    iterator_factory = partial(document_iterator, str(root / "wiki.train.raw"))

    # Load the tokenizer
    trie, _ = read_trie_from_spm(args.tokenizer_file)

    dset = (
        dx.stream_python_iterable(iterator_factory)
        .tokenize("document", trie, output_key="tokens")
        .filter_key("document", remove=True)
        .prefetch_if(workers > 0, 32, workers)
        .pad("tokens", 0, 1, 0, trie.search("<s>").id)
        .pad("tokens", 0, 0, 1, trie.search("</s>").id)
        .sliding_window("tokens", 1025, 1025)
        .shape("tokens", "tokens_length", 0)
        .shuffle(1000)
        .batch(args.batch_size)
        .prefetch(2, 1)
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

    benchmark = Benchmark("MLX Wikitext103")
    for i in range(3):
        benchmark.log_run("iterate_workers_4", iterate, args, 4)

    for i in range(3):
        benchmark.log_run("iterate_workers_8", iterate, args, 8)

    for i in range(3):
        benchmark.log_run("iterate_workers_16", iterate, args, 16)
    benchmark.report()
