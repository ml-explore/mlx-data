# Copyright © 2023 Apple Inc.


from ._c import *
from ._c import __version__

# fmt: off
# pybind11 will import numpy, and may do it in a thread.
# in that event, openblas initialization may lead to invalid address read errors.
# importing numpy in the main thread alleviate the issue.
# alternatively, one can set OPENBLAS_NUM_THREADS=1.
import numpy  # isort: skip
del numpy

# pybind11 may import numpy.core.multiarray (eg., for dtype()),
# and may do it in a thread. To prevent any GIL lock issue, we preload
# this subpackage.
import numpy.core.multiarray
del numpy.core.multiarray

from . import tokenizer_helpers
