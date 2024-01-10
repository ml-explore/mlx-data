import hashlib
import tarfile
from pathlib import Path

from ... import data as dx
from .common import (
    CACHE_DIR,
    ensure_exists,
    file_digest,
    gzip_decompress,
    urlretrieve_with_progress,
)

URL = "http://download.tensorflow.org/data/speech_commands_v0.02.tar.gz"
URL_HASH = "af14739ee7dc311471de98f5f9d2c9191b18aedfe957f4a6ff791c709868ff58"
EXCLUDE_FOLDER = "_background_noise_"
HASH_DIVIDER = "_nohash_"

SPLITS_INFO = {
    "validation": "./validation_list.txt",
    "test": "./testing_list.txt",
    "train": None,
}


def download_speechcommands(root=None, quiet=False, validate_download=True):
    """Download/fetch the speechcommands TAR archive and return the path to it for processing.

    Args:
        root (Path or str, optional): The The directory to load/save the data. If
            none is given the ``~/.cache/mlx.data/speechcommands`` is used.
        quiet (bool, optional): If true do not show download (and possibly decompression)
            progress. Default is False.
        validate_download (bool, optional): Validate the download using the checksum.
            Default is True.
    """
    if root is None:
        root = CACHE_DIR / "speechcommands"
    else:
        root = Path(root)
    ensure_exists(root)
    url, target_hash = URL, URL_HASH
    filename = Path(url).name
    target_compressed = root / filename
    target = root / filename.replace(".gz", "")
    if not target.is_file():
        if not target_compressed.is_file():
            urlretrieve_with_progress(url, target_compressed, quiet=quiet)
            if validate_download:
                h = file_digest(target_compressed, hashlib.sha256(), quiet=quiet)
                if h.hexdigest() != target_hash:
                    raise RuntimeError(
                        f"[speechcommands] File download corrupted. sha256sums don't match. Please manually delete {str(target_compressed)}."
                    )
        gzip_decompress(target_compressed, target, quiet=quiet)
        target_compressed.unlink()
    return target


def get_metadata(tarfile_path):
    """
    Helper function to get metadata from the tarfile.
    This includes file names by split, as well as class names and their mapping to integers.
    """
    output = {}
    with tarfile.open(tarfile_path) as tar:
        for split, split_file in SPLITS_INFO.items():
            if split_file is None:
                continue
            f = tar.extractfile(split_file)
            fileslist = ["./" + s.decode().strip() for s in f.readlines()]
            output[split] = fileslist

        all_wav_files = [
            f.name
            for f in tar.getmembers()
            if ".wav" in f.name
            and HASH_DIVIDER in f.name
            and EXCLUDE_FOLDER not in f.name
        ]
    output["train"] = set(all_wav_files) - set(output["validation"] + output["test"])
    classes = dict(
        map(reversed, enumerate(sorted(set(f.split("/")[-2] for f in output["train"]))))
    )
    return output, classes


def load_speechcommands(root=None, split="train", quiet=False, validate_download=True):
    """Load the Speech Commands (v0.0.2) [1] dataset directly from the TAR archive.

    Args:
        root (Path or str, optional): The The directory to load/save the data. If
            none is given the ``~/.cache/mlx.data/speechcommands`` is used.
        split (str): The split to use. It should be one of train,
            validation or test
        quiet (bool): If true do not show download (and possibly decompression)
            progress.

    References
    ----------
    [1] Warden, Pete. "Speech commands: A dataset for limited-vocabulary speech recognition." arXiv preprint arXiv:1804.03209 (2018).
    """
    target = download_speechcommands(
        root=root, quiet=quiet, validate_download=validate_download
    )
    target = str(target)
    assert (
        split in SPLITS_INFO
    ), f"Unknown split {split}. Should be one of [{', '.join(SPLITS_INFO.keys())}]"

    files_by_split, class_map = get_metadata(target)

    file_list = [
        {"file": f.encode(), "label": class_map[f.split("/")[-2]]}
        for f in files_by_split[split]
    ]
    dset = (
        dx.buffer_from_vector(file_list)
        .read_from_tar(target, "file", "audio")
        .load_audio("audio", from_memory=True)
    )
    return dset
