#include "JitExecutor.hpp"

#include <iostream>
#include <jit/oil-to-asm-realisation/AsmToBytes.hpp>
#include <jit/machine-code-runner/MachineCodeFunction.hpp>
#include <jit/machine-code-runner/MachineCodeRunner.hpp>
#include <jit/OilCommandAsmCompiler.hpp>

namespace ovum::vm::jit {

static uint64_t datatemp[512];

JitExecutor::JitExecutor(std::shared_ptr<std::vector<TokenPtr>> jit_body) : oil_body(std::move(jit_body)),
m_func(std::nullopt) {
}

bool JitExecutor::TryCompile() {
  auto oil_body_vec_ptr = this->oil_body.get();

  if (oil_body_vec_ptr == nullptr) {
    return false;
  }

  auto oil_body_vec = *oil_body_vec_ptr;

  auto packed_oil_body_exp = PackOilCommands(oil_body_vec);

  if (!packed_oil_body_exp) {
    return false;
  }

  auto packed_oil_body = packed_oil_body_exp.value();

  std::vector<AssemblyInstruction> asm_body = {{AsmCommand::MOV, {Register::R14, make_imm_arg(reinterpret_cast<int64_t>(datatemp))}}};
  auto asm_body2 = CreateOperationCaller(CalledOperationCode::PRINT_LINE);
  
  for (auto c : asm_body) {
    std::cout << (uint64_t)c.command << std::endl;
  }

  asm_body.insert(asm_body.end(), asm_body2.begin(), asm_body2.end()); 

  AsmToBytes asmtobytes;
  auto machinecode_body = asmtobytes.Convert(asm_body);

  if (!machinecode_body){
    return false;
  }
  m_func = MachineCodeFunctionSolved(code_vector(machinecode_body.value()));

  return true;
}

std::expected<void, std::runtime_error> JitExecutor::Run(execution_tree::PassedExecutionData& data) {
  if (!m_func) {
    return std::unexpected(std::runtime_error("JitExecutor::Run: compiled function not found! Call TryCompile first!"));
  }
  MachineCodeRunner::Run(m_func.value(), data);
}

} // namespace ovum::vm::jit
