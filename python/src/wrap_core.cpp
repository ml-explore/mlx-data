// Copyright © 2023 Apple Inc.

#include "wrap.h"

#if MLX_HAS_AWS
#include <pybind11/functional.h>
#include <pybind11/stl/filesystem.h>
#include "mlx/data/core/AWSFileFetcher.h"
#endif

#include "mlx/data/core/BPETokenizer.h"
#include "mlx/data/core/FileFetcher.h"
#include "mlx/data/core/Graph.h"
#include "mlx/data/core/Levenshtein.h"
#include "mlx/data/core/State.h"
#include "mlx/data/core/Tokenizer.h"
#include "mlx/data/core/Trie.h"
#include "mlx/data/core/Utils.h"

#include <cstring>

namespace py = pybind11;

using namespace mlx::data;
using namespace mlx::data::core;

void init_mlx_data_core(py::module& m) {
  m.doc() = "mlx data core helper classes and functions";

  py::enum_<ArrayType>(m, "ArrayType")
      .value("any", ArrayType::Any)
      .value("uint8", ArrayType::UInt8)
      .value("int8", ArrayType::Int8)
      .value("int32", ArrayType::Int32)
      .value("int64", ArrayType::Int64)
      .value("float", ArrayType::Float)
      .value("double", ArrayType::Double)
      .export_values();

  m.def("set_state", &set_state, py::arg("seed") = 1234);

  m.def(
      "uniq", [](py::array& psrc, py::array& psrc_length, int dim, double pad) {
        auto src = mlx::pybind::to_array(psrc);
        auto src_length = mlx::pybind::to_array(psrc_length);
        std::shared_ptr<Array> dst, dst_length;
        std::tie(dst, dst_length) = uniq(src, src_length, dim, pad);
        auto pdst = mlx::pybind::to_py_array(dst);
        auto pdst_length = mlx::pybind::to_py_array(dst_length);
        return std::make_pair(pdst, pdst_length);
      });
  m.def(
      "remove",
      [](py::array& psrc,
         py::array& psrc_length,
         int dim,
         double value,
         double pad) {
        auto src = mlx::pybind::to_array(psrc);
        auto src_length = mlx::pybind::to_array(psrc_length);
        std::shared_ptr<Array> dst, dst_length;
        std::tie(dst, dst_length) = remove(src, src_length, dim, value, pad);
        auto pdst = mlx::pybind::to_py_array(dst);
        auto pdst_length = mlx::pybind::to_py_array(dst_length);
        return std::make_pair(pdst, pdst_length);
      });
  m.def(
      "levenshtein",
      [](py::array& a, py::array& la, py::array& b, py::array& lb) {
        auto ax = mlx::pybind::to_array(a);
        auto bx = mlx::pybind::to_array(b);
        auto lax = mlx::pybind::to_array(la);
        auto lbx = mlx::pybind::to_array(lb);
        return mlx::pybind::to_py_array(levenshtein(ax, lax, bx, lbx));
      });
  m.def("levenshtein", [](py::array& a, py::array& b) {
    auto ax = mlx::pybind::to_array(a);
    auto bx = mlx::pybind::to_array(b);
    auto lax = std::make_shared<Array>(std::vector<int64_t>({ax->shape(0)}));
    auto lbx = std::make_shared<Array>(std::vector<int64_t>({bx->shape(0)}));
    auto res = levenshtein(ax, lax, bx, lbx);
    return mlx::pybind::to_py_array(res);
  });

  py::class_<TrieNode<char>>(m, "CharTrieNode")
      .def("accepts", &TrieNode<char>::accepts)
      .def_readonly("id", &TrieNode<char>::id)
      .def_readonly("uid", &TrieNode<char>::uid)
      .def_readonly("children", &TrieNode<char>::children) // DEBUG: policy?
      .def("__repr__", [](const TrieNode<char>& obj) {
        std::stringstream s;
        s << "TrieNode<char> at " << std::hex << &obj;
        return s.str();
      });

  py::class_<Trie<char>, std::shared_ptr<Trie<char>>>(
      m,
      "CharTrie",
      R"pbcopy(
      A Trie implementation for characters.

      It enables making a graph of all possible tokenizations and then
      searching for the shortest one.
    )pbcopy")
      .def(py::init<>())
      .def(
          "root",
          &Trie<char>::root,
          py::return_value_policy::reference_internal,
          "Get the root node of the trie")
      .def(
          "num_keys",
          &Trie<char>::num_keys,
          "Return how many keys/nodes have been inserted in the Trie.")
      .def(
          "key",
          &Trie<char>::key,
          py::arg("id"),
          "Get the ``id``-th token as a list of characters.")
      .def(
          "key_string",
          [](std::shared_ptr<Trie<char>> trie, int64_t id) {
            return trie->key_string(id);
          },
          py::arg("id"),
          "Get the string that corresponds to the ``id``-th token.")
      .def(
          "key_bytes",
          [](std::shared_ptr<Trie<char>> trie, int64_t id) {
            return py::bytes(trie->key_string(id));
          },
          py::arg("id"),
          "Get the ``id``-th token as bytes.")
      .def(
          "insert",
          [](std::shared_ptr<Trie<char>> trie,
             std::variant<std::string, std::vector<char>> token,
             int64_t id) {
            if (std::holds_alternative<std::string>(token)) {
              return trie->insert(std::get<std::string>(token), id);
            } else {
              return trie->insert(std::get<std::vector<char>>(token), id);
            }
          },
          py::return_value_policy::reference_internal,
          py::arg("token"),
          py::arg("id") = -1,
          R"pbcopy(
            Insert a token in the trie making a new token if it doesn't already exist.

            Args:
                token (str or list[char]): The new token to be inserted given
                  either as a string or a list of characters.
                id (int, optional): The id to assign to the new token to be
                  inserted. If negative then use ``num_keys()`` as default.
                  Default: ``-1``.
          )pbcopy")
      .def(
          "search",
          [](std::shared_ptr<Trie<char>> trie,
             std::variant<std::string, std::vector<char>> token) {
            if (std::holds_alternative<std::string>(token)) {
              return trie->search(std::get<std::string>(token));
            } else {
              return trie->search(std::get<std::vector<char>>(token));
            }
          },
          py::return_value_policy::reference_internal,
          py::arg("token"),
          R"pbcopy(
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
          )pbcopy");

  py::class_<Graph<int64_t>, std::shared_ptr<Graph<int64_t>>>(m, "GraphInt64");

  py::class_<TokenizerIterator, std::shared_ptr<TokenizerIterator>>(
      m, "TokenizerIterator")
      .def(py::init<std::shared_ptr<Graph<int64_t>>>())
      .def(
          "__iter__", [](std::shared_ptr<TokenizerIterator>& it) { return it; })
      .def("__next__", [](std::shared_ptr<TokenizerIterator>& it) {
        auto res = it->next();
        if (res.empty()) {
          throw py::stop_iteration();
        }
        return res;
      });

  py::class_<Tokenizer, std::shared_ptr<Tokenizer>>(
      m,
      "Tokenizer",
      R"pbcopy(
        A Tokenizer that can be used to tokenize arbitrary strings.

        Args:
            trie (mlx.data.core.CharTrie): The trie containing the possible tokens.
            ignore_unk (bool): Whether unknown tokens should be ignored or
                an error should be raised. (default: false)
            trie_key_scores (list[float]): A list containing one score per
                trie node. If left empty each score is assumed equal to 1.
                Tokenize shortest minimizes the sum of these scores over
                the sequence of tokens.
      )pbcopy")
      .def(
          py::init<
              std::shared_ptr<const Trie<char>>,
              bool,
              const std::vector<double>&>(),
          py::arg("trie"),
          py::arg("ignore_unk") = false,
          py::arg("trie_key_scores") = std::vector<double>({}),
          R"pbcopy(
            Make a tokenizer object that can be used to tokenize arbitrary strings.

            Args:
                trie (mlx.data.core.CharTrie): The trie containing the possible tokens.
                ignore_unk (bool): Whether unknown tokens should be ignored or
                    an error should be raised. (default: false)
                trie_key_scores (list[float]): A list containing one score per
                    trie node. If left empty each score is assumed equal to 1.
                    Tokenize shortest minimizes the sum of these scores over
                    the sequence of tokens.
          )pbcopy")
      .def(
          "tokenize_shortest",
          &Tokenizer::tokenize_shortest,
          py::arg("input"),
          R"pbcopy(
            Tokenize the input such that the sum of ``trie_key_scores`` is minimized.

            Args:
                input (str): The input string to be tokenized.
           )pbcopy")
      .def(
          "tokenize_rand",
          &Tokenizer::tokenize_rand,
          py::arg("input"),
          R"pbcopy(
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
           )pbcopy")
      .def(
          "tokenize",
          &Tokenizer::tokenize,
          py::arg("input"),
          R"pbcopy(
            Return the full graph of possible tokenizations.

            Args:
                input (str): The input string to be tokenized.
           )pbcopy");

  py::class_<BPEMerges, std::shared_ptr<BPEMerges>>(
      m,
      "BPEMerges",
      R"pbcopy(
        A datastructure that holds all possible merges and allows querying
        whether two strings can be merged in O(1) time.
      )pbcopy")
      .def(py::init<>())
      .def(
          "add",
          &BPEMerges::add,
          py::arg("left"),
          py::arg("right"),
          py::arg("token"),
          R"pbcopy(
            Add two strings as a possible merge that results in ``token``.

            Args:
              left (str): The left side to be merged.
              right (str): The right side to be merged.
              token (int): The resulting token.
          )pbcopy")
      .def(
          "can_merge",
          [](std::shared_ptr<BPEMerges>& merges,
             const std::string& left,
             const std::string& right) -> std::optional<int64_t> {
            auto [can_merge, token] = merges->can_merge(
                std::string_view(left.data(), left.size()),
                std::string_view(right.data(), right.size()));

            if (!can_merge) {
              return {};
            }

            return token;
          },
          py::arg("left"),
          py::arg("right"),
          R"pbcopy(
            Check if ``left`` and ``right`` can be merged to one token.

            Args:
              left (str): The left side of the possible token.
              right (str): The right side of the possible token.

            Returns:
              The token id is returned or None if ``left`` and ``right``
              couldn't be merged.
          )pbcopy");

  py::class_<BPETokenizer, std::shared_ptr<BPETokenizer>>(
      m,
      "BPETokenizer",
      R"pbcopy(
        A tokenizer that uses the BPE algorithm to tokenize strings.

        Args:
          symbol_trie (mlx.data.core.CharTrie): The trie containing the basic
            symbols that all merges start from.
          merges (mlx.data.core.BPEMerges): The datastructure holding the bpe
            merges.
      )pbcopy")
      .def(
          py::init<
              std::shared_ptr<const Trie<char>>,
              std::shared_ptr<const BPEMerges>>(),
          py::arg("symbols"),
          py::arg("merges"))
      .def(
          "tokenize",
          &BPETokenizer::tokenize,
          py::arg("input"),
          R"pbcopy(
            Tokenize the input according to the symbols and merges.

            Args:
              input (str): The input string to be tokenized.
          )pbcopy");

  py::class_<FileFetcherHandle, std::shared_ptr<FileFetcherHandle>>(
      m, "FileFetcherHandle");

  py::class_<FileFetcher, std::shared_ptr<FileFetcher>>(m, "FileFetcher")
      .def(
          py::init<int, int, int, bool>(),
          py::call_guard<py::gil_scoped_release>(),
          py::arg("num_prefetch_max") = 1,
          py::arg("num_prefetch_threads") = 1,
          py::arg("num_kept_files") = 0,
          py::arg("verbose") = false)
      .def(
          "prefetch",
          &FileFetcher::prefetch,
          py::call_guard<py::gil_scoped_release>(),
          py::arg("filenames"),
          R"pbcopy(
          Start prefetching these files.

          ``num_prefetch_max`` files are downloaded with
          ``num_prefetch_threads`` parallelism. When one of the prefetched
          files is accessed by ``fetch`` then more of the prefetch file list is
          downloaded.

          At any given point we keep ``num_kept_files`` in the local cache.

          Args:
            filenames (list[str]): A list of filenames to be prefetched in order.
        )pbcopy")
      .def(
          "cancel_prefetch",
          &FileFetcher::cancel_prefetch,
          py::call_guard<py::gil_scoped_release>())
      .def(
          "fetch",
          &FileFetcher::fetch,
          py::call_guard<py::gil_scoped_release>(),
          py::arg("filename"),
          R"pbcopy(
          Ensures the filename is in the local cache.

          It can either fetch it, return immediately if it was prefetched or
          wait until it is downloaded if it is currently being prefetched.

          Args:
            filename (str): A file to fetch from the remote.
        )pbcopy")
      .def(
          "erase",
          &FileFetcher::erase,
          py::arg("filename"),
          py::call_guard<py::gil_scoped_release>(),
          R"pbcopy(
          Erase the filename from the local cache (if present).

          Args:
            filename (str): A file to erase locally.
        )pbcopy");

#if MLX_HAS_AWS
  py::class_<AWSFileFetcher, FileFetcher, std::shared_ptr<AWSFileFetcher>>(
      m, "AWSFileFetcher")
      .def(
          py::init([](const std::string& bucket,
                      const std::string& endpoint,
                      const std::string& region,
                      const std::filesystem::path& prefix,
                      const std::filesystem::path& local_prefix,
                      const std::string& ca_bundle,
                      bool virtual_host,
                      bool verify_ssl,
                      int64_t connect_timeout_ms,
                      int64_t num_retry_max,
                      int num_connection_max,
                      int64_t buffer_size,
                      int num_threads,
                      int num_prefetch_max,
                      int num_prefetch_threads,
                      int num_kept_files,
                      const std::string& access_key_id,
                      const std::string& secret_access_key,
                      const std::string& session_token,
                      const std::string& expiration,
                      bool verbose) {
            AWSFileFetcherOptions opt = {
                endpoint,
                region,
                prefix,
                local_prefix,
                ca_bundle,
                virtual_host,
                verify_ssl,
                connect_timeout_ms,
                num_retry_max,
                num_connection_max,
                buffer_size,
                num_threads,
                num_prefetch_max,
                num_prefetch_threads,
                num_kept_files,
                access_key_id,
                secret_access_key,
                session_token,
                expiration,
                verbose};
            return std::make_shared<AWSFileFetcher>(bucket, opt);
          }),
          py::arg("bucket"),
          py::arg("endpoint") = "",
          py::arg("region") = "",
          py::arg("prefix") = "",
          py::arg("local_prefix") = "",
          py::arg("ca_bundle") = "",
          py::arg("virtual_host") = false,
          py::arg("verify_ssl") = true,
          py::arg("connect_timeout_ms") = 1000,
          py::arg("num_retry_max") = 10,
          py::arg("num_connection_max") = 25,
          py::arg("buffer_size") = 100 * 1024 * 1024, // 100 MB
          py::arg("num_threads") = 4,
          py::arg("num_prefetch_max") = 1,
          py::arg("num_prefetch_threads") = 1,
          py::arg("num_kept_files") = 0,
          py::arg("access_key_id") = "",
          py::arg("secret_access_key") = "",
          py::arg("session_token") = "",
          py::arg("expiration") = "",
          py::arg("verbose") = false,
          R"pbcopy(
            Make an AWSFileFetcher to fetch files from S3.

            Args:
              bucket (str): The S3 bucket to use.
              endpoint (str): The endpoint to use.
              region (str): The region to use.
              prefix (str): The remote prefix to use for all files requested. (default: '')
              local_prefix (str): The local cache directory to save the downloaded files in. (default: '')
              ca_bundle (str): The path to a certificate authority file for
                establishing SSL/TLS connections. The environment variable
                AWS_CA_BUNDLE can also be used instead. (default: '')
              virtual_host (bool): Whether to use virtual hosted style urls (ie
                the bucket in the domain part of the url). (default: false)
              verify_ssl (bool): Whether we should verify SSL certificates. (default: true)
              connect_timeout_ms (int): Assume it is a timeout after that many
                milliseconds. (default: 1000)
              num_retry_max (int): How many times should we attempt to fetch a
                file before deciding that we failed. The retry strategy is
                exponential backoff. (default: 10)
              num_connection_max (int): Specifies the maximum number of HTTP
                connections to the server. (default: 25)
              buffer_size (int): Fetch the files in parts of that size. (default: 100MB)
              num_threads (int): How many parts to fetch in parallel for each file. (default: 4)
              num_prefetch_max (int): How many files to prefetch from the prefetch list. (default: 1)
              num_prefetch_threads (int): How many files to prefetch in parallel from the prefetch list. (default: 1)
              num_kept_files (int): How many files to keep in the local cache.
                If 0 we keep everything however if the files are larger than our
                local disk this should be set to a positive number. (default: 0)
              access_key_id (str): Set the AWS access key id to authenticate to
                the remote service. (default: '')
              secret_access_key (str): Set the AWS secret access key id to
                authenticate to the remote service. (default: '')
              session_token (str): Set the AWS session token to authenticate to
                the remote service. (default: '')
              expiration (str): A date string defining the expiration of the
                authentication credentials (default: '')
              verbose (bool): Defines whether the file fetcher should write
                information messages to the standard output. (default: false)
          )pbcopy")
      .def(
          "update_credentials",
          &AWSFileFetcher::update_credentials,
          py::call_guard<py::gil_scoped_release>(),
          py::arg("access_key_id") = "",
          py::arg("secret_access_key") = "",
          py::arg("session_token") = "",
          py::arg("expiration") = "",
          R"pbcopy(
          Update the AWSFileFetcher credentials.

          Args:
              access_key_id (str): Set the AWS access key id to authenticate to
                the remote service. (default: '')
              secret_access_key (str): Set the AWS secret access key id to
                authenticate to the remote service. (default: '')
              session_token (str): Set the AWS session token to authenticate to
                the remote service. (default: '')
              expiration (str): A date string defining the expiration of the
                authentication credentials (default: '')
        )pbcopy")
      .def(
          "update_credentials_with_callback",
          &AWSFileFetcher::update_credentials_with_callback,
          py::arg("callback"),
          py::arg("period") = 0,
          py::call_guard<py::gil_scoped_release>(),
          R"pbcopy(
          Update the AWSFileFetcher credentials with a given callback.

          Args:
              callback (function): a callback which returns 4 strings
                (access_key_id, secret_access_key, session_token, expiration)
              period (int): time (in s) after which credentials will be renewed.
                 If 0, then renew credentials at each client request (default: 0)
        )pbcopy")
      .def(
          "are_credentials_expired",
          &AWSFileFetcher::are_credentials_expired,
          py::call_guard<py::gil_scoped_release>());

  AWSHandler::init();
  py::cpp_function aws_shutdown([](py::handle weakref) {
    AWSHandler::shutdown();
    weakref.dec_ref();
  });
  py::weakref(m.attr("AWSFileFetcher"), aws_shutdown).release();

#endif
}
