// Copyright © 2026 Apple Inc.

#include "mlx/data/stream/ArrowStream.h"

#include <cstring>
#include <mutex>
#include <stdexcept>

#include <nanoarrow/nanoarrow.h>

namespace mlx {
namespace data {
namespace stream {

struct ArrowStream::Impl {
  mutable ArrowArrayStream stream;
  // The schema outlives every batch: `ArrowArrayViewInitFromSchema` reads it
  // for each one, so it cannot be released once bound.
  ArrowSchema schema;
  std::vector<std::string> keys;
  std::vector<ArrayType> types;
  mutable std::mutex mutex;

  ~Impl() {
    if (stream.release != nullptr) {
      stream.release(&stream);
    }
    if (schema.release != nullptr) {
      schema.release(&schema);
    }
  }
};

namespace {

// Only the fixed-width types `ArrayType` can hold are accepted. nanoarrow
// parses the format string, so a type is refused by name -- "string",
// "timestamp" -- rather than by raw format character, and parameterised types
// (decimals, timestamps with a timezone, fixed-size lists) no longer have to
// be recognised here by hand.
ArrayType array_type_of(const ArrowSchema* schema, const std::string& key) {
  ArrowSchemaView view;
  ArrowError error;
  if (ArrowSchemaViewInit(&view, schema, &error) != NANOARROW_OK) {
    throw std::runtime_error(
        "ArrowStream: column '" + key +
        "' has an unreadable schema: " + ArrowErrorMessage(&error));
  }
  switch (view.type) {
    case NANOARROW_TYPE_INT8:
      return ArrayType::Int8;
    case NANOARROW_TYPE_UINT8:
      return ArrayType::UInt8;
    case NANOARROW_TYPE_INT32:
      return ArrayType::Int32;
    case NANOARROW_TYPE_INT64:
      return ArrayType::Int64;
    case NANOARROW_TYPE_FLOAT:
      return ArrayType::Float;
    case NANOARROW_TYPE_DOUBLE:
      return ArrayType::Double;
    case NANOARROW_TYPE_BOOL:
      // Arrow's boolean is a *bitmap*, one bit per value, and `ArrayType` has
      // no bool: adopting the buffer would reinterpret 8 values as 1.
      throw std::runtime_error(
          "ArrowStream: column '" + key +
          "' is boolean, which Arrow stores as a bitmap of one bit per value, so it "
          "cannot be adopted as a byte array; cast to int8 before exporting");
    default:
      throw std::runtime_error(
          "ArrowStream: column '" + key + "' is " + ArrowTypeString(view.type) +
          ", and no ArrayType holds it. Fixed-width int8/uint8/int32/int64/float/double "
          "are supported; strings, booleans, 16-bit and nested types are not.");
  }
}

} // namespace

ArrowStream::ArrowStream(ArrowArrayStream* stream) : impl_(new Impl()) {
  std::memset(&impl_->stream, 0, sizeof(ArrowArrayStream));
  std::memset(&impl_->schema, 0, sizeof(ArrowSchema));
  if (stream == nullptr || stream->release == nullptr) {
    throw std::runtime_error(
        "ArrowStream: got a released or null ArrowArrayStream");
  }
  // Move: the producer hands over ownership, and a moved-from stream is marked
  // released so nobody calls it twice. This is the convention the C Data
  // Interface specifies, and nanoarrow spells it.
  ArrowArrayStreamMove(stream, &impl_->stream);

  if (impl_->stream.get_schema(&impl_->stream, &impl_->schema) != 0) {
    const char* msg = impl_->stream.get_last_error
        ? impl_->stream.get_last_error(&impl_->stream)
        : nullptr;
    throw std::runtime_error(
        std::string("ArrowStream: get_schema failed: ") +
        (msg ? msg : "unknown"));
  }
  // A struct at the top level is how a batch is described; its children are
  // the columns.
  for (int64_t c = 0; c < impl_->schema.n_children; ++c) {
    const ArrowSchema* child = impl_->schema.children[c];
    std::string key = child->name ? child->name : ("col" + std::to_string(c));
    impl_->keys.push_back(key);
    impl_->types.push_back(array_type_of(child, key));
  }
}

ArrowStream::~ArrowStream() = default;

Sample ArrowStream::next() const {
  ArrowArray batch;
  {
    std::unique_lock lock(impl_->mutex);
    if (impl_->stream.release == nullptr) {
      return Sample();
    }
    if (impl_->stream.get_next(&impl_->stream, &batch) != 0) {
      const char* msg = impl_->stream.get_last_error
          ? impl_->stream.get_last_error(&impl_->stream)
          : nullptr;
      throw std::runtime_error(
          std::string("ArrowStream: get_next failed: ") +
          (msg ? msg : "unknown"));
    }
  }
  // A released array is how the interface says "end of stream".
  if (batch.release == nullptr) {
    return Sample();
  }

  // One owner for the whole batch, so its release callback fires exactly once
  // when the last column Array is dropped -- not per column, and not while a
  // column is still referenced.
  auto keep =
      std::shared_ptr<ArrowArray>(new ArrowArray(batch), [](ArrowArray* a) {
        if (a->release != nullptr) {
          a->release(a);
        }
        delete a;
      });

  Sample sample;
  for (int64_t c = 0; c < keep->n_children; ++c) {
    const std::string& key = impl_->keys.at(c);

    // A fresh view per batch rather than one reused across calls: `next` is
    // const and callers may be concurrent, and a view carries per-batch state.
    // Setting it also validates the batch against the schema, which is what
    // the hand-written `n_buffers` check used to approximate.
    ArrowArrayView view;
    ArrowError error;
    if (ArrowArrayViewInitFromSchema(
            &view, impl_->schema.children[c], &error) != NANOARROW_OK) {
      throw std::runtime_error(
          "ArrowStream: column '" + key +
          "' could not be viewed: " + ArrowErrorMessage(&error));
    }
    struct ViewGuard {
      ArrowArrayView* v;
      ~ViewGuard() {
        ArrowArrayViewReset(v);
      }
    } guard{&view};

    if (ArrowArrayViewSetArray(&view, keep->children[c], &error) !=
        NANOARROW_OK) {
      throw std::runtime_error(
          "ArrowStream: column '" + key +
          "' does not match its schema: " + ArrowErrorMessage(&error));
    }

    // An array's own `null_count` is -1 when the producer never counted, so
    // ask the view instead: it reads the validity bitmap, rather than
    // rejecting a null-free column that merely declined to report a count.
    const int64_t nulls = ArrowArrayViewComputeNullCount(&view);
    if (nulls != 0) {
      throw std::runtime_error(
          "ArrowStream: column '" + key + "' has " + std::to_string(nulls) +
          " null(s), and a Sample has nowhere to record which values are absent. Fill or "
          "drop them before exporting.");
    }

    const void* base = view.buffer_views[1].data.data;
    if (base == nullptr) {
      throw std::runtime_error(
          "ArrowStream: column '" + key +
          "' has no values buffer, so it is not a fixed-width column");
    }
    // The offset is in *values*, not bytes, and a sliced batch has a non-zero
    // one. Ignoring it would silently return the wrong rows.
    Array probe(impl_->types.at(c), std::vector<int64_t>{0});
    void* values = static_cast<char*>(const_cast<void*>(base)) +
        view.offset * probe.itemsize();
    // Aliasing constructor: points at this column's buffer, owns the whole
    // batch.
    sample[key] = std::make_shared<Array>(
        impl_->types.at(c),
        std::vector<int64_t>{view.length},
        std::shared_ptr<void>(keep, values));
  }
  return sample;
}

void ArrowStream::reset() {
  // The C Data Interface has no rewind: a stream is consumed once.
  // `Stream::reset` is what `repeat` and a second epoch call, so refusing is
  // the honest answer -- a caller who needs more than one pass exports a new
  // stream, which is what `stream_python_iterable`'s factory argument exists
  // for on the Python side.
  throw std::runtime_error(
      "ArrowStream: an ArrowArrayStream is consumed once and cannot be reset. Wrap a "
      "factory that exports a fresh stream per epoch.");
}

} // namespace stream
} // namespace data
} // namespace mlx
