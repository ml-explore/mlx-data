# Copyright © 2023 Apple Inc.

# -*- coding: utf-8 -*-

import os
import subprocess

# -- Project information -----------------------------------------------------

project = "MLX Data"
copyright = "2023, MLX Contributors"
author = "MLX Contributors"
version = "0.0.2"
release = "0.0.2"

# -- General configuration ---------------------------------------------------

extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.autosummary",
    "sphinx.ext.intersphinx",
    "sphinx.ext.napoleon",
]

autosummary_generate = True

intersphinx_mapping = {
    "https://docs.python.org/3": None,
    "https://numpy.org/doc/stable/": None,
}

templates_path = ["_templates"]
html_static_path = ["_static"]
source_suffix = ".rst"
master_doc = "index"
highlight_language = "python"
pygments_style = "sphinx"

# -- Options for HTML output -------------------------------------------------

html_theme = "sphinx_book_theme"

html_theme_options = {
    "show_toc_level": 2,
    "repository_url": "https://github.com/ml-explore/mlx-data",
    "use_repository_button": True,
    "navigation_with_keys": False,
    "logo": {
        "image_light": "_static/mlx_logo.png",
        "image_dark": "_static/mlx_logo_dark.png",
    },
}

# -- Options for HTMLHelp output ---------------------------------------------

htmlhelp_basename = "mlx_data_doc"
