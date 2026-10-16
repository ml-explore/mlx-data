// Runtime check for ArrowStream: a hand-built ArrowArrayStream, so neither side
// needs Arrow C++ or a test framework.
//
// Proves the three things compiling cannot: the values land in the right
// Arrays, a sliced batch (non-zero `offset`) is read from the right place, and
// the batch's release callback fires exactly once when the last column Array is
// dropped.

#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>

// For the C Data Interface structs the producer below fills in: ArrowStream.h
// only forward declares them.
#include <nanoarrow/nanoarrow.h>

#include "mlx/data/stream/ArrowStream.h"

using namespace mlx::data;

namespace {

int g_released = 0; // how many times a batch's release ran

struct Producer {
  int emitted = 0;
  int64_t offset = 0; // set on the second batch, to exercise a slice
};

// storage that outlives the batches
int64_t A[8] = {10, 11, 12, 13, 14, 15, 16, 17};
float B[8] = {1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f, 8.5f};
// every value valid: 8 set bits, so the computed null count is 0
uint8_t VALID[1] = {0xFF};

void release_child(ArrowArray* a) {
  delete[] a->buffers;
  a->release = nullptr;
}

void release_batch(ArrowArray* a) {
  for (int64_t i = 0; i < a->n_children; ++i) {
    if (a->children[i]->release) {
      a->children[i]->release(a->children[i]);
    }
    delete a->children[i];
  }
  delete[] a->children;
  a->release = nullptr;
  ++g_released;
}

ArrowArray* make_child(const void* values, int64_t length, int64_t offset) {
  auto* c = new ArrowArray();
  std::memset(c, 0, sizeof(ArrowArray));
  c->length = length;
  c->offset = offset;
  // -1 is "not computed", which the spec allows and real producers emit. The
  // validity bitmap says every value is present, so the count has to be
  // computed from it: trusting this field rejects a null-free column.
  c->null_count = -1;
  c->n_buffers = 2;
  auto** bufs = new const void*[2];
  bufs[0] = VALID;
  bufs[1] = values;
  c->buffers = bufs;
  c->release = release_child;
  return c;
}

int get_next(ArrowArrayStream* s, ArrowArray* out) {
  auto* p = static_cast<Producer*>(s->private_data);
  std::memset(out, 0, sizeof(ArrowArray));
  if (p->emitted >= 2) {
    out->release = nullptr; // end of stream
    return 0;
  }
  int64_t off = (p->emitted == 1) ? 3 : 0; // second batch is a slice
  int64_t len = 4;
  out->length = len;
  out->null_count = 0;
  out->n_buffers = 1;
  out->n_children = 2;
  auto** kids = new ArrowArray*[2];
  kids[0] = make_child(A, len, off);
  kids[1] = make_child(B, len, off);
  out->children = kids;
  out->release = release_batch;
  ++p->emitted;
  return 0;
}

ArrowSchema* make_schema_child(const char* name, const char* format) {
  auto* c = new ArrowSchema();
  std::memset(c, 0, sizeof(ArrowSchema));
  c->format = format;
  c->name = name;
  c->release = [](ArrowSchema* s) { s->release = nullptr; };
  return c;
}

int get_schema(ArrowArrayStream* s, ArrowSchema* out) {
  std::memset(out, 0, sizeof(ArrowSchema));
  out->format = "+s";
  out->n_children = 2;
  auto** kids = new ArrowSchema*[2];
  kids[0] = make_schema_child("a", "l"); // int64
  kids[1] = make_schema_child("b", "f"); // float
  out->children = kids;
  out->release = [](ArrowSchema* s) {
    for (int64_t i = 0; i < s->n_children; ++i) {
      s->children[i]->release(s->children[i]);
      delete s->children[i];
    }
    delete[] s->children;
    s->release = nullptr;
  };
  return 0;
}

void release_stream(ArrowArrayStream* s) {
  delete static_cast<Producer*>(s->private_data);
  s->release = nullptr;
}

ArrowArrayStream make_stream() {
  ArrowArrayStream s;
  std::memset(&s, 0, sizeof(s));
  s.get_schema = get_schema;
  s.get_next = get_next;
  s.get_last_error = [](ArrowArrayStream*) -> const char* { return nullptr; };
  s.release = release_stream;
  s.private_data = new Producer();
  return s;
}

#define CHECK(cond, what)              \
  do {                                 \
    if (!(cond)) {                     \
      std::printf("FAIL: %s\n", what); \
      return 1;                        \
    }                                  \
  } while (0)

} // namespace

int main() {
  ArrowArrayStream s = make_stream();
  {
    stream::ArrowStream st(&s);
    CHECK(
        s.release == nullptr,
        "the producer's stream should be marked moved-from");

    Sample b1 = st.next();
    CHECK(
        b1.size() == 2,
        "batch 1 should have two columns, with null_count=-1 accepted");
    std::shared_ptr<Array> a1 = b1.at("a");
    std::shared_ptr<Array> f1 = b1.at("b");
    CHECK(a1->shape(0) == 4, "batch 1 length");
    CHECK(a1->type() == ArrayType::Int64, "int64 mapped from format 'l'");
    CHECK(f1->type() == ArrayType::Float, "float mapped from format 'f'");
    const int64_t* av = static_cast<const int64_t*>(a1->data());
    CHECK(av[0] == 10 && av[3] == 13, "batch 1 values, offset 0");
    const float* fv = static_cast<const float*>(f1->data());
    CHECK(fv[0] == 1.5f && fv[3] == 4.5f, "batch 1 float values");

    Sample b2 = st.next();
    const int64_t* av2 = static_cast<const int64_t*>(b2.at("a")->data());
    CHECK(av2[0] == 13 && av2[3] == 16, "batch 2 must honour offset=3");

    // The property that matters: while *any* column Array still references it,
    // the batch is NOT released. Two earlier versions of this check were wrong
    // -- one asserted 1 before anything was dropped, the next dropped `b1`
    // while `a1` and `f1` still held the columns. Both times the code was right
    // and the test was not.
    CHECK(
        g_released == 0,
        "a batch must not release while a column still references it");
    b1 = Sample();
    CHECK(g_released == 0, "still held: a1 and f1 reference batch 1's columns");
    a1.reset();
    f1.reset();
    CHECK(
        g_released == 1,
        "batch 1 releases exactly once, on the last reference");

    Sample end = st.next();
    CHECK(end.empty(), "a released ArrowArray means end of stream");

    bool threw = false;
    try {
      st.reset();
    } catch (const std::exception&) {
      threw = true;
    }
    CHECK(threw, "reset must refuse: an ArrowArrayStream is consumed once");

    b2 = Sample();
    CHECK(g_released == 2, "batch 2 released exactly once");
  }
  std::printf(
      "ok: values, slice offset, and one release per batch (%d released)\n",
      g_released);
  return 0;
}
