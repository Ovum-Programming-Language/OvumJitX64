#include "JitExecutor.hpp"

#include <iostream>
#include <jit/oil-to-asm-realisation/AsmToBytes.hpp>
#include <jit/machine-code-runner/MachineCodeFunction.hpp>
#include <jit/machine-code-runner/MachineCodeRunner.hpp>
#include <jit/OilCommandAsmCompiler.hpp>

namespace ovum::vm::jit {

static uint64_t datatemp[512];

JitExecutor::JitExecutor(std::shared_ptr<std::vector<TokenPtr>> jit_body) : oil_body(std::move(jit_body)),
m_func(std::nullopt), m_machinecode(nullptr) {
}

bool JitExecutor::TryCompile() {
  std::cout << "TryCompile called" << std::endl;
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
  auto asm_body = OilCommandAsmCompiler::Compile(packed_oil_body);

  // Compile assembler code to machine code
  AsmToBytes asmtobytes;
  auto machinecode_body = asmtobytes.Convert(asm_body);

  if (!machinecode_body){
    // Something went wrong during assembler compilation
    return false;
  }

  std:: cout << "Machine Bytes: " << std::endl;
  for (auto c : machinecode_body.value()) {
    std::cout << std::hex << (uint64_t)c << " ";
  }
  std::cout << std::endl;
  
  m_machinecode = std::make_shared<code_vector>(machinecode_body.value());

  std::cout << "TryCompile success" << std::endl;
  return true;
}

std::expected<void, std::runtime_error> JitExecutor::Run(execution_tree::PassedExecutionData& data) {
  
  std::cout << "JitExecutor::Run called" << std::endl;


  if (!m_machinecode) {
    return std::unexpected(std::runtime_error("JitExecutor::Run: compiled function not found! Call TryCompile first!"));
  }

  auto _m_func = MachineCodeFunctionSolved(*m_machinecode);

  MachineCodeRunner::Run(_m_func, data);

  return {};
}

} // namespace ovum::vm::jit
