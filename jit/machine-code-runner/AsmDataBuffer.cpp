#include <jit/machine-code-runner/AsmDataBuffer.hpp>

namespace ovum::vm::jit {

uint64_t AsmDataBuffer::GetOffset(Register reg) {
  switch (reg) {
    case Register::RAX:
      return 0;
      break;
    case Register::RCX:
      return 8;
      break;
    case Register::RDX:
      return 16;
      break;
    case Register::RSI:
      return 24;
      break;
    case Register::RDI:
      return 32;
      break;
    case Register::R8:
      return 40;
      break;
    case Register::R9:
      return 48;
      break;
    case Register::R10:
      return 56;
      break;
    case Register::R11:
      return 64;
      break;
    case Register::RSP:
      return 72;
      break;
    #ifdef _WIN32
    case Register::XMM0:
      return 32;
      break;
    case Register::XMM1:
      return 40;
      break;
    case Register::XMM2:
      return 48;
      break;
    case Register::XMM3:
      return 56;
      break;
    case Register::XMM4:
      return 64;
      break;
    case Register::XMM5:
      return 72;
      break;
    #endif
    default:
      return 80;
  }
}

} // namespace ovum::vm::jit
