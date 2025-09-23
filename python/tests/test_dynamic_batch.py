# Copyright © 2024 Apple Inc.

import array
import unittest

import numpy as np

import mlx.data as dx

np.random.seed(42)


def random_sample(idx):
    N = int(np.random.rand() * (1024 - 64) + 64)
    return {"tokens": np.random.rand(N), "length": N, "idx": idx}


def count_padding(sample):
    return (sample["tokens"].shape[-1] - sample["length"]).sum()


class TestDynamicBatch(unittest.TestCase):
    def test_buffer_dynamic_batch_padding(self):
        dset = dx.buffer_from_vector([random_sample(idx) for idx in range(10_000)])
        # Compute the average padding size with naive batching
        naive_padding = sum(count_padding(s) for s in dset.batch(16))
        dynbatch_padding = sum(
            count_padding(s)
            for s in dset.dynamic_batch("tokens", max_data_size=16 * 1024)
        )
        # Count the total valid tokens
        valid_tokens = sum(d["length"] for d in dset)
        simple_pad_ratio = naive_padding / (valid_tokens + naive_padding)
        dynamic_pad_ratio = dynbatch_padding / (valid_tokens + dynbatch_padding)
        self.assertTrue(simple_pad_ratio > 0.43)
        self.assertTrue(dynamic_pad_ratio < 0.004)

    def test_stream_dynamic_batch_padding(self):
        dset = dx.buffer_from_vector([random_sample(idx) for idx in range(10_000)])
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

    def test_buffer_dynamic_batch_indexing(self):
        dset = dx.buffer_from_vector([random_sample(idx) for idx in range(1000)])
        found_indices = np.zeros((1000,))
        for s in dset.dynamic_batch("tokens", max_data_size=16 * 1024):
            found_indices[s["idx"]] = 1
        self.assertTrue(found_indices.sum() == 1000)

    def test_stream_dynamic_batch_indexing(self):
        dset = dx.buffer_from_vector([random_sample(idx) for idx in range(1000)])
        found_indices = np.zeros((1000,))
        for s in dset.to_stream().dynamic_batch(512, "tokens", max_data_size=16 * 1024):
            found_indices[s["idx"]] = 1
        self.assertTrue(found_indices.sum() == 1000)

    def test_stream_dynamic_batch_max_token_size(self):
        dset = dx.buffer_from_vector([random_sample(idx) for idx in range(1000)])
        max_token_size = 0
        min_token_size = 1e10
        for s in dset.to_stream().dynamic_batch(512, "tokens", max_data_size=16 * 1024):
            max_token_size = max(max_token_size, s["tokens"].size)
            min_token_size = min(min_token_size, s["tokens"].size)
        self.assertTrue(max_token_size <= 16 * 1024)
        self.assertFalse(min_token_size >= 15 * 1024)

    def test_stream_dynamic_batch_min_token_size(self):
        dset = dx.buffer_from_vector([random_sample(idx) for idx in range(1000)])
        max_token_size = 0
        min_token_size = 1e10
        for s in dset.to_stream().dynamic_batch(
            512, "tokens", min_data_size=15 * 1024, max_data_size=16 * 1024
        ):
            max_token_size = max(max_token_size, s["tokens"].size)
            min_token_size = min(min_token_size, s["tokens"].size)
        self.assertTrue(max_token_size <= 16 * 1024)
        self.assertTrue(min_token_size >= 15 * 1024)


if __name__ == "__main__":
    unittest.main()
