# Copyright © 2024 Apple Inc.

import string
import unittest

from mlx.data.core import BPEMerges, BPETokenizer, CharTrie


class TestBpe(unittest.TestCase):
    def test_bpe(self):
        symbols = CharTrie()
        symbols.insert(" ")
        for s in string.ascii_letters:
            symbols.insert(s)
        n = symbols.num_keys()
        merges = BPEMerges()

        tokenizer = BPETokenizer(symbols, merges)

        self.assertEqual(tokenizer.tokenize("abcd"), [1, 2, 3, 4])

        merges.add("a", "b", n + 1)
        self.assertEqual(tokenizer.tokenize("abcd"), [n + 1, 3, 4])

        merges.add("c", "d", n + 2)
        merges.add("b", "cd", n + 3)
        self.assertEqual(tokenizer.tokenize("abcd"), [n + 1, n + 2])


if __name__ == "__main__":
    unittest.main()
