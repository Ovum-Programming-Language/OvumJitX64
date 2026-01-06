#include "JitExecutor.hpp"

namespace ovum::vm::jit {

bool JitExecutor::TryCompile() const {
  return false;
}

std::expected<void, std::runtime_error> JitExecutor::Run(
    std::stack<std::variant<int64_t, double, bool, char, uint8_t, void*>>& /* stack */) {
  return std::unexpected(std::runtime_error("JitExecutor::Run: not implemented"));
}

} // namespace ovum::vm::jit
