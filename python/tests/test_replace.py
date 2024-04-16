# Copyright © 2024 Apple Inc.

import unittest

import mlx.data as dx


class TestReplace(unittest.TestCase):
    def test_replace(self):
        s = "Hello world".encode()
        dset = dx.buffer_from_vector([dict(text=s)])

        ds = dset.replace("text", "world", "everybody!")
        self.assertEqual(bytes(ds[0]["text"]), b"Hello everybody!")

        ds = dset.replace("text", "l", "b")
        self.assertEqual(bytes(ds[0]["text"]), b"Hebbo worbd")

        ds = dset.replace("text", "l", "b", 2)
        self.assertEqual(bytes(ds[0]["text"]), b"Hebbo world")


if __name__ == "__main__":
    unittest.main()
