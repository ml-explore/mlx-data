#include "mlx/data/op/ReadFromTAR.h"

namespace mlx {
namespace data {
namespace op {
ReadFromTAR::ReadFromTAR(
    const std::string& tarkey,
    const std::string& ikey,
    const std::string& okey,
    const std::filesystem::path& prefix,
    const std::filesystem::path& tar_prefix,
    bool from_key,
    std::shared_ptr<core::FileFetcher> fetcher,
    bool nested,
    int num_threads)
    : tarkey_(tarkey),
      ikey_(ikey),
      okey_(okey),
      prefix_(prefix),
      tarPrefix_(tar_prefix),
      fromKey_(from_key),
      fetcher_(fetcher),
      nested_(nested),
      numThreads_(num_threads) {
  if (!from_key) {
    // load the tar index
    get_tar_reader_(tarkey);
  }
}
std::pair<
    std::shared_ptr<core::TARReader>,
    std::shared_ptr<core::FileFetcherHandle>>
ReadFromTAR::get_tar_reader_(const std::string& key) const {
  // make sure tar file is actually on disk
  std::shared_ptr<core::FileFetcherHandle> handle;
  if (fetcher_) {
    handle = fetcher_->fetch(key);
  }
  {
    std::shared_lock slock(mutex_);
    auto it = tars_.find(key);
    if (it != tars_.end()) {
      return std::make_pair(it->second, handle);
    }
  }
  {
    std::unique_lock ulock(mutex_);
    auto key_path = tarPrefix_ / key;
    auto tar = std::make_shared<core::TARReader>(
        key_path.string(), nested_, numThreads_);
    tars_[key] = tar;
    return std::make_pair(tar, handle);
  }
}
Sample ReadFromTAR::apply(const Sample& sample) const {
  std::string tarfilename;
  if (fromKey_) {
    auto tarfilename_array =
        sample::check_key(sample, tarkey_, ArrayType::Int8);
    tarfilename = std::string(
        reinterpret_cast<char*>(tarfilename_array->data()),
        tarfilename_array->size());
  } else {
    tarfilename = tarkey_;
  }
  auto tar = get_tar_reader_(tarfilename);
  std::shared_ptr<Array> input_array;
  input_array = sample::check_key(sample, ikey_, ArrayType::Int8);
  std::string filename(
      reinterpret_cast<char*>(input_array->data()), input_array->size());
  auto filepath = prefix_ / filename;
  auto output_array = tar.first->get(filepath.string());
  auto res = sample;
  res[okey_] = output_array;
  return res;
}
} // namespace op
} // namespace data
} // namespace mlx
