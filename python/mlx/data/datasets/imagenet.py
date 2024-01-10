# Copyright © 2023 Apple Inc.

import hashlib
import os
import pickle
import shutil
import tarfile
import tempfile
from functools import lru_cache
from pathlib import Path

from ... import data as dx
from .common import CACHE_DIR, file_digest, gzip_decompress

META_FILE = "ILSVRC2012_devkit_t12/data/meta.mat"
VAL_GT_FILE = "ILSVRC2012_devkit_t12/data/ILSVRC2012_validation_ground_truth.txt"
ARCHIVES = {
    "train": (
        "ILSVRC2012_img_train.tar",
        "ILSVRC2012_img_train",
        "1d675b47d978889d74fa0da5fadfb00e",
    ),
    "val": (
        "ILSVRC2012_img_val.tar",
        "ILSVRC2012_img_val",
        "29b22e2961454d5413ddabcf34fc5622",
    ),
    "devkit": (
        "ILSVRC2012_devkit_t12.tar.gz",
        "ILSVRC2012_devkit_t12",
        "fa75699e90414af021442c21a62c3abf",
    ),
}


def _metafile_to_python(meta):
    parsed_meta = {}
    for row in meta["synsets"]:
        parsed_meta[row[1]] = {
            "id": row[0],
            "label": row[0] - 1,
            "description": row[2],
            "num_train_images": row[7],
        }
    return parsed_meta


@lru_cache
def load_imagenet_devkit(root=None, quiet=False, validate_download=True):
    """Load the metadata and the validation ground truth from the devkit."""
    from scipy.io import loadmat

    def process_meta_and_val_gt(metafile_fd, valgt_fd, dstdir):
        meta = loadmat(metafile_fd, squeeze_me=True)
        meta = _metafile_to_python(meta)
        with open(dstdir / "meta.pkl", "wb") as f:
            pickle.dump(meta, f)

        val_gt = [int(l) - 1 for l in valgt_fd]
        with open(dstdir / "val_gt.pkl", "wb") as f:
            pickle.dump(val_gt, f)

    if root is None:
        root = CACHE_DIR / "imagenet"
    else:
        root = Path(root)

    archive, folder, target_hash = ARCHIVES["devkit"]
    archive = root / archive
    folder = root / folder

    if not (archive.is_file() or folder.is_dir()):
        raise RuntimeError(
            f"The devkit is missing from {str(root)}. Make sure to download {archives.name} from the imagenet website."
        )

    if not folder.is_dir():
        if validate_download:
            h = file_digest(archive, hashlib.md5(), quiet=quiet)
            if h.hexdigest() != target_hash:
                raise RuntimeError(
                    f"Imagenet devkit download corrupted, md5sums don't match. Please manually re-download {archive.name}."
                )

        tmpdir = Path(tempfile.mkdtemp(prefix=str(root.absolute())))
        try:
            with tarfile.open(archive, mode="r:gz") as tar:
                process_meta_and_val_gt(
                    tar.extractfile(META_FILE),
                    tar.extractfile(VAL_GT_FILE),
                    tmpdir,
                )
            tmpdir.rename(folder)
        finally:
            if tmpdir.exists():
                shutil.rmtree(tmpdir)

    if not (folder / "val_gt.pkl").is_file():
        if not (
            (folder.parent / META_FILE).is_file()
            and (folder.parent / VAL_GT_FILE).is_file()
        ):
            raise RuntimeError(
                f"The imagenet devkit is corrupt or incorrectly extracted in {str(folder)}."
            )

        process_meta_and_val_gt(
            (folder.parent / META_FILE).open("rb"),
            (folder.parent / VAL_GT_FILE).open("r"),
            folder,
        )

    metadata = pickle.load((folder / "meta.pkl").open("rb"))
    val_gt = pickle.load((folder / "val_gt.pkl").open("rb"))

    return metadata, val_gt


def load_imagenet_metadata(root=None, quiet=False, validate_download=True):
    """Load the metadata for the imagenet classes."""
    return load_imagenet_devkit(root, quiet, validate_download)[0]


def load_imagenet(
    root=None,
    split="train",
    quiet=False,
    validate_download=True,
    tar_index_threads=None,
):
    """Load the ImageNet dataset from the downloaded archives.

    ImageNet cannot be automatically downloaded so you have to manually
    download it from http://image-net.org/ . You need the split you want to
    load and the devkit for tasks 1 and 2.

    Args:
        root (Path or str, optional): The directory to load the data from. If
            none is given then ``~/.cache/mlx.data/imagenet`` is used. However,
            if the data is not there it *cannot* be downloaded automatically.
        split (str): The split to use. It must be either 'train' or 'val'.
        quiet (bool): If true do not show progress bars.
        validate_download (bool): If true validate the download if it isn't
            already validated.
        tar_index_threads (int, optional): How many threads to use to index the
            nested tar file for the imagenet training set. This is not used for
            the validation set or if the tar file is extracted.
    """
    if split not in ["train", "val"]:
        raise ValueError(
            f"Unknown imagenet split '{split}'. It must be either 'train' or 'val'."
        )

    if root is None:
        root = CACHE_DIR / "imagenet"
    else:
        root = Path(root)

    metadata, val_gt = load_imagenet_devkit(root, quiet, validate_download)
    tarfile, folder, target_hash = ARCHIVES[split]

    if not ((root / tarfile).is_file() or (root / folder).is_dir()):
        raise RuntimeError(
            f"Could not find imagenet {split} split. Make sure that the "
            f"data is either untarred in {folder} or that {tarfile} exists."
        )

    if validate_download:
        if not (root / (tarfile + ".hash")).is_file():
            h = file_digest(root / tarfile, hashlib.md5(), quiet=quiet)
            if h.hexdigest() != target_hash:
                raise RuntimeError(
                    f"The imagenet {split} split is corrupt, "
                    "md5sums don't match. Please redownload."
                )
            open(root / (tarfile + ".hash"), "w").write(h.hexdigest())
        else:
            hexdigest = open(root / (tarfile + ".hash"), "r").read()
            if hexdigest != target_hash:
                raise RuntimeError(
                    f"The imagenet {split} split is corrupt, md5sums don't match. "
                    f"Please redownload and make sure to delete {str(root / tarfile + '.hash')}."
                )

    if split == "val":
        if (root / folder).is_dir():
            files = (root / folder).glob("*.JPEG")
            files = [
                {"file": str(f.relative_to(root / folder)).encode("ascii"), "label": l}
                for f, l in zip(sorted(files), val_gt)
            ]
            return dx.buffer_from_vector(files).load_image(
                "file", prefix=str(root / folder), output_key="image"
            )

        else:
            files = dx.files_from_tar(str(root / tarfile))
            files = [bytes(f["file"]) for f in files]
            files = [{"file": f, "label": l} for f, l in zip(sorted(files), val_gt)]
            return (
                dx.buffer_from_vector(files)
                .read_from_tar(tarfile, "file", "image", tar_prefix=str(root))
                .load_image("image", from_memory=True)
            )

    else:
        if (root / folder).is_dir():
            files = (root / folder).rglob("*.JPEG")
            files = [f.relative_to(root / folder) for f in files]
            files = [
                {
                    "file": str(f).encode("ascii"),
                    "label": metadata[f.parent.name]["label"],
                }
                for f in files
            ]
            return dx.buffer_from_vector(files).load_image(
                "file", prefix=str(root / folder), output_key="image"
            )

        else:

            def file_to_label(x):
                directory = bytes(x).split(b"/")[0].decode("ascii")
                return metadata[directory]["label"]

            tar_index_threads = tar_index_threads or os.cpu_count()
            return (
                dx.files_from_tar(
                    str(root / tarfile), nested=True, num_threads=tar_index_threads
                )
                .key_transform("file", file_to_label, "label")
                .read_from_tar(
                    tarfile,
                    "file",
                    "image",
                    tar_prefix=str(root),
                    nested=True,
                    num_threads=tar_index_threads,
                )
                .load_image("image", from_memory=True)
            )
