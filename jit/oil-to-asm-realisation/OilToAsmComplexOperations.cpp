#include <jit/OilCommandAsmCompiler.hpp>

namespace ovum::vm::jit {

std::vector<AssemblyInstruction> CreateOperationCaller(CalledOperationCode op_code) {
  std::vector<AssemblyInstruction> result = {
    // Caller-saved registers:
    // RAX, RCX, RDX, RSI, RDI, R8, R9, R10, R11 (в обоих ABI)
    // For Windows additional: XMM0-XMM5
    
    // Save caller-saved regs
    {AsmCommand::MOV, {addr(Register::R14, AsmDataBuffer::GetOffset(Register::RAX)), Register::RAX}},
    {AsmCommand::MOV, {addr(Register::R14, AsmDataBuffer::GetOffset(Register::RCX)), Register::RCX}},
    {AsmCommand::MOV, {addr(Register::R14, AsmDataBuffer::GetOffset(Register::RDX)), Register::RDX}},
    {AsmCommand::MOV, {addr(Register::R14, AsmDataBuffer::GetOffset(Register::RSI)), Register::RSI}},
    {AsmCommand::MOV, {addr(Register::R14, AsmDataBuffer::GetOffset(Register::RDI)), Register::RDI}},
    {AsmCommand::MOV, {addr(Register::R14, AsmDataBuffer::GetOffset(Register::R8)), Register::R8}},
    {AsmCommand::MOV, {addr(Register::R14, AsmDataBuffer::GetOffset(Register::R9)), Register::R9}},
    {AsmCommand::MOV, {addr(Register::R14, AsmDataBuffer::GetOffset(Register::R10)), Register::R10}},
    {AsmCommand::MOV, {addr(Register::R14, AsmDataBuffer::GetOffset(Register::R11)), Register::R11}},
    
    #ifdef _WIN32
        // Windows x64
        
        // XMM0-5 (caller-saved for Windows)
        {AsmCommand::MOVSD, {addr(Register::R14, AsmDataBuffer::GetOffset(Register::XMM0)), Register::XMM0}},
        {AsmCommand::MOVSD, {addr(Register::R14, AsmDataBuffer::GetOffset(Register::XMM1)), Register::XMM1}},
        {AsmCommand::MOVSD, {addr(Register::R14, AsmDataBuffer::GetOffset(Register::XMM2)), Register::XMM2}},
        {AsmCommand::MOVSD, {addr(Register::R14, AsmDataBuffer::GetOffset(Register::XMM3)), Register::XMM3}},
        {AsmCommand::MOVSD, {addr(Register::R14, AsmDataBuffer::GetOffset(Register::XMM4)), Register::XMM4}},
        {AsmCommand::MOVSD, {addr(Register::R14, AsmDataBuffer::GetOffset(Register::XMM5)), Register::XMM5}},

        // Create shadow space
        {AsmCommand::SUB, {Register::RSP, make_imm_arg(ShadowSpaceSizeBytes)}},

        // Arguments for call
        // RCX = arg[0]
        {AsmCommand::MOV, {Register::RCX, Register::RSP}},
        // RDX = arg[1]
        {AsmCommand::MOV, {Register::RDX, static_cast<int64_t>(op_code)}},
        
    #else
        // System V ABI (Linux/macOS)
        
        // Create shadow space
        {AsmCommand::SUB, {Register::RSP, make_imm_arg(ShadowSpaceSizeBytes)}},
        
        // Arguments for call
        // RDI = arg[0]
        {AsmCommand::MOV, {Register::RDI, Register::RSP}},
        // RSI = arg[1]
        {AsmCommand::MOV, {Register::RSI, static_cast<int64_t>(op_code)}},

        // Test RSP for fix needed
        {AsmCommand::MOV, {Register::RAX, Register::RSP}},
        {AsmCommand::AND, {Register::RAX, make_imm_arg(0x8)}},
        {AsmCommand::MOV, {Register::R13, Register::RAX}},
        
        // fix the stack if needed
        {AsmCommand::SUB, {Register::RSP, Register::RAX}},
        
    #endif
    
    // Call function by pointer
    {AsmCommand::MOV, {Register::RAX, make_imm_arg(reinterpret_cast<uint64_t>(&AsmComplexOperationManager))}},
    {AsmCommand::CALL, {Register::RAX}},
    
    // Restore new RSP from function
    {AsmCommand::MOV, {Register::RSP, Register::RAX}},
    
    // Restore caller-saved registers
    {AsmCommand::MOV, {Register::R11, addr(Register::R14, AsmDataBuffer::GetOffset(Register::R11))}},
    {AsmCommand::MOV, {Register::R10, addr(Register::R14, AsmDataBuffer::GetOffset(Register::R10))}},
    {AsmCommand::MOV, {Register::R9,  addr(Register::R14, AsmDataBuffer::GetOffset(Register::R9))}},
    {AsmCommand::MOV, {Register::R8,  addr(Register::R14, AsmDataBuffer::GetOffset(Register::R8))}},
    {AsmCommand::MOV, {Register::RDI, addr(Register::R14, AsmDataBuffer::GetOffset(Register::RDI))}},
    {AsmCommand::MOV, {Register::RSI, addr(Register::R14, AsmDataBuffer::GetOffset(Register::RSI))}},
    {AsmCommand::MOV, {Register::RDX, addr(Register::R14, AsmDataBuffer::GetOffset(Register::RDX))}},
    {AsmCommand::MOV, {Register::RCX, addr(Register::R14, AsmDataBuffer::GetOffset(Register::RCX))}},
    {AsmCommand::MOV, {Register::RAX, addr(Register::R14, AsmDataBuffer::GetOffset(Register::RAX))}},
    
    #ifdef _WIN32
        // Restore XMM registers for Windows
        {AsmCommand::MOVSD, {Register::XMM5, addr(Register::R14, AsmDataBuffer::GetOffset(Register::XMM5))}},
        {AsmCommand::MOVSD, {Register::XMM4, addr(Register::R14, AsmDataBuffer::GetOffset(Register::XMM4))}},
        {AsmCommand::MOVSD, {Register::XMM3, addr(Register::R14, AsmDataBuffer::GetOffset(Register::XMM3))}},
        {AsmCommand::MOVSD, {Register::XMM2, addr(Register::R14, AsmDataBuffer::GetOffset(Register::XMM2))}},
        {AsmCommand::MOVSD, {Register::XMM1, addr(Register::R14, AsmDataBuffer::GetOffset(Register::XMM1))}},
        {AsmCommand::MOVSD, {Register::XMM0, addr(Register::R14, AsmDataBuffer::GetOffset(Register::XMM0))}},
    #else
        // System V: fix RSP if needed (func must return RSP without additional bytes)
        //{AsmCommand::ADD, {Register::RSP, Register::R13}},
    #endif
  };
  return result;
}

} // namespace ovum::vm::jit