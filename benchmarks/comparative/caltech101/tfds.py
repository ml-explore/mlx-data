# Copyright © 2023 Apple Inc.

import argparse
from functools import partial
from pathlib import Path

import tensorflow as tf
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


def process_sample(sample):
    file_path = sample["file"]
    data = tf.io.read_file(file_path)
    img = tf.io.decode_jpeg(data, channels=3)

    height, width = to_float32([tf.shape(img)[0], tf.shape(img)[1]])
    min_side = to_float32(tf.minimum(height, width))
    scale_factor = min_side / tf.constant(256, dtype=tf.float32)
    img = tf.image.resize(img, to_int32([scale_factor * height, scale_factor * width]))
    img = tf.image.resize_with_crop_or_pad(img, 224, 224)
    img = img / tf.constant(255, dtype=tf.float32)

    return dict(image=img, label=sample["label"])


def iterate(args, workers):
    root = Path(args.data_dir)
    files, classes = files_and_classes(root)
    ds = tf.data.Dataset.zip(
        dict(
            file=tf.data.Dataset.from_tensor_slices(files),
            label=tf.data.Dataset.from_tensor_slices(classes),
        )
    )
    ds = (
        ds.shuffle(buffer_size=1000)
        .map(process_sample)
        .batch(args.batch_size)
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
    args = parser.parse_args()

    benchmark = Benchmark("TFDS Caltech 101")
    for i in range(3):
        benchmark.log_run("iterate_no_workers", iterate, args, 1)

    for i in range(3):
        benchmark.log_run("iterate_workers_8", iterate, args, 8)

    for i in range(3):
        benchmark.log_run("iterate_workers_16", iterate, args, 16)
    benchmark.report()
