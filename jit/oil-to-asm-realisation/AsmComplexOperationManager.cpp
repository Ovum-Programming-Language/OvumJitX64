#include "AsmComplexOperationManager.hpp"

#include <iostream>

#include <lib/execution_tree/BytecodeCommands.hpp>

namespace ovum::vm::jit {

void* AsmComplexOperationManager(void* rsp, CalledOperationCode op_code) {
  std::cout << "called from asm code" << std::endl;

  std::cout << "Called operation with op_code" << (uint64_t) op_code << std::endl;

  std::cout << "returning to asm code" << std::endl;
  return rsp + 32;
}

} // namespace ovum::vm::jit
