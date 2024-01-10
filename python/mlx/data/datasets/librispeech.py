# Copyright © 2023 Apple Inc.

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
        "https://www.openslr.org/resources/12/dev-clean.tar.gz",
        "76f87d090650617fca0cac8f88b9416e0ebf80350acb97b343a85fa903728ab3",
    ),
    "dev-other": (
        "https://www.openslr.org/resources/12/dev-other.tar.gz",
        "12661c48e8c3fe1de2c1caa4c3e135193bfb1811584f11f569dd12645aa84365",
    ),
    "test-clean": (
        "https://www.openslr.org/resources/12/test-clean.tar.gz",
        "39fde525e59672dc6d1551919b1478f724438a95aa55f874b576be21967e6c23",
    ),
    "test-other": (
        "https://www.openslr.org/resources/12/test-other.tar.gz",
        "d09c181bba5cf717b3dee7d4d592af11a3ee3a09e08ae025c5506f6ebe961c29",
    ),
    "train-clean-100": (
        "https://www.openslr.org/resources/12/train-clean-100.tar.gz",
        "d4ddd1d5a6ab303066f14971d768ee43278a5f2a0aa43dc716b0e64ecbbbf6e2",
    ),
    "train-clean-360": (
        "https://www.openslr.org/resources/12/train-clean-360.tar.gz",
        "146a56496217e96c14334a160df97fffedd6e0a04e66b9c5af0d40be3c792ecf",
    ),
    "train-other-500": (
        "https://www.openslr.org/resources/12/train-other-500.tar.gz",
        "ddb22f27f96ec163645d53215559df6aa36515f26e01dd70798188350adcb6d2",
    ),
}


def _to_audio_and_transcript(sample):
    # Split the line
    file_part, transcript = bytes(sample["sample"]).split(b" ", 1)

    # Extract the audio path
    parts = file_part.split(b"-")
    parts[-1] = file_part + b".flac"
    audio_path = b"/".join(parts)

    # Prepare the transcript
    transcript = transcript.lower()

    return {"audio_file": audio_path, "transcript": transcript}


def load_librispeech_tarfile(
    root=None, split="dev-clean", quiet=False, validate_download=True
):
    """Fetch the librispeech TAR archive and return the path to it for manual processing.

    Args:
        root (Path or str, optional): The The directory to load/save the data. If
            none is given the ``~/.cache/mlx.data/librispeech`` is used.
        split (str): The split to use. It should be one of dev-clean,
            dev-other, test-clean, test-other, train-clean-100,
            train-clean-360, train-other-500 .
        quiet (bool): If true do not show download (and possibly decompression)
            progress.
    """
    if split not in SPLITS:
        raise ValueError(
            f"Unknown librispeech split '{split}'. It should be one of [{', '.join(SPLITS.keys())}]"
        )

    if root is None:
        root = CACHE_DIR / "librispeech"
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
                h = file_digest(target_compressed, hashlib.sha256(), quiet=quiet)
                if h.hexdigest() != target_hash:
                    raise RuntimeError(
                        f"[librispeech] File download corrupted sha256sums don't match. Please manually delete {str(target_compressed)}."
                    )

        gzip_decompress(target_compressed, target, quiet=quiet)
        target_compressed.unlink()

    return target


def load_librispeech(root=None, split="dev-clean", quiet=False, validate_download=True):
    """Load the librispeech dataset directly from the TAR archive.

    Args:
        root (Path or str, optional): The The directory to load/save the data. If
            none is given the ``~/.cache/mlx.data/librispeech`` is used.
        split (str): The split to use. It should be one of dev-clean,
            dev-other, test-clean, test-other, train-clean-100,
            train-clean-360, train-other-500 .
        quiet (bool): If true do not show download (and possibly decompression)
            progress.
    """
    target = load_librispeech_tarfile(
        root=root, split=split, quiet=quiet, validate_download=validate_download
    )
    target = str(target)
    prefix = f"LibriSpeech/{split}"

    dset = (
        dx.files_from_tar(target)
        .to_stream()
        .sample_transform(lambda s: s if bytes(s["file"]).endswith(b".txt") else dict())
        .read_from_tar(target, "file", "samples")
        .line_reader_from_key("samples", "sample", from_memory=True)
        .sample_transform(_to_audio_and_transcript)
        .prefetch(8, 4)
        .to_buffer()
        .read_from_tar(target, "audio_file", "audio", prefix=prefix)
        .load_audio("audio", from_memory=True)
    )

    return dset
