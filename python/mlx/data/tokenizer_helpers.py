# Copyright © 2023 Apple Inc.

import re
from pathlib import Path

try:
    from sentencepiece import SentencePieceProcessor
except ImportError:
    SentencePieceProcessor = None

from .core import CharTrie


def read_trie_from_spm(spm_file):
    """Read an :class:`mlx.data.core.CharTrie` from a sentencepiece file.

    Reading directly from a model file requires installing sentencepiece,
    however if the vocabulary and the scores are exported the file can be read
    without installing sentencepiece.

    Args:
        spm_file (str): Either a sentencepiece model file or a vocab file
            extracted from a sentencepiece model.

    Returns:
        tuple[:class:`mlx.data.core.CharTrie`, list[float]]: The trie and the
        corresponding weights from the SPM mdoel.
    """

    def iterate_tokens(spm_file):
        if spm_file.endswith(".model"):
            if SentencePieceProcessor is None:
                raise RuntimeError(
                    "sentencepiece must be installed to read directly from a binary model"
                )

            spm_tok = SentencePieceProcessor(spm_file)
            for i in range(spm_tok.vocab_size()):
                yield spm_tok.id_to_piece(i).encode("utf-8"), spm_tok.get_score(i)

        elif spm_file.endswith(".vocab"):
            f = open(spm_file, "rb")
            for line in f:
                line = line.rstrip()
                token, score = line.split(b"\t")
                yield token, float(score)

        else:
            raise ValueError(
                f"Sentencepiece file extenstion must be in [.vocab, .model] but it was {spm_file}"
            )

    def to_special_token(token):
        return b"<0x" + token.hex().encode() + b">"

    sep = "\u2581".encode("utf-8")

    # We parse the model in two passes. First we save the tokens in tmp_tokens
    # and tmp_scores and go back and replace special tokens that already exist
    # to a special token representation. This happens so we can keep the same
    # ids as the original sentencepiece model.
    tokenmap = {}
    tmp_tokens = []
    tmp_scores = []
    max_scores = set()
    for token, score in iterate_tokens(spm_file):
        score = -score

        if re.match(b"^<.*>$", token):
            # Make sure to set the max score for all special tokens
            max_scores.add(len(tmp_scores))

            hex_byte = re.match(b"^<0x(..)>$", token)
            if hex_byte:
                (token,) = hex_byte.groups()
                token = bytes.fromhex(token.decode())

        token = token.replace(sep, b" ")

        # Token already exists so we should choose either the previous one or
        # this one.
        if token in tokenmap:
            existing_token_id = tokenmap[token]
            existing_token_score = tmp_scores[existing_token_id]

            # We should replace that token with our token
            if score < existing_token_score:
                tmp_tokens[existing_token_id] = to_special_token(token)
                max_scores.add(existing_token_id)
                tmp_tokens.append(token)
                tmp_scores.append(score)
                tokenmap[token] = len(tmp_tokens) - 1

            # We should ignore this token
            else:
                tmp_tokens.append(to_special_token(token))
                tmp_scores.append(score)
                max_scores.add(len(tmp_tokens) - 1)

        # Token doesn't exist so add it
        else:
            tmp_tokens.append(token)
            tmp_scores.append(score)
            tokenmap[token] = len(tmp_tokens) - 1

    # Set the max score to duplicates
    max_score = max(tmp_scores) + 1
    for token_id in max_scores:
        tmp_scores[token_id] = max_score

    # Build the trie and the scores
    trie = CharTrie()
    trie_key_scores = tmp_scores
    for token in tmp_tokens:
        if trie.search(token):
            raise RuntimeError(f"Token {token} found twice")
        trie.insert(token)

    return trie, trie_key_scores


def read_trie_from_vocab(vocab_file):
    """Read an :class:`mlx.data.core.CharTrie` from a file with one token per line.

    Args:
        vocab_file (path or file like): The text file containing one token per line.

    Returns:
        :class:`mlx.data.core.CharTrie`: containing the the vocabulary from ``vocab_file``.
    """
    if isinstance(vocab_file, str):
        vocab_file = open(vocab_file, "rb")
    elif isinstance(vocab_file, Path):
        vocab_file = vocab_file.open("rb")

    trie = CharTrie()
    for token in vocab_file:
        token = token.strip()
        if not isinstance(token, bytes):
            token = bytes(token, "utf-8")
        trie.insert(token)

    return trie
