# Copyright © 2023 Apple Inc.

import gzip
import os
import sys
import time
import zipfile
from pathlib import Path
from urllib import request

CACHE_DIR = Path.home() / ".cache" / "mlx.data"


class FileProgressBar:
    def __init__(self, total_size, msg="", quiet=False, outfile=sys.stdout):
        self.quiet = quiet
        self.outfile = outfile
        self.msg = msg
        self.total_size = total_size
        self.start_time = time.time()
        self.current_size = 0
        self.current_time = self.start_time + 1
        self.max_length = 0

    def add(self, size):
        self.update(self.current_size + size)

    def update(self, current_size):
        self.current_size = current_size
        self.current_time = time.time()
        if not self.quiet:
            print("\r" + self.message(), end="", flush=True, file=self.outfile)

    def message(self, width=40):
        speed = self.current_size / (self.current_time - self.start_time)
        if self.total_size >= self.current_size:
            fill = "█"
            portion = self.current_size / self.total_size
            filled = int(round(portion * width))
            bar = fill * filled + " " * (width - filled)
            msg = (
                f"{self.msg} |{bar}| "
                f"{self.format_size(self.current_size)} / {self.format_size(self.total_size)} "
                f"({self.format_size(speed)}/s)"
            )
        else:
            msg = f"{self.msg} {self.format_size(self.current_size)} ({self.format_size(speed)}/s)"
        self.max_length = max(self.max_length, len(msg))
        return msg + " " * (self.max_length - len(msg))

    def urlretrieve_hook(self, num_blocks, block_size, total_size):
        if self.total_size != total_size:
            self.total_size = total_size
            self.start_time = time.time()
        self.update(num_blocks * block_size)

    def finalize(self):
        if not self.quiet:
            print(file=self.outfile)

    @staticmethod
    def format_size(size, suffix="B"):
        for unit in ("", "Ki", "Mi", "Gi", "Ti", "Pi", "Ei", "Zi"):
            if abs(size) < 1024.0:
                return f"{size:3.1f}{unit}{suffix}"
            size /= 1024.0
        return f"{size:.1f}Yi{suffix}"


def ensure_exists(directory):
    directory.mkdir(parents=True, exist_ok=True)


def urlretrieve_with_progress(src, dst, quiet=False):
    progress = FileProgressBar(0, f"Downloading {src}", quiet)
    hook = (lambda *args: None) if quiet else progress.urlretrieve_hook
    request.urlretrieve(src, dst, reporthook=hook)
    progress.finalize()


def file_digest(filepath, _hash, buffer_size=16 * 1024 * 1024, quiet=False):
    progress = FileProgressBar(
        os.stat(filepath).st_size, f"Computing hash of {filepath}", quiet
    )
    buff = bytearray(buffer_size)
    with open(filepath, "rb", buffering=buffer_size) as f:
        while True:
            n = f.readinto(buff)
            progress.add(n)
            if n == 0:
                break
            _hash.update(buff[:n])
    progress.finalize()
    return _hash


def gzip_decompress(src, dst, buffer_size=16 * 1024 * 1024, quiet=False):
    buff = bytearray(buffer_size)
    progress = FileProgressBar(0, f"Decompressing {src}", quiet)
    with gzip.open(src) as f_in:
        with open(dst, "wb") as f_out:
            while True:
                n = f_in.readinto(buff)
                if n == 0:
                    break
                f_out.write(buff[:n])
                progress.add(n)
    progress.finalize()


def unzip(
    src,
    dst,
    path_filter=lambda x: "" if x.endswith("/") else x,
    buffer_size=16 * 1024 * 1024,
    quiet=False,
):
    """Unzip files from src into dst with a progress bar.

    ``path_filter`` is used to both edit and filter out the files from the zip.
    If an empty string is returned then that file is not extracted otherwise
    the string is used as the destination file path.
    """
    buff = bytearray(buffer_size)
    dst = Path(dst)
    progress = FileProgressBar(0, f"Extracting {src}", quiet)
    with zipfile.ZipFile(src) as f_zip:
        for file in f_zip.namelist():
            target = path_filter(file)
            if not target:
                continue

            target = dst / target
            if not target.parent.exists():
                target.parent.mkdir(parents=True, exist_ok=True)

            with f_zip.open(file, "r") as f_in:
                with open(target, "wb") as f_out:
                    while True:
                        n = f_in.readinto(buff)
                        if n == 0:
                            break
                        f_out.write(buff[:n])
                        progress.add(n)
    progress.finalize()
