// Copyright © 2023 Apple Inc.

#include <aws/core/client/DefaultRetryStrategy.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>

#include <cstdlib>
#include <fstream>
#include <iostream>

#include "mlx/data/core/AWSFileFetcher.h"
#include "mlx/data/core/ThreadPool.h"

namespace mlx {
namespace data {
namespace core {

Aws::SDKOptions AWSHandler::options_;

void AWSHandler::init() {
  auto aws_log_level_env = std::getenv("AWS_LOG_LEVEL");
  auto aws_log_level =
      std::string((aws_log_level_env ? aws_log_level_env : ""));
  if (!aws_log_level.empty()) {
    if (aws_log_level == "trace") {
      options_.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
    } else if (aws_log_level == "info") {
      options_.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Info;
    } else if (aws_log_level == "fatal") {
      options_.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Fatal;
    } else if (aws_log_level == "off") {
      options_.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Off;
    } else {
      throw std::runtime_error(
          "AWSHandler: invalid AWS_LOG_LEVEL (expected trace/info/fatal/off)");
    }
  }
  Aws::InitAPI(options_);
}
void AWSHandler::shutdown() {
  Aws::ShutdownAPI(options_);
}

AWSFileFetcher::AWSFileFetcher(
    const std::string& bucket,
    const AWSFileFetcherOptions& opt)
    : FileFetcher(
          opt.num_prefetch_max,
          opt.num_prefetch_threads,
          opt.num_kept_files,
          opt.verbose),
      bucket_(bucket),
      prefix_(opt.prefix),
      local_prefix_(opt.local_prefix),
      buffer_size_(opt.buffer_size),
      num_threads_(opt.num_threads),
      dtor_called_(false) {
  config_ = std::make_unique<Aws::Client::ClientConfiguration>();
  config_->endpointOverride = opt.endpoint;
  config_->verifySSL = opt.verify_ssl;
  config_->connectTimeoutMs = opt.connect_timeout_ms;
  config_->retryStrategy =
      std::make_shared<Aws::Client::DefaultRetryStrategy>(opt.num_retry_max);
  config_->maxConnections = opt.num_connection_max;
  config_->region = opt.region;

  // C++ SDK does not check AWS_CA_BUNDLE, so we do it ourselves
  if (opt.ca_bundle.empty()) {
    auto aws_ca_bundle_env = std::getenv("AWS_CA_BUNDLE");
    if (aws_ca_bundle_env) {
      config_->caFile = std::string(aws_ca_bundle_env);
    }
  } else {
    config_->caFile = opt.ca_bundle;
  }
  config_virtual_host_ = opt.virtual_host;

  update_credentials(
      opt.access_key_id,
      opt.secret_access_key,
      opt.session_token,
      opt.expiration);
}

void AWSFileFetcher::update_credentials_(
    const std::string& access_key_id,
    const std::string& secret_access_key,
    const std::string& session_token,
    const std::string& expiration) const {
  if (verbose_ && client_) {
    std::cout << "AWSFileFetcher (" << std::hex << this << std::dec
              << ") : updating credentials" << std::endl;
  }

  credentials_ = nullptr;

  if (access_key_id.empty() && secret_access_key.empty() &&
      session_token.empty() && expiration.empty()) {
    if (config_virtual_host_) {
      client_ = std::make_unique<Aws::S3::S3Client>(*config_);
    } else {
      client_ = std::make_unique<Aws::S3::S3Client>(
          *config_,
          Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never,
          /* use virtual host */ false);
    }
  } else {
    Aws::Utils::DateTime aws_expiration(
        expiration, Aws::Utils::DateFormat::AutoDetect);
    credentials_ = std::make_unique<Aws::Auth::AWSCredentials>(
        access_key_id, secret_access_key, session_token, aws_expiration);
    if (config_virtual_host_) {
      client_ = std::make_unique<Aws::S3::S3Client>(
          *credentials_,
          Aws::MakeShared<Aws::S3::S3EndpointProvider>("MLX"),
          *config_);
    } else {
      client_ = std::make_unique<Aws::S3::S3Client>(
          *credentials_,
          *config_,
          Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never,
          /* use virtual host */ false);
    }
  }
  credentials_timestamp_ = std::chrono::system_clock::now();
}

void AWSFileFetcher::update_credentials(
    const std::string& access_key_id,
    const std::string& secret_access_key,
    const std::string& session_token,
    const std::string& expiration) const {
  std::unique_lock ulock(client_mutex_);
  update_credentials_(
      access_key_id, secret_access_key, session_token, expiration);
}

void AWSFileFetcher::update_credentials_with_callback(
    std::function<
        std::tuple<std::string, std::string, std::string, std::string>()>
        callback,
    int64_t period) {
  credentials_callback_ = callback;
  credentials_period_ = period;
}

void AWSFileFetcher::check_credentials_() const {
  auto is_credentials_outdated = [this]() {
    if (credentials_callback_ && !credentials_) {
      return true;
    } else if (credentials_callback_ && credentials_period_ >= 0) {
      auto now = std::chrono::system_clock::now();
      auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
          now - credentials_timestamp_);
      std::cout << "ELAPSED TIME: " << elapsed.count() << std::endl;
      if (elapsed.count() >= credentials_period_) {
        return true;
      }
    }
    return false;
  };

  bool renew_credentials = false;
  {
    std::shared_lock slock(client_mutex_);
    if ((credentials_ && credentials_->IsExpired()) ||
        is_credentials_outdated()) {
      renew_credentials = true;
    }
  }
  if (renew_credentials) {
    if (!credentials_callback_) {
      throw std::runtime_error("AWSFileFetcher: credentials are expired");
    }
    std::cout << "lock and call callback" << std::endl;
    std::unique_lock ulock(client_mutex_);
    // check if someone updated credentials in the meantime
    if (is_credentials_outdated()) {
      auto [access_key_id, secret_access_key, session_token, expiration] =
          credentials_callback_();
      std::cout << "credentials: " << access_key_id << " | "
                << secret_access_key << " | " << session_token << " | "
                << expiration << std::endl;
      update_credentials_(
          access_key_id, secret_access_key, session_token, expiration);
    } else {
      std::cout << "SOMEONE UPDATED THE CREDENTIALS IN MY BACK!" << std::endl;
    }
  }
}

bool AWSFileFetcher::are_credentials_expired() const {
  if (credentials_) {
    return credentials_->IsExpired();
  } else {
    return false;
  }
}

int64_t AWSFileFetcher::get_size(const std::string& filename) const {
  std::cout << "GET_SIZE" << std::endl;
  check_credentials_();

  std::shared_lock slock(client_mutex_);

  Aws::S3::Model::HeadObjectRequest request;
  auto remote_file_path = (prefix_ / filename);
  request.SetBucket(bucket_);
  request.SetKey(remote_file_path.string());

  Aws::S3::Model::HeadObjectOutcome outcome = client_->HeadObject(request);

  if (outcome.IsSuccess() == true) {
    return (int64_t)outcome.GetResult().GetContentLength();
  } else {
    const Aws::S3::S3Error& err = outcome.GetError();
    throw std::runtime_error(
        "AWSFileFetcher: unable to fetch <s3://" + bucket_ + "/" +
        remote_file_path.string() + "> header: " + err.GetExceptionName() +
        " : " + err.GetMessage());
  }
}

void AWSFileFetcher::backend_fetch(const std::string& filename) const {
  // Do not fetch a file already present on disk
  auto remoteFilePath = (prefix_ / filename);
  auto localFilePath = (local_prefix_ / filename);
  if (std::filesystem::exists(localFilePath)) {
    if (verbose_) {
      std::cout << "AWSFileFetcher (" << std::hex << this << std::dec
                << ") : file s3://" << bucket_ << "/" << remoteFilePath
                << " already exists in " << localFilePath << std::endl;
    }
    return;
  }

  auto size = get_size(filename);
  auto numPart = size / buffer_size_;
  if (size % buffer_size_) {
    numPart++;
  }
  if (verbose_) {
    std::cout << "AWSFileFetcher (" << std::hex << this << std::dec
              << ") : fetching s3://" << bucket_ << "/" << remoteFilePath
              << " (" << size << " bytes) into " << localFilePath << std::endl;
  }

  // Note: the threads might fetch data faster than what
  // the disk can write, in which case memory might blow up
  // for large files. Could have gone with another approach,
  // but here one can control number of threads.
  std::cout << "FETCH" << std::endl;
  std::deque<std::future<Aws::S3::Model::GetObjectOutcome>> parts;
  ThreadPool threadPool(num_threads_);
  {
    for (int64_t part = 0; part < numPart; part++) {
      parts.emplace_back(
          threadPool.enqueue([part, size, this, remoteFilePath]() {
            if (dtor_called_.load()) {
              return Aws::S3::Model::GetObjectOutcome();
            }
            std::cout << "thread with part " << part << " check credentials "
                      << std::endl;
            check_credentials_();
            std::cout << "thread with part " << part
                      << " check credentials done" << std::endl;

            {
              std::shared_lock slock(client_mutex_);
              auto beg = part * buffer_size_;
              auto end = (part + 1) * buffer_size_;
              if (end > size) {
                end = size;
              }
              end--; // inclusive;
              std::stringstream range;
              range << "bytes=" << beg << '-' << end;
              Aws::S3::Model::GetObjectRequest request;
              request.SetBucket(bucket_);
              request.SetKey(remoteFilePath.string());
              request.SetRange(range.str());
              return client_->GetObject(request);
            }
          }));
    }
  }

  auto localDir = localFilePath.parent_path();
  if (!localDir.empty() && !std::filesystem::exists(localDir)) {
    if (verbose_) {
      std::cout << "AWSFileFetcher (" << std::hex << this << std::dec
                << ") : creating directory " << localDir << std::endl;
    }
    std::filesystem::create_directories(localDir);
  }
  auto localFilePathTmp = localFilePath;
  localFilePathTmp += ".download";
  std::ofstream f(
      localFilePathTmp,
      std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
  if (!f.good()) {
    throw std::runtime_error(
        "AWSFileFetcher: could not open <" + localFilePathTmp.string() +
        "> for writing");
  }
  int64_t part = 0;
  int64_t totalWrittenSize = 0;
  while (!parts.empty() && !dtor_called_) {
    auto beg = part * buffer_size_;
    auto end = (part + 1) * buffer_size_;
    if (end > size) {
      end = size;
    }
    auto partSize = end - beg;

    if (dtor_called_) {
      return; // empty result, we are ending anyways
    }

    if (!parts.front().valid()) {
      throw std::runtime_error(
          "AWSFileFetcher: invalid future (internal error, please report)");
    }
    auto outcome = parts.front().get();
    parts.pop_front();
    if (!outcome.IsSuccess()) {
      const Aws::S3::S3Error& err = outcome.GetError();
      throw std::runtime_error(
          "AWSFileFetcher: unable to fetch <s3://" + bucket_ + "/" +
          remoteFilePath.string() + "> : " + err.GetExceptionName() + " : " +
          err.GetMessage());
    } else {
      auto& buf = outcome.GetResult().GetBody();
      f << buf.rdbuf();
      totalWrittenSize += partSize;
      if (!f.good()) {
        throw std::runtime_error(
            "AWSFileFetcher: could not write in <" + localFilePathTmp.string() +
            ">");
      }
      if (f.tellp() != totalWrittenSize) {
        throw std::runtime_error(
            "AWSFileFetcher: unexpected write size in <" +
            localFilePathTmp.string() + ">");
      }
    }
    part++;
  }

  // rename file when done
  f.close();
  std::filesystem::rename(localFilePathTmp, localFilePath);

  if (verbose_) {
    std::cout << "AWSFileFetcher (" << std::hex << this << std::dec
              << ") : " << (dtor_called_.load() ? "aborted" : "done")
              << " fetching s3://" << bucket_ << "/" << remoteFilePath << " ("
              << totalWrittenSize << "/" << size << " bytes) into "
              << localFilePath << std::endl;
  }
}

void AWSFileFetcher::backend_erase(const std::string& filename) const {
  auto localFilePath = (local_prefix_ / filename);
  auto status = std::filesystem::remove(localFilePath);
  if (verbose_) {
    std::cout << "AWSFileFetcher (" << std::hex << this << std::dec
              << ") : erasing " << localFilePath
              << (status ? " (done)" : " (file does not exist)") << std::endl;
  }
}

AWSFileFetcher::~AWSFileFetcher() {
  dtor_called_ = true;
  cancel_prefetch();
}

} // namespace core
} // namespace data
} // namespace mlx
