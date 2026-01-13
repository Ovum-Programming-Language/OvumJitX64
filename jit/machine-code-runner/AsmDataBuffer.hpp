#ifndef JIT_CALLERDATABUFFER_HPP
#define JIT_CALLERDATABUFFER_HPP

#include <stdint.h>

#include <jit/AsmData.hpp>

namespace ovum::vm::jit {

struct AsmDataBuffer {
  //      |        Reg        | Def   | Offset (Bytes)
  uint64_t RegisterRAXDataCell = 0; //| 0
  uint64_t RegisterRCXDataCell = 0; //| 8
  uint64_t RegisterRDXDataCell = 0; //| 16
  uint64_t RegisterRSIDataCell = 0; //| 24
  uint64_t RegisterRDIDataCell = 0; //| 32
  uint64_t RegisterR8DataCell = 0;  //| 40
  uint64_t RegisterR9DataCell = 0;  //| 48
  uint64_t RegisterR10DataCell = 0; //| 56
  uint64_t RegisterR11DataCell = 0; //| 64
  uint64_t RegisterRSPDataCell = 0; //| 72
  uint64_t Reserved = 0; //           | 80
  #ifdef _WIN32
  uint64_t RegisterXMM0DataCell = 0; //| 88
  uint64_t RegisterXMM1DataCell = 0; //| 96
  uint64_t RegisterXMM2DataCell = 0; //| 104
  uint64_t RegisterXMM3DataCell = 0; //| 112
  uint64_t RegisterXMM4DataCell = 0; //| 120
  uint64_t RegisterXMM5DataCell = 0; //| 128
  #endif

  AsmDataBuffer() = default;
  ~AsmDataBuffer() = default;

  static uint64_t GetOffset(Register reg);
};

} // namespace ovum::vm::jit

#endif
