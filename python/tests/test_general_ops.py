# Copyright © 2024 Apple Inc.

import unittest

import numpy as np

import mlx.data as dx


class TestGeneralOps(unittest.TestCase):
    def test_slice(self):
        dset = dx.buffer_from_vector([{"a": b"hello"}, {"a": b"world"}])
        sliced_dset = dset.slice("a", 0, 1, 3)
        self.assertEqual(bytes(sliced_dset[0]["a"]), b"el")
        self.assertEqual(bytes(sliced_dset[1]["a"]), b"or")

        dset = dx.buffer_from_vector(
            [
                {"a": np.arange(12).reshape(3, 4)},
                {"a": np.arange(12).reshape(3, 4) + 10},
            ]
        )
        sliced_dset = dset.slice("a", 1, 1, 3)
        self.assertTrue(np.all(sliced_dset[0]["a"] == dset[0]["a"][:, 1:3]))
        self.assertTrue(np.all(sliced_dset[1]["a"] == dset[1]["a"][:, 1:3]))
        sliced_dset = dset.slice("a", 0, 1, 12)
        self.assertTrue(np.all(sliced_dset[0]["a"] == dset[0]["a"][1:, :]))
        self.assertTrue(np.all(sliced_dset[1]["a"] == dset[1]["a"][1:, :]))
        sliced_dset = dset.slice("a", [0, 1], [0, 1], [1, 3])
        self.assertTrue(np.all(sliced_dset[0]["a"] == dset[0]["a"][0:1, 1:3]))
        self.assertTrue(np.all(sliced_dset[1]["a"] == dset[1]["a"][0:1, 1:3]))

        with self.assertRaises(ValueError):
            sliced_dset = dset.slice("a", [0, 1], 2, 3)

    def test_random_slice(self):
        dset = dx.buffer_from_vector([{"a": b"hello"}, {"a": b"world"}])
        sliced_dset = dset.to_stream().repeat(-1).random_slice("a", 0, 3)
        options = [
            set([b"hel", b"ell", b"llo"]),
            set([b"wor", b"orl", b"rld"]),
        ]
        for i, s in zip(range(20), sliced_dset):
            self.assertTrue(bytes(s["a"]) in options[i % 2])


if __name__ == "__main__":
    unittest.main()
