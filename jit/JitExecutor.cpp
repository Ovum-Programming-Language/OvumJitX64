#include "JitExecutor.hpp"

#include <iostream>
#include <jit/OilCommandAsmCompiler.hpp>
#include <jit/machine-code-runner/MachineCodeFunction.hpp>
#include <jit/oil-to-asm-realisation/AsmToBytes.hpp>
#include <jit/oil-to-asm-realisation/optimisers/PushPopOptimiser.hpp>

namespace ovum::vm::jit {

static uint64_t datatemp[512];

JitExecutor::JitExecutor(std::shared_ptr<std::vector<TokenPtr>> jit_body, const std::string& jit_function_name) :
    oil_body(std::move(jit_body)), m_func(std::nullopt), m_machinecode(nullptr) {
}

bool JitExecutor::TryCompile() {
  // std::cout << "TryCompile called" << std::endl;
  if (m_machinecode) {
    // Compilation already done
    return true;
  }

  // Function was not compiled, trying to do it now
  // Getting oil body
  auto oil_body_vec_ptr = this->oil_body.get();
  if (oil_body_vec_ptr == nullptr) {
    // No oil body provided, cannot compile nullptr
    return false;
  }

  // Some oil body provided
  auto oil_body_vec = *oil_body_vec_ptr;

  // Packing (parsing) oil commands, attaching arguments to them
  auto packed_oil_body_exp = PackOilCommands(oil_body_vec);
  if (!packed_oil_body_exp) {
    // Error during parsing commands. Perhaps, incorrect oil body provided.
    return false;
  }

  // Oil bytecode parsed correctly
  auto packed_oil_body = packed_oil_body_exp.value();

  // Compile oil bytecode to assembler code
  auto asm_body = optimize_push_pop_pairs(OilCommandAsmCompiler::Compile(packed_oil_body));

  // Compile assembler code to machine code
  AsmToBytes asmtobytes;
  auto machinecode_body = asmtobytes.Convert(asm_body);

  if (!machinecode_body) {
    // Something went wrong during assembler compilation
    return false;
  }

  // std:: cout << "Machine Bytes: " << std::endl;
  // for (auto c : machinecode_body.value()) {
  //   std::cout << std::hex << (uint64_t)c << " ";
  // }
  // std::cout << std::endl;

  m_machinecode = std::make_shared<code_vector>(machinecode_body.value());

  // std::cout << "TryCompile success" << std::endl;
  return true;
}

std::expected<void, std::runtime_error> JitExecutor::Run(execution_tree::PassedExecutionData& data) {
  if (!m_machinecode) {
    return std::unexpected(std::runtime_error("JitExecutor::Run: compiled function not found! Call TryCompile first!"));
  }

  auto _m_func = MachineCodeFunctionSolved(*m_machinecode);

  if (data.memory.stack_frames.empty()) {
    return std::unexpected(std::runtime_error("JitExecutor::Run: empty stack frames. No memory for local data!"));
  }

  size_t argc = data.memory.stack_frames.top().local_variables.size();
  uint64_t* argv = nullptr;

  if (argc != 0) {
    argv = new uint64_t[argc];
    uint64_t* argv_ptr_copy = argv;
    for (auto arg : data.memory.stack_frames.top().local_variables) {
      if (std::holds_alternative<int64_t>(arg)) {
        *argv_ptr_copy = std::bit_cast<uint64_t>(std::get<int64_t>(arg));
      } else if (std::holds_alternative<double>(arg)) {
        *argv_ptr_copy = std::bit_cast<uint64_t>(std::get<double>(arg));
      } else if (std::holds_alternative<bool>(arg)) {
        *argv_ptr_copy = static_cast<uint64_t>(std::get<bool>(arg));
      } else if (std::holds_alternative<char>(arg)) {
        *argv_ptr_copy = static_cast<uint64_t>(std::get<char>(arg));
      } else if (std::holds_alternative<uint8_t>(arg)) {
        *argv_ptr_copy = static_cast<uint64_t>(std::get<uint8_t>(arg));
      } else if (std::holds_alternative<void*>(arg)) {
        *argv_ptr_copy = reinterpret_cast<uint64_t>(std::get<void*>(arg));
      } else {
        if (argv) {
          delete argv;
        }
        return std::unexpected(std::runtime_error("JitExecutor::Run: unknown argument type in stack frame."));
      }
      // std::cout << "    arg: " << std::hex << *argv_ptr_copy << std::endl;
      ++argv_ptr_copy;
    }
  }

  // std::cout << "Run: running m_func" << std::endl;

  AsmDataBuffer data_buffer;
  _m_func(reinterpret_cast<void*>(&data_buffer), reinterpret_cast<uint64_t>(argc), reinterpret_cast<void*>(argv));

  // std::cout << "Run: func end, with result: " << std::hex << data_buffer.Result << std::endl;

  switch (res_type) {
    case JitExecutorResultType::PTR:
      data.memory.machine_stack.push(std::bit_cast<void*>(data_buffer.Result));
      break;
    case JitExecutorResultType::FLOAT:
      data.memory.machine_stack.push(std::bit_cast<double>(data_buffer.Result));
      break;
    case JitExecutorResultType::INT64:
      data.memory.machine_stack.push(std::bit_cast<int64_t>(data_buffer.Result));
      break;
    case JitExecutorResultType::BYTE:
      data.memory.machine_stack.push(static_cast<uint8_t>(data_buffer.Result & 0xFF));
      break;
    case JitExecutorResultType::BOOL:
      data.memory.machine_stack.push(data_buffer.Result == 0);
      break;
    case JitExecutorResultType::CHAR:
      data.memory.machine_stack.push(static_cast<char>(data_buffer.Result));
      break;
    case JitExecutorResultType::kVoid:
      // Nothing to push
      break;
  }

  if (argv) {
    delete argv;
  }

  return {};
}

} // namespace ovum::vm::jit
