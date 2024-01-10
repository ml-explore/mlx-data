# Copyright © 2023 Apple Inc.

import argparse

import torch
from torchvision.datasets import ImageFolder
from torchvision.transforms import v2 as transforms
from utils import Benchmark


class Caltech101(ImageFolder):
    def __init__(self, root: str):
        super().__init__(
            str(root),
            transform=transforms.Compose(
                [
                    transforms.ToImage(),
                    transforms.Resize(256),
                    transforms.CenterCrop(224),
                    transforms.ToDtype(torch.float32, scale=True),
                ]
            ),
            is_valid_file=self.is_valid_file,
        )

    def is_valid_file(self, filepath: str):
        return "BACKGROUND" not in filepath and filepath.endswith("jpg")


def iterate(args, workers):
    dset = Caltech101(args.data_dir)
    data_loader = torch.utils.data.DataLoader(
        dset, shuffle=True, batch_size=args.batch_size, num_workers=workers
    )

    cnt = 0
    for batch in data_loader:
        cnt += 1
    return cnt


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("data_dir")
    parser.add_argument("--batch_size", type=int, default=32)
    args = parser.parse_args()

    benchmark = Benchmark("PyTorch Caltech 101")
    for i in range(3):
        benchmark.log_run("iterate_no_workers", iterate, args, 0)

    for i in range(3):
        benchmark.log_run("iterate_workers_8", iterate, args, 8)

    for i in range(3):
        benchmark.log_run("iterate_workers_16", iterate, args, 16)
    benchmark.report()
