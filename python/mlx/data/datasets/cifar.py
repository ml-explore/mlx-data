# Copyright © 2023 Apple Inc.

import hashlib
import pickle
import shutil
import tarfile
from tempfile import NamedTemporaryFile

import numpy as np

from ... import data as dx
from .common import CACHE_DIR, ensure_exists, file_digest, urlretrieve_with_progress

URLS = {
    "CIFAR10": (
        "https://www.cs.toronto.edu/~kriz/cifar-10-python.tar.gz",
        "cifar-10-python.tar.gz",
        "c58f30108f718f92721af3b95e74349a",
    ),
    "CIFAR100": (
        "https://www.cs.toronto.edu/~kriz/cifar-100-python.tar.gz",
        "cifar-100-python.tar.gz",
        "eb9058c3a382ffc7106e4002c42a8d85",
    ),
}


def unpickle_from_tar(tarfd):
    with NamedTemporaryFile() as tmpfd:
        shutil.copyfileobj(tarfd, tmpfd)
        tmpfd.seek(0)
        return pickle.load(tmpfd, encoding="latin1")


def _download(root, data, quiet, validate_download):
    url, name, target_hash = URLS[data]
    target = root / name
    if not target.is_file():
        urlretrieve_with_progress(url, target, quiet=quiet)

    if validate_download:
        h = file_digest(target, hashlib.md5(), quiet=quiet)
        if h.hexdigest() != target_hash:
            raise RuntimeError(
                f"[CIFAR] File download corrupted md5sums don't match. Please manually delete {str(target)}."
            )

    if data == "CIFAR10":
        train_files = [f"cifar-10-batches-py/data_batch_{i}" for i in range(1, 6)]
        test_file = "cifar-10-batches-py/test_batch"
        class_names = "cifar-10-batches-py/batches.meta"
        label_prefix = ""

    elif data == "CIFAR100":
        train_files = ["cifar-100-python/train"]
        test_file = "cifar-100-python/test"
        class_names = "cifar-100-python/meta"
        label_prefix = "fine_"

    train_data = []
    test_data = None
    metadata = None
    with tarfile.open(target, "r|gz") as tar:
        for member in tar:
            if member.name in train_files:
                train_data.append(unpickle_from_tar(tar.extractfile(member)))
            elif member.name == test_file:
                test_data = unpickle_from_tar(tar.extractfile(member))
            elif member.name == class_names:
                metadata = unpickle_from_tar(tar.extractfile(member))

    train_set = []
    for i in range(len(train_files)):
        for j in range(len(train_data[i]["data"])):
            train_set.append(
                {
                    "image": np.ascontiguousarray(
                        train_data[i]["data"][j].reshape(3, 32, 32).transpose(1, 2, 0)
                    ),
                    "label": train_data[i][f"{label_prefix}labels"][j],
                }
            )

    test_set = []
    for i in range(len(test_data["data"])):
        test_set.append(
            {
                "image": np.ascontiguousarray(
                    test_data["data"][i].reshape(3, 32, 32).transpose(1, 2, 0)
                ),
                "label": test_data[f"{label_prefix}labels"][i],
            }
        )

    prefix = "cifar10" if data == "CIFAR10" else "cifar100"
    with (root / f"{prefix}_meta.pkl").open("wb") as f:
        pickle.dump(metadata[f"{label_prefix}label_names"], f)
    with (root / f"{prefix}_train.pkl").open("wb") as f:
        pickle.dump(train_set, f)
    with (root / f"{prefix}_test.pkl").open("wb") as f:
        pickle.dump(test_set, f)


def load_cifar10(root=None, train=True, quiet=False, validate_download=True):
    """Load a buffer with the CIFAR-10 dataset.

    Args:
        root (Path or str, optional): The directory to load/save the data. If
            none is given the ``~/.cache/mlx.data/cifar`` is used.
        train (bool): Load the training or test set.
    """
    if root is None:
        root = CACHE_DIR / "cifar"
    else:
        root = Path(root)

    ensure_exists(root)

    if not (root / "cifar10_test.pkl").is_file():
        _download(root, "CIFAR10", quiet, validate_download)

    pkl_file = (root / "cifar10_train.pkl") if train else (root / "cifar10_test.pkl")
    with pkl_file.open("rb") as f:
        return dx.buffer_from_vector(pickle.load(f))


def load_cifar100(root=None, train=True, quiet=False, validate_download=True):
    """Load a buffer with the CIFAR-100 dataset.

    Args:
        root (Path or str, optional): The directory to load/save the data. If
            none is given the ``~/.cache/mlx.data/cifar`` is used.
        train (bool): Load the training or test set.
    """
    if root is None:
        root = CACHE_DIR / "cifar"
    else:
        root = Path(root)

    ensure_exists(root)

    if not (root / "cifar100_test.pkl").is_file():
        _download(root, "CIFAR100", quiet, validate_download)

    pkl_file = (root / "cifar100_train.pkl") if train else (root / "cifar100_test.pkl")
    with pkl_file.open("rb") as f:
        return dx.buffer_from_vector(pickle.load(f))
