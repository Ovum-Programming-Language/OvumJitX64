#include "MachineCodeRunner.hpp"

namespace ovum::vm::jit {

void MachineCodeRunner::Run(MachineCodeFunction<void(void*, uint64_t, void*)>& machine_code_func, execution_tree::PassedExecutionData& data) {
  AsmDataBuffer data_buffer;

  if (data.memory.stack_frames.empty()) {
    return;// TODO err handling
  }

  size_t argc = data.memory.stack_frames.top().local_variables.size();
  uint64_t* argv = nullptr;

  if (argc != 0) {
    argv = new uint64_t[argc];
    uint64_t* argv_ptr_copy = argv;
    for (auto arg : data.memory.stack_frames.top().local_variables) {
      if (std::holds_alternative<int64_t>(arg)) {
        *argv_ptr_copy = static_cast<uint64_t>(std::get<int64_t>(arg));
      } else if (std::holds_alternative<double>(arg)) {
        *argv_ptr_copy = static_cast<uint64_t>(std::get<double>(arg));
      } else if (std::holds_alternative<bool>(arg)) {
        *argv_ptr_copy = static_cast<uint64_t>(std::get<bool>(arg));
      } else if (std::holds_alternative<char>(arg)) {
        *argv_ptr_copy = static_cast<uint64_t>(std::get<char>(arg));
      } else if (std::holds_alternative<uint8_t>(arg)) {
        *argv_ptr_copy = static_cast<uint64_t>(std::get<uint8_t>(arg));
      } else if (std::holds_alternative<void*>(arg)) {
        *argv_ptr_copy = reinterpret_cast<uint64_t>(std::get<void*>(arg));
      } else {
        // TODO: error hadling
      }
      ++argv_ptr_copy;
    }
  }
  
  machine_code_func(reinterpret_cast<void*>(&data_buffer),
                    reinterpret_cast<uint64_t>(argc),
                    reinterpret_cast<void*>(argv));

  if (argv) {
    delete argv;
  }
}

} // namespace ovum::vm::jit
