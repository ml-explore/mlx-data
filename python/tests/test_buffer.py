# Copyright © 2024 Apple Inc.

import array
import unittest

import numpy as np

import mlx.data as dx


class TestBuffer(unittest.TestCase):
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

    def test_passing_python_objects(self):
        with self.assertRaises(ValueError):
            b = dx.buffer_from_vector([{"a": "hello"}])
        with self.assertRaises(ValueError):
            b = dx.buffer_from_vector([{"a": object()}])
        with self.assertRaises(ValueError):
            b = dx.buffer_from_vector([{"a": list()}])

        x = array.array("f")
        x.append(10)
        x.append(-2.5)
        y = np.random.randn(10)
        b = dx.buffer_from_vector(
            [
                {
                    "a": 1,
                    "b": 1.2,
                    "c": b"Hello world",
                    "d": y,
                    "e": x,
                }
            ]
        )
        self.assertEqual(-2.5, b[0]["e"][1])
        self.assertEqual(1, b[0]["a"])
        self.assertTrue(np.all(y == b[0]["d"]))
        self.assertTrue(np.all(x == b[0]["e"]))

        # Check that we take np arrays without a copy
        y[0] = 0
        self.assertTrue(np.all(y == b[0]["d"]))

        # and buffers via copy
        x[0] = 0
        self.assertEqual(10, b[0]["e"][0])


if __name__ == "__main__":
    unittest.main()
