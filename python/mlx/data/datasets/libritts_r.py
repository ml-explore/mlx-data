import hashlib
from pathlib import Path

from ... import data as dx
from .common import (
    CACHE_DIR,
    ensure_exists,
    file_digest,
    gzip_decompress,
    urlretrieve_with_progress,
)

SPLITS = {
    "dev-clean": (
        "https://www.openslr.org/resources/141/dev_clean.tar.gz",
        "2c1f5312914890634cc2d15783032ff3",
    ),
    "dev-other": (
        "https://www.openslr.org/resources/141/dev_other.tar.gz",
        "62d3a80ad8a282b6f31b3904f0507e4f",
    ),
    "test-clean": (
        "https://www.openslr.org/resources/141/test_clean.tar.gz",
        "4d373d453eb96c0691e598061bbafab7",
    ),
    "test-other": (
        "https://www.openslr.org/resources/141/test_other.tar.gz",
        "dbc0959d8bdb6d52200595cabc9995ae",
    ),
    "train-clean-100": (
        "https://www.openslr.org/resources/141/train_clean_100.tar.gz",
        "6df668d8f5f33e70876bfa33862ad02b",
    ),
    "train-clean-360": (
        "https://www.openslr.org/resources/141/train_clean_360.tar.gz",
        "382eb3e64394b3da6a559f864339b22c",
    ),
    "train-other-500": (
        "https://www.openslr.org/resources/141/train_other_500.tar.gz",
        "a37a8e9f4fe79d20601639bf23d1add8",
    ),
}


def _get_transcript_file(sample):
    audio_file = Path(bytes(sample["file"]).decode("utf-8"))
    transcript_file = audio_file.with_suffix(".normalized.txt")
    sample["transcript_file"] = transcript_file.as_posix().encode("utf-8")
    return sample


def load_libritts_r_tarfile(
    root=None, split="dev-clean", quiet=False, validate_download=True
):
    """Fetch the LibriTTS-R TAR archive and return the path to it for manual processing.

    Args:
        root (Path or str, optional): The The directory to load/save the data. If
            none is given the ``~/.cache/mlx.data/libritts_r`` is used.
        split (str): The split to use. It should be one of dev-clean,
            dev-other, test-clean, test-other, train-clean-100,
            train-clean-360, train-other-500 .
        quiet (bool): If true do not show download (and possibly decompression)
            progress.
    """
    if split not in SPLITS:
        raise ValueError(
            f"Unknown libritts_r split '{split}'. It should be one of [{', '.join(SPLITS.keys())}]"
        )

    if root is None:
        root = CACHE_DIR / "libritts_r"
    else:
        root = Path(root)
    ensure_exists(root)

    url, target_hash = SPLITS[split]
    filename = Path(url).name
    target_compressed = root / filename
    target = root / filename.replace(".gz", "")

    if not target.is_file():
        if not target_compressed.is_file():
            urlretrieve_with_progress(url, target_compressed, quiet=quiet)
            if validate_download:
                h = file_digest(target_compressed, hashlib.md5(), quiet=quiet)
                if h.hexdigest() != target_hash:
                    raise RuntimeError(
                        f"[libritts_r] File download corrupted md5sums don't match. Please manually delete {str(target_compressed)}."
                    )

        gzip_decompress(target_compressed, target, quiet=quiet)
        target_compressed.unlink()

    return target


def load_libritts_r(root=None, split="dev-clean", quiet=False, validate_download=True):
    """Load the LibriTTS-R dataset directly from the TAR archive.

    Args:
        root (Path or str, optional): The The directory to load/save the data. If
            none is given the ``~/.cache/mlx.data/libritts_r`` is used.
        split (str): The split to use. It should be one of dev-clean,
            dev-other, test-clean, test-other, train-clean-100,
            train-clean-360, train-other-500 .
        quiet (bool): If true do not show download (and possibly decompression)
            progress.
    """

    target = load_libritts_r_tarfile(
        root=root, split=split, quiet=quiet, validate_download=validate_download
    )
    target = str(target)

    dset = (
        dx.files_from_tar(target)
        .to_stream()
        .sample_transform(lambda s: s if bytes(s["file"]).endswith(b".wav") else dict())
        .sample_transform(_get_transcript_file)
        .read_from_tar(target, "transcript_file", "transcript")
        .read_from_tar(target, "file", "audio")
        .load_audio("audio", from_memory=True)
    )

    return dset
