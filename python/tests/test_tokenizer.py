# Copyright © 2024 Apple Inc.

import os
from unittest import TestCase

import numpy as np
from mlx.data.tokenizer_helpers import read_trie_from_vocab

from mlx.data.core import Tokenizer

TEST_MERGES = os.path.join(os.path.dirname(__file__), "inputs/merges.txt")
TEST_ATOMS_VOCAB = os.path.join(os.path.dirname(__file__), "inputs/atoms_vocab.txt")


class TestTokenizer(TestCase):
    def test__tokenize_bpe(self):
        # Load merges and ranks for BPE
        bpe_merges = {}
        bpe_ranks = {}
        with open(TEST_MERGES, "r") as fs:
            for rank, line in enumerate(fs.readlines()):
                left, right, res = line.strip().split(" ")
                left = int(left)
                right = int(right)
                res = int(res)
                bpe_merges[(left, right)] = res
                bpe_ranks[(left, right)] = rank
        # Load atoms trie
        vocab_trie = read_trie_from_vocab(TEST_ATOMS_VOCAB)
        # Build tokenizer
        tokenizer = Tokenizer(vocab_trie, False, [], bpe_merges, bpe_ranks)
        # Set up tests
        tests = [
            ("a</w>", [320]),
            ("in</w>", [530]),
            ("a</w>photo</w>of</w>a</w>cat</w>", [320, 1125, 539, 320, 2368]),
            ("a</w> photo</w> of</w> a</w> cat</w>", [320, 1125, 539, 320, 2368]),
            ("a</w> photo</w> of</w> a</w> dog</w>", [320, 1125, 539, 320, 1929]),
            ("a</w>photo</w>of</w>a</w>dog</w>", [320, 1125, 539, 320, 1929]),
        ]
        # Test
        for test_input, expected_output in tests:
            np.array_equal(tokenizer.tokenize_bpe(test_input), expected_output)
