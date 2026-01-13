#ifndef JIT_MACHINECODERUNNER_HPP
#define JIT_MACHINECODERUNNER_HPP

#include <jit/machine-code-runner/MachineCodeFunction.hpp>
#include <lib/execution_tree/PassedExecutionData.hpp>
#include <jit/machine-code-runner/AsmDataBuffer.hpp>

namespace ovum::vm::jit {

class MachineCodeRunner {
public:
  static void Run(MachineCodeFunction<void(void*, uint64_t, void*)>& machine_code_func, execution_tree::PassedExecutionData& data);
private:

};

} // namespace ovum::vm::jit

#endif // JIT_MACHINECODERUNNER_HPP