#include "mlx/data/buffer/Perm.h"

namespace mlx {
namespace data {
namespace buffer {

Perm::Perm(const std::shared_ptr<Buffer>& op, const std::vector<int64_t>& perm)
    : op_(op) {
  set_perm_(perm);
}

Sample Perm::get(int64_t idx) const {
  if (idx < 0 || idx >= perm_.size()) {
    throw std::runtime_error("Perm: index out of range");
  }
  return op_->get(perm_[idx]);
}

int64_t Perm::size() const {
  return perm_.size();
}

void Perm::set_perm_(const std::vector<int64_t>& perm) {
  for (auto idx : perm) {
    if (idx < 0 || idx >= op_->size()) {
      throw std::runtime_error("Perm: permutation index out of range");
    }
  }
  perm_ = perm;
}

const std::vector<int64_t>& Perm::get_perm() {
  return perm_;
}

} // namespace buffer
} // namespace data
} // namespace mlx
