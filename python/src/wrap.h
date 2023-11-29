#pragma once

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "mlx/data/Array.h"
#include "mlx/data/Sample.h"

namespace py = pybind11;

namespace mlx {
namespace pybind {

std::shared_ptr<mlx::data::Array> to_array(py::array a);
py::array to_py_array(const std::shared_ptr<const mlx::data::Array>& a);

py::dict to_py_sample(const mlx::data::Sample& s);
mlx::data::Sample to_sample(py::dict s);

template <typename T>
struct NogilDeleter {
  void operator()(T* ptr) {
    py::gil_scoped_release nogil;
    delete ptr;
  }
};

// An implementation of basic_streambuf for input only. It is backed by a
// python object that has a read() and seek().
class py_istream_buf : public std::streambuf {
 public:
  py_istream_buf(py::object file, int N) {
    if (!py::hasattr(file, "read") || !py::hasattr(file, "seek")) {
      throw std::invalid_argument(
          "The passed file has no read() and/or seek()");
    }
    // If the object has readinto then we take advantage of it to avoid one
    // copy.
    has_readinto = py::hasattr(file, "readinto");

    // Allocate the buffer and set the member variables
    buffer_ = new char[N];
    buffer_size_ = N;
    file_ = file;

    // Finally set the streambuf read pointers to be "exhausted" (current is
    // same as end)
    setg(buffer_, buffer_ + N, buffer_ + N);
  }

  virtual ~py_istream_buf() {
    // We need to delete the buffer
    delete[] buffer_;

    // And also release the file under GIL.
    {
      py::gil_scoped_acquire gil;
      file_.release().dec_ref();
    }
  }

 protected:
  // Means the stream ran out so we need to read more or return eof
  virtual int underflow() override {
    // If we didn't ran out then simply return the value under the current
    // pointer.
    if (gptr() != egptr()) {
      return *gptr();
    } else {
      py::gil_scoped_acquire gil;

      // Ok we ran out so read buffer_size_ things into the buffer and reset
      // the pointers.
      int n_read = readinto_buffer();
      setg(buffer_, buffer_, buffer_ + n_read);
      if (n_read > 0) {
        return *gptr();
      } else {
        return traits_type::eof();
      }
    }
  }

  // Seek the underlying python "file" to the requested position.
  virtual pos_type seekpos(
      pos_type pos,
      std::ios_base::openmode which = std::ios_base::in |
          std::ios_base::out) override {
    // We can only seek the input side of things.
    if (!(which | std::ios_base::in)) {
      return -1;
    }

    pos_type abs_pos;
    {
      // Seek the python side under GIL.
      py::gil_scoped_acquire gil;
      auto abs_pos_tmp =
          file_.attr("seek")(static_cast<int64_t>(pos), 0).cast<int64_t>();
      abs_pos = static_cast<pos_type>(abs_pos_tmp);
    }

    // Reset the pointers to "exhausted". This may be slightly inefficient in
    // case we did not seek too far.
    setg(buffer_, buffer_ + buffer_size_, buffer_ + buffer_size_);
    return abs_pos;
  }

 private:
  // Simple convenience function to utilize readinto if it is present in the
  // python "file".
  int readinto_buffer() {
    if (has_readinto) {
      return file_
          .attr("readinto")(py::memoryview::from_buffer(
              buffer_, {buffer_size_}, {sizeof(char)}))
          .cast<int>();
    } else {
      std::string buff =
          file_.attr("read")(buffer_size_).template cast<std::string>();
      if (buff.size() > 0) {
        std::memcpy(buffer_, buff.c_str(), buff.size() * sizeof(char));
      }

      return buff.size();
    }
  }

  char* buffer_;
  int buffer_size_;
  py::object file_;
  bool has_readinto;
};

// A simple input stream that owns its streambuffer.
class py_istream : public std::istream {
 public:
  py_istream(py::object file, int N)
      : std::istream(&buffer_), buffer_(file, N) {}

 private:
  py_istream_buf buffer_;
};

} // namespace pybind
} // namespace mlx
