# Copyright © 2024 Apple Inc.

import unittest

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


if __name__ == "__main__":
    unittest.main()
