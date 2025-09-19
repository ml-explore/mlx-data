# Copyright © 2024 Apple Inc.

import array
import unittest

import numpy as np

import mlx.data as dx

np.random.seed(42)


def random_sample():
    N = int(np.random.rand() * (1024 - 64) + 64)
    return {"tokens": np.random.rand(N), "length": N}


def count_padding(sample):
    return (sample["tokens"].shape[-1] - sample["length"]).sum()


class TestDynamicBatch(unittest.TestCase):
    def test_stream_dynamic_batch(self):
        dset = dx.buffer_from_vector([random_sample() for _ in range(10_000)])
        # Compute the average padding size with naive batching
        naive_padding = sum(count_padding(s) for s in dset.to_stream().batch(16))
        dynbatch_padding = sum(
            count_padding(s)
            for s in dset.to_stream().dynamic_batch(
                500, "tokens", max_data_size=16 * 1024
            )
        )
        # Count the total valid tokens
        valid_tokens = sum(d["length"] for d in dset)
        simple_pad_ratio = naive_padding / (valid_tokens + naive_padding)
        dynamic_pad_ratio = dynbatch_padding / (valid_tokens + dynbatch_padding)
        self.assertTrue(simple_pad_ratio > 0.43)
        self.assertTrue(dynamic_pad_ratio < 0.06)


if __name__ == "__main__":
    unittest.main()
