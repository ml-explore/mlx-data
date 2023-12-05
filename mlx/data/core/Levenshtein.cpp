// Copyright © 2023 Apple Inc.

#include "mlx/data/core/Levenshtein.h"

#include <array>
#include <vector>

namespace mlx {
namespace data {
namespace core {

inline int64_t sum(const std::array<int64_t, 3>& vec) {
  return vec[0] + vec[1] + vec[2];
}

inline int argmin(const std::array<int64_t, 3>& arr) {
  if ((arr[0] <= arr[1]) && (arr[0] <= arr[2])) {
    return 0;
  }
  if ((arr[1] <= arr[0]) && (arr[1] <= arr[2])) {
    return 1;
  }
  return 2;
}

template <class T>
std::array<int64_t, 3>
levenshtein_t(const T* arr1, int64_t len1, const T* arr2, int64_t len2) {
  std::vector<std::array<int64_t, 3>> vals(len1 + 1, {0, 0, 0});
  for (int64_t i = 0; i < len1 + 1; i++) {
    vals[i][1] = i;
  }
  std::array<int64_t, 3> cases;
  for (int64_t idx2 = 0; idx2 < len2; idx2++) {
    auto diag = vals[0];
    vals[0][0] = idx2 + 1;
    for (int64_t idx1 = 0; idx1 < len1; idx1++) {
      auto prevdiag = vals[idx1 + 1];
      cases[0] = sum(vals[idx1 + 1]) + 1;
      cases[1] = sum(vals[idx1]) + 1;
      cases[2] = sum(diag) + (arr1[idx1] != arr2[idx2]);
      auto mincase = argmin(cases);
      switch (mincase) {
        case 0: // del error
          vals[idx1 + 1][0] += 1;
          break;
        case 1: // ins error
          vals[idx1 + 1] = vals[idx1];
          vals[idx1 + 1][1] += 1;
          break;
        default:
          vals[idx1 + 1] = diag;
          // sub error?
          if (arr1[idx1] != arr2[idx2]) {
            vals[idx1 + 1][2] += 1;
          }
      }
      diag = prevdiag;
    }
  }

  return vals.back();
}

template <class T>
void levenshtein_arr(
    void* result,
    const void* arr1,
    const void* len1,
    int64_t maxlen1,
    int64_t stride1,
    const void* arr2,
    const void* len2,
    int64_t maxlen2,
    int64_t stride2,
    int64_t size) {
  auto result_t = reinterpret_cast<int64_t*>(result);
  auto arr1_t = reinterpret_cast<const T*>(arr1);
  auto arr2_t = reinterpret_cast<const T*>(arr2);
  auto len1_t = reinterpret_cast<const int64_t*>(len1);
  auto len2_t = reinterpret_cast<const int64_t*>(len2);
  for (int64_t n = 0; n < size; n++) {
    if ((len1_t[n] > maxlen1) || (len2_t[n] > maxlen2)) {
      throw std::runtime_error(
          "levenshtein: provided length exceeds input shape");
    }
    auto result_vec = levenshtein_t(
        arr1_t + n * stride1, len1_t[n], arr2_t + n * stride2, len2_t[n]);
    for (int i = 0; i < 3; i++) {
      result_t[i + n * 3] = result_vec[i];
    }
  }
}

std::shared_ptr<Array> levenshtein(
    const std::shared_ptr<Array> arr1,
    const std::shared_ptr<Array> len1,
    const std::shared_ptr<Array> arr2,
    const std::shared_ptr<Array> len2) {
  if (arr1->type() != arr2->type()) {
    throw std::runtime_error("levenshtein: inconsistent array type");
  }
  if (len1->type() != ArrayType::Int64 || len2->type() != ArrayType::Int64) {
    throw std::runtime_error("levenshtein: length should be int64");
  }
  if (arr1->ndim() != arr2->ndim()) {
    throw std::runtime_error("levenshtein: inconsistent array dimension");
  }
  if (len1->ndim() != 1 || len2->ndim() != 1) {
    throw std::runtime_error("levenshtein: length arrays should be 1d");
  }
  if (len1->shape(0) != len2->shape(0)) {
    throw std::runtime_error("levenshtein: inconsistent length size");
  }
  std::shared_ptr<Array> res;
  if (arr1->ndim() == 1) {
    if (len1->shape(0) != 1) {
      throw std::runtime_error(
          "levenshtein: inconsistent array/length dimension");
    }
    res = std::make_shared<Array>(ArrayType::Int64, 3);
    ARRAY_DISPATCH(
        arr1,
        levenshtein_arr,
        res->data(),
        arr1->data(),
        len1->data(),
        arr1->shape(0),
        0,
        arr2->data(),
        len2->data(),
        arr2->shape(0),
        0,
        1);
  } else if (arr1->ndim() == 2) {
    if (len1->shape(0) != arr1->shape(0) || len2->shape(0) != arr2->shape(0)) {
      throw std::runtime_error(
          "levenshtein: inconsistent array/length dimension");
    }
    res = std::make_shared<Array>(ArrayType::Int64, len1->shape(0), 3);
    ARRAY_DISPATCH(
        arr1,
        levenshtein_arr,
        res->data(),
        arr1->data(),
        len1->data(),
        arr1->shape(1),
        arr1->shape(1),
        arr2->data(),
        len2->data(),
        arr2->shape(1),
        arr2->shape(1),
        len1->shape(0));
  } else {
    throw std::runtime_error("levenshtein: 1d or 2d array (batch) expected");
  }
  return res;
}

} // namespace core
} // namespace data
} // namespace mlx
