// Copyright © 2023 Apple Inc.

#include "mlx/data/core/FileFetcher.h"

#include <algorithm>
#include <iostream>

namespace mlx {
namespace data {
namespace core {

FileFetcher::FileFetcher(
    int num_prefetch_max,
    int num_prefetch_threads,
    int num_kept_files,
    bool verbose)
    : threadPool_(std::make_unique<ThreadPool>(num_prefetch_threads)),
      numPrefetchMax_(num_prefetch_max),
      numKeptFiles_(num_kept_files),
      fileRank_(0),
      verbose_(verbose) {}

void FileFetcher::fill_queue_() const {
  while ((prefetchFilenames_.size()) > 0 &&
         ((numPrefetchMax_ < 0) || (queuedFiles_.size() < numPrefetchMax_))) {
    auto filename = prefetchFilenames_.front();
    prefetchFilenames_.pop_front();
    auto it = cachedFiles_.find(filename);
    auto qit = queuedFiles_.find(filename);
    if (it == cachedFiles_.end() && qit == queuedFiles_.end()) {
      if (verbose_) {
        std::cout << "FileFetcher (" << std::hex << this << std::dec << ") : \""
                  << filename << "\" queued for background prefetch"
                  << std::endl;
      }
      queuedFiles_.emplace(
          std::make_pair(filename, threadPool_->enqueue([this, filename]() {
            this->backend_fetch(filename);
          })));
    } else {
      if (verbose_) {
        std::string status = ((it != cachedFiles_.end()) ? "cached" : "queued");
        std::cout << "FileFetcher (" << std::hex << this << std::dec << ") : \""
                  << filename << "\" not prefetched as it is already " << status
                  << std::endl;
      }
    }
  }
}

void FileFetcher::prefetch(const std::vector<std::string>& filenames) {
  std::unique_lock ulock(mutex_);
  prefetchFilenames_.insert(
      prefetchFilenames_.end(), filenames.begin(), filenames.end());
  fill_queue_();
}

std::shared_ptr<FileFetcherHandle> FileFetcher::fetch(
    const std::string& filename) const {
  // cached?
  {
    std::shared_lock slock(mutex_);
    auto it = cachedFiles_.find(filename);
    if (it != cachedFiles_.end()) {
      auto handle = it->second;
      // Note: if numKeptFiles == 0, fileRank_ stays at 0
      if (handle->rank_ == fileRank_) {
        return handle;
      }
    }
  }
  {
    std::unique_lock ulock(mutex_);
    // cached?
    auto it = cachedFiles_.find(filename);
    if (it != cachedFiles_.end()) {
      auto handle = it->second;
      if (handle->rank_ == fileRank_) {
        return handle;
      } else {
        handle->rank_ = ++fileRank_;
        return handle;
      }
    }
    // queued?
    auto qit = queuedFiles_.find(filename);
    if (qit == queuedFiles_.end()) {
      backend_fetch(filename);
      if (verbose_) {
        std::cout << "FileFetcher (" << std::hex << this << std::dec
                  << ") : fetching \"" << filename
                  << "\" (not queued, nor cached yet)" << std::endl;
      }
    } else {
      if (verbose_) {
        std::cout << "FileFetcher (" << std::hex << this << std::dec
                  << ") : fetching \"" << filename << "\" (queued, waiting)"
                  << std::endl;
      }
      if (!qit->second.valid()) {
        throw std::runtime_error(
            "FileFetcher: invalid future (internal error, please report)");
      }
      qit->second.get();
      queuedFiles_.erase(qit);
      fill_queue_();
    }
    auto handle = std::make_shared<FileFetcherHandle>(
        ((numKeptFiles_ <= 0) ? 0 : ++fileRank_));
    cachedFiles_[filename] = handle;
    if (numKeptFiles_ > 0) {
      while (cachedFiles_.size() > numKeptFiles_) {
        std::string min_key;
        int64_t min_rank = fileRank_;
        for (auto& e : cachedFiles_) {
          // Note: use_count cannot go lower than 1
          if ((e.second.use_count() == 1) && (e.second->rank_ < min_rank)) {
            min_key = e.first;
            min_rank = e.second->rank_;
          }
        }
        if (min_key.empty()) {
          break;
        } else {
          cachedFiles_.erase(min_key);
          backend_erase(min_key);
        }
      }
    }
    return handle;
  }
}

void FileFetcher::backend_fetch(const std::string& filename) const {}
void FileFetcher::backend_erase(const std::string& filename) const {}

void FileFetcher::cancel_prefetch() {
  prefetchFilenames_.clear();
  for (auto& future : queuedFiles_) {
    if (!future.second.valid()) {
      std::cout << "FileFetcher: invalid future (cancelPrefetch)" << std::endl;
    }
    future.second.get();
  }
  queuedFiles_.clear();
}

FileFetcher::~FileFetcher() {
  cancel_prefetch();
}

} // namespace core
} // namespace data
} // namespace mlx
