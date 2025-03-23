# Copyright © 2023 Apple Inc.

import gzip
import pickle
from pathlib import Path
from tempfile import NamedTemporaryFile

import numpy as np

from ... import data as dx
from .common import CACHE_DIR, ensure_exists, urlretrieve_with_progress


def _load_mnist_wrapper(root=None, train=True, dataset="mnist"):
    url_dict = {
        "mnist": "https://raw.githubusercontent.com/fgnt/mnist/master/",
        "fashion-mnist": "http://fashion-mnist.s3-website.eu-central-1.amazonaws.com/",
    }
    base_url = url_dict[dataset]

    if root is None:
        root = CACHE_DIR / dataset
    else:
        root = Path(root)

    ensure_exists(root)

    def download():
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
                    -1, 28 * 28
                )
        for out_file, key, _ in filename[-2:]:
            with gzip.open(out_file.name, "rb") as f:
                mnist[key] = np.frombuffer(f.read(), np.uint8, offset=8)

        train_set = []
        for i in range(len(mnist["training_images"])):
            train_set.append(
                {
                    "image": np.ascontiguousarray(mnist["training_images"][i]),
                    "label": mnist["training_labels"][i].item(),
                }
            )

        test_set = []
        for i in range(len(mnist["test_images"])):
            test_set.append(
                {
                    "image": np.ascontiguousarray(mnist["test_images"][i]),
                    "label": mnist["test_labels"][i].item(),
                }
            )

        with (root / "train.pkl").open("wb") as f:
            pickle.dump(train_set, f)
        with (root / "test.pkl").open("wb") as f:
            pickle.dump(test_set, f)

    if not (root / "test.pkl").is_file():
        download()

    pkl_file = (root / "train.pkl") if train else (root / "test.pkl")
    with pkl_file.open("rb") as f:
        return dx.buffer_from_vector(pickle.load(f))


def load_mnist(root=None, train=True):
    """Load a buffer with the MNIST dataset.

    If the data doesn't exist download it and save it for the next time.

    Args:
        root (Path or str, optional): The directory to load/save the data. If
            none is given the ``~/.cache/mlx.data/mnist`` is used.
        train (bool): Load the training or test set.
    """

    return _load_mnist_wrapper(root, train, "mnist")


def load_fashion_mnist(root=None, train=True):
    """Load a buffer with the Fashion-MNIST dataset.

    If the data doesn't exist download it and save it for the next time.

    Args:
        root (Path or str, optional): The directory to load/save the data. If
            none is given the ``~/.cache/mlx.data/fashion-mnist`` is used.
        train (bool): Load the training or test set.
    """

    return _load_mnist_wrapper(root, train, "fashion-mnist")
