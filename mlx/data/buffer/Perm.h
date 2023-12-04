// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/buffer/Buffer.h"

namespace mlx {
namespace data {
namespace buffer {

class Perm : public Buffer {
 public:
  Perm(const std::shared_ptr<Buffer>& op, const std::vector<int64_t>& perm);

  Sample get(int64_t idx) const override;
  virtual int64_t size() const override;

  const std::vector<int64_t>& get_perm();

 private:
  std::shared_ptr<Buffer> op_;
  std::vector<int64_t> perm_;

  // unsafe if it was public
  void set_perm_(const std::vector<int64_t>& perm);
};

} // namespace buffer
} // namespace data
} // namespace mlx
