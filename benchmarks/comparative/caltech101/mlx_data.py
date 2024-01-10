# Copyright © 2023 Apple Inc.

import argparse
from pathlib import Path

from utils import Benchmark

import mlx.data as dx


def files_and_classes(root: Path):
    files = [str(f) for f in root.glob("**/*.jpg")]
    files = [f for f in files if "BACKGROUND" not in f]
    classes = dict(
        map(reversed, enumerate(sorted(set(f.split("/")[-2] for f in files))))
    )

    return [
        dict(image=f.encode("ascii"), label=classes[f.split("/")[-2]]) for f in files
    ]


def iterate(args, workers):
    root = Path(args.data_dir)
    dset = (
        dx.buffer_from_vector(files_and_classes(root))
        .shuffle()
        .to_stream()
        .load_image("image")
        .image_resize_smallest_side("image", 256)
        .image_center_crop("image", 224, 224)
        .batch(args.batch_size)
        .key_transform("image", lambda x: x.astype("float32") / 255)
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
    args = parser.parse_args()

    benchmark = Benchmark("MLX Caltech 101")
    for i in range(3):
        benchmark.log_run("iterate_no_workers", iterate, args, 1)

    for i in range(3):
        benchmark.log_run("iterate_workers_8", iterate, args, 8)

    for i in range(3):
        benchmark.log_run("iterate_workers_16", iterate, args, 16)
    benchmark.report()
