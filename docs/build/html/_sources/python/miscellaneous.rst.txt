Miscellaneous
==============

.. currentmodule:: mlx.data

FileFetcher
-----------

Several functions in MLX data can make use of a :class:`FileFetcher` object to
fetch files from a remote location. See the `installation instructions
<install.html>`_ to build MLX data with AWS support which adds the
:class:`core.AWSFileFetcher` described below.

.. autosummary::
   :toctree: _autosummary
   :recursive:

    core.AWSFileFetcher.__init__
    core.AWSFileFetcher.fetch
    core.AWSFileFetcher.prefetch

A :class:`FileFetcher` can also be used standalone in your scripts to
efficiently fetch remote content in background threads.

.. code-block:: python

    from pathlib import Path
    from mlx.data.core import AWSFileFetcher

    LOCAL_CACHE = Path("/path/to/local/cache")

    ff = AWSFileFetcher(
        "my-cool-bucket",
        endpoint="https://my.endpoint.com/"
        local_prefix=LOCAL_CACHE,
        num_kept_files=100,
    )

    # When fetch returns my/remote/path/foo.npy will be in LOCAL_CACHE
    ff.fetch("my/remote/path/foo.npy")
    assert (LOCAL_CACHE / "my/remote/path/foo.npy").is_file()

    # We can prefetch in the background
    ff.prefetch(["foo_1.npy", "foo_2.npy"])
    ff.fetch("foo_1.npy")
    # process foo_1 while foo_2 downloads in the background
