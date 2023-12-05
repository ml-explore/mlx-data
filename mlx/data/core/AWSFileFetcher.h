// Copyright © 2023 Apple Inc.

#pragma once

#include <atomic>
#include <filesystem>

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/s3/S3Client.h>

#include "mlx/data/core/FileFetcher.h"

namespace mlx {
namespace data {
namespace core {

class AWSHandler {
 public:
  static void init();
  static void shutdown();

 private:
  static Aws::SDKOptions options_;
};

struct AWSFileFetcherOptions {
  std::string endpoint = "";
  std::string region = "us-east-1";
  std::filesystem::path prefix = "";
  std::filesystem::path local_prefix = "";
  std::string ca_bundle = "";
  bool virtual_host = false;
  bool verify_ssl = true;
  int64_t connect_timeout_ms = 1000;
  int64_t num_retry_max = 10;
  int num_connection_max = 25;
  int64_t buffer_size = 100 * 1024 * 1024; // 100MB
  int num_threads = 4;
  int num_prefetch_max = 1;
  int num_prefetch_threads = 1;
  int64_t num_kept_files = 0;
  std::string access_key_id = "";
  std::string secret_access_key = "";
  std::string session_token = "";
  std::string expiration = "";
  bool verbose = false;
};

class AWSFileFetcher : public FileFetcher {
 public:
  AWSFileFetcher(
      const std::string& bucket,
      const AWSFileFetcherOptions& opt = AWSFileFetcherOptions());

  int64_t get_size(const std::string& filename) const;

  virtual void backend_fetch(const std::string& filename) const override;
  virtual void backend_erase(const std::string& filename) const override;

  bool are_credentials_expired() const;

  virtual ~AWSFileFetcher();

 protected:
  std::string bucket_;
  std::filesystem::path prefix_;
  std::filesystem::path local_prefix_;
  std::unique_ptr<Aws::Client::ClientConfiguration> config_;
  std::unique_ptr<Aws::Auth::AWSCredentials> credentials_;
  std::unique_ptr<Aws::S3::S3Client> client_;
  int64_t buffer_size_;
  int num_threads_;
  mutable std::atomic<bool> dtor_called_;
};

} // namespace core
} // namespace data
} // namespace mlx
