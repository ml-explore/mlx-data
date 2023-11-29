#include <string>
#include <vector>

#include "mlx/data/core/CSVReader.h"

namespace mlx {
namespace data {
namespace core {

CSVReader::CSVReader(const std::string& file, const char sep, const char quote)
    : filename_(file), sep_(sep), quote_(quote) {
  uf_ = std::make_shared<std::ifstream>(filename_, std::ios_base::binary);
  f_ = std::make_shared<bxz::istream>(*uf_);
  if (!uf_->good() || !f_->good()) {
    throw std::runtime_error(
        "CSVReader: could not open file <" + filename_ + ">");
  }
}

CSVReader::CSVReader(
    const std::shared_ptr<std::istream>& uf,
    const char sep,
    const char quote)
    : filename_("<stream>"), sep_(sep), quote_(quote), uf_(uf) {
  f_ = std::make_shared<bxz::istream>(*uf_);
  if (!uf_->good() || !f_->good()) {
    throw std::runtime_error("CSVReader: could not open memory file");
  }
}

void CSVReader::parse_line_(
    const std::string& line,
    std::vector<std::string>& fields,
    int& state,
    std::string& field) const {
  for (int i = 0; i < line.size() + 1; i++) {
    auto c = (i < line.size() ? line[i] : lf_);

    switch (state) {
      // end node
      case 0:
        return;

        // start node
      case 1:
        field = "";
        if (c == lf_) {
          state = 0;
        } else if (c == cr_) {
          state = 6;
        } else if (c == sep_) {
          fields.push_back(field);
          state = 1;
        } else if (c == quote_) {
          state = 3;
        } else {
          field += c;
          state = 2;
        }
        break;

        // unquoted field
      case 2:
        if (c == lf_) {
          fields.push_back(field);
          state = 0;
        } else if (c == cr_) {
          fields.push_back(field);
          state = 6;
        } else if (c == sep_) {
          fields.push_back(field);
          state = 1;
        } else if (c == quote_) {
          throw std::runtime_error(
              "CSVReader: unexpected quote at line " +
              std::to_string(numLine_) + " in file <" + filename_ + ">");
        } else {
          field += c;
        }
        break;

        // quoted field
      case 3:
        if (c == quote_) {
          state = 4;
        } else {
          field += c;
        }
        break;

        // end-of-quote?
      case 4:
        if (c == lf_) {
          fields.push_back(field);
          state = 0;
        } else if (c == cr_) {
          fields.push_back(field);
          state = 6;
        } else if (c == sep_) {
          fields.push_back(field);
          state = 1;
        } else if (c == quote_) {
          field += c;
          state = 5;
        } else {
          throw std::runtime_error(
              "CSVReader: unexpected character after quote at line " +
              std::to_string(numLine_) + " in file <" + filename_ + ">");
        }
        break;

      // quoted-quote?
      case 5:
        if (c == quote_) {
          state = 4;
        } else {
          field += c;
          state = 3;
        }
        break;

        // cr->lf
      case 6:
        // note that all cr/lf checks
        // lead to state 0
        if (c == lf_) {
          state = 0;
        } else {
          throw std::runtime_error(
              "CSVReader: unexpected character after carriage return at line " +
              std::to_string(numLine_) + " in file <" + filename_ + ">");
        }
        break;

      default:
        throw std::runtime_error(
            "CSVReader: internal parsing error at line " +
            std::to_string(numLine_) + " in file <" + filename_ + ">");
    }
  }
}

std::vector<std::string> CSVReader::next() {
  std::string line;
  std::vector<std::string> fields;
  std::string field;
  int state = 1;
  bool iseof = false;
  do {
    if (!std::getline(*f_, line)) {
      if (state == 1) {
        iseof = true;
      }
      break;
    }
    numLine_++;
    parse_line_(line, fields, state, field);
  } while (state != 0);

  if (!iseof) {
    if (state != 0) {
      throw std::runtime_error(
          "CSVReader: unexpected end of stream at line " +
          std::to_string(numLine_) + " in file <" + filename_ + ">");
    }
    if (numFields_ < 0) {
      numFields_ = fields.size();
    } else {
      if (numFields_ != fields.size()) {
        throw std::runtime_error(
            "CSVReader: inconsistent number of fields at line " +
            std::to_string(numLine_) + " in file <" + filename_ + ">");
      }
    }
  }

  return fields;
}

void CSVReader::reset() {
  f_->clear();
  f_->seekg(0);
  if (!uf_->good() || !f_->good()) {
    throw std::runtime_error(
        "CSVReader: could not seek to beginning of file <" + filename_ + ">");
  }
  numLine_ = 0;
}

} // namespace core
} // namespace data
} // namespace mlx
