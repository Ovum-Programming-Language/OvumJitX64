#include <jit/OilCommandAsmCompiler.hpp>

namespace ovum::vm::jit {

void OilCommandAsmCompiler::InitializeIntegerOperations() {
  // IntAdd: a + b
  std::vector<AssemblyInstruction> int_add_asm = {{AsmCommand::POP, {Register::RBX}},
                                                  {AsmCommand::POP, {Register::RAX}},
                                                  {AsmCommand::ADD, {Register::RAX, Register::RBX}},
                                                  {AsmCommand::PUSH, {Register::RAX}}};
  AddStandardAssembly("IntAdd", std::move(int_add_asm));

  // IntSubtract: a - b
  std::vector<AssemblyInstruction> int_subtract_asm = {{AsmCommand::POP, {Register::RBX}},
                                                       {AsmCommand::POP, {Register::RAX}},
                                                       {AsmCommand::SUB, {Register::RAX, Register::RBX}},
                                                       {AsmCommand::PUSH, {Register::RAX}}};
  AddStandardAssembly("IntSubtract", std::move(int_subtract_asm));

  // IntMultiply: a * b
  std::vector<AssemblyInstruction> int_multiply_asm = {{AsmCommand::POP, {Register::RBX}},
                                                       {AsmCommand::POP, {Register::RAX}},
                                                       {AsmCommand::IMUL, {Register::RAX, Register::RBX}},
                                                       {AsmCommand::PUSH, {Register::RAX}}};
  AddStandardAssembly("IntMultiply", std::move(int_multiply_asm));

  // IntDivide: a / b
  std::vector<AssemblyInstruction> int_divide_asm = {{AsmCommand::POP, {Register::RBX}},
                                                     {AsmCommand::POP, {Register::RAX}},
                                                     {AsmCommand::MOV, {Register::RDX, Register::RAX}},
                                                     {AsmCommand::SAR, {Register::RDX, static_cast<int64_t>(63)}},
                                                     {AsmCommand::IDIV, {Register::RBX}},
                                                     {AsmCommand::PUSH, {Register::RAX}}};
  AddStandardAssembly("IntDivide", std::move(int_divide_asm));

  // IntModulo: a % b
  std::vector<AssemblyInstruction> int_modulo_asm = {{AsmCommand::POP, {Register::RBX}},
                                                     {AsmCommand::POP, {Register::RAX}},
                                                     {AsmCommand::MOV, {Register::RDX, Register::RAX}},
                                                     {AsmCommand::SAR, {Register::RDX, static_cast<int64_t>(63)}},
                                                     {AsmCommand::IDIV, {Register::RBX}},
                                                     {AsmCommand::PUSH, {Register::RDX}}};
  AddStandardAssembly("IntModulo", std::move(int_modulo_asm));

  // IntNegate: -a
  std::vector<AssemblyInstruction> int_negate_asm = {{AsmCommand::POP, {Register::RAX}},
                                                     {AsmCommand::NEG, {Register::RAX}},
                                                     {AsmCommand::PUSH, {Register::RAX}}};
  AddStandardAssembly("IntNegate", std::move(int_negate_asm));

  // IntIncrement: a + 1
  std::vector<AssemblyInstruction> int_increment_asm = {{AsmCommand::POP, {Register::RAX}},
                                                        {AsmCommand::INC, {Register::RAX}},
                                                        {AsmCommand::PUSH, {Register::RAX}}};
  AddStandardAssembly("IntIncrement", std::move(int_increment_asm));

  // IntDecrement: a - 1
  std::vector<AssemblyInstruction> int_decrement_asm = {{AsmCommand::POP, {Register::RAX}},
                                                        {AsmCommand::DEC, {Register::RAX}},
                                                        {AsmCommand::PUSH, {Register::RAX}}};
  AddStandardAssembly("IntDecrement", std::move(int_decrement_asm));

  // IntEqual: a == b
  std::vector<AssemblyInstruction> int_equal_asm = {{AsmCommand::POP, {Register::RBX}},
                                                    {AsmCommand::POP, {Register::RAX}},
                                                    {AsmCommand::CMP, {Register::RAX, Register::RBX}},
                                                    {AsmCommand::MOV, {Register::RAX, static_cast<int64_t>(0)}},
                                                    {AsmCommand::SETZ, {Register::AL}},
                                                    {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
                                                    {AsmCommand::PUSH, {Register::RAX}}};
  AddStandardAssembly("IntEqual", std::move(int_equal_asm));

  // IntNotEqual: a != b
  std::vector<AssemblyInstruction> int_not_equal_asm = {{AsmCommand::POP, {Register::RBX}},
                                                        {AsmCommand::POP, {Register::RAX}},
                                                        {AsmCommand::CMP, {Register::RAX, Register::RBX}},
                                                        {AsmCommand::MOV, {Register::RAX, static_cast<int64_t>(0)}},
                                                        {AsmCommand::SETNZ, {Register::AL}},
                                                        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
                                                        {AsmCommand::PUSH, {Register::RAX}}};
  AddStandardAssembly("IntNotEqual", std::move(int_not_equal_asm));

  // IntLessThan: a < b
  std::vector<AssemblyInstruction> int_less_than_asm = {{AsmCommand::POP, {Register::RBX}},
                                                        {AsmCommand::POP, {Register::RAX}},
                                                        {AsmCommand::CMP, {Register::RAX, Register::RBX}},
                                                        {AsmCommand::MOV, {Register::RAX, static_cast<int64_t>(0)}},
                                                        {AsmCommand::SETL, {Register::AL}},
                                                        {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
                                                        {AsmCommand::PUSH, {Register::RAX}}};
  AddStandardAssembly("IntLessThan", std::move(int_less_than_asm));

  // IntLessEqual: a <= b
  std::vector<AssemblyInstruction> int_less_equal_asm = {{AsmCommand::POP, {Register::RBX}},
                                                         {AsmCommand::POP, {Register::RAX}},
                                                         {AsmCommand::CMP, {Register::RAX, Register::RBX}},
                                                         {AsmCommand::MOV, {Register::RAX, static_cast<int64_t>(0)}},
                                                         {AsmCommand::SETLE, {Register::AL}},
                                                         {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
                                                         {AsmCommand::PUSH, {Register::RAX}}};
  AddStandardAssembly("IntLessEqual", std::move(int_less_equal_asm));

  // IntGreaterThan: a > b
  std::vector<AssemblyInstruction> int_greater_than_asm = {{AsmCommand::POP, {Register::RBX}},
                                                           {AsmCommand::POP, {Register::RAX}},
                                                           {AsmCommand::CMP, {Register::RAX, Register::RBX}},
                                                           {AsmCommand::MOV, {Register::RAX, static_cast<int64_t>(0)}},
                                                           {AsmCommand::SETNLE, {Register::AL}},
                                                           {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
                                                           {AsmCommand::PUSH, {Register::RAX}}};
  AddStandardAssembly("IntGreaterThan", std::move(int_greater_than_asm));

  // IntGreaterEqual: a >= b
  std::vector<AssemblyInstruction> int_greater_equal_asm = {
      {AsmCommand::POP, {Register::RBX}},
      {AsmCommand::POP, {Register::RAX}},
      {AsmCommand::CMP, {Register::RAX, Register::RBX}},
      {AsmCommand::MOV, {Register::RAX, static_cast<int64_t>(0)}},
      {AsmCommand::SETNL, {Register::AL}},
      {AsmCommand::MOVZX, {Register::RAX, Register::AL}},
      {AsmCommand::PUSH, {Register::RAX}}};
  AddStandardAssembly("IntGreaterEqual", std::move(int_greater_equal_asm));

  // IntAnd: a & b
  std::vector<AssemblyInstruction> int_and_asm = {{AsmCommand::POP, {Register::RBX}},
                                                  {AsmCommand::POP, {Register::RAX}},
                                                  {AsmCommand::AND, {Register::RAX, Register::RBX}},
                                                  {AsmCommand::PUSH, {Register::RAX}}};
  AddStandardAssembly("IntAnd", std::move(int_and_asm));

  // IntOr: a | b
  std::vector<AssemblyInstruction> int_or_asm = {{AsmCommand::POP, {Register::RBX}},
                                                 {AsmCommand::POP, {Register::RAX}},
                                                 {AsmCommand::OR, {Register::RAX, Register::RBX}},
                                                 {AsmCommand::PUSH, {Register::RAX}}};
  AddStandardAssembly("IntOr", std::move(int_or_asm));

  // IntXor: a ^ b
  std::vector<AssemblyInstruction> int_xor_asm = {{AsmCommand::POP, {Register::RBX}},
                                                  {AsmCommand::POP, {Register::RAX}},
                                                  {AsmCommand::XOR, {Register::RAX, Register::RBX}},
                                                  {AsmCommand::PUSH, {Register::RAX}}};
  AddStandardAssembly("IntXor", std::move(int_xor_asm));

  // IntNot: ~a
  std::vector<AssemblyInstruction> int_not_asm = {{AsmCommand::POP, {Register::RAX}},
                                                  {AsmCommand::NOT, {Register::RAX}},
                                                  {AsmCommand::PUSH, {Register::RAX}}};
  AddStandardAssembly("IntNot", std::move(int_not_asm));

  // IntLeftShift: a << b
  std::vector<AssemblyInstruction> int_left_shift_asm = {{AsmCommand::POP, {Register::RCX}},
                                                         {AsmCommand::POP, {Register::RAX}},
                                                         {AsmCommand::SHL, {Register::RAX, Register::CL}},
                                                         {AsmCommand::PUSH, {Register::RAX}}};
  AddStandardAssembly("IntLeftShift", std::move(int_left_shift_asm));

  // IntRightShift: a >> b
  std::vector<AssemblyInstruction> int_right_shift_asm = {{AsmCommand::POP, {Register::RCX}},
                                                          {AsmCommand::POP, {Register::RAX}},
                                                          {AsmCommand::SAR, {Register::RAX, Register::CL}},
                                                          {AsmCommand::PUSH, {Register::RAX}}};
  AddStandardAssembly("IntRightShift", std::move(int_right_shift_asm));
}

} // namespace ovum::vm::jit
