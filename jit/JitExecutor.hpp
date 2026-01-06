#ifndef JIT_JITEXECUTOR_HPP
#define JIT_JITEXECUTOR_HPP

#include "lib/executor/IJitExecutor.hpp"

namespace ovum::vm::jit {

class JitExecutor : public executor::IJitExecutor {
public:
  JitExecutor() = default;

  [[nodiscard]] bool TryCompile() const override;

  [[nodiscard]] std::expected<void, std::runtime_error> Run(
      std::stack<std::variant<int64_t, double, bool, char, uint8_t, void*>>& stack) override;
};

} // namespace ovum::vm::jit

#endif // JIT_JITEXECUTOR_HPP
