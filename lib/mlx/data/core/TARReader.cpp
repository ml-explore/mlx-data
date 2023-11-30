// Copyright © 2023 Apple Inc.

#include <cstring>
#include <functional>
#include <string>
#include <vector>

#include "mlx/data/core/TARReader.h"
#include "mlx/data/core/ThreadPool.h"

namespace {
size_t strnlength(const char* s, size_t n) {
  const char* found = (const char*)std::memchr(s, '\0', n);
  return found ? (size_t)(found - s) : n;
}

size_t parse_payload_size(unsigned char* str, int len) {
  // "first bit set" means large file (GNU tar)
  if (len > 0 && (str[0] & 0x80)) {
    // encoding is binary, base 256
    // ignore first bit
    size_t val = (size_t)(str[0] & 0x7f);
    for (int i = 1; i < len; i++) {
      val = (val << 8) | (size_t)str[i];
    }
    return val;
  } else {
    // encoding is string, base 8
    size_t val = 0;
    for (int i = 0; i < len && str[i] >= '0' && str[i] <= '7'; i++) {
      val = (val << 3) | (size_t)(str[i] - '0');
    }
    return val;
  }
}
void check_stream(
    const std::ifstream& f,
    const std::string filename,
    const std::string action) {
  if (!f.good()) {
    throw std::runtime_error(
        std::string("TARReader: error " + action + " archive <") + filename +
        ">");
  }
}

struct TARHeader {
  char filename[100]; // 0-terminated
  char mode[8];
  char uid[8];
  char gid[8];
  unsigned char payloadSize[12];
  char lastModification[12];
  char checksum[8];
  char type;
  char linkedfilename[100];
  // USTAR-specific:
  char ustarIndicator[6]; // "ustar"?
  char ustarVersion[2];
  char ownerUserName[32];
  char ownerGroupName[32];
  char deviceMajorNumber[8];
  char deviceMinorNumber[8];
  char filenamePrefix[155];
  char padding[12];
};

// Index a TAR file and return the index
mlx::data::core::TARFileIndex index_worker(
    const std::string& tarfilename,
    std::string prefix,
    int64_t start_offset) {
  mlx::data::core::TARFileIndex index;
  char zero_header[512];
  std::memset(zero_header, 0, 512);

  std::ifstream f(tarfilename, std::ios_base::in | std::ios_base::binary);
  f.seekg(start_offset, std::ios_base::beg);
  check_stream(f, tarfilename, "opening");

  std::string long_filename;
  while (f) {
    TARHeader header;
    f.read((char*)&header, 512);
    check_stream(f, tarfilename, "reading");
    if (memcmp(&header, zero_header, 512) == 0) {
      break;
    }

    auto payload_size =
        parse_payload_size(header.payloadSize, sizeof(header.payloadSize));
    auto padding_size = (512 - (payload_size % 512)) % 512;

    if (header.type == 'L') {
      std::vector<char> data(payload_size);
      f.read(data.data(), payload_size);
      check_stream(f, tarfilename, "reading");
      long_filename =
          std::string(data.data(), strnlength(data.data(), payload_size));
      f.ignore(padding_size);
      check_stream(f, tarfilename, "reading");
      continue;
    }

    if ((header.type == '0') || (header.type == 0) ||
        (long_filename.size() > 0)) { // file
      auto prefix_length = strnlength(header.filenamePrefix, 155);
      std::string filename(header.filename, strnlength(header.filename, 100));
      if (long_filename.size() > 0) {
        filename = long_filename;
        long_filename.clear();
      } else if (prefix_length > 0) {
        filename =
            std::string(header.filenamePrefix, prefix_length) + "/" + filename;
      }
      int64_t file_offset = f.tellg();
      f.seekg(payload_size + padding_size, std::ios_base::cur);
      check_stream(f, tarfilename, "indexing");
      index[prefix + filename] = std::make_pair(file_offset, payload_size);
      continue;
    }

    // unknown payload, skip it
    if (payload_size > 0) {
      f.ignore(payload_size + padding_size);
      check_stream(f, tarfilename, "reading");
      continue;
    }
  }

  return index;
}

} // namespace

namespace mlx {
namespace data {
namespace core {

TARReader::TARReader(
    const std::string& tarfilename,
    bool nested,
    int num_threads)
    : filename_(tarfilename) {
  // If nested TARs are not supported scan the tar in order
  if (!nested) {
    index_ = index_worker(tarfilename, "", 0);
  } else {
    // Otherwise make a threadpool to scan each nested tar in parallel
    ThreadPool pool(num_threads);
    std::queue<std::future<TARFileIndex>> index_futures;

    index_futures.push(
        pool.enqueue(std::bind(index_worker, tarfilename, "", 0)));
    while (!index_futures.empty()) {
      auto index = index_futures.front().get();
      index_futures.pop();

      for (auto& item : index) {
        auto& path = item.first;
        auto& offset = item.second.first;

        // If it ends in .tar untar it inline
        if (path.size() > 4 && path.substr(path.size() - 4) == ".tar") {
          auto prefix = path.substr(0, path.size() - 4) + "/";
          index_futures.push(pool.enqueue(
              std::bind(index_worker, tarfilename, prefix, offset)));
        }

        // Otherwise just add it to the index
        else {
          index_.insert(item);
        }
      }
    }
  }
}

bool TARReader::contains(const std::string& filename) {
  auto it = index_.find(filename);
  return (it != index_.end());
}

std::shared_ptr<Array> TARReader::get(const std::string& filename) {
  auto it = index_.find(filename);
  if (it == index_.end()) {
    throw std::runtime_error(
        std::string("TARReader: archive <") + filename_ +
        "> does not contain file <" + filename + ">");
  }
  auto file_offset = it->second.first;
  auto payload_size = it->second.second;
  auto array = std::make_shared<Array>(
      ArrayType::UInt8, static_cast<int64_t>(payload_size));
  std::ifstream f(filename_, std::ios_base::in | std::ios_base::binary);
  f.seekg(file_offset);
  if (!f.good()) {
    throw std::runtime_error(
        std::string("TARReader: could not seek in archive <") + filename_ +
        "> when fetching file <" + filename + ">");
  }
  f.read((char*)array->data(), payload_size);
  if (!f.good()) {
    throw std::runtime_error(
        std::string("TARReader: could not read in archive <") + filename_ +
        "> when fetching file <" + filename + ">");
  }
  return array;
}

std::vector<std::string> TARReader::get_file_list() {
  std::vector<std::string> files;
  for (auto& file_entry : index_) {
    files.push_back(file_entry.first);
  }
  return files;
}

} // namespace core
} // namespace data
} // namespace mlx
