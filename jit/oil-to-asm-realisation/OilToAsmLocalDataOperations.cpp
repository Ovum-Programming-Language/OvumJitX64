#include <jit/OilCommandAsmCompiler.hpp>

namespace ovum::vm::jit {

void OilCommandAsmCompiler::InitializeLocalDataOperations() {
  // Loadlocal n
  // argument number (n) is placed to R11, void* pointer to local data in R13
  std::vector<AssemblyInstruction> load_local_asm = {
    {AsmCommand::SHL, {Register::R11, make_imm_arg(3)}},
    {AsmCommand::ADD, {Register::R11, Register::R13}},
    {AsmCommand::MOV, {Register::RAX, addr(Register::R11)}},
    {AsmCommand::PUSH, {Register::RAX}}
  };
  AddStandardAssembly("LoadLocal", std::move(load_local_asm));
  
  // Savelocal n
  // argument number (n) is placed to R11, void* pointer to local data in R13
  std::vector<AssemblyInstruction> save_local_asm = {
    {AsmCommand::SHL, {Register::R11, make_imm_arg(3)}},
    {AsmCommand::ADD, {Register::R11, Register::R13}},
    {AsmCommand::POP, {Register::RAX}},
    {AsmCommand::MOV, {addr(Register::R11), Register::RAX}}
  };
  AddStandardAssembly("SaveLocal", std::move(save_local_asm));
}

} // namespace ovum::vm::jit