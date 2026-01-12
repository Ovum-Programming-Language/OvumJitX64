#include <jit/OilCommandAsmCompiler.hpp>

namespace ovum::vm::jit {

void OilCommandAsmCompiler::InitializeBooleanOperations() {
    // BoolAnd: a && b
    std::vector<AssemblyInstruction> bool_and_asm = {
        {AsmCommand::POP, {Register::RBX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::TEST, {Register::RAX, Register::RAX}},
        {AsmCommand::SETNZ, {Register::AL}},
        {AsmCommand::TEST, {Register::RBX, Register::RBX}},
        {AsmCommand::SETNZ, {Register::BL}},
        {AsmCommand::AND, {Register::AL, Register::BL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("BoolAnd", std::move(bool_and_asm));
    
    // BoolOr: a || b
    std::vector<AssemblyInstruction> bool_or_asm = {
        {AsmCommand::POP, {Register::RBX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::TEST, {Register::RAX, Register::RAX}},
        {AsmCommand::SETNZ, {Register::AL}},
        {AsmCommand::TEST, {Register::RBX, Register::RBX}},
        {AsmCommand::SETNZ, {Register::BL}},
        {AsmCommand::OR, {Register::AL, Register::BL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("BoolOr", std::move(bool_or_asm));
    
    // BoolNot: !a
    std::vector<AssemblyInstruction> bool_not_asm = {
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::TEST, {Register::RAX, Register::RAX}},
        {AsmCommand::SETZ, {Register::AL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("BoolNot", std::move(bool_not_asm));
    
    // BoolXor: a ^ b
    std::vector<AssemblyInstruction> bool_xor_asm = {
        {AsmCommand::POP, {Register::RBX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::TEST, {Register::RAX, Register::RAX}},
        {AsmCommand::SETNZ, {Register::AL}},
        {AsmCommand::TEST, {Register::RBX, Register::RBX}},
        {AsmCommand::SETNZ, {Register::BL}},
        {AsmCommand::XOR, {Register::AL, Register::BL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("BoolXor", std::move(bool_xor_asm));
    
    // BoolToByte: bool → byte (0/1)
    std::vector<AssemblyInstruction> bool_to_byte_asm = {
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("BoolToByte", std::move(bool_to_byte_asm));
}

} // namespace ovum::vm::jit