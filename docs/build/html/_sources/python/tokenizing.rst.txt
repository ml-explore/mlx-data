Tokenizing with MLX data
========================

.. currentmodule:: mlx.data

MLX data allows sample transformations with the full flexibility of python which
means that you could use any python tokenizer in a
:meth:`Buffer.key_transform`. However, this is likely to be subject to the GIL
which means that effectively only one sample can be tokenized at a time.

A better choice is to use an :class:`mlx.data.core.CharTrie` to tokenize your
data, taking full advatage of a multicore system. You can build the trie
yourself or use one of the provided helpers to build a trie from an SentencePiece model
or a plain text vocabulary file.

.. code-block:: python

    from mlx.data.core import CharTrie, Tokenizer

    # We can build a trie ourselves
    trie = CharTrie()
    for t in b"a quick brown fox jumped over the lazy dog".split():
        trie.insert(t)
    trie.insert(b" ")

    tokenizer = Tokenizer(trie)
    print(tokenizer.tokenize_shortest(b"a quick brown fox jumped over the lazy dog"))
    # [0, 9, 1, 9, 2, 9, 3, 9, 4, 9, 5, 9, 6, 9, 7, 9, 8]

    # We can also add all the letters in the trie and then tokenize anything we want
    import string
    for l in string.ascii_letters:
        trie.insert(bytes(l, "utf-8"))

    print(tokenizer.tokenize_shortest(b"This is a quick example"))
    # [54, 16, 17, 27, 9, 17, 27, 9, 0, 9, 1, 9, 13, 32, 0, 21, 24, 20, 13]

    # The more useful option is to read the trie from a file, for instance an spm model
    from mlx.data.tokenizer_helpers import read_trie_from_spm

    trie = read_trie_from_spm("path/to/spm/model")
    tokenizer = Tokenizer(trie)
    tokenizer.tokenize_shortest(b"This is some more text to tokenize")


.. autosummary::
   :toctree: _autosummary
   :template: data_core_modules.rst
   :recursive:

   core.Tokenizer
   core.CharTrie
   tokenizer_helpers.read_trie_from_vocab
   tokenizer_helpers.read_trie_from_spm
