# Copyright © 2024 Apple Inc.

from unittest import TestCase

import pytest
import mlx.data as dx


class TestBuffer(TestCase):
    def test__getitem__(self):
        n = 5
        b = dx.buffer_from_vector(list(dict(i=i) for i in range(n)))
        for i in range(n):
            self.assertEqual(b[i]["i"], i)
            i += 1
            self.assertEqual(b[-i], b[n - i])

        with self.assertRaises(IndexError):
            _ = b[n]
        with self.assertRaises(IndexError):
            _ = b[-(n + 1)]

    def test_ordered_prefetch(self):
        """Test that elements are fetched in order."""
        num_threads = 8
        prefetch_size = 16
        n = prefetch_size * 10
        buffer = dx.buffer_from_vector(list(dict(i=i) for i in range(n)))
        stream = buffer.ordered_prefetch(prefetch_size, num_threads)
        for i, e in enumerate(stream):
            self.assertEqual(i, e["i"])

    def test_ordered_prefetch_edge_case(self):
        """Test when the buffer is smaller than dataset size."""
        num_threads = 4
        prefetch_size = 12
        n = int(prefetch_size * 0.5)
        buffer = dx.buffer_from_vector(list(dict(i=i) for i in range(n)))
        stream = buffer.ordered_prefetch(prefetch_size, num_threads)
        for i, e in enumerate(stream):
            self.assertEqual(i, e["i"])

