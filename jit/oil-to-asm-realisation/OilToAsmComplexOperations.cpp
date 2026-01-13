#include <jit/OilCommandAsmCompiler.hpp>

namespace ovum::vm::jit {

std::vector<AssemblyInstruction> CreateOperationCaller(CalledOperationCode op_code) {
  std::vector<AssemblyInstruction> result = {
    // Save caller-saved registers
    {AsmCommand::MOV, {addr(Register::R14, static_cast<int64_t>(0)), Register::RAX}},
    {AsmCommand::MOV, {addr(Register::R14, static_cast<int64_t>(8)), Register::RCX}},
    {AsmCommand::MOV, {addr(Register::R14, static_cast<int64_t>(16)), Register::RDX}},
    {AsmCommand::MOV, {addr(Register::R14, static_cast<int64_t>(24)), Register::RSI}},
    {AsmCommand::MOV, {addr(Register::R14, static_cast<int64_t>(32)), Register::RDI}},
    {AsmCommand::MOV, {addr(Register::R14, static_cast<int64_t>(40)), Register::R8}},
    {AsmCommand::MOV, {addr(Register::R14, static_cast<int64_t>(48)), Register::R9}},
    {AsmCommand::MOV, {addr(Register::R14, static_cast<int64_t>(56)), Register::R10}},
    {AsmCommand::MOV, {addr(Register::R14, static_cast<int64_t>(64)), Register::R11}},
    
    // Create shadow space [4 x 64 bit (8 byte) memory cells]
    {AsmCommand::MOV, {Register::RAX, make_imm_arg(reinterpret_cast<uint64_t>(&AsmComplexOperationManager))}},
    {AsmCommand::SUB, {Register::RSP, make_imm_arg(32)}},
    
    // Adding op_code    to arg[1] for function manager
    {AsmCommand::MOV, {Register::RSI, static_cast<int64_t>(op_code)}},
    
    // Adding RSP        to arg[0] for function manager
    {AsmCommand::MOV, {Register::RDI, Register::RSP}},
    
    // Call cpp function manager
    {AsmCommand::CALL, {Register::RAX}},
    {AsmCommand::MOV, {Register::RSP, Register::RAX}},
    
    // Save new stack pointer
    // WARNING! Callee MUST return correct RSP as a result!
    {AsmCommand::MOV, {Register::RSP, Register::RAX}},
    
    // Restore caller-saved registers
    {AsmCommand::MOV, {Register::R11, addr(Register::R14, static_cast<int64_t>(64))}},
    {AsmCommand::MOV, {Register::R10, addr(Register::R14, static_cast<int64_t>(56))}},
    {AsmCommand::MOV, {Register::R9,  addr(Register::R14, static_cast<int64_t>(48))}},
    {AsmCommand::MOV, {Register::R8,  addr(Register::R14, static_cast<int64_t>(40))}},
    {AsmCommand::MOV, {Register::RDI, addr(Register::R14, static_cast<int64_t>(32))}},
    {AsmCommand::MOV, {Register::RSI, addr(Register::R14, static_cast<int64_t>(24))}},
    {AsmCommand::MOV, {Register::RDX, addr(Register::R14, static_cast<int64_t>(16))}},
    {AsmCommand::MOV, {Register::RCX, addr(Register::R14, static_cast<int64_t>(8))}},
    {AsmCommand::MOV, {Register::RAX, addr(Register::R14, static_cast<int64_t>(0))}},
    
    // Return
    {AsmCommand::RET, {}}
  };
  return result;
}

} // namespace ovum::vm::jit