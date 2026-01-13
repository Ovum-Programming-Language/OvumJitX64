#ifndef JIT_JITEXECUTOR_HPP
#define JIT_JITEXECUTOR_HPP

#include <tokens/Token.hpp>

#include "jit/AsmCompiler.hpp"
#include "lib/executor/IJitExecutor.hpp"
#include <jit/machine-code-runner/MachineCodeFunction.hpp>
#include <optional>

namespace ovum::vm::jit {

using MachineCodeFunctionSolved = MachineCodeFunction<void(void*, uint64_t, void*)>;
using MachineCodeFunctionSolvedOpt = std::optional<MachineCodeFunctionSolved>;

class JitExecutor : public executor::IJitExecutor {
public:
  JitExecutor(std::shared_ptr<std::vector<TokenPtr>> jit_body);

  [[nodiscard]] bool TryCompile() const override;

  [[nodiscard]] std::expected<void, std::runtime_error> Run(execution_tree::PassedExecutionData& data) override;

private:
  std::shared_ptr<std::vector<TokenPtr>> oil_body;
  MachineCodeFunctionSolvedOpt m_func;
};

} // namespace ovum::vm::jit

#endif // JIT_JITEXECUTOR_HPP
