# Copyright © 2023 Apple Inc.

import gzip
import pickle
from pathlib import Path
from tempfile import NamedTemporaryFile

import numpy as np

from ... import data as dx
from .common import CACHE_DIR, ensure_exists, urlretrieve_with_progress


def load_mnist(root=None, train=True):
    """Load a buffer with the MNIST dataset.

    If the data doesn't exist download it and save it for the next time.

    Args:
        root (Path or str, optional): The directory to load/save the data. If
            none is given the ``~/.cache/mlx.data/mnist`` is used.
        train (bool): Load the training or test set.
    """
    if root is None:
        root = CACHE_DIR / "mnist"
    else:
        root = Path(root)

    ensure_exists(root)

    def download():
        base_url = "http://yann.lecun.com/exdb/mnist/"
        filename = [
            [NamedTemporaryFile(), "training_images", "train-images-idx3-ubyte.gz"],
            [NamedTemporaryFile(), "test_images", "t10k-images-idx3-ubyte.gz"],
            [NamedTemporaryFile(), "training_labels", "train-labels-idx1-ubyte.gz"],
            [NamedTemporaryFile(), "test_labels", "t10k-labels-idx1-ubyte.gz"],
        ]

        mnist = {}
        for out_file, _, name in filename:
            urlretrieve_with_progress(base_url + name, out_file.name)

        for out_file, key, _ in filename[:2]:
            with gzip.open(out_file.name, "rb") as f:
                mnist[key] = np.frombuffer(f.read(), np.uint8, offset=16).reshape(
                    -1, 28, 28, 1
                )
        for out_file, key, _ in filename[-2:]:
            with gzip.open(out_file.name, "rb") as f:
                mnist[key] = np.frombuffer(f.read(), np.uint8, offset=8)
        train_set = [
            {"image": mnist["training_images"][i], "label": mnist["training_labels"][i]}
            for i in range(len(mnist["training_images"]))
        ]
        test_set = [
            {"image": mnist["test_images"][i], "label": mnist["test_labels"][i]}
            for i in range(len(mnist["test_images"]))
        ]

        with (root / "train.pkl").open("wb") as f:
            pickle.dump(train_set, f)
        with (root / "test.pkl").open("wb") as f:
            pickle.dump(test_set, f)

    if not (root / "test.pkl").is_file():
        download()

    pkl_file = (root / "train.pkl") if train else (root / "test.pkl")
    with pkl_file.open("rb") as f:
        return dx.buffer_from_vector(pickle.load(f))
