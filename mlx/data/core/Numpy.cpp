// Copyright © 2023 Apple Inc.

#include "mlx/data/core/Numpy.h"

#include <iostream>
#include <string>

namespace {

static void check_stream(
    const std::istream& f,
    const std::string filename,
    const std::string action) {
  if (!f.good()) {
    throw std::runtime_error(
        std::string("loadNumpy: error " + action + " file <") + filename + ">");
  }
}

/// structure of the npy file format string (a string-serialized python
/// dictionary), see parseNumpyFormat
struct Format {
  std::string descr;
  bool fortran_order;
  std::vector<int64_t> shape;
};

/// tokens in the python dictionary, see parseNumpyFormat
enum Token {
  LEFT_BRACE,
  RIGHT_BRACE,
  QUOTE,
  COLON,
  COMMA,
  LEFT_PAREN,
  RIGHT_PAREN,
  CHARACTER,
  WHITESPACE,
};

/// states in the python dictionary, see parseNumpyFormat
enum State {
  START,

  // top level of the interior of the dictionary -- looking for a key or right
  // brace
  READING_DICTIONARY,

  // consume characters in the key string
  READING_KEY,

  // the colon separator between the key and value
  READING_AFTER_KEY,

  // starting to read the value -- will determine type
  READING_VALUE,

  // reading a quoted string
  READING_VALUE_STRING,

  // reading a literal (True/False)
  READING_VALUE_LITERAL,

  // reading a tuple (for these purposes required to be a list of ints)
  READING_VALUE_TUPLE,

  // done with value -- time to process
  READING_AFTER_VALUE,
  READING_AFTER_LITERAL,

  DONE,
};

/// parse a numpy format string like `{'descr': '<f2', 'fortran_order': False,
/// 'shape': (4, 3), }`
static Format parse_numpy_format(
    std::vector<unsigned char>& format,
    const std::string filename) {
  Format result;
  State state = START;

  // current key and value as strings
  std::string key = "";
  std::string value = "";

  // when reading tuples, current int and collection of ints
  int64_t int_value = 0;
  bool int_value_valid = false;
  std::vector<int64_t> values;

  for (auto c : format) {
    Token t;

    switch (c) {
      case '{':
        t = LEFT_BRACE;
        break;
      case '}':
        t = RIGHT_BRACE;
        break;
      case '\'':
        t = QUOTE;
        break;
      case ':':
        t = COLON;
        break;
      case ',':
        t = COMMA;
        break;
      case '(':
        t = LEFT_PAREN;
        break;
      case ')':
        t = RIGHT_PAREN;
        break;
      case ' ':
      case '\n':
      case '\t':
      case '\r':
        t = WHITESPACE;
        break;
      default:
        t = CHARACTER;
        break;
    }

    switch (state) {
      case START:
        if (t == LEFT_BRACE) {
          state = READING_DICTIONARY;
        } else {
          throw std::runtime_error(
              std::string("loadNumpy: bad format, expected '{', got ") +
              std::string(1, c) + " file <" + filename + ">");
        }
        break;
      case READING_DICTIONARY:
        if (t == WHITESPACE) {
          continue;
        }
        if (t == QUOTE) {
          state = READING_KEY;
          key = "";
        } else if (t == RIGHT_BRACE) {
          state = DONE;
        } else {
          throw std::runtime_error(
              std::string("loadNumpy: bad format, expected ''', got ") +
              std::string(1, c) + " file <" + filename + ">");
        }
        break;
      case READING_KEY:
        if (t != QUOTE) {
          key += c;
        } else {
          state = READING_AFTER_KEY;
        }
        break;
      case READING_AFTER_KEY:
        if (t == WHITESPACE) {
          continue;
        }
        if (t == COLON) {
          state = READING_VALUE;

          value = "";
          int_value = 0;
          int_value_valid = false;
          values.clear();
        } else {
          throw std::runtime_error(
              std::string("loadNumpy: bad format, expected ':', got ") +
              std::string(1, c) + " file <" + filename + ">");
        }
        break;
      case READING_VALUE:
        if (t == WHITESPACE) {
          continue;
        }

        // determine the type of value
        if (t == QUOTE) {
          state = READING_VALUE_STRING;
        } else if (t == CHARACTER) {
          // this first character is part of the literal
          value = c;
          state = READING_VALUE_LITERAL;
        } else if (t == LEFT_PAREN) {
          state = READING_VALUE_TUPLE;
        } else {
          throw std::runtime_error(
              std::string("loadNumpy: bad format, expected value, got ") +
              std::string(1, c) + " file <" + filename + ">");
        }
        break;
      case READING_VALUE_STRING:
        if (t != QUOTE) {
          value += c;
        } else {
          state = READING_AFTER_VALUE;
        }
        break;
      case READING_VALUE_LITERAL:
        if (t == COMMA || t == RIGHT_BRACE) {
          state = READING_AFTER_LITERAL;
        } else {
          value += c;
        }
        break;
      case READING_VALUE_TUPLE:
        if (t != RIGHT_PAREN) {
          value += c;

          if (t == CHARACTER) {
            if (c >= '0' && c <= '9') {
              int_value = int_value * 10 + (c - '0');
              int_value_valid = true;
            }
          } else if (t == COMMA) {
            values.push_back(int_value);
            int_value = 0;
            int_value_valid = false;
          }

        } else {
          if (int_value_valid) {
            values.push_back(int_value);
          }
          state = READING_AFTER_VALUE;
        }
        break;
      case READING_AFTER_VALUE:
        if (t == WHITESPACE) {
          continue;
        }
        if (t == RIGHT_BRACE) {
          state = DONE;
        } else if (t == COMMA) {
          state = READING_DICTIONARY;
        } else {
          throw std::runtime_error(
              std::string("loadNumpy: bad format, expected ''' or '}', got ") +
              std::string(1, c) + " file <" + filename + ">");
        }
        break;
      case READING_AFTER_LITERAL:
        // handled below
        break;
      case DONE:
        // ignore
        break;
    }

    if (state == DONE) {
      break;
    }

    // if we have just consumed a value, handle it
    if (state == READING_AFTER_VALUE || state == READING_AFTER_LITERAL) {
      if (key == "descr") {
        result.descr = value;

      } else if (key == "fortran_order") {
        if (value == "False") {
          result.fortran_order = false;
        } else if (value == "True") {
          result.fortran_order = true;
        } else {
          throw std::runtime_error(
              std::string("loadNumpy: unexpected literal: ") + value +
              " file <" + filename + ">");
        }

      } else if (key == "shape") {
        result.shape = values;

      } else {
        throw std::runtime_error(
            std::string("loadNumpy: unexpected key: ") + key + " file <" +
            filename + ">");
      }
    }

    // in the case of a literal, e.g. "'key': False," we are at the comma
    // and need to determine the next state
    if (state == READING_AFTER_LITERAL) {
      if (t == RIGHT_BRACE) {
        state = DONE;
      } else {
        state = READING_DICTIONARY;
      }
    }
  }

  return result;
}

} // namespace

namespace mlx {
namespace data {
namespace core {

/// See https://numpy.org/doc/stable/reference/generated/numpy.lib.format.html
struct NumpyHeader {
  char magic[6];
  unsigned char major;
  unsigned char minor;
  unsigned char headerLength0;
  unsigned char headerLength1;
};

/// additional bytes for version 2.0 and higher headers
struct NumpyHeaderV2 {
  unsigned char headerLength2;
  unsigned char headerLength3;
};

static const auto* numpy_magic = "\x93NUMPY";

std::shared_ptr<Array> load_numpy(const std::string& filename) {
  std::ifstream f(filename, std::ios_base::in | std::ios_base::binary);

  return load_numpy(f, filename);
}

std::shared_ptr<Array> load_numpy(
    std::istream& stream,
    const std::string& filename) {
  check_stream(stream, filename, "opening");

  // there is a fixed header with magic and version numbers and a
  // variable length header which is a string serialized python dictionary
  // describing the layout.

  NumpyHeader header;
  size_t additional_header_size = 0;
  stream.read((char*)&header, sizeof(header));
  check_stream(stream, filename, "reading header");

  if (strncmp(header.magic, numpy_magic, sizeof(header.magic)) != 0) {
    throw std::runtime_error(
        std::string("loadNumpy: bad magic file <") + filename + ">");
  }

  // compute the size of the format section of the header
  switch (header.major) {
    case 1:
      additional_header_size =
          header.headerLength0 + (header.headerLength1 << 8);
      break;
    case 2:
    case 3:
      // version 2 and 3 have 2 more bytes for the additional header length
      NumpyHeaderV2 additional_header;
      stream.read((char*)&additional_header, sizeof(additional_header));
      check_stream(stream, filename, "reading header additional");

      additional_header_size = header.headerLength0 +
          (header.headerLength1 << 8) +
          (additional_header.headerLength2 << 16) +
          (additional_header.headerLength3 << 24);
      break;
    default:
      throw std::runtime_error(
          std::string("loadNumpy: unknown major version") +
          std::to_string((int)header.major) + " file <" + filename + ">");
  }

  std::vector<unsigned char> format_str(additional_header_size);
  stream.read((char*)format_str.data(), additional_header_size);
  check_stream(stream, filename, "reading format");

  Format format = parse_numpy_format(format_str, filename);
  if (format.fortran_order) {
    throw std::runtime_error(
        std::string("loadNumpy: unhandled fortran_order = True file <") +
        filename + ">");
  }

  // convert from descr to our type.  this is:
  // np.lib.format.dtype_to_descr(np.dtype("uint8"))
  //
  // Note: we only support native byte orders.

  ArrayType array_type;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  if (format.descr == "|u1") {
    array_type = ArrayType::UInt8;
  } else if (format.descr == "|i1") {
    array_type = ArrayType::Int8;
  } else if (format.descr == "<i4") {
    array_type = ArrayType::Int32;
  } else if (format.descr == "<i8") {
    array_type = ArrayType::Int64;
  } else if (format.descr == "<f4") {
    array_type = ArrayType::Float;
  } else if (format.descr == "<f8") {
    array_type = ArrayType::Double;
  } else {
    throw std::runtime_error(
        std::string("loadNumpy: unknown dtype: ") + format.descr + " file <" +
        filename + ">");
  }
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  if (format.descr == "|u1") {
    arrayType = ArrayType::UInt8;
  } else if (format.descr == "|i1") {
    arrayType = ArrayType::Int8;
  } else if (format.descr == ">i4") {
    arrayType = ArrayType::Int32;
  } else if (format.descr == ">i8") {
    arrayType = ArrayType::Int64;
  } else if (format.descr == ">f4") {
    arrayType = ArrayType::Float;
  } else if (format.descr == ">f8") {
    arrayType = ArrayType::Double;
  } else {
    throw std::runtime_error(
        std::string("loadNumpy: unknown dtype: ") + format.descr + " file <" +
        filename + ">");
  }
#else
#error "__BYTE_ORDER__ not defined"
#endif

  auto array = std::make_shared<Array>(array_type, format.shape);
  stream.read((char*)array->data(), array->size() * array->itemsize());
  check_stream(stream, filename, "reading data");

  return array;
}

} // namespace core
} // namespace data
} // namespace mlx
