#include <jit/OilCommandAsmCompiler.hpp>

namespace ovum::vm::jit {

void OilCommandAsmCompiler::InitializeFloatOperations() {
    // FloatAdd: a + b
    std::vector<AssemblyInstruction> float_add_asm = {
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MOVQ, {Register::XMM1, Register::RAX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MOVQ, {Register::XMM0, Register::RAX}},
        {AsmCommand::ADDSD, {Register::XMM0, Register::XMM1}},
        {AsmCommand::MOVQ, {Register::RAX, Register::XMM0}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("FloatAdd", std::move(float_add_asm));
    
    // FloatSubtract: a - b
    std::vector<AssemblyInstruction> float_subtract_asm = {
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MOVQ, {Register::XMM1, Register::RAX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MOVQ, {Register::XMM0, Register::RAX}},
        {AsmCommand::SUBSD, {Register::XMM0, Register::XMM1}},
        {AsmCommand::MOVQ, {Register::RAX, Register::XMM0}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("FloatSubtract", std::move(float_subtract_asm));
    
    // FloatMultiply: a * b
    std::vector<AssemblyInstruction> float_multiply_asm = {
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MOVQ, {Register::XMM1, Register::RAX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MOVQ, {Register::XMM0, Register::RAX}},
        {AsmCommand::MULSD, {Register::XMM0, Register::XMM1}},
        {AsmCommand::MOVQ, {Register::RAX, Register::XMM0}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("FloatMultiply", std::move(float_multiply_asm));
    
    // FloatDivide: a / b
    std::vector<AssemblyInstruction> float_divide_asm = {
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MOVQ, {Register::XMM1, Register::RAX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MOVQ, {Register::XMM0, Register::RAX}},
        {AsmCommand::DIVSD, {Register::XMM0, Register::XMM1}},
        {AsmCommand::MOVQ, {Register::RAX, Register::XMM0}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("FloatDivide", std::move(float_divide_asm));
    
    // FloatNegate: -a
    std::vector<AssemblyInstruction> float_negate_asm = {
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MOVQ, {Register::XMM0, Register::RAX}},
        {AsmCommand::XOR, {Register::XMM1, Register::XMM1}},
        {AsmCommand::SUBSD, {Register::XMM1, Register::XMM0}},
        {AsmCommand::MOVQ, {Register::RAX, Register::XMM1}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("FloatNegate", std::move(float_negate_asm));
    
    // FloatSqrt: sqrt(a) 
    //std::vector<AssemblyInstruction> float_sqrt_asm = {
    //    {AsmCommand::POP, {Register::RAX}},
    //    {AsmCommand::MOVQ, {Register::XMM0, Register::RAX}},
    //    // sqrtsd xmm0, xmm0
    //    // Нужна команда SQRTSD
    //    {AsmCommand::CALL, {std::string("_sqrt")}}, // Временное решение
    //    {AsmCommand::MOVQ, {Register::RAX, Register::XMM0}},
    //    {AsmCommand::PUSH, {Register::RAX}}
    //};
    //AddStandardAssembly("FloatSqrt", std::move(float_sqrt_asm));
    
    // FloatEqual: a == b (возвращает bool)
    std::vector<AssemblyInstruction> float_equal_asm = {
        // Снимаем операнды
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MOVQ, {Register::XMM1, Register::RAX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MOVQ, {Register::XMM0, Register::RAX}},
        // Сравнение
        {AsmCommand::CMP, {Register::XMM0, Register::XMM1}},
        // Подготовка bool результата
        {AsmCommand::MOV, {Register::RAX, static_cast<int64_t>(0)}},
        {AsmCommand::SETZ, {Register::AL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        // Помещаем результат на стек
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("FloatEqual", std::move(float_equal_asm));
    
    // FloatNotEqual: a != b
    std::vector<AssemblyInstruction> float_not_equal_asm = {
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MOVQ, {Register::XMM1, Register::RAX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MOVQ, {Register::XMM0, Register::RAX}},
        {AsmCommand::CMP, {Register::XMM0, Register::XMM1}},
        {AsmCommand::MOV, {Register::RAX, static_cast<int64_t>(0)}},
        {AsmCommand::SETNZ, {Register::AL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("FloatNotEqual", std::move(float_not_equal_asm));
    
    // FloatLessThan: a < b
    std::vector<AssemblyInstruction> float_less_than_asm = {
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MOVQ, {Register::XMM1, Register::RAX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MOVQ, {Register::XMM0, Register::RAX}},
        {AsmCommand::CMP, {Register::XMM0, Register::XMM1}},
        {AsmCommand::MOV, {Register::RAX, static_cast<int64_t>(0)}},
        {AsmCommand::SETB, {Register::AL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("FloatLessThan", std::move(float_less_than_asm));
    
    // FloatLessEqual: a <= b
    std::vector<AssemblyInstruction> float_less_equal_asm = {
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MOVQ, {Register::XMM1, Register::RAX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MOVQ, {Register::XMM0, Register::RAX}},
        {AsmCommand::CMP, {Register::XMM0, Register::XMM1}},
        {AsmCommand::MOV, {Register::RAX, static_cast<int64_t>(0)}},
        {AsmCommand::SETBE, {Register::AL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("FloatLessEqual", std::move(float_less_equal_asm));
    
    // FloatGreaterThan: a > b
    std::vector<AssemblyInstruction> float_greater_than_asm = {
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MOVQ, {Register::XMM1, Register::RAX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MOVQ, {Register::XMM0, Register::RAX}},
        {AsmCommand::CMP, {Register::XMM0, Register::XMM1}},
        {AsmCommand::MOV, {Register::RAX, static_cast<int64_t>(0)}},
        {AsmCommand::SETNBE, {Register::AL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("FloatGreaterThan", std::move(float_greater_than_asm));
    
    // FloatGreaterEqual: a >= b
    std::vector<AssemblyInstruction> float_greater_equal_asm = {
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MOVQ, {Register::XMM1, Register::RAX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::MOVQ, {Register::XMM0, Register::RAX}},
        {AsmCommand::CMP, {Register::XMM0, Register::XMM1}},
        {AsmCommand::MOV, {Register::RAX, static_cast<int64_t>(0)}},
        {AsmCommand::SETNB, {Register::AL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("FloatGreaterEqual", std::move(float_greater_equal_asm));
}

} // namespace ovum::vm::jit