# -*- coding: utf-8 -*-

import os
import subprocess

# -- Project information -----------------------------------------------------

project = "MLX Data"
copyright = "2023, MLX Contributors"
author = "MLX Contributors"
version = "0.0.0"
release = "0.0.0"

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
source_suffix = ".rst"
master_doc = "index"
highlight_language = "python"
pygments_style = "sphinx"

# -- Options for HTML output -------------------------------------------------

html_theme = "sphinx_rtd_theme"

# -- Options for HTMLHelp output ---------------------------------------------

htmlhelp_basename = "mlx_data_doc"
