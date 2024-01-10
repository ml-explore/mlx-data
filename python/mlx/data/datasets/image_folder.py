# Copyright © 2023 Apple Inc.

from pathlib import Path

from ... import data as dx


def load_images_from_folder(image_folder):
    """Load images from a folder.

    For a directory structure like the following

    .. code-block::

       image_folder/
       ...class_1/
       ......foo.jpg
       ......bar.png
       ...class_2/
       ......baz.jpg
       ......foo_again.png

    this function will return a :class:`~mlx.data.Buffer` that contains samples
    with the following keys

    - folder: the name of the category of this sample (e.g. class_1, class_2 etc)
    - label: an integer that corresponds to the sorted position of the folder
      names (e.g. class_1 gets 0 and class_2 gets 1)
    - file: the path to the image relative to the provided root folder
    - image: the loaded image array

    Args:
        image_folder: (Path or str): The directory to load the images from.
    """
    root = Path(image_folder)
    if not root.is_dir():
        raise ValueError(f"The provided path {root} is not a directory")

    directories = sorted(
        [f for f in root.iterdir() if f.is_dir()], key=lambda x: x.name
    )
    if not directories:
        raise ValueError(f"The provided path {root} contains no directories")

    classes = {f.name: i for i, f in enumerate(directories)}
    samples = [
        dict(
            folder=folder.name.encode(),
            label=classes[folder.name],
            file=str(img.relative_to(root)).encode(),
        )
        for folder in directories
        for img in folder.iterdir()
        if img.is_file()
    ]

    return dx.buffer_from_vector(samples).load_image(
        "file", prefix=str(root), output_key="image"
    )
