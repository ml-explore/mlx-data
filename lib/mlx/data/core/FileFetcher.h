// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/Array.h"
#include "mlx/data/core/ThreadPool.h"

#include <deque>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace mlx {
namespace data {
namespace core {

class FileFetcherHandle {
 public:
  FileFetcherHandle(int64_t rank) : rank_(rank){};

 private:
  int64_t rank_;
  friend class FileFetcher;
};

// Note that FileFetcher holds a weak reference on a given filename
// If it is not valid anymore, it will be fetched again
class FileFetcher {
 public:
  // In a multi-threaded environment, use numKeptFiles with caution: Make
  // sure it is large enough, to ensure threads won't compete on the files
  // to keep locally.
  FileFetcher(
      int num_prefetch_max = 1,
      int num_prefetch_threads = 1,
      int num_kept_files = 0,
      bool verbose = false);

  void prefetch(const std::vector<std::string>& filenames);

  // Must be called in the destructor of any subclass
  // because prefetch calls the virtual backendFetch()
  // which would then be destroyed before ~FileFetcher()
  void cancel_prefetch();

  std::shared_ptr<FileFetcherHandle> fetch(const std::string& filename) const;

  virtual void backend_fetch(const std::string& filename) const;

  virtual void backend_erase(const std::string& filename) const;

  virtual ~FileFetcher();

 protected:
  void fill_queue_() const;
  std::unique_ptr<ThreadPool> threadPool_;
  mutable std::deque<std::string> prefetchFilenames_;
  mutable std::shared_mutex mutex_;
  int numPrefetchMax_;
  int numKeptFiles_;
  mutable int64_t fileRank_;
  bool verbose_;

  mutable std::unordered_map<std::string, std::future<void>> queuedFiles_;

  mutable std::unordered_map<std::string, std::shared_ptr<FileFetcherHandle>>
      cachedFiles_;
};

} // namespace core
} // namespace data
} // namespace mlx
