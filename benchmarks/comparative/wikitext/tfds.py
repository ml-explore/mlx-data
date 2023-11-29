import argparse
from functools import partial
from pathlib import Path

import tensorflow as tf
import tensorflow_text as tf_text

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


def process_sample(tokenizer):
    def process_sample_impl(sample):
        tokens = tokenizer.tokenize(sample)
        windows = tf_text.sliding_window(tokens, 1025)
        windows = windows[::1025]

        return {"tokens": windows}

    return process_sample_impl


def iterate(args):
    root = Path(args.data_dir)

    # Load the list of lists of files
    filelist = [str(f).encode("ascii") for f in root.glob("**/*.txt")]

    with open(args.tokenizer_file, "rb") as f:
        tokenizer = tf_text.SentencepieceTokenizer(f.read())
        tokenizer.add_bos = True
        tokenizer.add_eos = True
    ds = (
        tf.data.Dataset.from_generator(
            document_iterator,
            args=[str(root / "wiki.train.raw")],
            output_types=tf.string,
            output_shapes=tuple(),
        )
        .map(process_sample(tokenizer), num_parallel_calls=tf.data.AUTOTUNE)
        .unbatch()
        .shuffle(buffer_size=1000)
        .batch(args.batch_size)
        .prefetch(tf.data.AUTOTUNE)
    )

    cnt = 0
    for batch in ds:
        cnt += 1
    return cnt


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("data_dir")
    parser.add_argument("--batch_size", type=int, default=32)
    parser.add_argument("--tokenizer_file", default="tokenizer.model")
    args = parser.parse_args()

    benchmark = Benchmark("TFDS Wikitext103")
    for i in range(3):
        benchmark.log_run("iterate_autotuned", iterate, args)
    benchmark.report()
