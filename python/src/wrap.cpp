// Copyright © 2023 Apple Inc.

#include <sstream>

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

std::shared_ptr<mlx::data::Array> to_array(py::buffer a) {
  auto is_contiguous = [](const py::buffer_info& info) {
    bool contiguous =
        info.ndim > 0 && info.strides[info.ndim - 1] == info.itemsize;
    for (int i = 0; i < info.ndim - 1; i++) {
      contiguous &= info.shape[i + 1] * info.strides[i + 1] == info.strides[i];
    }
    return contiguous;
  };

  py::buffer_info info = a.request();
  if (!is_contiguous(info)) {
    throw std::runtime_error(
        "[to_array] Contiguous buffer expected -- maybe cast to np.array");
  }

  mlx::data::ArrayType dtype;
  switch (info.format[0]) {
    case 'b':
    case 'S':
    case 'U':
      dtype = mlx::data::ArrayType::Int8;
      break;
    case 'B':
      dtype = mlx::data::ArrayType::UInt8;
      break;
    case 'i':
      dtype = mlx::data::ArrayType::Int32;
      break;
    case 'l':
      dtype = mlx::data::ArrayType::Int64;
      break;
    case 'd':
      dtype = mlx::data::ArrayType::Double;
      break;
    case 'f':
      dtype = mlx::data::ArrayType::Float;
      break;
    default: {
      std::ostringstream msg;
      msg << "[to_array] Unsupported buffer type '" << info.format << "'";
      throw std::invalid_argument(msg.str());
    }
  }

  std::vector<int64_t> shape;
  shape.reserve(info.ndim);
  for (auto i : info.shape) {
    shape.push_back(static_cast<int64_t>(i));
  }
  auto nbytes = info.strides[0] * info.shape[0];
  auto arr = std::make_shared<mlx::data::Array>(dtype, shape);
  std::memcpy(arr->data(), info.ptr, nbytes);

  return arr;
}

std::shared_ptr<mlx::data::Array> to_array(py::array a) {
  // make sure "a" is contiguous
  const auto c_contiguous =
      py::detail::npy_api::constants::NPY_ARRAY_C_CONTIGUOUS_;
  if (!(c_contiguous == (a.flags() & c_contiguous))) {
    throw std::runtime_error(
        "[to_array] Contiguous array expected -- use numpy.ascontiguousarray()");
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
      shape.push_back(a.itemsize());
      return std::make_shared<mlx::data::Array>(
          mlx::data::ArrayType::Int8, shape, data);

    default: {
      std::ostringstream msg;
      msg << "[to_array] Unsupported array type '" << a.dtype().char_() << "'";
      throw std::invalid_argument(msg.str());
    }
  }
}

std::shared_ptr<mlx::data::Array> to_array(py::handle obj) {
  // A numpy array so we can actually ask for mutable data and avoid a copy.
  if (py::isinstance<py::array>(obj)) {
    return to_array(obj.cast<py::array>());
  }

  // Check for python scalars. Handle bytes and string here.
  if (py::isinstance<py::int_>(obj)) {
    return std::make_shared<mlx::data::Array>(obj.cast<int64_t>());
  }
  if (py::isinstance<py::float_>(obj)) {
    return std::make_shared<mlx::data::Array>(obj.cast<double>());
  }
  if (py::isinstance<py::bytes>(obj)) {
    return std::make_shared<mlx::data::Array>(obj.cast<std::string_view>());
  }
  if (py::isinstance<py::str>(obj)) {
    throw std::invalid_argument(
        "[to_array] Cannot convert strings to arrays. Please encode them as bytes first.");
  }

  // A buffer must be copied because the data may not be mutable eg a python
  // bytes object.
  if (py::isinstance<py::buffer>(obj)) {
    return to_array(obj.cast<py::buffer>());
  }

  // Attempt to cast to numpy array no matter what the type is. This should
  // handle all manner of types that define the array interface.
  try {
    return to_array(obj.cast<py::array>());
  } catch (std::invalid_argument e) {
    auto objtype = py::type::of(obj);
    std::ostringstream msg;
    msg << "[to_array] Cannot convert type "
        << py::str(objtype).cast<std::string>()
        << " to an array. Use a numpy array, a python buffer or scalar.";
    throw std::invalid_argument(msg.str());
  }
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
    res[key] = to_array(el.second);
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
