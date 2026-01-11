#ifndef JIT_JITEXECUTOR_HPP
#define JIT_JITEXECUTOR_HPP

#include <tokens/Token.hpp>

#include "lib/executor/IJitExecutor.hpp"
#include "jit/AsmCompiler.hpp"

namespace ovum::vm::jit {

class JitExecutor : public executor::IJitExecutor {
public:
  JitExecutor(std::shared_ptr<std::vector<TokenPtr>> jit_body);

  [[nodiscard]] bool TryCompile() const override;

  [[nodiscard]] std::expected<void, std::runtime_error> Run(
      execution_tree::PassedExecutionData& data) override;

private:
  std::shared_ptr<std::vector<TokenPtr>> oil_body;
};

} // namespace ovum::vm::jit

#endif // JIT_JITEXECUTOR_HPP
