# Copyright © 2024 Apple Inc.

import math
import re
from pathlib import Path

try:
    from sentencepiece import SentencePieceProcessor
except ImportError:
    SentencePieceProcessor = None

from .core import BPEMerges, CharTrie


def _iterate_spm_tokens(spm_file):
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


def read_trie_from_spm(spm_file):
    """Read an :class:`mlx.data.core.CharTrie` from a sentencepiece file.

    Reading directly from a model file requires installing sentencepiece,
    however if the vocabulary and the scores are exported the file can be read
    without installing sentencepiece.

    .. note::

        Sentencepiece models are almost always BPE models with scores being the
        associated log likelihood of from a unigram language model. Using the
        :class:`mlx.data.core.CharTrie` and the loaded scores will provide the
        shortest possible tokenization with the highest possible log likelihood
        but it can be slightly different than the BPE one.

        Use :func:`read_bpe_from_spm` to load the model to be used with a
        :class:`mlx.data.core.BPETokenizer`.

    Args:
        spm_file (str): Either a sentencepiece model file or a vocab file
            extracted from a sentencepiece model.

    Returns:
        tuple[:class:`mlx.data.core.CharTrie`, list[float]]: The trie and the
        corresponding weights from the SPM mdoel.
    """

    def to_special_token(token):
        return b"<0x" + token.hex().encode() + b">"

    # We parse the model in two passes. First we save the tokens in tmp_tokens
    # and go back and replace special tokens that already exist or tokens that
    # have a better score to a special token representation. This happens so we
    # can keep the same ids as the original sentencepiece model.
    tokenmap = {}
    tmp_tokens = []
    trie_key_scores = []
    for token, score in _iterate_spm_tokens(spm_file):
        if re.match(b"^<.*>$", token):
            hex_byte = re.match(b"^<0x(..)>$", token)
            if hex_byte:
                (token,) = hex_byte.groups()
                token = bytes.fromhex(token.decode())

        # Token already exists so we should choose either the previous one or
        # this one.
        if token in tokenmap:
            existing_token_id = tokenmap[token]
            existing_token_score = trie_key_scores[existing_token_id]

            # We should replace that token with our token
            if score < existing_token_score:
                tmp_tokens[existing_token_id] = to_special_token(token)
                tmp_tokens.append(token)
                trie_key_scores.append(score)
                tokenmap[token] = len(tmp_tokens) - 1

            # We should ignore this token
            else:
                tmp_tokens.append(to_special_token(token))
                trie_key_scores.append(score)

        # Token doesn't exist so add it
        else:
            tmp_tokens.append(token)
            trie_key_scores.append(score)
            tokenmap[token] = len(tmp_tokens) - 1

    # SPM is a BPE tokenizer so it doesn't exactly work like the MLX tokenizer.
    # Favoring the shortest sequence and taking into account the scores at the
    # same time yields the closest tokenization.
    min_score = min(trie_key_scores)
    for i in range(len(trie_key_scores)):
        trie_key_scores[i] = -min_score - trie_key_scores[i]

    # Build the trie
    trie = CharTrie()
    for token in tmp_tokens:
        if trie.search(token):
            raise RuntimeError(f"Token {token} found twice")
        trie.insert(token)

    return trie, trie_key_scores


def read_bpe_from_spm(spm_file):
    """Read a sentencepiece file and decompose it to a symbol trie and BPE
    merges for use with :class:`mlx.data.core.BPETokenizer`.

    Because it isn't straightforward to extract the merges from the SPM file,
    we create a trie of basic symbols by considering all single unicode
    character tokens as basic symbols as well as any special tokens provided.

    To extract the merges we run the BPE algorithm on the tokens in order of
    probability as suggested in https://github.com/openai/tiktoken/issues/60
    for exporting an SPM model to huggingface tokenizers.

    Args:
        spm_file (str): Either a sentencepiece model file or a vocab file
            extracted from a sentencepiece model.

    Returns:
        tuple[:class:`mlx.data.core.CharTrie`, :class:`mlx.data.core.BPEMerges`]: The
        trie and the corresponding BPE merges from the SPM mdoel.
    """
    symbols = []
    merged = []
    tokenmap = {}
    for token_id, (token, score) in enumerate(_iterate_spm_tokens(spm_file)):
        if re.match(b"^<.*>$", token):
            hex_byte = re.match(b"^<0x(..)>$", token)
            if hex_byte:
                (token,) = hex_byte.groups()
                token = bytes.fromhex(token.decode())

        if len(token) == 1 or score == 0 or len(token.decode(errors="ignore")) == 1:
            symbols.append(token)
        else:
            merged.append(token)

        tokenmap[token] = token_id

    trie = CharTrie()
    for s in symbols:
        trie.insert(s, tokenmap[s])

    merges = BPEMerges()

    def bpe(tokenmap, token, max_rank):
        parts = list(token)
        while True:
            min_idx = None
            min_rank = None
            for i, pair in enumerate(zip(parts[:-1], parts[1:])):
                rank = tokenmap.get((pair[0] + pair[1]).encode())
                if rank is not None and (min_rank is None or rank < min_rank):
                    min_idx = i
                    min_rank = rank
            if min_rank is None or (max_rank is not None and min_rank >= max_rank):
                break
            assert min_idx is not None
            parts = (
                parts[:min_idx]
                + [parts[min_idx] + parts[min_idx + 1]]
                + parts[min_idx + 2 :]
            )
        return parts

    for t in merged:
        left, right = bpe(tokenmap, t.decode(), tokenmap[t])
        merges.add(left.encode(), right.encode(), tokenmap[t])

    return trie, merges


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
