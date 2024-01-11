# Copyright © 2023 Apple Inc.


from ._c import *
from ._c import __version__

# fmt: off
# Passing strings from python to be casted by pybind11 crashes on some builds
# if we don't import numpy.
#
# TODO: Evaluate if it is due to improper linking and fix correctly.
import numpy  # isort: skip
del numpy
