// Copyright © 2023 Apple Inc.

#pragma once

#include <atomic>
#include <chrono>
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

  void update_credentials(
      const std::string& access_key_id = "",
      const std::string& secret_access_key = "",
      const std::string& session_token = "",
      const std::string& expiration = "") const;

  void update_credentials_with_callback(
      std::function<
          std::tuple<std::string, std::string, std::string, std::string>()>
          callback,
      int64_t period = 0);

  bool are_credentials_expired() const;

  virtual ~AWSFileFetcher();

 protected:
  void check_credentials() const;

  std::string bucket_;
  std::filesystem::path prefix_;
  std::filesystem::path local_prefix_;
  std::unique_ptr<Aws::Client::ClientConfiguration> config_;
  bool config_virtual_host_;
  int64_t buffer_size_;
  int num_threads_;
  mutable std::unique_ptr<Aws::Auth::AWSCredentials> credentials_;
  mutable std::unique_ptr<Aws::S3::S3Client> client_;
  mutable std::atomic<bool> dtor_called_;
  mutable std::shared_mutex client_mutex_;

  // credentials periodic update
  std::function<
      std::tuple<std::string, std::string, std::string, std::string>()>
      credentials_callback_;
  int64_t credentials_period_;
  mutable std::chrono::time_point<std::chrono::system_clock>
      credentials_timestamp_;
  mutable std::shared_mutex credentials_mutex_;
};

} // namespace core
} // namespace data
} // namespace mlx
