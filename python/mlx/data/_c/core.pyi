"""
mlx data core helper classes and functions
"""

from __future__ import annotations

import typing

import numpy

__all__: list[str] = [
    "ArrayType",
    "BPEMerges",
    "BPETokenizer",
    "CharTrie",
    "CharTrieNode",
    "FileFetcher",
    "FileFetcherHandle",
    "GraphInt64",
    "Tokenizer",
    "TokenizerIterator",
    "any",
    "double",
    "float",
    "int32",
    "int64",
    "int8",
    "levenshtein",
    "libs_version",
    "remove",
    "set_state",
    "uint8",
    "uniq",
    "version",
]

class ArrayType:
    """
    Members:

      any

      uint8

      int8

      int32

      int64

      float

      double
    """

    __members__: typing.ClassVar[
        dict[str, ArrayType]
    ]  # value = {'any': <ArrayType.any: 0>, 'uint8': <ArrayType.uint8: 1>, 'int8': <ArrayType.int8: 2>, 'int32': <ArrayType.int32: 3>, 'int64': <ArrayType.int64: 4>, 'float': <ArrayType.float: 5>, 'double': <ArrayType.double: 6>}
    any: typing.ClassVar[ArrayType]  # value = <ArrayType.any: 0>
    double: typing.ClassVar[ArrayType]  # value = <ArrayType.double: 6>
    float: typing.ClassVar[ArrayType]  # value = <ArrayType.float: 5>
    int32: typing.ClassVar[ArrayType]  # value = <ArrayType.int32: 3>
    int64: typing.ClassVar[ArrayType]  # value = <ArrayType.int64: 4>
    int8: typing.ClassVar[ArrayType]  # value = <ArrayType.int8: 2>
    uint8: typing.ClassVar[ArrayType]  # value = <ArrayType.uint8: 1>
    def __eq__(self, other: typing.Any) -> bool: ...
    def __getstate__(self) -> int: ...
    def __hash__(self) -> int: ...
    def __index__(self) -> int: ...
    def __init__(self, value: int) -> None: ...
    def __int__(self) -> int: ...
    def __ne__(self, other: typing.Any) -> bool: ...
    def __repr__(self) -> str: ...
    def __setstate__(self, state: int) -> None: ...
    def __str__(self) -> str: ...
    @property
    def name(self) -> str: ...
    @property
    def value(self) -> int: ...

class BPEMerges:
    """

    A datastructure that holds all possible merges and allows querying
    whether two strings can be merged in O(1) time.

    """

    def __init__(self) -> None: ...
    def add(self, left: str, right: str, token: int) -> None:
        """
        Add two strings as a possible merge that results in ``token``.

        Args:
          left (str): The left side to be merged.
          right (str): The right side to be merged.
          token (int): The resulting token.
        """

    def can_merge(self, left: str, right: str) -> int | None:
        """
        Check if ``left`` and ``right`` can be merged to one token.

        Args:
          left (str): The left side of the possible token.
          right (str): The right side of the possible token.

        Returns:
          The token id is returned or None if ``left`` and ``right``
          couldn't be merged.
        """

class BPETokenizer:
    """

    A tokenizer that uses the BPE algorithm to tokenize strings.

    Args:
      symbol_trie (mlx.data.core.CharTrie): The trie containing the basic
        symbols that all merges start from.
      merges (mlx.data.core.BPEMerges): The datastructure holding the bpe
        merges.

    """

    def __init__(self, symbols: CharTrie, merges: BPEMerges) -> None: ...
    def tokenize(self, input: str) -> list[int]:
        """
        Tokenize the input according to the symbols and merges.

        Args:
          input (str): The input string to be tokenized.
        """

class CharTrie:
    """

    A Trie implementation for characters.

    It enables making a graph of all possible tokenizations and then
    searching for the shortest one.

    """

    def __init__(self) -> None: ...
    def insert(self, token: str | list[str], id: int = -1) -> CharTrieNode:
        """
        Insert a token in the trie making a new token if it doesn't already exist.

        Args:
            token (str or list[char]): The new token to be inserted given
              either as a string or a list of characters.
            id (int, optional): The id to assign to the new token to be
              inserted. If negative then use ``num_keys()`` as default.
              Default: ``-1``.
        """

    def key(self, id: int) -> list[str]:
        """
        Get the ``id``-th token as a list of characters.
        """

    def key_bytes(self, id: int) -> bytes:
        """
        Get the ``id``-th token as bytes.
        """

    def key_string(self, id: int) -> str:
        """
        Get the string that corresponds to the ``id``-th token.
        """

    def num_keys(self) -> int:
        """
        Return how many keys/nodes have been inserted in the Trie.
        """

    def root(self) -> CharTrieNode:
        """
        Get the root node of the trie
        """

    def search(self, token: str | list[str]) -> CharTrieNode:
        """
        Search a the passed string or list of characters in the trie and
        return the node or None if not found.

        To get the id of a token it suffices to do:

        .. code-block:: python

          print(trie.search("hello").id)

        However, if 'hello' is not in the vocabulary then
        :meth:`CharTrie.search` will return None.

        Args:
            token (str or list[char]): The token to be searched given
              either as a string or a list of characters.

        Returns:
          :class:`mlx.data.core.CharTrieNode`: The node corresponding to
          the ``token`` or None.
        """

class CharTrieNode:
    def __repr__(self) -> str: ...
    def accepts(self) -> bool: ...
    @property
    def children(self) -> dict[str, CharTrieNode]: ...
    @property
    def id(self) -> int: ...
    @property
    def uid(self) -> int: ...

class FileFetcher:
    def __init__(
        self,
        num_prefetch_max: int = 1,
        num_prefetch_threads: int = 1,
        num_kept_files: int = 0,
        verbose: bool = False,
    ) -> None: ...
    def cancel_prefetch(self) -> None: ...
    def erase(self, filename: str) -> None:
        """
        Erase the filename from the local cache (if present).

        Args:
          filename (str): A file to erase locally.
        """

    def fetch(self, filename: str) -> FileFetcherHandle:
        """
        Ensures the filename is in the local cache.

        It can either fetch it, return immediately if it was prefetched or
        wait until it is downloaded if it is currently being prefetched.

        Args:
          filename (str): A file to fetch from the remote.
        """

    def prefetch(self, filenames: list[str]) -> None:
        """
        Start prefetching these files.

        ``num_prefetch_max`` files are downloaded with
        ``num_prefetch_threads`` parallelism. When one of the prefetched
        files is accessed by ``fetch`` then more of the prefetch file list is
        downloaded.

        At any given point we keep ``num_kept_files`` in the local cache.

        Args:
          filenames (list[str]): A list of filenames to be prefetched in order.
        """

class FileFetcherHandle:
    pass

class GraphInt64:
    pass

class Tokenizer:
    """

    A Tokenizer that can be used to tokenize arbitrary strings.

    Args:
        trie (mlx.data.core.CharTrie): The trie containing the possible tokens.
        ignore_unk (bool): Whether unknown tokens should be ignored or
            an error should be raised. (default: false)
        trie_key_scores (list[float]): A list containing one score per
            trie node. If left empty each score is assumed equal to 1.
            Tokenize shortest minimizes the sum of these scores over
            the sequence of tokens.

    """

    def __init__(
        self,
        trie: CharTrie,
        ignore_unk: bool = False,
        trie_key_scores: list[float] = [],
    ) -> None:
        """
        Make a tokenizer object that can be used to tokenize arbitrary strings.

        Args:
            trie (mlx.data.core.CharTrie): The trie containing the possible tokens.
            ignore_unk (bool): Whether unknown tokens should be ignored or
                an error should be raised. (default: false)
            trie_key_scores (list[float]): A list containing one score per
                trie node. If left empty each score is assumed equal to 1.
                Tokenize shortest minimizes the sum of these scores over
                the sequence of tokens.
        """

    def tokenize(self, input: str) -> GraphInt64:
        """
        Return the full graph of possible tokenizations.

        Args:
            input (str): The input string to be tokenized.
        """

    def tokenize_rand(self, input: str) -> list[int]:
        """
        Tokenize the input with a valid tokenization chosen randomly from
        the set of valid tokenizations.

        For instance if our set of tokens is {'a', 'aa', 'b'} then the
        string 'aab' can have 2 different tokenizations:

        - 0, 0, 2
        - 1, 2

        :meth:`Tokenizer.tokenize_shortest` will return the second one if no
        ``trie_key_scores`` are provided while
        :meth:`Tokenizer.tokenize_rand` will sample either of the two.

        Args:
            input (str): The input string to be tokenized.
        """

    def tokenize_shortest(self, input: str) -> list[int]:
        """
        Tokenize the input such that the sum of ``trie_key_scores`` is minimized.

        Args:
            input (str): The input string to be tokenized.
        """

class TokenizerIterator:
    def __init__(self, arg0: GraphInt64) -> None: ...
    def __iter__(self) -> TokenizerIterator: ...
    def __next__(self) -> list[int]: ...

@typing.overload
def levenshtein(
    arg0: numpy.ndarray, arg1: numpy.ndarray, arg2: numpy.ndarray, arg3: numpy.ndarray
) -> numpy.ndarray: ...
@typing.overload
def levenshtein(arg0: numpy.ndarray, arg1: numpy.ndarray) -> numpy.ndarray: ...
def libs_version() -> dict[str, str]: ...
def remove(
    arg0: numpy.ndarray, arg1: numpy.ndarray, arg2: int, arg3: float, arg4: float
) -> tuple[numpy.ndarray, numpy.ndarray]: ...
def set_state(seed: int = 1234) -> None: ...
def uniq(
    arg0: numpy.ndarray, arg1: numpy.ndarray, arg2: int, arg3: float
) -> tuple[numpy.ndarray, numpy.ndarray]: ...
def version() -> str: ...

any: ArrayType  # value = <ArrayType.any: 0>
double: ArrayType  # value = <ArrayType.double: 6>
float: ArrayType  # value = <ArrayType.float: 5>
int32: ArrayType  # value = <ArrayType.int32: 3>
int64: ArrayType  # value = <ArrayType.int64: 4>
int8: ArrayType  # value = <ArrayType.int8: 2>
uint8: ArrayType  # value = <ArrayType.uint8: 1>
