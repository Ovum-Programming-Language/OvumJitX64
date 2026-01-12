#include <jit/OilCommandAsmCompiler.hpp>

namespace ovum::vm::jit {

void OilCommandAsmCompiler::InitializeByteOperations() {
    // ByteAdd: a + b
    std::vector<AssemblyInstruction> byte_add_asm = {
        {AsmCommand::POP, {Register::RBX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::ADD, {Register::AL, Register::BL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("ByteAdd", std::move(byte_add_asm));
    
    // ByteSubtract: a - b
    std::vector<AssemblyInstruction> byte_subtract_asm = {
        {AsmCommand::POP, {Register::RBX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::SUB, {Register::AL, Register::BL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("ByteSubtract", std::move(byte_subtract_asm));
    
    // ByteMultiply: a * b
    std::vector<AssemblyInstruction> byte_multiply_asm = {
        {AsmCommand::POP, {Register::RBX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MUL, {Register::BL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("ByteMultiply", std::move(byte_multiply_asm));
    
    // ByteDivide: a / b
    std::vector<AssemblyInstruction> byte_divide_asm = {
        {AsmCommand::POP, {Register::RBX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::XOR, {Register::AH, Register::AH}},
        {AsmCommand::DIV, {Register::BL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("ByteDivide", std::move(byte_divide_asm));
    
    // ByteModulo: a % b
    std::vector<AssemblyInstruction> byte_modulo_asm = {
        {AsmCommand::POP, {Register::RBX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::XOR, {Register::AH, Register::AH}},
        {AsmCommand::DIV, {Register::BL}},
        {AsmCommand::MOV, {Register::AL, Register::AH}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("ByteModulo", std::move(byte_modulo_asm));
    
    // ByteNegate: -a
    std::vector<AssemblyInstruction> byte_negate_asm = {
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::NEG, {Register::AL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("ByteNegate", std::move(byte_negate_asm));
    
    // ByteIncrement: a + 1
    std::vector<AssemblyInstruction> byte_increment_asm = {
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::INC, {Register::AL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("ByteIncrement", std::move(byte_increment_asm));
    
    // ByteDecrement: a - 1
    std::vector<AssemblyInstruction> byte_decrement_asm = {
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::DEC, {Register::AL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("ByteDecrement", std::move(byte_decrement_asm));
    
    // ByteEqual: a == b
    std::vector<AssemblyInstruction> byte_equal_asm = {
        {AsmCommand::POP, {Register::RBX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::CMP, {Register::AL, Register::BL}},
        {AsmCommand::MOV, {Register::RAX, static_cast<int64_t>(0)}},
        {AsmCommand::SETZ, {Register::AL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("ByteEqual", std::move(byte_equal_asm));
    
    // ByteNotEqual: a != b
    std::vector<AssemblyInstruction> byte_not_equal_asm = {
        {AsmCommand::POP, {Register::RBX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::CMP, {Register::AL, Register::BL}},
        {AsmCommand::MOV, {Register::RAX, static_cast<int64_t>(0)}},
        {AsmCommand::SETNZ, {Register::AL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("ByteNotEqual", std::move(byte_not_equal_asm));
    
    // ByteLessThan: a < b (беззнаковое сравнение)
    std::vector<AssemblyInstruction> byte_less_than_asm = {
        {AsmCommand::POP, {Register::RBX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::CMP, {Register::AL, Register::BL}},
        {AsmCommand::MOV, {Register::RAX, static_cast<int64_t>(0)}},
        {AsmCommand::SETB, {Register::AL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("ByteLessThan", std::move(byte_less_than_asm));
    
    // ByteLessEqual: a <= b
    std::vector<AssemblyInstruction> byte_less_equal_asm = {
        {AsmCommand::POP, {Register::RBX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::CMP, {Register::AL, Register::BL}},
        {AsmCommand::MOV, {Register::RAX, static_cast<int64_t>(0)}},
        {AsmCommand::SETBE, {Register::AL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("ByteLessEqual", std::move(byte_less_equal_asm));
    
    // ByteGreaterThan: a > b
    std::vector<AssemblyInstruction> byte_greater_than_asm = {
        {AsmCommand::POP, {Register::RBX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::CMP, {Register::AL, Register::BL}},
        {AsmCommand::MOV, {Register::RAX, static_cast<int64_t>(0)}},
        {AsmCommand::SETNBE, {Register::AL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("ByteGreaterThan", std::move(byte_greater_than_asm));
    
    // ByteGreaterEqual: a >= b
    std::vector<AssemblyInstruction> byte_greater_equal_asm = {
        {AsmCommand::POP, {Register::RBX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::CMP, {Register::AL, Register::BL}},
        {AsmCommand::MOV, {Register::RAX, static_cast<int64_t>(0)}},
        {AsmCommand::SETNB, {Register::AL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("ByteGreaterEqual", std::move(byte_greater_equal_asm));
    
    // ByteAnd: a & b
    std::vector<AssemblyInstruction> byte_and_asm = {
        {AsmCommand::POP, {Register::RBX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::AND, {Register::AL, Register::BL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("ByteAnd", std::move(byte_and_asm));
    
    // ByteOr: a | b
    std::vector<AssemblyInstruction> byte_or_asm = {
        {AsmCommand::POP, {Register::RBX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::OR, {Register::AL, Register::BL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("ByteOr", std::move(byte_or_asm));
    
    // ByteXor: a ^ b
    std::vector<AssemblyInstruction> byte_xor_asm = {
        {AsmCommand::POP, {Register::RBX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::XOR, {Register::AL, Register::BL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("ByteXor", std::move(byte_xor_asm));
    
    // ByteNot: ~a
    std::vector<AssemblyInstruction> byte_not_asm = {
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::NOT, {Register::AL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("ByteNot", std::move(byte_not_asm));
    
    // ByteLeftShift: a << b
    std::vector<AssemblyInstruction> byte_left_shift_asm = {
        {AsmCommand::POP, {Register::RCX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::SHL, {Register::AL, Register::CL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("ByteLeftShift", std::move(byte_left_shift_asm));
    
    // ByteRightShift: a >> b
    std::vector<AssemblyInstruction> byte_right_shift_asm = {
        {AsmCommand::POP, {Register::RCX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::SHR, {Register::AL, Register::CL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("ByteRightShift", std::move(byte_right_shift_asm));
}

} // namespace ovum::vm::jit