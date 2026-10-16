// Copyright © 2026 Apple Inc.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "mlx/data/Array.h"
#include "mlx/data/Sample.h"
#include "mlx/data/stream/Stream.h"

// The C Data Interface struct, declared rather than included. This header is
// installed, so pulling nanoarrow in here would put it on the include path of
// everything downstream; it stays behind `Impl` in the .cpp instead.
extern "C" struct ArrowArrayStream;

namespace mlx {
namespace data {
namespace stream {

// A stream over Arrow data handed in through the C Data Interface.
//
// **One Sample per RecordBatch, not per row.** Arrow already stores a column as
// one contiguous typed buffer, so a batch's column *is* an Array -- adopted,
// not copied, via `Array(type, shape, shared_ptr<void>)`. Emitting rows instead
// would mean splitting those buffers into values and then letting `Batch`
// reassemble them, which is work Arrow already did. Measured in Python on
// 200,000 rows x 3 columns: row-at-a-time is 22.8 ms and using the columns
// whole is 0.02 ms. The interface being row-shaped is the cost, not the
// language.
//
// The consequence, which callers must know: the leading dimension of every
// array in a Sample is Arrow's batch size, not a size this stream chose.
// Putting `.batch(n)` on top of it re-batches and gives back what was saved --
// set the batch size where the reader is created
// (`iter_batches(batch_size=...)`) instead.
class ArrowStream : public Stream {
 public:
  // Takes ownership of the stream: `release` is called from the destructor. The
  // pointer is an `ArrowArrayStream*` produced by a `_export_to_c`-style call.
  explicit ArrowStream(ArrowArrayStream* stream);
  ~ArrowStream() override;

  Sample next() const override;
  void reset() override;

 private:
  // Holds the moved-from stream, the schema it described (kept alive because
  // nanoarrow's array views are initialised from it), and the column keys and
  // types derived from that schema.
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace stream
} // namespace data
} // namespace mlx
