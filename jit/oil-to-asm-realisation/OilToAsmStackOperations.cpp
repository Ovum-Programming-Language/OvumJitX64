#include <jit/OilCommandAsmCompiler.hpp>

namespace ovum::vm::jit {

void OilCommandAsmCompiler::InitializeStackOperations() {
    // PushNull
    std::vector<AssemblyInstruction> push_null_asm = {
        {AsmCommand::MOV, {Register::RAX, static_cast<int64_t>(0)}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("PushNull", std::move(push_null_asm));
    
    // Pop
    std::vector<AssemblyInstruction> pop_asm = {
        {AsmCommand::POP, {Register::RAX}}
    };
    AddStandardAssembly("Pop", std::move(pop_asm));
    
    // Dup
    std::vector<AssemblyInstruction> dup_asm = {
        {AsmCommand::MOV, {Register::RAX, create_memory_addr(Register::RSP)}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("Dup", std::move(dup_asm));
    
    // Swap
    std::vector<AssemblyInstruction> swap_asm = {
        {AsmCommand::POP, {Register::RBX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::PUSH, {Register::RBX}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("Swap", std::move(swap_asm));
    
    // IsNull
    std::vector<AssemblyInstruction> is_null_asm = {
        {AsmCommand::MOV, {Register::RAX, create_memory_addr(Register::RSP)}},
        {AsmCommand::TEST, {Register::RAX, Register::RAX}},
        {AsmCommand::SETZ, {Register::AL}},
        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
        {AsmCommand::MOV, {create_memory_addr(Register::RSP), Register::RAX}}
    };
    AddStandardAssembly("IsNull", std::move(is_null_asm));
    
    // Unwrap
    //std::vector<AssemblyInstruction> unwrap_asm = {
    //    {AsmCommand::MOV, {Register::RAX, create_memory_addr(Register::RSP)}},
    //    {AsmCommand::TEST, {Register::RAX, Register::RAX}},
    //    {AsmCommand::JE, {std::string("_panic_null")}},
    //};
    //AddStandardAssembly("Unwrap", std::move(unwrap_asm));
    
    // NullCoalesce: a ?? b
    std::vector<AssemblyInstruction> null_coalesce_asm = {
        {AsmCommand::POP, {Register::RBX}},
        {AsmCommand::POP, {Register::RAX}},
        {AsmCommand::TEST, {Register::RAX, Register::RAX}},
        {AsmCommand::CMOVE, {Register::RAX, Register::RBX}},
        {AsmCommand::PUSH, {Register::RAX}}
    };
    AddStandardAssembly("NullCoalesce", std::move(null_coalesce_asm));
}

} // namespace ovum::vm::jit
