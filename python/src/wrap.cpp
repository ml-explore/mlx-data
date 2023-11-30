// Copyright © 2023 Apple Inc.

#include "wrap.h"

#include "mlx/data/Dataset.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

namespace py = pybind11;

using namespace mlx::data;

void init_mlx_data_core(py::module&);
void init_mlx_data_buffer(py::module&);
void init_mlx_data_stream(py::module&);
void init_mlx_data_ops_buffer(py::module&);
void init_mlx_data_ops_oparray(py::module&);
void init_mlx_data_ops_op(py::module&);
void init_mlx_data_ops_image(py::module&);

// class PyOp : public mlx::data::ops::Op {
//   using mlx::data::ops::Op::Op;
//   std::unordered_map<std::string, std::shared_ptr<mlx::data::Array>>
//   apply(const std::unordered_map<std::string,
//   std::shared_ptr<mlx::data::Array>>& input) override {
//     PYBIND11_OVERRIDE_PURE(
//       (std::unordered_map<std::string, std::shared_ptr<mlx::data::Array>>),
//                            mlx::data::ops::Op,
//                            apply,
//                            input);
//   };

// };

namespace mlx {
namespace pybind {

std::shared_ptr<mlx::data::Array> to_array(py::array a) {
  // make sure "a" is contiguous
  const auto c_contiguous =
      py::detail::npy_api::constants::NPY_ARRAY_C_CONTIGUOUS_;
  if (!(c_contiguous == (a.flags() & c_contiguous))) {
    throw std::runtime_error(
        "contiguous array expected -- use numpy.ascontiguousarray()");
  }
  std::vector<int64_t> shape(a.ndim());
  for (int i = 0; i < a.ndim(); i++) {
    shape[i] = a.shape(i);
  }
  // Note: if we are here, we are under GIL.
  // We manually increase/decrease the refcount instead of passing "a" as a
  // py::object upvalue to the closure. In the latter case, the destructor of
  // "a" might then be called when the GIL is not locked, leading to a segfault.
  auto handle = a.inc_ref();
  auto data = std::shared_ptr<void>(a.mutable_data(), [handle](void*) {
    py::gil_scoped_acquire gil;
    handle.dec_ref();
  });
  switch (a.dtype().char_()) {
    case 'f':
      return std::make_shared<mlx::data::Array>(
          mlx::data::ArrayType::Float, shape, data);
    case 'd':
      return std::make_shared<mlx::data::Array>(
          mlx::data::ArrayType::Double, shape, data);
    case 'i':
      return std::make_shared<mlx::data::Array>(
          mlx::data::ArrayType::Int32, shape, data);
    case 'l':
      return std::make_shared<mlx::data::Array>(
          mlx::data::ArrayType::Int64, shape, data);
    case 'b':
      return std::make_shared<mlx::data::Array>(
          mlx::data::ArrayType::Int8, shape, data);
    case 'B':
      return std::make_shared<mlx::data::Array>(
          mlx::data::ArrayType::UInt8, shape, data);
    case 'S':
    case 'U':
      shape.push_back(a.itemsize());
      return std::make_shared<mlx::data::Array>(
          mlx::data::ArrayType::Int8, shape, data);

    default:
      throw std::runtime_error(
          "toArray: NYI (unknown array type: " +
          std::string(1, a.dtype().char_()) + ")");
  }
  return nullptr;
}

struct PyArrayPayload {
  std::shared_ptr<const mlx::data::Array> a;
};

py::array to_py_array(const std::shared_ptr<const mlx::data::Array>& a) {
  const py::capsule free_when_done(new PyArrayPayload({a}), [](void* payload) {
    delete reinterpret_cast<PyArrayPayload*>(payload);
  });
  std::vector<int64_t> shape;
  std::vector<int64_t> stride;
  py::dtype pytype;
  switch (a->type()) {
    case mlx::data::ArrayType::Int8:
      pytype = py::dtype("b");
      break;
    case mlx::data::ArrayType::UInt8:
      pytype = py::dtype("B");
      break;
    case mlx::data::ArrayType::Int32:
      pytype = py::dtype("i4");
      break;
    case mlx::data::ArrayType::Int64:
      pytype = py::dtype("i8");
      break;
    case mlx::data::ArrayType::Float:
      pytype = py::dtype("f");
      break;
    case mlx::data::ArrayType::Double:
      pytype = py::dtype("d");
      break;
    default:
      throw std::runtime_error("internal error: unknown type");
  }
  auto ndim = a->shape().size();
  if (ndim > 0) {
    stride = std::vector<int64_t>(ndim);
    shape = a->shape();
    stride[ndim - 1] = a->itemsize();
    for (int i = ndim - 1; i > 0; i--) {
      stride[i - 1] = stride[i] * a->shape(i);
    }
  }
  return py::array(pytype, shape, stride, a->data(), free_when_done);
}

py::dict to_py_sample(const Sample& s) {
  py::dict res;
  for (auto& el : s) {
    res[py::str(el.first)] = to_py_array(el.second);
  }
  return res;
}

Sample to_sample(py::dict s) {
  Sample res;
  for (auto& el : s) {
    std::string key = el.first.cast<std::string>();
    py::array value = el.second.cast<py::array>();
    res[key] = to_array(value);
  }
  return res;
}

} // namespace pybind
} // namespace mlx

PYBIND11_MODULE(_c, m) {
  m.doc() = "mlx data";

  py::enum_<TokenizeMode>(m, "TokenizeMode")
      .value("Shortest", TokenizeMode::shortest)
      .value("Rand", TokenizeMode::rand)
      .export_values();

  py::enum_<LoadAudioInfo>(m, "LoadAudioInfo")
      .value("All", LoadAudioInfo::All)
      .value("NumFrames", LoadAudioInfo::NumFrames)
      .value("NumChannels", LoadAudioInfo::NumChannels)
      .value("SampleRate", LoadAudioInfo::SampleRate)
      .value("NumSeconds", LoadAudioInfo::NumSeconds)
      .export_values();

  init_mlx_data_stream(m);
  init_mlx_data_buffer(m);

  py::module_ m_core = m.def_submodule("core", "mlx.data.core");
  init_mlx_data_core(m_core);

  m.attr("__version__") = TOSTRING(_VERSION_);
}
