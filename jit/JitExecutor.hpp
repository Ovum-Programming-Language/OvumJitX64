#ifndef JIT_JITEXECUTOR_HPP
#define JIT_JITEXECUTOR_HPP

#include <tokens/Token.hpp>

#include <jit/machine-code-runner/MachineCodeFunction.hpp>
#include <optional>
#include "jit/AsmCompiler.hpp"
#include "lib/executor/IJitExecutor.hpp"

namespace ovum::vm::jit {

using MachineCodeFunctionSolved = MachineCodeFunction<void(void*, uint64_t, void*)>;
using MachineCodeFunctionSolvedOpt = std::optional<MachineCodeFunctionSolved>;

enum JitExecutorResultType : uint8_t { PTR, FLOAT, INT64, BYTE, BOOL, CHAR, kVoid };

class JitExecutor : public executor::IJitExecutor {
public:
  JitExecutor(std::shared_ptr<std::vector<TokenPtr>> jit_body, const std::string& jit_function_name);

  [[nodiscard]] bool TryCompile() override;

  [[nodiscard]] std::expected<void, std::runtime_error> Run(execution_tree::PassedExecutionData& data) override;

private:
  std::shared_ptr<std::vector<TokenPtr>> oil_body;
  std::shared_ptr<code_vector> m_machinecode;
  MachineCodeFunctionSolvedOpt m_func;
  JitExecutorResultType res_type = JitExecutorResultType::PTR;
};

} // namespace ovum::vm::jit

#endif // JIT_JITEXECUTOR_HPP
