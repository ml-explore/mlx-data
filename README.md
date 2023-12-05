MLX Data
=========

MLX Data is a framework agnostic data loading library brought to you by Apple
machine learning research. It works with PyTorch, Jax or
[MLX](https://ml-explore.github.io/mlx/).

The goal of the project is to be efficient but also flexible, enabling for
instance the loading and processing of 1,000s of images per second but also
running arbitrary python transformations on the resulting batches.

It can be used from Python as is shown in the following examples or from C++
with a very similar intuitive API.

For more details see the [documentation](https://ml-explore.github.io/mlx-data/).

Example
=======

The following pipeline is taken from the `Caltech 101` benchmark found in
`benchmarks/comparative/caltech101/mlx_data.py`.

```python
# A simple python function returning a list of dicts. All samples in MLX data
# are dicts of arrays.
def files_and_classes(root: Path):
    files = [str(f) for f in root.glob("**/*.jpg")]
    files = [f for f in files if "BACKGROUND" not in f]
    classes = dict(
        map(reversed, enumerate(sorted(set(f.split("/")[-2] for f in files))))
    )

    return [
        dict(image=f.encode("ascii"), label=classes[f.split("/")[-2]]) for f in files
    ]


dset = (
    # Make a buffer (finite length container of samples) from the python list
    dx.buffer_from_vector(files_and_classes(root))

    # Shuffle and transform to a stream
    .shuffle()
    .to_stream()

    # Implement a simple image pipeline. No random augmentations here but they
    # could be applied.
    .load_image("image")  # load the file pointed to by the 'image' key as an image
    .image_resize_smallest_side("image", 256)
    .image_center_crop("image", 224, 224)

    # Accumulate into batches
    .batch(batch_size)

    # Cast to float32 and scale to [0, 1]. We do this in python and we could
    # have done any transformation we could think of.
    .key_transform("image", lambda x: x.astype("float32") / 255)

    # Finally, fetch batches in background threads
    .prefetch(prefetch_size=8, num_threads=8)
)

# dset is a python iterable so one could simply
for sample in dset:
    # access sample["image"] and sample["label"]
    pass
```
