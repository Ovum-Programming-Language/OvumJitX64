#ifndef JIT_ASMCOMAPLEXOPERATIONMANAGER_HPP
#define JIT_ASMCOMAPLEXOPERATIONMANAGER_HPP

#include <stdint.h>

namespace ovum::vm::jit {

enum class CalledOperationCode : uint64_t {
  FLOAT_SQRT = 0x00000001,
  PRINT,
  PRINT_LINE
};

void* AsmComplexOperationManager(void* rsp, CalledOperationCode op_code);

} // namespace ovum::vm::jit

#endif // JIT_ASMCOMAPLEXOPERATIONMANAGER_HPP
