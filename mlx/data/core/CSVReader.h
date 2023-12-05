// Copyright © 2023 Apple Inc.

#pragma once

#include <fstream>
#include <istream>
#include <string>
#include <vector>

#include "bxzstr/bxzstr.hpp"

namespace mlx {
namespace data {
namespace core {

class CSVReader {
 public:
  CSVReader(
      const std::string& file,
      const char sep = ',',
      const char quote = '"');
  CSVReader(
      const std::shared_ptr<std::istream>& uf,
      const char sep = ',',
      const char quote = '"');
  std::vector<std::string> next();
  void reset();

 private:
  void parse_line_(
      const std::string& line,
      std::vector<std::string>& fields,
      int& current_state,
      std::string& current_field) const;

  std::string filename_;
  int numFields_ = -1;
  int numLine_ = 0;
  char sep_ = ',';
  char quote_ = '"';
  const char lf_ = '\n';
  const char cr_ = '\r';
  std::shared_ptr<std::istream> uf_;
  std::shared_ptr<bxz::istream> f_;
};

} // namespace core
} // namespace data
} // namespace mlx
