#include <jit/machine-code-runner/AsmDataBuffer.hpp>

#include <cstddef>

namespace ovum::vm::jit {

uint64_t AsmDataBuffer::GetOffset(Register reg) {
  switch (reg) {
    case Register::RAX:
      return offsetof(AsmDataBuffer, RegisterRAXDataCell);
      break;
    case Register::RCX:
      return offsetof(AsmDataBuffer, RegisterRCXDataCell);
      break;
    case Register::RDX:
      return offsetof(AsmDataBuffer, RegisterRDXDataCell);
      break;
    case Register::RSI:
      return offsetof(AsmDataBuffer, RegisterRSIDataCell);
      break;
    case Register::RDI:
      return offsetof(AsmDataBuffer, RegisterRDIDataCell);
      break;
    case Register::R8:
      return offsetof(AsmDataBuffer, RegisterR8DataCell);
      break;
    case Register::R9:
      return offsetof(AsmDataBuffer, RegisterR9DataCell);
      break;
    case Register::R10:
      return offsetof(AsmDataBuffer, RegisterR10DataCell);
      break;
    case Register::R11:
      return offsetof(AsmDataBuffer, RegisterR11DataCell);
      break;
    case Register::RSP:
      return offsetof(AsmDataBuffer, RegisterRSPDataCell);
      break;
#ifdef _WIN32
    case Register::XMM0:
      return offsetof(AsmDataBuffer, RegisterXMM0DataCell);
      break;
    case Register::XMM1:
      return offsetof(AsmDataBuffer, RegisterXMM1DataCell);
      break;
    case Register::XMM2:
      return offsetof(AsmDataBuffer, RegisterXMM2DataCell);
      break;
    case Register::XMM3:
      return offsetof(AsmDataBuffer, RegisterXMM3DataCell);
      break;
    case Register::XMM4:
      return offsetof(AsmDataBuffer, RegisterXMM4DataCell);
      break;
    case Register::XMM5:
      return offsetof(AsmDataBuffer, RegisterXMM5DataCell);
      break;
#endif
    default:
      return GetResultOffset();
  }
}

uint64_t AsmDataBuffer::GetResultOffset() {
  return offsetof(AsmDataBuffer, Result);
}

} // namespace ovum::vm::jit
