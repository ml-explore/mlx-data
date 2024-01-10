# Copyright © 2023 Apple Inc.

import hashlib
from pathlib import Path

from ... import data as dx
from .common import (
    CACHE_DIR,
    ensure_exists,
    file_digest,
    unzip,
    urlretrieve_with_progress,
)

SUBSETS = {
    "wikitext-2": (
        "https://s3.amazonaws.com/research.metamind.io/wikitext/wikitext-2-v1.zip",
        "92675f1d63015c1c8b51f1656a52d5bdbc33aafa60cc47a218a66e7ee817488c",
    ),
    "wikitext-2-raw": (
        "https://s3.amazonaws.com/research.metamind.io/wikitext/wikitext-2-raw-v1.zip",
        "ef7edb566e3e2b2d31b29c1fdb0c89a4cc683597484c3dc2517919c615435a11",
    ),
    "wikitext-103": (
        "https://s3.amazonaws.com/research.metamind.io/wikitext/wikitext-103-v1.zip",
        "242ba0f20b329cfdf1ccc61e9e9e5b59becf189db7f7a81cd2a0e2fc31539590",
    ),
    "wikitext-103-raw": (
        "https://s3.amazonaws.com/research.metamind.io/wikitext/wikitext-103-raw-v1.zip",
        "91c00ae287f0d699e18605c84afc9e45c192bc6b7797ff8837e5474655a33794",
    ),
}


def load_wikitext_lines(
    root=None,
    split="train",
    subset="wikitext-103-raw",
    quiet=False,
    validate_download=True,
):
    """Fetch the WikiText dataset and load it as a stream of lines.

    Args:
        root (Path or str, optional): The The directory to load/save the data. If
            none is given the ``~/.cache/mlx.data/wikitext`` is used.
        split (str): The split to use. It should be one of train, valid, test. (default: train)
        subset (str): The subset to use. It should be one of wikitext-103,
            wikitext-103-raw, wikitext-2, wikitext-2-raw . (default: wikitext-103-raw)
        quiet (bool): If true do not show progress bars.
    """

    if subset not in SUBSETS:
        raise ValueError(
            f"Unknown wikitext subset '{subset}'. It should be one of [{', '.join(SUBSETS.keys())}]"
        )

    if root is None:
        root = CACHE_DIR / "wikitext"
    else:
        root = Path(root)
    ensure_exists(root)

    url, target_hash = SUBSETS[subset]
    filename = Path(url).name
    target_compressed = root / filename
    target = root / subset

    if not target.is_dir():
        if not target_compressed.is_file():
            urlretrieve_with_progress(url, target_compressed, quiet=quiet)
            if validate_download:
                h = file_digest(target_compressed, hashlib.sha256(), quiet=quiet)
                if h.hexdigest() != target_hash:
                    raise RuntimeError(
                        f"[wikitext] File download corrupted sha256sums don't match. Please manually delete {str(target_compressed)}."
                    )
        unzip(target_compressed, root, quiet=quiet)
        target_compressed.unlink(missing_ok=True)

    if "raw" in subset:
        target = target / f"wiki.{split}.raw"
    else:
        target = target / f"wiki.{split}.tokens"

    return dx.stream_line_reader(str(target), "line")
