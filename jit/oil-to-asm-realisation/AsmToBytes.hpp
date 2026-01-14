#ifndef JIT_ASMTOBYTES_HPP
#define JIT_ASMTOBYTES_HPP

#include <cstdint>
#include <expected>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "jit/AsmData.hpp"
#include <jit/machine-code-runner/ExecutableMemory.hpp>

namespace ovum::vm::jit {

class AsmToBytes {
public:
  AsmToBytes();

  // Convert assembly instructions to machine code bytes
  std::expected<code_vector, std::runtime_error> Convert(
      const std::vector<AssemblyInstruction>& instructions);

  // Get label addresses (for resolving jump targets)
  const std::map<std::string, size_t>& GetLabelAddresses() const {
    return label_addresses_;
  }

private:
  // Encode a single instruction
  std::expected<void, std::runtime_error> EncodeInstruction(const AssemblyInstruction& instr,
                                                            std::vector<uint8_t>& output);

  // Encode register to ModR/M byte
  uint8_t EncodeRegister(Register reg) const;

  // Get register size (8, 16, 32, 64 bits)
  uint8_t GetRegisterSize(Register reg) const;

  // Check if register is extended (R8-R15)
  bool IsExtendedRegister(Register reg) const;
  bool IsXMMRegister(Register reg) const;
  uint8_t GetSSEPrefix(AsmCommand cmd) const;
  uint16_t GetSSEOpcode(AsmCommand cmd) const;
  uint16_t GetMOVQOpcode(bool xmm_to_reg) const;

  // Encode immediate value
  void EncodeImmediate(int64_t value, uint8_t size, std::vector<uint8_t>& output);
  void EncodeImmediate(uint64_t value, uint8_t size, std::vector<uint8_t>& output);

  // Encode memory address
  void EncodeMemoryAddress(const MemoryAddress& addr, std::vector<uint8_t>& output, uint8_t reg_field);
  void EncodeMemoryAddressWithReg(const MemoryAddress& mem, 
                                  std::vector<uint8_t>& output, 
                                  uint8_t reg_field,
                                  uint8_t base_low3,
                                  uint8_t index_low3);

  // Encode specific instruction types
  std::expected<void, std::runtime_error> EncodeMov(const AssemblyInstruction& instr,
                                                     std::vector<uint8_t>& output);
  std::expected<void, std::runtime_error> EncodeMOVQ(const AssemblyInstruction& instr,
                                                                std::vector<uint8_t>& output);
  std::expected<void, std::runtime_error> EncodeArithmetic(const AssemblyInstruction& instr,
                                                            std::vector<uint8_t>& output);
  std::expected<void, std::runtime_error> EncodeJump(const AssemblyInstruction& instr,
                                                       std::vector<uint8_t>& output);
  void EncodeStackOp(const AssemblyInstruction& instr, std::vector<uint8_t>& output);
  std::expected<void, std::runtime_error> EncodeSSE2(const AssemblyInstruction& instr,
                                                      std::vector<uint8_t>& output);
  void EncodeLabel(const AssemblyInstruction& instr, std::vector<uint8_t>& output);

  // Get opcode for instruction
  std::vector<uint8_t> GetOpcode(AsmCommand cmd, Register reg1, Register reg2) const;

  // Label address map (label name -> byte offset)
  std::map<std::string, size_t> label_addresses_;

  // Positions where jump offsets need to be patched (position -> label name)
  std::vector<std::pair<size_t, std::string>> jump_patches_;

  // Current output position (for label resolution)
  size_t current_position_;
};

} // namespace ovum::vm::jit

#endif // JIT_ASMTOBYTES_HPP
