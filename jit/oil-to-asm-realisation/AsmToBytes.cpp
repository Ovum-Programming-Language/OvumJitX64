#include "AsmToBytes.hpp"

#include <cstring>
#include <stdexcept>

#include <iostream>

namespace ovum::vm::jit {

AsmToBytes::AsmToBytes() : current_position_(0) {
}

std::expected<code_vector, std::runtime_error> AsmToBytes::Convert(
    const std::vector<AssemblyInstruction>& instructions) {
  code_vector output;
  label_addresses_.clear();
  jump_patches_.clear();
  current_position_ = 0;

  // First pass: encode all instructions and collect label addresses
  for (const auto& instr : instructions) {
    if (instr.command == AsmCommand::LABEL) {
      // Store label address at current position
      if (instr.arguments.size() > 0) {
        if (auto label = instr.get_argument<std::string>(0)) {
          label_addresses_[*label] = current_position_;
        }
      }
      // Labels don't generate code, continue to next instruction
      continue;
    }

    // Encode instruction
    size_t before = output.size();
    auto encode_result = EncodeInstruction(instr, output);
    if (!encode_result) {
      return std::unexpected(encode_result.error());
    }

    //for (auto c : output) {
    //  std::cout << std::hex << (uint64_t)c << " ";
    //}
    //std::cout << std::endl;
    size_t encoded_size = output.size() - before;
    current_position_ += encoded_size;
  }

  // Second pass: patch all jump offsets
  for (const auto& [offset_pos, label_name] : jump_patches_) {
    auto label_it = label_addresses_.find(label_name);
    if (label_it == label_addresses_.end()) {
      return std::unexpected(std::runtime_error("Label not found: " + label_name));
    }

    size_t label_address = label_it->second;
    // Offset is relative to the byte AFTER the offset bytes (end of instruction)
    // offset_pos is where the 4-byte offset starts, so end of instruction is offset_pos + 4
    size_t jump_address = offset_pos + 4;
    int64_t relative_offset = static_cast<int64_t>(label_address) - static_cast<int64_t>(jump_address);

    // Patch the 32-bit offset at the stored position
    if (offset_pos + 4 > output.size()) {
      return std::unexpected(std::runtime_error("Invalid patch position for label: " + label_name));
    }

    // Write little-endian 32-bit offset
    output[offset_pos] = static_cast<uint8_t>(relative_offset & 0xFF);
    output[offset_pos + 1] = static_cast<uint8_t>((relative_offset >> 8) & 0xFF);
    output[offset_pos + 2] = static_cast<uint8_t>((relative_offset >> 16) & 0xFF);
    output[offset_pos + 3] = static_cast<uint8_t>((relative_offset >> 24) & 0xFF);
  }

  return output;
}

std::expected<void, std::runtime_error> AsmToBytes::EncodeInstruction(const AssemblyInstruction& instr,
                                                                       std::vector<uint8_t>& output) {
  switch (instr.command) {
    // Move operations
    case AsmCommand::MOV:
    case AsmCommand::MOVSX:
    case AsmCommand::MOVZX:
    case AsmCommand::LEA:
    case AsmCommand::XCHG:
    case AsmCommand::MOVQ: {
      auto result = EncodeMov(instr, output);
      if (!result) {
        return result;
      }
      break;
    }

    // Arithmetic operations
    case AsmCommand::ADD:
    case AsmCommand::SUB:
    case AsmCommand::IMUL:
    case AsmCommand::MUL:
    case AsmCommand::IDIV:
    case AsmCommand::DIV:
    case AsmCommand::INC:
    case AsmCommand::DEC:
    case AsmCommand::NEG:
    case AsmCommand::AND:
    case AsmCommand::OR:
    case AsmCommand::XOR:
    case AsmCommand::NOT:
    case AsmCommand::TEST:
    case AsmCommand::SHL:
    case AsmCommand::SHR:
    case AsmCommand::SAR:
    case AsmCommand::ROL:
    case AsmCommand::ROR:
    case AsmCommand::RCL:
    case AsmCommand::RCR:
    case AsmCommand::CMP: {
      auto result = EncodeArithmetic(instr, output);
      if (!result) {
        return result;
      }
      break;
    }

    // Jump operations
    case AsmCommand::JMP:
    case AsmCommand::CALL:
    case AsmCommand::RET:
    case AsmCommand::JE:
    case AsmCommand::JNE:
    case AsmCommand::JG:
    case AsmCommand::JGE:
    case AsmCommand::JL:
    case AsmCommand::JLE:
    case AsmCommand::JA:
    case AsmCommand::JAE:
    case AsmCommand::JB:
    case AsmCommand::JBE:
    case AsmCommand::LOOP:
    case AsmCommand::LOOPE:
    case AsmCommand::LOOPNE: {
      auto result = EncodeJump(instr, output);
      if (!result) {
        return result;
      }
      break;
    }

    // Stack operations
    case AsmCommand::PUSH:
    case AsmCommand::POP:
    case AsmCommand::PUSHF:
    case AsmCommand::POPF:
      EncodeStackOp(instr, output);
      break;

    // SSE2 operations
    case AsmCommand::ADDSD:
    case AsmCommand::SUBSD:
    case AsmCommand::MULSD:
    case AsmCommand::DIVSD:
    case AsmCommand::SQRTSD:
    case AsmCommand::COMISD:
    case AsmCommand::UCOMISD:
    case AsmCommand::CVTSI2SD:
    case AsmCommand::CVTSD2SI:
    case AsmCommand::CVTSS2SD:
    case AsmCommand::CVTSD2SS:
    case AsmCommand::MOVSD:
    case AsmCommand::MOVAPD:
    case AsmCommand::MOVUPD:
    case AsmCommand::ANDPD:
    case AsmCommand::ANDNPD:
    case AsmCommand::ORPD:
    case AsmCommand::XORPD:
    case AsmCommand::CVTTSD2SI:
    case AsmCommand::CVTTSD2SIQ: {
      auto result = EncodeSSE2(instr, output);
      if (!result) {
        return result;
      }
      break;
    }

    // Labels
    case AsmCommand::LABEL:
      EncodeLabel(instr, output);
      break;

    // SETcc operations
    case AsmCommand::SETO:
    case AsmCommand::SETNO:
    case AsmCommand::SETB:
    case AsmCommand::SETNB:
    case AsmCommand::SETZ:
    case AsmCommand::SETNZ:
    case AsmCommand::SETBE:
    case AsmCommand::SETNBE:
    case AsmCommand::SETS:
    case AsmCommand::SETNS:
    case AsmCommand::SETP:
    case AsmCommand::SETNP:
    case AsmCommand::SETL:
    case AsmCommand::SETNL:
    case AsmCommand::SETLE:
    case AsmCommand::SETNLE:
      // SETcc r/m8
      if (instr.arguments.size() >= 1) {
        if (auto reg = instr.get_argument<Register>(0)) {
          uint8_t opcode = 0x0F;
          uint8_t setcc_opcode = 0x90; // Base for SETcc

          switch (instr.command) {
            case AsmCommand::SETO:
              setcc_opcode = 0x90;
              break;
            case AsmCommand::SETNO:
              setcc_opcode = 0x91;
              break;
            case AsmCommand::SETB:
              setcc_opcode = 0x92;
              break;
            case AsmCommand::SETNB:
              setcc_opcode = 0x93;
              break;
            case AsmCommand::SETZ:
              setcc_opcode = 0x94;
              break;
            case AsmCommand::SETNZ:
              setcc_opcode = 0x95;
              break;
            case AsmCommand::SETBE:
              setcc_opcode = 0x96;
              break;
            case AsmCommand::SETNBE:
              setcc_opcode = 0x97;
              break;
            case AsmCommand::SETS:
              setcc_opcode = 0x98;
              break;
            case AsmCommand::SETNS:
              setcc_opcode = 0x99;
              break;
            case AsmCommand::SETP:
              setcc_opcode = 0x9A;
              break;
            case AsmCommand::SETNP:
              setcc_opcode = 0x9B;
              break;
            case AsmCommand::SETL:
              setcc_opcode = 0x9C;
              break;
            case AsmCommand::SETNL:
              setcc_opcode = 0x9D;
              break;
            case AsmCommand::SETLE:
              setcc_opcode = 0x9E;
              break;
            case AsmCommand::SETNLE:
              setcc_opcode = 0x9F;
              break;
            default:
              break;
          }

          output.push_back(opcode);
          output.push_back(setcc_opcode);

          uint8_t modrm = 0xC0; // Register mode
          modrm |= EncodeRegister(*reg);
          output.push_back(modrm);
        }
      }
      break;

    // Conditional moves
    case AsmCommand::CMOVE:
    case AsmCommand::CMOVNE:
    case AsmCommand::CMOVB:
    case AsmCommand::CMOVBE:
    case AsmCommand::CMOVA:
    case AsmCommand::CMOVAE:
      // CMOVcc r64, r/m64
      if (instr.arguments.size() >= 2) {
        if (auto reg1 = instr.get_argument<Register>(0)) {
          if (auto reg2 = instr.get_argument<Register>(1)) {
            output.push_back(0x48); // REX.W prefix
            output.push_back(0x0F);

            uint8_t cmov_opcode = 0x40; // Base for CMOVcc
            switch (instr.command) {
              case AsmCommand::CMOVE:
                cmov_opcode = 0x44;
                break;
              case AsmCommand::CMOVNE:
                cmov_opcode = 0x45;
                break;
              case AsmCommand::CMOVB:
                cmov_opcode = 0x42;
                break;
              case AsmCommand::CMOVBE:
                cmov_opcode = 0x46;
                break;
              case AsmCommand::CMOVA:
                cmov_opcode = 0x47;
                break;
              case AsmCommand::CMOVAE:
                cmov_opcode = 0x43;
                break;
              default:
                break;
            }

            output.push_back(cmov_opcode);

            uint8_t modrm = 0xC0;
            modrm |= (EncodeRegister(*reg1) << 3);
            modrm |= EncodeRegister(*reg2);
            output.push_back(modrm);
          }
        }
      }
      break;

    // Miscellaneous
    case AsmCommand::NOP:
      output.push_back(0x90);
      break;

    case AsmCommand::HLT:
      output.push_back(0xF4);
      break;

    case AsmCommand::CLC:
      output.push_back(0xF8);
      break;

    case AsmCommand::STC:
      output.push_back(0xF9);
      break;

    case AsmCommand::CMC:
      output.push_back(0xF5);
      break;

    case AsmCommand::CQO:
      output.push_back(0x48); // REX.W
      output.push_back(0x99);
      break;

    case AsmCommand::SYSCALL:
      output.push_back(0x0F);
      output.push_back(0x05);
      break;

    default:
      return std::unexpected(
          std::runtime_error("Unsupported instruction: " + std::to_string(static_cast<uint16_t>(instr.command))));
  }

  return {};
}

uint8_t AsmToBytes::EncodeRegister(Register reg) const {
  uint8_t value = static_cast<uint8_t>(reg);

  // Map register enum to ModR/M register encoding
  if (value >= 0 && value <= 15) {
    // RAX-R15 (64-bit) - need special handling for R8-R15
    return value & 0x0F; // RAX-RDI: 0-7
  } else if (value >= 0x20 && value <= 0x2F) {
    // EAX-R15D (32-bit)
    if (value >= 0x28) { // R8D-R15D
      return (value - 0x28) & 0x0F;
    }
    return (value - 0x20) & 0x0F; // EAX-EDI
  } else if (value >= 0x40 && value <= 0x4F) {
    // AX-R15W (16-bit)
    if (value >= 0x48) { // R8W-R15W
      return (value - 0x48) & 0x0F;
    }
    return (value - 0x40) & 0x0F; // AX-DI
  } else if (value >= 0x60 && value <= 0x6F) {
    // AL-R15B (8-bit)
    if (value >= 0x68) { // R8B-R15B
      return (value - 0x68) & 0x0F;
    }
    return (value - 0x60) & 0x0F; // AL-DIL
  } else if (value >= 0x80 && value <= 0x8F) {
    // XMM0-XMM15 (SSE registers)
    return (value - 0x80) & 0x0F; // Only 3 bits needed for ModR/M
  } else if (value >= 0x90 && value <= 0x9F) {
    // XMM0-XMM15 (для GetRegisterSize)
    return (value - 0x90) & 0x0F;
  }

  return value & 0x0F;
}

uint8_t AsmToBytes::GetRegisterSize(Register reg) const {
  uint8_t value = static_cast<uint8_t>(reg);

  // Check ranges for each size
  if (value >= 0 && value <= 15) {  // RAX-R15 (64-bit)
    return 64;
  } else if (value >= 0x20 && value <= 0x2F) {  // EAX-R15D (32-bit)
    return 32;
  } else if (value >= 0x40 && value <= 0x4F) {  // AX-R15W (16-bit)
    return 16;
  } else if (value >= 0x60 && value <= 0x6F) {  // AL-R15B (8-bit)
    return 8;
  } else if (value >= 0x80 && value <= 0x9F) {  // XMM0-XMM15 (128-bit)
    return 128;
  }

  return 64; // Default
}

bool AsmToBytes::IsXMMRegister(Register reg) const {
  uint8_t value = static_cast<uint8_t>(reg);
  return (value >= 0x80 && value <= 0x8F) || (value >= 0x90 && value <= 0x9F);
}

bool AsmToBytes::IsExtendedRegister(Register reg) const {
  uint8_t value = static_cast<uint8_t>(reg);
  // Check if register is R8-R15 or their derivatives
  return (value >= 8 && value <= 15) ||        // R8-R15 (64-bit)
         (value >= 0x28 && value <= 0x2F) ||   // R8D-R15D (32-bit)
         (value >= 0x48 && value <= 0x4F) ||   // R8W-R15W (16-bit)
         (value >= 0x68 && value <= 0x6F) ||   // R8B-R15B (8-bit)
         (value >= 0x88 && value <= 0x8F) ||   // XMM8-XMM15 (старый диапазон)
         (value >= 0x98 && value <= 0x9F);     // XMM8-XMM15 (новый диапазон)
}

uint8_t AsmToBytes::GetSSEPrefix(AsmCommand cmd) const {
  switch (cmd) {
    case AsmCommand::ADDSD:
    case AsmCommand::SUBSD:
    case AsmCommand::MULSD:
    case AsmCommand::DIVSD:
    case AsmCommand::SQRTSD:
    case AsmCommand::COMISD:
    case AsmCommand::UCOMISD:
    case AsmCommand::CVTSI2SD:
    case AsmCommand::CVTSD2SI:
    case AsmCommand::CVTTSD2SI:
    case AsmCommand::CVTTSD2SIQ:
    case AsmCommand::MOVSD:
      return 0xF2; // REPNE prefix for scalar double-precision
    case AsmCommand::MOVAPD:
    case AsmCommand::MOVUPD:
    case AsmCommand::ANDPD:
    case AsmCommand::ANDNPD:
    case AsmCommand::ORPD:
    case AsmCommand::XORPD:
      return 0x66; // Operand size prefix for packed double-precision
    case AsmCommand::CVTSS2SD:
    case AsmCommand::CVTSD2SS:
      return 0xF3; // REP prefix for scalar single-precision
    default:
      return 0x00;
  }
}

uint16_t AsmToBytes::GetSSEOpcode(AsmCommand cmd) const {
  switch (cmd) {
    case AsmCommand::ADDSD: return 0x580F;
    case AsmCommand::SUBSD: return 0x5C0F;
    case AsmCommand::MULSD: return 0x590F;
    case AsmCommand::DIVSD: return 0x5E0F;
    case AsmCommand::SQRTSD: return 0x510F;
    case AsmCommand::COMISD: return 0x2F0F;
    case AsmCommand::UCOMISD: return 0x2E0F;
    case AsmCommand::CVTSI2SD: return 0x2A0F;
    case AsmCommand::CVTSD2SI: return 0x2D0F;
    case AsmCommand::CVTSS2SD: return 0x5A0F;
    case AsmCommand::CVTSD2SS: return 0x5A0F;
    case AsmCommand::MOVSD: return 0x100F;
    case AsmCommand::MOVAPD: return 0x280F;
    case AsmCommand::MOVUPD: return 0x100F;
    case AsmCommand::ANDPD: return 0x540F;
    case AsmCommand::ANDNPD: return 0x550F;
    case AsmCommand::ORPD: return 0x560F;
    case AsmCommand::XORPD: return 0x570F;
    case AsmCommand::CVTTSD2SI: return 0x2C0F;
    case AsmCommand::CVTTSD2SIQ: return 0x2C0F;
    default: return 0x0000;
  }
}

uint16_t AsmToBytes::GetMOVQOpcode(bool xmm_to_reg) const {
  // MOVQ has different opcodes depending on direction:
  // MOVQ xmm, r/m64: 0x6E0F (66 0F 6E for xmm from r/m64)
  // MOVQ r/m64, xmm: 0x7E0F (66 0F 7E for xmm to r/m64)
  return xmm_to_reg ? 0x6E0F : 0x7E0F;
}

void AsmToBytes::EncodeImmediate(int64_t value, uint8_t size, std::vector<uint8_t>& output) {
  if (size == 8) {
    output.push_back(static_cast<uint8_t>(value & 0xFF));
  } else if (size == 16) {
    uint16_t v = static_cast<uint16_t>(value);
    output.push_back(v & 0xFF);
    output.push_back((v >> 8) & 0xFF);
  } else if (size == 32) {
    uint32_t v = static_cast<uint32_t>(value);
    output.push_back(v & 0xFF);
    output.push_back((v >> 8) & 0xFF);
    output.push_back((v >> 16) & 0xFF);
    output.push_back((v >> 24) & 0xFF);
  } else if (size == 64) {
    uint64_t v = static_cast<uint64_t>(value);
    output.push_back(v & 0xFF);
    output.push_back((v >> 8) & 0xFF);
    output.push_back((v >> 16) & 0xFF);
    output.push_back((v >> 24) & 0xFF);
    output.push_back((v >> 32) & 0xFF);
    output.push_back((v >> 40) & 0xFF);
    output.push_back((v >> 48) & 0xFF);
    output.push_back((v >> 56) & 0xFF);
  }
}

void AsmToBytes::EncodeImmediate(uint64_t value, uint8_t size, std::vector<uint8_t>& output) {
  if (size == 8) {
    output.push_back(static_cast<uint8_t>(value & 0xFF));
  } else if (size == 16) {
    output.push_back(value & 0xFF);
    output.push_back((value >> 8) & 0xFF);
  } else if (size == 32) {
    output.push_back(value & 0xFF);
    output.push_back((value >> 8) & 0xFF);
    output.push_back((value >> 16) & 0xFF);
    output.push_back((value >> 24) & 0xFF);
  } else if (size == 64) {
    output.push_back(value & 0xFF);
    output.push_back((value >> 8) & 0xFF);
    output.push_back((value >> 16) & 0xFF);
    output.push_back((value >> 24) & 0xFF);
    output.push_back((value >> 32) & 0xFF);
    output.push_back((value >> 40) & 0xFF);
    output.push_back((value >> 48) & 0xFF);
    output.push_back((value >> 56) & 0xFF);
  }
}

void AsmToBytes::EncodeMemoryAddress(const MemoryAddress& addr, std::vector<uint8_t>& output, uint8_t reg_field) {
  uint8_t modrm = 0x00;
  modrm |= (reg_field << 3);

  bool has_base = addr.base.has_value();
  bool has_index = addr.index.has_value();

  if (!has_base && !has_index) {
    // Direct address (displacement only)
    modrm |= 0x05; // Mod=00, R/M=101 (SIB with no base)
    output.push_back(modrm);

    // SIB byte
    output.push_back(0x25); // Scale=00, Index=100 (none), Base=101 (none)

    // 32-bit displacement
    int32_t disp = static_cast<int32_t>(addr.displacement);
    EncodeImmediate(static_cast<int64_t>(disp), 32, output);
  } else if (has_base && !has_index) {
    uint8_t base_reg = EncodeRegister(*addr.base);

    if (base_reg == 5 && addr.displacement == 0) {
      // Special case: [RBP] requires Mod=01 with 8-bit displacement of 0
      modrm |= 0x45; // Mod=01, R/M=101
      output.push_back(modrm);
      output.push_back(0x00);
    } else if (addr.displacement == 0 && base_reg != 5) {
      modrm |= base_reg; // Mod=00
      output.push_back(modrm);
    } else if (addr.displacement >= -128 && addr.displacement <= 127) {
      modrm |= 0x40; // Mod=01
      modrm |= base_reg;
      output.push_back(modrm);
      EncodeImmediate(addr.displacement, 8, output);
    } else {
      modrm |= 0x80; // Mod=10
      modrm |= base_reg;
      output.push_back(modrm);
      EncodeImmediate(addr.displacement, 32, output);
    }
  } else {
    // Has index (SIB required)
    uint8_t base_reg = has_base ? EncodeRegister(*addr.base) : 5;
    uint8_t index_reg = EncodeRegister(*addr.index);

    uint8_t scale = 0;
    if (addr.scale == 1)
      scale = 0;
    else if (addr.scale == 2)
      scale = 1;
    else if (addr.scale == 4)
      scale = 2;
    else if (addr.scale == 8)
      scale = 3;

    // SIB encoding rules:
    // - If no base (base_reg=5) and Mod=00: base=5 means "no base", 32-bit displacement required
    // - If base is RBP (base_reg=5) and Mod=00: invalid, must use Mod=01 with 8-bit disp of 0
    // - If base_reg != 5 and displacement == 0: Mod=00, no displacement
    // - If base_reg == 5 and displacement == 0: Mod=01 with 8-bit disp of 0 (if has_base) or Mod=00 with 32-bit disp (if !has_base)
    if (!has_base) {
      // No base register: always use Mod=00, base=5 means "no base", displacement required
      modrm |= 0x04; // Mod=00, R/M=100 (SIB)
      output.push_back(modrm);
      uint8_t sib = (scale << 6) | (index_reg << 3) | 5; // base=5 means "no base"
      output.push_back(sib);
      EncodeImmediate(static_cast<int64_t>(addr.displacement), 32, output);
    } else if (addr.displacement == 0 && base_reg != 5) {
      // Has base, displacement == 0, base != RBP: Mod=00, no displacement
      modrm |= 0x04; // Mod=00, R/M=100 (SIB)
      output.push_back(modrm);
      uint8_t sib = (scale << 6) | (index_reg << 3) | base_reg;
      output.push_back(sib);
    } else if (base_reg == 5 && addr.displacement == 0) {
      // Base is RBP and displacement == 0: Mod=01 with 8-bit displacement of 0
      modrm |= 0x44; // Mod=01, R/M=100 (SIB)
      output.push_back(modrm);
      uint8_t sib = (scale << 6) | (index_reg << 3) | base_reg;
      output.push_back(sib);
      EncodeImmediate(static_cast<int64_t>(0), 8, output);
    } else if (addr.displacement >= -128 && addr.displacement <= 127) {
      // Small displacement: Mod=01, 8-bit displacement
      modrm |= 0x44; // Mod=01, R/M=100 (SIB)
      output.push_back(modrm);
      uint8_t sib = (scale << 6) | (index_reg << 3) | base_reg;
      output.push_back(sib);
      EncodeImmediate(static_cast<int64_t>(addr.displacement), 8, output);
    } else {
      // Large displacement: Mod=10, 32-bit displacement
      modrm |= 0x84; // Mod=10, R/M=100 (SIB)
      output.push_back(modrm);
      uint8_t sib = (scale << 6) | (index_reg << 3) | base_reg;
      output.push_back(sib);
      EncodeImmediate(static_cast<int64_t>(addr.displacement), 32, output);
    }
  }
}

void AsmToBytes::EncodeMemoryAddressWithReg(const MemoryAddress& mem, 
                                std::vector<uint8_t>& output, 
                                uint8_t reg_field,
                                uint8_t base_low3,
                                uint8_t index_low3) {
  
  // Simple case: direct address [disp32] or [RIP+disp32]
  if (!mem.base && !mem.index && mem.scale == 1) {
    output.push_back(0x05 | (reg_field << 3)); // mod=00, r/m=101
    EncodeImmediate(mem.displacement, 32, output);
    return;
  }
  
  // [base] or [base + disp]
  if (mem.base && !mem.index && mem.scale == 1) {
    uint8_t mod = 0;
    int64_t disp = mem.displacement;
    
    if (disp == 0 && base_low3 != 5) { // RBP/R13 needs special handling
      mod = 0x00; // [base]
      output.push_back((mod << 6) | (reg_field << 3) | base_low3);
    } else if (disp >= -128 && disp <= 127) {
      mod = 0x01; // [base + disp8]
      output.push_back((mod << 6) | (reg_field << 3) | base_low3);
      EncodeImmediate(disp, 8, output);
    } else {
      mod = 0x02; // [base + disp32]
      output.push_back((mod << 6) | (reg_field << 3) | base_low3);
      EncodeImmediate(disp, 32, output);
    }
    return;
  }
  
  // [base + index*scale + disp] - needs SIB byte
  if (mem.base || mem.index) {
    uint8_t mod = 0;
    int64_t disp = mem.displacement;
    
    // Determine mod bits
    if (disp == 0 && (!mem.base || base_low3 != 5)) {
      mod = 0x00;
    } else if (disp >= -128 && disp <= 127) {
      mod = 0x01;
    } else {
      mod = 0x02;
    }
    
    // For [index*scale] (no base), use ESP as base
    uint8_t sib_base = mem.base ? base_low3 : 0x04; // 0x04 = ESP/RSP
    if (!mem.base && mod == 0x00) {
      mod = 0x00; // Special case for [index*scale]
      sib_base = 0x05; // Means no base
    }
    
    // Create SIB byte
    uint8_t scale_bits = 0;
    switch (mem.scale) {
      case 1: scale_bits = 0; break;
      case 2: scale_bits = 1; break;
      case 4: scale_bits = 2; break;
      case 8: scale_bits = 3; break;
      default: scale_bits = 0; break;
    }
    
    uint8_t sib = (scale_bits << 6) | (index_low3 << 3) | sib_base;
    
    // Output ModR/M and SIB
    output.push_back((mod << 6) | (reg_field << 3) | 0x04); // r/m=100 means SIB
    output.push_back(sib);
    
    // Output displacement if needed
    if (mod == 0x01) {
      EncodeImmediate(disp, 8, output);
    } else if (mod == 0x02 || (!mem.base && mod == 0x00)) {
      EncodeImmediate(disp, 32, output);
    }
  }
}

std::expected<void, std::runtime_error> AsmToBytes::EncodeMov(const AssemblyInstruction& instr,
                                                               std::vector<uint8_t>& output) {
  if (instr.arguments.size() < 2) {
    return std::unexpected(std::runtime_error("MOV requires at least 2 arguments"));
  }

  auto arg1 = instr.arguments[0];
  auto arg2 = instr.arguments[1];

  // Handle MOVQ between XMM and integer registers separately
  if (instr.command == AsmCommand::MOVQ) {
    return EncodeMOVQ(instr, output);
  }

  // Determine data transfer direction
  bool is_reg_to_reg = std::holds_alternative<Register>(arg1) && std::holds_alternative<Register>(arg2);
  bool is_reg_to_mem = std::holds_alternative<MemoryAddress>(arg1) && std::holds_alternative<Register>(arg2);
  bool is_mem_to_reg = std::holds_alternative<Register>(arg1) && std::holds_alternative<MemoryAddress>(arg2);
  bool is_imm_to_reg = std::holds_alternative<Register>(arg1) && std::holds_alternative<int64_t>(arg2);
  bool is_imm_to_mem = std::holds_alternative<MemoryAddress>(arg1) && std::holds_alternative<int64_t>(arg2);

  // Helper function to get register code with proper REX handling
  auto encodeRegWithRex = [&](Register reg, uint8_t& rex, uint8_t rex_bit) -> uint8_t {
    uint8_t reg_code = EncodeRegister(reg);
    if (reg_code >= 8) {
      rex |= rex_bit;  // Set appropriate REX bit
      return reg_code & 0x07;  // Return only lower 3 bits
    }
    return reg_code;  // Return as-is (0-7)
  };

  // Handle MOV reg, reg
  if (is_reg_to_reg) {
    Register dst = std::get<Register>(arg1);
    Register src = std::get<Register>(arg2);
    uint8_t reg_size = GetRegisterSize(dst);

    // Initialize REX
    uint8_t rex = 0x40;
    bool need_rex = false;
    
    // Get register codes with REX bits
    uint8_t dst_low3 = encodeRegWithRex(dst, rex, 0x01); // REX.B for r/m field
    uint8_t src_low3 = encodeRegWithRex(src, rex, 0x04); // REX.R for reg field
    
    if (reg_size == 64) {
      rex |= 0x08; // REX.W
      need_rex = true;
    }
    
    if (IsExtendedRegister(dst) || IsExtendedRegister(src) || reg_size == 64) {
      need_rex = true;
    }
    
    if (need_rex) {
      output.push_back(rex);
    }

    // Select opcode based on size
    if (reg_size == 8) {
      output.push_back(0x88); // MOV r/m8, r8
    } else if (reg_size == 16) {
      output.push_back(0x66); // 16-bit prefix
      output.push_back(0x89); // MOV r/m16, r16
    } else if (reg_size == 32) {
      output.push_back(0x89); // MOV r/m32, r32
    } else if (reg_size == 64) {
      output.push_back(0x89); // MOV r/m64, r64
    }

    // Form ModR/M byte: [mod=11][reg=src_low3][r/m=dst_low3]
    uint8_t modrm = 0xC0 | (src_low3 << 3) | dst_low3;
    output.push_back(modrm);

  } 
  // Handle MOV reg, imm
  else if (is_imm_to_reg) {
    Register dst = std::get<Register>(arg1);
    int64_t imm = std::get<int64_t>(arg2);
    uint8_t reg_size = GetRegisterSize(dst);

    // For MOV reg, imm there's special opcode format: 0xB8 + reg_code
    uint8_t dst_code = EncodeRegister(dst);
    
    // Handle REX prefix
    uint8_t rex = 0x40;
    bool need_rex = false;
    
    if (reg_size == 64) {
      rex |= 0x08; // REX.W
      need_rex = true;
    }
    
    if (dst_code >= 8) {
      // For extended registers, we need REX and adjust opcode
      rex |= 0x01; // REX.B
      dst_code &= 0x07; // Use only lower 3 bits
      need_rex = true;
    }
    
    if (need_rex) {
      output.push_back(rex);
    }

    if (reg_size == 8) {
      output.push_back(0xB0 | dst_code); // MOV r8, imm8
      EncodeImmediate(imm, 8, output);
    } else if (reg_size == 16) {
      output.push_back(0x66);
      output.push_back(0xB8 | dst_code); // MOV r16, imm16
      EncodeImmediate(imm, 16, output);
    } else if (reg_size == 32) {
      output.push_back(0xB8 | dst_code); // MOV r32, imm32
      EncodeImmediate(imm, 32, output);
    } else if (reg_size == 64) {
      // Special handling for 64-bit immediate
      output.push_back(0xB8 | dst_code); // MOV r64, imm64
      EncodeImmediate(imm, 64, output);
    }

  } 
  // Handle MOV reg, [mem]
  else if (is_mem_to_reg) {
    Register dst = std::get<Register>(arg1);
    MemoryAddress mem = std::get<MemoryAddress>(arg2);
    uint8_t reg_size = GetRegisterSize(dst);

    // Initialize REX
    uint8_t rex = 0x40;
    bool need_rex = false;
    
    // Handle destination register (goes in reg field)
    uint8_t dst_low3 = encodeRegWithRex(dst, rex, 0x04); // REX.R for reg field
    
    // Handle source memory addressing
    uint8_t base_low3 = 0;
    uint8_t index_low3 = 0;
    
    if (mem.base) {
      uint8_t base_code = EncodeRegister(*mem.base);
      if (base_code >= 8) {
        rex |= 0x01; // REX.B for base
        need_rex = true;
      }
      base_low3 = base_code & 0x07;
    }
    
    if (mem.index) {
      uint8_t index_code = EncodeRegister(*mem.index);
      if (index_code >= 8) {
        rex |= 0x02; // REX.X for index
        need_rex = true;
      }
      index_low3 = index_code & 0x07;
    }
    
    if (reg_size == 64) {
      rex |= 0x08; // REX.W
      need_rex = true;
    }
    
    if (IsExtendedRegister(dst) || need_rex) {
      output.push_back(rex);
    }

    if (reg_size == 8) {
      output.push_back(0x8A); // MOV r8, r/m8
    } else if (reg_size == 16) {
      output.push_back(0x66);
      output.push_back(0x8B); // MOV r16, r/m16
    } else if (reg_size == 32) {
      output.push_back(0x8B); // MOV r32, r/m32
    } else if (reg_size == 64) {
      output.push_back(0x8B); // MOV r64, r/m64
    }

    // Pass dst_low3 as reg field for EncodeMemoryAddress
    EncodeMemoryAddressWithReg(mem, output, dst_low3, base_low3, index_low3);
  } 
  // Handle MOV [mem], reg - FIXED VERSION
  else if (is_reg_to_mem) {
    MemoryAddress mem = std::get<MemoryAddress>(arg1);
    Register src = std::get<Register>(arg2);
    uint8_t reg_size = GetRegisterSize(src);

    // Initialize REX
    uint8_t rex = 0x40;
    bool need_rex = false;
    
    // Handle source register (goes in reg field)
    uint8_t src_low3 = encodeRegWithRex(src, rex, 0x04); // REX.R for reg field
    
    // Handle destination memory addressing
    uint8_t base_low3 = 0;
    uint8_t index_low3 = 0;
    
    if (mem.base) {
      uint8_t base_code = EncodeRegister(*mem.base);
      if (base_code >= 8) {
        rex |= 0x01; // REX.B for base
        need_rex = true;
      }
      base_low3 = base_code & 0x07;
    }
    
    if (mem.index) {
      uint8_t index_code = EncodeRegister(*mem.index);
      if (index_code >= 8) {
        rex |= 0x02; // REX.X for index
        need_rex = true;
      }
      index_low3 = index_code & 0x07;
    }
    
    if (reg_size == 64) {
      rex |= 0x08; // REX.W
      need_rex = true;
    }
    
    if (IsExtendedRegister(src) || need_rex) {
      output.push_back(rex);
    }

    if (reg_size == 8) {
      output.push_back(0x88); // MOV r/m8, r8
    } else if (reg_size == 16) {
      output.push_back(0x66);
      output.push_back(0x89); // MOV r/m16, r16
    } else if (reg_size == 32) {
      output.push_back(0x89); // MOV r/m32, r32
    } else if (reg_size == 64) {
      output.push_back(0x89); // MOV r/m64, r64
    }

    // Pass src_low3 as reg field for EncodeMemoryAddress
    EncodeMemoryAddressWithReg(mem, output, src_low3, base_low3, index_low3);
  } 
  // Handle MOV [mem], imm
  else if (is_imm_to_mem) {
    MemoryAddress mem = std::get<MemoryAddress>(arg1);
    int64_t imm = std::get<int64_t>(arg2);
    
    // Initialize REX
    uint8_t rex = 0x40;
    bool need_rex = false;
    
    // Handle memory addressing
    uint8_t base_low3 = 0;
    uint8_t index_low3 = 0;
    
    if (mem.base) {
      uint8_t base_code = EncodeRegister(*mem.base);
      if (base_code >= 8) {
        rex |= 0x01; // REX.B for base
        need_rex = true;
      }
      base_low3 = base_code & 0x07;
    }
    
    if (mem.index) {
      uint8_t index_code = EncodeRegister(*mem.index);
      if (index_code >= 8) {
        rex |= 0x02; // REX.X for index
        need_rex = true;
      }
      index_low3 = index_code & 0x07;
    }
    
    // For 64-bit operations
    rex |= 0x08; // REX.W
    
    if (need_rex) {
      output.push_back(rex);
    }
    
    output.push_back(0xC7); // MOV r/m64, imm32 (sign-extended)
    
    // For MOV [mem], imm, reg field is 0 in ModR/M
    EncodeMemoryAddressWithReg(mem, output, 0, base_low3, index_low3);
    EncodeImmediate(imm, 32, output);
  } 
  else {
    return std::unexpected(std::runtime_error("Unsupported MOV operand combination"));
  }

  return {};
}

std::expected<void, std::runtime_error> AsmToBytes::EncodeMOVQ(const AssemblyInstruction& instr,
                                                                std::vector<uint8_t>& output) {
  if (instr.arguments.size() < 2) {
    return std::unexpected(std::runtime_error("MOVQ requires 2 arguments"));
  }

  auto arg1 = instr.arguments[0];
  auto arg2 = instr.arguments[1];

  bool xmm_to_int = false;
  bool int_to_xmm = false;
  bool xmm_to_xmm = false;
  bool mem_to_xmm = false;
  bool xmm_to_mem = false;

  if (std::holds_alternative<Register>(arg1) && std::holds_alternative<Register>(arg2)) {
    Register reg1 = std::get<Register>(arg1);
    Register reg2 = std::get<Register>(arg2);
    
    if (IsXMMRegister(reg1) && !IsXMMRegister(reg2)) {
      // MOVQ xmm, r64 - integer to XMM
      int_to_xmm = true;
    } else if (!IsXMMRegister(reg1) && IsXMMRegister(reg2)) {
      // MOVQ r64, xmm - XMM to integer
      xmm_to_int = true;
    } else if (IsXMMRegister(reg1) && IsXMMRegister(reg2)) {
      // MOVQ xmm, xmm - XMM to XMM
      xmm_to_xmm = true;
    }
  } else if (std::holds_alternative<Register>(arg1) && std::holds_alternative<MemoryAddress>(arg2)) {
    Register reg = std::get<Register>(arg1);
    if (IsXMMRegister(reg)) {
      // MOVQ xmm, [mem] - memory to XMM
      mem_to_xmm = true;
    }
  } else if (std::holds_alternative<MemoryAddress>(arg1) && std::holds_alternative<Register>(arg2)) {
    Register reg = std::get<Register>(arg2);
    if (IsXMMRegister(reg)) {
      // MOVQ [mem], xmm - XMM to memory
      xmm_to_mem = true;
    }
  }

  // Handle MOVQ xmm, r64 (integer to XMM)
  if (int_to_xmm) {
    Register xmm_reg = std::get<Register>(arg1);
    Register int_reg = std::get<Register>(arg2);

    // First check if this is actually MOVD (32-bit) based on register size
    uint8_t int_size = GetRegisterSize(int_reg);
    
    if (int_size == 32) {
      // This is actually MOVD, not MOVQ
      uint8_t rex = 0x40; // Base REX
      if (int_size == 64) rex |= 0x08; // REX.W only for 64-bit
      if (IsExtendedRegister(int_reg)) rex |= 0x01; // REX.B for integer reg
      if (IsExtendedRegister(xmm_reg)) rex |= 0x04; // REX.R for XMM reg
      if (rex != 0x40) output.push_back(rex); // Only push if needed

      output.push_back(0x66); // Operand size prefix
      output.push_back(0x0F);
      output.push_back(0x6E); // MOVD xmm, r/m32

      uint8_t modrm = 0xC0;
      modrm |= (EncodeRegister(xmm_reg) << 3);
      modrm |= EncodeRegister(int_reg);
      output.push_back(modrm);
    } else if (int_size == 64) {
      // This is MOVQ for 64-bit registers
      // Order: 66 REX.W 0F 6E /r
      output.push_back(0x66); // Operand size prefix FIRST
      
      uint8_t rex = 0x48; // REX.W = 1 (0x40 | 0x08)
      if (IsExtendedRegister(int_reg)) rex |= 0x01; // REX.B for integer reg
      if (IsExtendedRegister(xmm_reg)) rex |= 0x04; // REX.R for XMM reg
      output.push_back(rex);
      
      output.push_back(0x0F);
      output.push_back(0x6E); // MOVQ xmm, r/m64

      uint8_t modrm = 0xC0;
      modrm |= (EncodeRegister(xmm_reg) << 3);
      modrm |= EncodeRegister(int_reg);
      output.push_back(modrm);
    }
  }
  // Handle MOVQ r64, xmm (XMM to integer)
  else if (xmm_to_int) {
    Register int_reg = std::get<Register>(arg1);
    Register xmm_reg = std::get<Register>(arg2);

    // First check if this is actually MOVD (32-bit) based on register size
    uint8_t int_size = GetRegisterSize(int_reg);
    
    if (int_size == 32) {
      // This is actually MOVD, not MOVQ
      uint8_t rex = 0x40; // Base REX
      if (int_size == 64) rex |= 0x08; // REX.W only for 64-bit
      if (IsExtendedRegister(int_reg)) rex |= 0x01; // REX.B for integer reg
      if (IsExtendedRegister(xmm_reg)) rex |= 0x04; // REX.R for XMM reg
      if (rex != 0x40) output.push_back(rex); // Only push if needed

      output.push_back(0x66); // Operand size prefix
      output.push_back(0x0F);
      output.push_back(0x7E); // MOVD r/m32, xmm

      uint8_t modrm = 0xC0;
      modrm |= (EncodeRegister(xmm_reg) << 3);
      modrm |= EncodeRegister(int_reg);
      output.push_back(modrm);
    } else if (int_size == 64) {
      // This is MOVQ for 64-bit registers
      // Order: 66 REX.W 0F 7E /r
      output.push_back(0x66); // Operand size prefix FIRST
      
      uint8_t rex = 0x48; // REX.W = 1 (0x40 | 0x08)
      if (IsExtendedRegister(int_reg)) rex |= 0x01; // REX.B for integer reg
      if (IsExtendedRegister(xmm_reg)) rex |= 0x04; // REX.R for XMM reg
      output.push_back(rex);
      
      output.push_back(0x0F);
      output.push_back(0x7E); // MOVQ r/m64, xmm

      uint8_t modrm = 0xC0;
      modrm |= (EncodeRegister(xmm_reg) << 3);
      modrm |= EncodeRegister(int_reg);
      output.push_back(modrm);
    }
  }
  // Handle MOVQ xmm, xmm (XMM to XMM)
  else if (xmm_to_xmm) {
    Register dst = std::get<Register>(arg1);
    Register src = std::get<Register>(arg2);

    // Order: F3 REX 0F 7E /r
    output.push_back(0xF3); // REP prefix for XMM to XMM MOVQ FIRST
    
    uint8_t rex = 0x40; // Base REX
    if (IsExtendedRegister(dst)) rex |= 0x04; // REX.R
    if (IsExtendedRegister(src)) rex |= 0x01; // REX.B
    if (rex != 0x40) output.push_back(rex);
    
    output.push_back(0x0F);
    output.push_back(0x7E); // MOVQ xmm, xmm/m64

    uint8_t modrm = 0xC0;
    modrm |= (EncodeRegister(dst) << 3);
    modrm |= EncodeRegister(src);
    output.push_back(modrm);
  }
  // Handle MOVQ xmm, [mem] (memory to XMM)
  else if (mem_to_xmm) {
    Register xmm_reg = std::get<Register>(arg1);
    MemoryAddress mem = std::get<MemoryAddress>(arg2);

    // Order: F3 REX 0F 7E /r
    output.push_back(0xF3); // REP prefix for memory to XMM FIRST
    
    uint8_t rex = 0x40; // Base REX
    if (IsExtendedRegister(xmm_reg)) rex |= 0x04; // REX.R for XMM reg
    if (mem.base && IsExtendedRegister(*mem.base)) rex |= 0x01; // REX.B for base
    if (mem.index && IsExtendedRegister(*mem.index)) rex |= 0x02; // REX.X for index
    if (rex != 0x40) output.push_back(rex);
    
    output.push_back(0x0F);
    output.push_back(0x7E); // MOVQ xmm, m64

    // Используем EncodeMemoryAddressWithReg вместо EncodeMemoryAddress
    uint8_t base_low3 = 0;
    uint8_t index_low3 = 0;
    
    if (mem.base) {
      uint8_t base_code = EncodeRegister(*mem.base);
      base_low3 = base_code & 0x07;
    }
    
    if (mem.index) {
      uint8_t index_code = EncodeRegister(*mem.index);
      index_low3 = index_code & 0x07;
    }
    
    EncodeMemoryAddressWithReg(mem, output, EncodeRegister(xmm_reg), base_low3, index_low3);
  }
  // Handle MOVQ [mem], xmm (XMM to memory)
  else if (xmm_to_mem) {
    MemoryAddress mem = std::get<MemoryAddress>(arg1);
    Register xmm_reg = std::get<Register>(arg2);

    // Order: 66 REX 0F D6 /r
    output.push_back(0x66); // Operand size prefix FIRST
    
    uint8_t rex = 0x40; // Base REX
    if (IsExtendedRegister(xmm_reg)) rex |= 0x04; // REX.R for XMM reg
    if (mem.base && IsExtendedRegister(*mem.base)) rex |= 0x01; // REX.B for base
    if (mem.index && IsExtendedRegister(*mem.index)) rex |= 0x02; // REX.X for index
    if (rex != 0x40) output.push_back(rex);
    
    output.push_back(0x0F);
    output.push_back(0xD6); // MOVQ m64, xmm

    // Используем EncodeMemoryAddressWithReg вместо EncodeMemoryAddress
    uint8_t base_low3 = 0;
    uint8_t index_low3 = 0;
    
    if (mem.base) {
      uint8_t base_code = EncodeRegister(*mem.base);
      base_low3 = base_code & 0x07;
    }
    
    if (mem.index) {
      uint8_t index_code = EncodeRegister(*mem.index);
      index_low3 = index_code & 0x07;
    }
    
    EncodeMemoryAddressWithReg(mem, output, EncodeRegister(xmm_reg), base_low3, index_low3);
  }
  else {
    return std::unexpected(std::runtime_error("Unsupported MOVQ operand combination"));
  }

  return {};
}

std::expected<void, std::runtime_error> AsmToBytes::EncodeArithmetic(const AssemblyInstruction& instr,
                                                                       std::vector<uint8_t>& output) {
  if (instr.arguments.size() < 1) {
    return std::unexpected(std::runtime_error("Arithmetic instruction requires at least 1 argument"));
  }

  // Основные opcode для арифметических операций
  uint8_t opcode_1_reg = 0;  // для формата r/m, reg
  uint8_t opcode_2_reg = 0;  // для формата reg, r/m
  uint8_t opcode_imm = 0;    // для формата r/m, imm
  
  switch (instr.command) {
    case AsmCommand::ADD:
      opcode_1_reg = 0x00;  // ADD r/m8, r8
      opcode_2_reg = 0x02;  // ADD r8, r/m8
      opcode_imm = 0x80;    // ADD r/m, imm (с opcode-extension = 0)
      break;
    case AsmCommand::SUB:
      opcode_1_reg = 0x28;  // SUB r/m8, r8
      opcode_2_reg = 0x2A;  // SUB r8, r/m8
      opcode_imm = 0x80;    // SUB r/m, imm (с opcode-extension = 5)
      break;
    case AsmCommand::AND:
      opcode_1_reg = 0x20;  // AND r/m8, r8
      opcode_2_reg = 0x22;  // AND r8, r/m8
      opcode_imm = 0x80;    // AND r/m, imm (с opcode-extension = 4)
      break;
    case AsmCommand::OR:
      opcode_1_reg = 0x08;  // OR r/m8, r8
      opcode_2_reg = 0x0A;  // OR r8, r/m8
      opcode_imm = 0x80;    // OR r/m, imm (с opcode-extension = 1)
      break;
    case AsmCommand::XOR:
      opcode_1_reg = 0x30;  // XOR r/m8, r8
      opcode_2_reg = 0x32;  // XOR r8, r/m8
      opcode_imm = 0x80;    // XOR r/m, imm (с opcode-extension = 6)
      break;
    case AsmCommand::CMP:
      opcode_1_reg = 0x38;  // CMP r/m8, r8
      opcode_2_reg = 0x3A;  // CMP r8, r/m8
      opcode_imm = 0x80;    // CMP r/m, imm (с opcode-extension = 7)
      break;
    case AsmCommand::TEST:
      // TEST имеет только формат r/m, reg и r/m, imm
      opcode_1_reg = 0x84;  // TEST r/m8, r8
      opcode_imm = 0xF6;    // TEST r/m, imm (с opcode-extension = 0)
      break;
    // Однооперандные инструкции обрабатываются отдельно
    case AsmCommand::INC:
    case AsmCommand::DEC:
    case AsmCommand::NEG:
    case AsmCommand::NOT:
    case AsmCommand::SHL:
    case AsmCommand::SHR:
    case AsmCommand::SAR:
      // Будут обработаны в отдельной секции
      break;
    default:
      return std::unexpected(std::runtime_error("Unknown arithmetic command"));
  }

  auto arg1 = instr.arguments[0];

  // Обработка однооперандных инструкций (INC, DEC, NEG, NOT)
  if (std::holds_alternative<Register>(arg1) && 
      (instr.command == AsmCommand::INC || instr.command == AsmCommand::DEC ||
       instr.command == AsmCommand::NEG || instr.command == AsmCommand::NOT)) {
    
    Register reg = std::get<Register>(arg1);
    uint8_t reg_size = GetRegisterSize(reg);

    // Определяем REX префикс
    uint8_t rex = 0x40;  // Базовый REX
    bool need_rex = false;
    
    if (reg_size == 64) {
      rex |= 0x08; // REX.W
      need_rex = true;
    }
    
    uint8_t reg_enc = EncodeRegister(reg);
    if (reg_enc >= 8) {
      rex |= 0x01; // REX.B для extended registers
      reg_enc &= 0x07; // Берем только младшие 3 бита
      need_rex = true;
    }
    
    if (need_rex) {
      output.push_back(rex);
    }

    if (instr.command == AsmCommand::INC) {
      if (reg_size == 8) {
        output.push_back(0xFE); // INC r/m8
        uint8_t modrm = 0xC0 | (0 << 3) | reg_enc;
        output.push_back(modrm);
      } else if (reg_size == 16) {
        output.push_back(0x66); // 16-bit operand size
        output.push_back(0xFF); // INC r/m16
        uint8_t modrm = 0xC0 | (0 << 3) | reg_enc;
        output.push_back(modrm);
      } else if (reg_size == 32) {
        output.push_back(0xFF); // INC r/m32
        uint8_t modrm = 0xC0 | (0 << 3) | reg_enc;
        output.push_back(modrm);
      } else { // 64-bit
        output.push_back(0xFF); // INC r/m64
        uint8_t modrm = 0xC0 | (0 << 3) | reg_enc;
        output.push_back(modrm);
      }
    } 
    else if (instr.command == AsmCommand::DEC) {
      if (reg_size == 8) {
        output.push_back(0xFE); // DEC r/m8
        uint8_t modrm = 0xC0 | (1 << 3) | reg_enc;
        output.push_back(modrm);
      } else if (reg_size == 16) {
        output.push_back(0x66); // 16-bit operand size
        output.push_back(0xFF); // DEC r/m16
        uint8_t modrm = 0xC0 | (1 << 3) | reg_enc;
        output.push_back(modrm);
      } else if (reg_size == 32) {
        output.push_back(0xFF); // DEC r/m32
        uint8_t modrm = 0xC0 | (1 << 3) | reg_enc;
        output.push_back(modrm);
      } else { // 64-bit
        output.push_back(0xFF); // DEC r/m64
        uint8_t modrm = 0xC0 | (1 << 3) | reg_enc;
        output.push_back(modrm);
      }
    }
    else if (instr.command == AsmCommand::NEG) {
      if (reg_size == 8) {
        output.push_back(0xF6); // NEG r/m8
        uint8_t modrm = 0xC0 | (3 << 3) | reg_enc;
        output.push_back(modrm);
      } else if (reg_size == 16) {
        output.push_back(0x66); // 16-bit operand size
        output.push_back(0xF7); // NEG r/m16
        uint8_t modrm = 0xC0 | (3 << 3) | reg_enc;
        output.push_back(modrm);
      } else if (reg_size == 32) {
        output.push_back(0xF7); // NEG r/m32
        uint8_t modrm = 0xC0 | (3 << 3) | reg_enc;
        output.push_back(modrm);
      } else { // 64-bit
        output.push_back(0xF7); // NEG r/m64
        uint8_t modrm = 0xC0 | (3 << 3) | reg_enc;
        output.push_back(modrm);
      }
    }
    else if (instr.command == AsmCommand::NOT) {
      if (reg_size == 8) {
        output.push_back(0xF6); // NOT r/m8
        uint8_t modrm = 0xC0 | (2 << 3) | reg_enc;
        output.push_back(modrm);
      } else if (reg_size == 16) {
        output.push_back(0x66); // 16-bit operand size
        output.push_back(0xF7); // NOT r/m16
        uint8_t modrm = 0xC0 | (2 << 3) | reg_enc;
        output.push_back(modrm);
      } else if (reg_size == 32) {
        output.push_back(0xF7); // NOT r/m32
        uint8_t modrm = 0xC0 | (2 << 3) | reg_enc;
        output.push_back(modrm);
      } else { // 64-bit
        output.push_back(0xF7); // NOT r/m64
        uint8_t modrm = 0xC0 | (2 << 3) | reg_enc;
        output.push_back(modrm);
      }
    }
    
    return {};
  }

  // Обработка сдвигов (SHL, SHR, SAR)
  if (std::holds_alternative<Register>(arg1) && 
      (instr.command == AsmCommand::SHL || instr.command == AsmCommand::SHR || 
       instr.command == AsmCommand::SAR)) {
    
    Register reg = std::get<Register>(arg1);
    uint8_t reg_size = GetRegisterSize(reg);
    
    // Получаем второй аргумент (счетчик сдвига)
    if (instr.arguments.size() < 2) {
      return std::unexpected(std::runtime_error("Shift instruction requires count"));
    }
    
    auto arg2 = instr.arguments[1];
    
    // Определяем REX префикс
    uint8_t rex = 0x40;  // Базовый REX
    bool need_rex = false;
    
    if (reg_size == 64) {
      rex |= 0x08; // REX.W
      need_rex = true;
    }
    
    uint8_t reg_enc = EncodeRegister(reg);
    if (reg_enc >= 8) {
      rex |= 0x01; // REX.B для extended registers
      reg_enc &= 0x07; // Берем только младшие 3 бита
      need_rex = true;
    }
    
    if (need_rex) {
      output.push_back(rex);
    }

    // Определяем opcode-extension для modrm
    uint8_t op_ext = 0;
    switch (instr.command) {
      case AsmCommand::SHL: op_ext = 4; break; // /4
      case AsmCommand::SHR: op_ext = 5; break; // /5
      case AsmCommand::SAR: op_ext = 7; break; // /7
      default: break;
    }

    if (std::holds_alternative<Register>(arg2)) {
      // Сдвиг на значение в CL
      Register count_reg = std::get<Register>(arg2);
      if (count_reg != Register::CL) {
        return std::unexpected(std::runtime_error("Shift count must be in CL register"));
      }
      
      if (reg_size == 8) {
        output.push_back(0xD2); // SHL/SHR/SAR r/m8, CL
        uint8_t modrm = 0xC0 | (op_ext << 3) | reg_enc;
        output.push_back(modrm);
      } else if (reg_size == 16) {
        output.push_back(0x66); // 16-bit operand size
        output.push_back(0xD3); // SHL/SHR/SAR r/m16, CL
        uint8_t modrm = 0xC0 | (op_ext << 3) | reg_enc;
        output.push_back(modrm);
      } else if (reg_size == 32) {
        output.push_back(0xD3); // SHL/SHR/SAR r/m32, CL
        uint8_t modrm = 0xC0 | (op_ext << 3) | reg_enc;
        output.push_back(modrm);
      } else { // 64-bit
        output.push_back(0xD3); // SHL/SHR/SAR r/m64, CL
        uint8_t modrm = 0xC0 | (op_ext << 3) | reg_enc;
        output.push_back(modrm);
      }
    } 
    else if (std::holds_alternative<int64_t>(arg2)) {
      // Сдвиг на непосредственное значение
      int64_t imm = std::get<int64_t>(arg2);
      
      if (imm == 1) {
        // Специальная короткая форма для сдвига на 1
        if (reg_size == 8) {
          output.push_back(0xD0); // SHL/SHR/SAR r/m8, 1
          uint8_t modrm = 0xC0 | (op_ext << 3) | reg_enc;
          output.push_back(modrm);
        } else if (reg_size == 16) {
          output.push_back(0x66); // 16-bit operand size
          output.push_back(0xD1); // SHL/SHR/SAR r/m16, 1
          uint8_t modrm = 0xC0 | (op_ext << 3) | reg_enc;
          output.push_back(modrm);
        } else if (reg_size == 32) {
          output.push_back(0xD1); // SHL/SHR/SAR r/m32, 1
          uint8_t modrm = 0xC0 | (op_ext << 3) | reg_enc;
          output.push_back(modrm);
        } else { // 64-bit
          output.push_back(0xD1); // SHL/SHR/SAR r/m64, 1
          uint8_t modrm = 0xC0 | (op_ext << 3) | reg_enc;
          output.push_back(modrm);
        }
      } else {
        // Общая форма с непосредственным значением
        if (reg_size == 8) {
          output.push_back(0xC0); // SHL/SHR/SAR r/m8, imm8
          uint8_t modrm = 0xC0 | (op_ext << 3) | reg_enc;
          output.push_back(modrm);
          EncodeImmediate(imm, 8, output);
        } else if (reg_size == 16) {
          output.push_back(0x66); // 16-bit operand size
          output.push_back(0xC1); // SHL/SHR/SAR r/m16, imm8
          uint8_t modrm = 0xC0 | (op_ext << 3) | reg_enc;
          output.push_back(modrm);
          EncodeImmediate(imm, 8, output);
        } else if (reg_size == 32) {
          output.push_back(0xC1); // SHL/SHR/SAR r/m32, imm8
          uint8_t modrm = 0xC0 | (op_ext << 3) | reg_enc;
          output.push_back(modrm);
          EncodeImmediate(imm, 8, output);
        } else { // 64-bit
          output.push_back(0xC1); // SHL/SHR/SAR r/m64, imm8
          uint8_t modrm = 0xC0 | (op_ext << 3) | reg_enc;
          output.push_back(modrm);
          EncodeImmediate(imm, 8, output);
        }
      }
    }
    
    return {};
  }

  // Обработка двухоперандных инструкций (ADD, SUB, AND, OR, XOR, CMP, TEST)
  if (std::holds_alternative<Register>(arg1) && instr.arguments.size() >= 2) {
    Register reg1 = std::get<Register>(arg1);
    uint8_t reg_size = GetRegisterSize(reg1);
    auto arg2 = instr.arguments[1];
    
    // Определяем REX префикс
    uint8_t rex = 0x40;  // Базовый REX
    bool need_rex = false;
    
    if (reg_size == 64) {
      rex |= 0x08; // REX.W
      need_rex = true;
    }
    
    uint8_t reg1_enc = EncodeRegister(reg1);
    if (IsExtendedRegister(reg1)) {
      rex |= 0x01; // REX.B для первого регистра (будет в r/m поле)
      reg1_enc &= 0x07;
      need_rex = true;
    }

    if (std::holds_alternative<Register>(arg2)) {
      // Формат: регистр, регистр
      Register reg2 = std::get<Register>(arg2);
      
      // Проверка совместимости размеров
      if (GetRegisterSize(reg2) != reg_size && 
          !(reg_size == 64 && GetRegisterSize(reg2) == 32) &&
          !(reg_size == 32 && GetRegisterSize(reg2) == 64)) {
        return std::unexpected(std::runtime_error("Register size mismatch"));
      }
      
      uint8_t reg2_enc = EncodeRegister(reg2);
      if (IsExtendedRegister(reg2)) {
        rex |= 0x04; // REX.R для второго регистра (будет в reg поле)
        reg2_enc &= 0x07;
        need_rex = true;
      }
      
      if (need_rex) {
        output.push_back(rex);
      }

      // Определяем, какой opcode использовать
      uint8_t base_opcode = 0;
      uint8_t modrm = 0xC0; // mod = 11 (регистр-регистр)
      
      if (reg_size == 8) {
        // Для 8-битных используем opcode_1_reg или opcode_2_reg
        // Выбираем направление: обычно dest = reg1, src = reg2
        base_opcode = opcode_1_reg; // Формат: r/m8, r8
        modrm |= (reg2_enc << 3) | reg1_enc; // reg2 -> reg поле, reg1 -> r/m поле
      } else {
        // Для 16/32/64 бит используем опкоды 01/03 и т.д.
        if (instr.command == AsmCommand::ADD) {
          base_opcode = 0x01; // ADD r/m, r (формат r/m, reg)
          modrm |= (reg2_enc << 3) | reg1_enc;
        } else if (instr.command == AsmCommand::SUB) {
          base_opcode = 0x29; // SUB r/m, r
          modrm |= (reg2_enc << 3) | reg1_enc;
        } else if (instr.command == AsmCommand::AND) {
          base_opcode = 0x21; // AND r/m, r
          modrm |= (reg2_enc << 3) | reg1_enc;
        } else if (instr.command == AsmCommand::OR) {
          base_opcode = 0x09; // OR r/m, r
          modrm |= (reg2_enc << 3) | reg1_enc;
        } else if (instr.command == AsmCommand::XOR) {
          base_opcode = 0x31; // XOR r/m, r
          modrm |= (reg2_enc << 3) | reg1_enc;
        } else if (instr.command == AsmCommand::CMP) {
          base_opcode = 0x39; // CMP r/m, r
          modrm |= (reg2_enc << 3) | reg1_enc;
        } else if (instr.command == AsmCommand::TEST) {
          base_opcode = 0x85; // TEST r/m, r
          modrm |= (reg2_enc << 3) | reg1_enc;
        }
      }
      
      if (reg_size == 16) {
        output.push_back(0x66); // 16-bit operand size
      }
      
      output.push_back(base_opcode);
      output.push_back(modrm);
      
    } 
    else if (std::holds_alternative<int64_t>(arg2)) {
      // Формат: регистр, непосредственное значение
      int64_t imm = std::get<int64_t>(arg2);
      
      if (need_rex) {
        output.push_back(rex);
      }
      
      // Определяем opcode-extension для инструкции
      uint8_t op_ext = 0;
      switch (instr.command) {
        case AsmCommand::ADD: op_ext = 0; break;
        case AsmCommand::OR:  op_ext = 1; break;
        case AsmCommand::AND: op_ext = 4; break;
        case AsmCommand::SUB: op_ext = 5; break;
        case AsmCommand::XOR: op_ext = 6; break;
        case AsmCommand::CMP: op_ext = 7; break;
        case AsmCommand::TEST: op_ext = 0; break; // TEST использует отдельный opcode
        default: break;
      }
      
      if (instr.command == AsmCommand::TEST) {
        // TEST имеет особый формат
        if (reg_size == 8) {
          output.push_back(0xF6); // TEST r/m8, imm8
          uint8_t modrm = 0xC0 | (0 << 3) | reg1_enc; // op_ext = 0
          output.push_back(modrm);
          EncodeImmediate(imm, 8, output);
        } else if (reg_size == 16) {
          output.push_back(0x66);
          output.push_back(0xF7); // TEST r/m16, imm16
          uint8_t modrm = 0xC0 | (0 << 3) | reg1_enc;
          output.push_back(modrm);
          EncodeImmediate(imm, 16, output);
        } else if (reg_size == 32) {
          output.push_back(0xF7); // TEST r/m32, imm32
          uint8_t modrm = 0xC0 | (0 << 3) | reg1_enc;
          output.push_back(modrm);
          EncodeImmediate(imm, 32, output);
        } else { // 64-bit
          output.push_back(0xF7); // TEST r/m64, imm32
          uint8_t modrm = 0xC0 | (0 << 3) | reg1_enc;
          output.push_back(modrm);
          EncodeImmediate(imm, 32, output);
        }
      } else {
        // Остальные инструкции
        if (reg_size == 8) {
          output.push_back(0x80); // ALU r/m8, imm8
          uint8_t modrm = 0xC0 | (op_ext << 3) | reg1_enc;
          output.push_back(modrm);
          EncodeImmediate(imm, 8, output);
        } else if (reg_size == 16) {
          output.push_back(0x66);
          output.push_back(0x81); // ALU r/m16, imm16
          uint8_t modrm = 0xC0 | (op_ext << 3) | reg1_enc;
          output.push_back(modrm);
          EncodeImmediate(imm, 16, output);
        } else if (reg_size == 32) {
          output.push_back(0x81); // ALU r/m32, imm32
          uint8_t modrm = 0xC0 | (op_ext << 3) | reg1_enc;
          output.push_back(modrm);
          EncodeImmediate(imm, 32, output);
        } else { // 64-bit
          output.push_back(0x81); // ALU r/m64, imm32 (sign-extended)
          uint8_t modrm = 0xC0 | (op_ext << 3) | reg1_enc;
          output.push_back(modrm);
          EncodeImmediate(imm, 32, output);
        }
      }
    } else {
      return std::unexpected(std::runtime_error("Invalid second argument type"));
    }
    
    return {};
  }

  return std::unexpected(std::runtime_error("Unsupported arithmetic instruction format"));
}

std::expected<void, std::runtime_error> AsmToBytes::EncodeJump(const AssemblyInstruction& instr,
                                                                std::vector<uint8_t>& output) {
  // Handle RET
  if (instr.command == AsmCommand::RET) {
    if (instr.arguments.empty()) {
      output.push_back(0xC3); // RET without operand
    } else if (instr.arguments.size() == 1) {
      if (auto imm = instr.get_argument<int64_t>(0)) {
        uint64_t imm_val = static_cast<uint64_t>(*imm);
        if (imm_val == 0) {
          output.push_back(0xC3); // RET
        } else {
          output.push_back(0xC2); // RET imm16
          EncodeImmediate(static_cast<uint64_t>(imm_val), 16, output);
        }
      }
    }
    return {};
  }

  // Determine opcode for jumps
  uint8_t opcode = 0;
  bool is_conditional_jump = false;

  switch (instr.command) {
    case AsmCommand::JMP:
      opcode = 0xE9; // JMP rel32
      break;
    case AsmCommand::CALL:
      opcode = 0xE8; // CALL rel32
      break;
    
    // Conditional jumps (all use 0x0F prefix)
    case AsmCommand::JE:
      opcode = 0x84;
      is_conditional_jump = true;
      break;
    case AsmCommand::JNE:
      opcode = 0x85;
      is_conditional_jump = true;
      break;
    case AsmCommand::JG:
      opcode = 0x8F;
      is_conditional_jump = true;
      break;
    case AsmCommand::JGE:
      opcode = 0x8D;
      is_conditional_jump = true;
      break;
    case AsmCommand::JL:
      opcode = 0x8C;
      is_conditional_jump = true;
      break;
    case AsmCommand::JLE:
      opcode = 0x8E;
      is_conditional_jump = true;
      break;
    case AsmCommand::JA:
      opcode = 0x87;
      is_conditional_jump = true;
      break;
    case AsmCommand::JAE:
      opcode = 0x83;
      is_conditional_jump = true;
      break;
    case AsmCommand::JB:
      opcode = 0x82;
      is_conditional_jump = true;
      break;
    case AsmCommand::JBE:
      opcode = 0x86;
      is_conditional_jump = true;
      break;
    
    case AsmCommand::LOOP:
      opcode = 0xE2; // LOOP rel8
      break;
    case AsmCommand::LOOPE:
      opcode = 0xE1; // LOOPE rel8
      break;
    case AsmCommand::LOOPNE:
      opcode = 0xE0; // LOOPNE rel8
      break;
    
    default:
      return std::unexpected(std::runtime_error("Unsupported jump instruction"));
  }

  if (instr.arguments.empty()) {
    return std::unexpected(std::runtime_error("Jump instruction requires an argument"));
  }

  auto arg = instr.arguments[0];
  
  // Handle jump to label (relative address)
  if (std::holds_alternative<std::string>(arg)) {
    std::string label = std::get<std::string>(arg);
    
    if (is_conditional_jump) {
      output.push_back(0x0F); // Prefix for conditional jumps
      output.push_back(opcode);
    } else if (instr.command == AsmCommand::LOOP || 
               instr.command == AsmCommand::LOOPE || 
               instr.command == AsmCommand::LOOPNE) {
      // LOOP uses 8-bit offset
      output.push_back(opcode);
      size_t offset_pos = output.size();
      EncodeImmediate(static_cast<int64_t>(0), 8, output);
      jump_patches_.emplace_back(offset_pos, label);
      return {};
    } else {
      // JMP/CALL with relative address
      output.push_back(opcode);
    }
    
    // Save position for patching offset
    size_t offset_pos = output.size();
    EncodeImmediate(static_cast<int64_t>(0), 32, output);
    jump_patches_.emplace_back(offset_pos, label);
  }
  // Handle jump by immediate relative address
  else if (std::holds_alternative<int64_t>(arg)) {
    int64_t imm = std::get<int64_t>(arg);
    
    if (is_conditional_jump) {
      output.push_back(0x0F);
      output.push_back(opcode);
      EncodeImmediate(imm, 32, output);
    } else if (instr.command == AsmCommand::LOOP || 
               instr.command == AsmCommand::LOOPE || 
               instr.command == AsmCommand::LOOPNE) {
      output.push_back(opcode);
      EncodeImmediate(imm, 8, output);
    } else {
      output.push_back(opcode);
      EncodeImmediate(imm, 32, output);
    }
  }
  // Handle CALL/JMP by register (indirect call)
  else if (std::holds_alternative<Register>(arg)) {
    Register reg = std::get<Register>(arg);
    uint8_t reg_size = GetRegisterSize(reg);
    
    if (reg_size != 64) {
      return std::unexpected(std::runtime_error("CALL/JMP register operand must be 64-bit"));
    }
    
    // Determine if REX prefix is needed
    bool rex_needed = IsExtendedRegister(reg);
    uint8_t rex = 0x40;
    
    if (rex_needed) {
      rex |= 0x08; // REX.W for 64-bit operations
      if (IsExtendedRegister(reg)) {
        rex |= 0x01; // REX.B for registers R8-R15
      }
      output.push_back(rex);
    }
    
    if (instr.command == AsmCommand::CALL) {
      // CALL r/m64: FF /2
      output.push_back(0xFF);
      uint8_t modrm = 0xC0 | (2 << 3) | EncodeRegister(reg); // /2 for CALL
      output.push_back(modrm);
    } else if (instr.command == AsmCommand::JMP) {
      // JMP r/m64: FF /4
      output.push_back(0xFF);
      uint8_t modrm = 0xC0 | (4 << 3) | EncodeRegister(reg); // /4 for JMP
      output.push_back(modrm);
    } else {
      // Conditional jumps don't support indirect jumps by register
      return std::unexpected(std::runtime_error("Conditional jumps do not support register operands"));
    }
  }
  // Handle CALL/JMP by memory address
  else if (std::holds_alternative<MemoryAddress>(arg)) {
    MemoryAddress mem = std::get<MemoryAddress>(arg);
    
    // Determine if REX prefix is needed
    bool rex_needed = (mem.base && IsExtendedRegister(*mem.base)) ||
                      (mem.index && IsExtendedRegister(*mem.index));
    uint8_t rex = 0x40;
    
    if (rex_needed) {
      rex |= 0x08; // REX.W for 64-bit operations
      if (mem.base && IsExtendedRegister(*mem.base)) {
        rex |= 0x01; // REX.B
      }
      if (mem.index && IsExtendedRegister(*mem.index)) {
        rex |= 0x02; // REX.X
      }
      output.push_back(rex);
    }
    
    if (instr.command == AsmCommand::CALL) {
      // CALL [mem]: FF /2
      output.push_back(0xFF);
      // Используем EncodeMemoryAddressWithReg
      uint8_t base_low3 = 0;
      uint8_t index_low3 = 0;
      
      if (mem.base) {
        uint8_t base_code = EncodeRegister(*mem.base);
        base_low3 = base_code & 0x07;
      }
      
      if (mem.index) {
        uint8_t index_code = EncodeRegister(*mem.index);
        index_low3 = index_code & 0x07;
      }
      
      EncodeMemoryAddressWithReg(mem, output, 2, base_low3, index_low3);
    } else if (instr.command == AsmCommand::JMP) {
      // JMP [mem]: FF /4
      output.push_back(0xFF);
      // Используем EncodeMemoryAddressWithReg
      uint8_t base_low3 = 0;
      uint8_t index_low3 = 0;
      
      if (mem.base) {
        uint8_t base_code = EncodeRegister(*mem.base);
        base_low3 = base_code & 0x07;
      }
      
      if (mem.index) {
        uint8_t index_code = EncodeRegister(*mem.index);
        index_low3 = index_code & 0x07;
      }
      
      EncodeMemoryAddressWithReg(mem, output, 4, base_low3, index_low3);
    } else {
      // Conditional jumps don't support indirect jumps by memory
      return std::unexpected(std::runtime_error("Conditional jumps do not support memory operands"));
    }
  }
  else {
    return std::unexpected(std::runtime_error("Unsupported jump operand type"));
  }

  return {};
}

void AsmToBytes::EncodeStackOp(const AssemblyInstruction& instr, std::vector<uint8_t>& output) {
  if (instr.command == AsmCommand::PUSH) {
    if (instr.arguments.size() > 0) {
      auto arg = instr.arguments[0];

      if (std::holds_alternative<Register>(arg)) {
        Register reg = std::get<Register>(arg);
        uint8_t reg_size = GetRegisterSize(reg);

        bool rex_needed = (reg_size == 64) || IsExtendedRegister(reg);
        if (rex_needed) {
          uint8_t rex = 0x40;
          if (reg_size == 64)
            rex |= 0x08; // REX.W
          if (IsExtendedRegister(reg))
            rex |= 0x01; // REX.B
          output.push_back(rex);
        }

        if (reg_size == 64) {
          output.push_back(0x50 | EncodeRegister(reg)); // PUSH r64
        } else if (reg_size == 32) {
          output.push_back(0x50 | EncodeRegister(reg)); // PUSH r32
        } else if (reg_size == 16) {
          output.push_back(0x66);
          output.push_back(0x50 | EncodeRegister(reg)); // PUSH r16
        }
      } else if (std::holds_alternative<int64_t>(arg)) {
        int64_t imm = std::get<int64_t>(arg);
        if (imm >= -128 && imm <= 127) {
          output.push_back(0x6A); // PUSH imm8
          EncodeImmediate(imm, 8, output);
        } else {
          output.push_back(0x68); // PUSH imm32
          EncodeImmediate(imm, 32, output);
        }
      }
    }
  } else if (instr.command == AsmCommand::POP) {
    if (instr.arguments.size() > 0) {
      if (auto reg = instr.get_argument<Register>(0)) {
        uint8_t reg_size = GetRegisterSize(*reg);

        bool rex_needed = (reg_size == 64) || IsExtendedRegister(*reg);
        if (rex_needed) {
          uint8_t rex = 0x40;
          if (reg_size == 64)
            rex |= 0x08; // REX.W
          if (IsExtendedRegister(*reg))
            rex |= 0x01; // REX.B
          output.push_back(rex);
        }

        if (reg_size == 64) {
          output.push_back(0x58 | EncodeRegister(*reg)); // POP r64
        } else if (reg_size == 32) {
          output.push_back(0x58 | EncodeRegister(*reg)); // POP r32
        } else if (reg_size == 16) {
          output.push_back(0x66);
          output.push_back(0x58 | EncodeRegister(*reg)); // POP r16
        }
      }
    }
  } else if (instr.command == AsmCommand::PUSHF) {
    output.push_back(0x9C);
  } else if (instr.command == AsmCommand::POPF) {
    output.push_back(0x9D);
  }
}

std::expected<void, std::runtime_error> AsmToBytes::EncodeSSE2(const AssemblyInstruction& instr,
                                                                std::vector<uint8_t>& output) {
  if (instr.arguments.size() < 2) {
    return std::unexpected(std::runtime_error("SSE2 instruction requires at least 2 arguments"));
  }

  auto arg1 = instr.arguments[0];
  auto arg2 = instr.arguments[1];

  // Get SSE prefix and opcode
  uint8_t prefix = GetSSEPrefix(instr.command);
  uint16_t opcode16 = GetSSEOpcode(instr.command);
  
  if (opcode16 == 0x0000) {
    return std::unexpected(std::runtime_error("Unsupported SSE2 instruction"));
  }

  bool reg_to_reg = std::holds_alternative<Register>(arg1) && std::holds_alternative<Register>(arg2);
  bool mem_to_reg = std::holds_alternative<Register>(arg1) && std::holds_alternative<MemoryAddress>(arg2);
  bool reg_to_mem = std::holds_alternative<MemoryAddress>(arg1) && std::holds_alternative<Register>(arg2);

  // Handle CVTSI2SD: integer to XMM
  if (instr.command == AsmCommand::CVTSI2SD) {
    if (reg_to_reg) {
      Register xmm_reg = std::get<Register>(arg1);
      Register int_reg = std::get<Register>(arg2);
      
      if (!IsXMMRegister(xmm_reg)) {
        return std::unexpected(std::runtime_error("CVTSI2SD first operand must be XMM register"));
      }
      
      uint8_t rex = 0x40;
      if (IsExtendedRegister(xmm_reg)) rex |= 0x04; // REX.R for XMM
      if (IsExtendedRegister(int_reg)) rex |= 0x01; // REX.B for integer
      output.push_back(rex);
      
      if (prefix != 0x00) output.push_back(prefix);
      output.push_back(static_cast<uint8_t>(opcode16 & 0xFF));
      output.push_back(static_cast<uint8_t>(opcode16 >> 8));
      
      uint8_t modrm = 0xC0;
      modrm |= (EncodeRegister(xmm_reg) << 3);
      modrm |= EncodeRegister(int_reg);
      output.push_back(modrm);
    } else if (mem_to_reg) {
      Register xmm_reg = std::get<Register>(arg1);
      MemoryAddress mem = std::get<MemoryAddress>(arg2);
      
      uint8_t rex = 0x40;
      if (IsExtendedRegister(xmm_reg)) rex |= 0x04; // REX.R for XMM
      if (mem.base && IsExtendedRegister(*mem.base)) rex |= 0x01; // REX.B for base
      if (mem.index && IsExtendedRegister(*mem.index)) rex |= 0x02; // REX.X for index
      output.push_back(rex);
      
      if (prefix != 0x00) output.push_back(prefix);
      output.push_back(static_cast<uint8_t>(opcode16 & 0xFF));
      output.push_back(static_cast<uint8_t>(opcode16 >> 8));
      
      // Используем EncodeMemoryAddressWithReg
      uint8_t base_low3 = 0;
      uint8_t index_low3 = 0;
      
      if (mem.base) {
        uint8_t base_code = EncodeRegister(*mem.base);
        base_low3 = base_code & 0x07;
      }
      
      if (mem.index) {
        uint8_t index_code = EncodeRegister(*mem.index);
        index_low3 = index_code & 0x07;
      }
      
      EncodeMemoryAddressWithReg(mem, output, EncodeRegister(xmm_reg), base_low3, index_low3);
    }
  }
  // Handle CVTSD2SI: XMM to integer
  else if (instr.command == AsmCommand::CVTSD2SI || 
           instr.command == AsmCommand::CVTTSD2SI ||
           instr.command == AsmCommand::CVTTSD2SIQ) {
    if (reg_to_reg) {
      Register int_reg = std::get<Register>(arg1);
      Register xmm_reg = std::get<Register>(arg2);
      
      if (!IsXMMRegister(xmm_reg)) {
        return std::unexpected(std::runtime_error("CVTSD2SI second operand must be XMM register"));
      }
      
      uint8_t rex = 0x40;
      if (instr.command == AsmCommand::CVTTSD2SIQ) rex |= 0x08; // REX.W for 64-bit result
      if (IsExtendedRegister(int_reg)) rex |= 0x01; // REX.B for integer
      if (IsExtendedRegister(xmm_reg)) rex |= 0x04; // REX.R for XMM
      output.push_back(rex);
      
      if (prefix != 0x00) output.push_back(prefix);
      output.push_back(static_cast<uint8_t>(opcode16 & 0xFF));
      output.push_back(static_cast<uint8_t>(opcode16 >> 8));
      
      uint8_t modrm = 0xC0;
      modrm |= (EncodeRegister(xmm_reg) << 3);
      modrm |= EncodeRegister(int_reg);
      output.push_back(modrm);
    } else if (mem_to_reg) {
      Register int_reg = std::get<Register>(arg1);
      MemoryAddress mem = std::get<MemoryAddress>(arg2);
      
      uint8_t rex = 0x40;
      if (instr.command == AsmCommand::CVTTSD2SIQ) rex |= 0x08; // REX.W for 64-bit result
      if (IsExtendedRegister(int_reg)) rex |= 0x01; // REX.B for integer
      if (mem.base && IsExtendedRegister(*mem.base)) rex |= 0x01; // REX.B for base
      if (mem.index && IsExtendedRegister(*mem.index)) rex |= 0x02; // REX.X for index
      output.push_back(rex);
      
      if (prefix != 0x00) output.push_back(prefix);
      output.push_back(static_cast<uint8_t>(opcode16 & 0xFF));
      output.push_back(static_cast<uint8_t>(opcode16 >> 8));
      
      // Используем EncodeMemoryAddressWithReg
      uint8_t base_low3 = 0;
      uint8_t index_low3 = 0;
      
      if (mem.base) {
        uint8_t base_code = EncodeRegister(*mem.base);
        base_low3 = base_code & 0x07;
      }
      
      if (mem.index) {
        uint8_t index_code = EncodeRegister(*mem.index);
        index_low3 = index_code & 0x07;
      }
      
      EncodeMemoryAddressWithReg(mem, output, EncodeRegister(int_reg), base_low3, index_low3);
    }
  }
  // Handle XMM to XMM operations
  else if (reg_to_reg) {
    Register dst = std::get<Register>(arg1);
    Register src = std::get<Register>(arg2);
    
    if (!IsXMMRegister(dst) || !IsXMMRegister(src)) {
      return std::unexpected(std::runtime_error("SSE2 operation requires XMM registers"));
    }
    
    uint8_t rex = 0x40;
    if (IsExtendedRegister(dst)) rex |= 0x04; // REX.R
    if (IsExtendedRegister(src)) rex |= 0x01; // REX.B
    output.push_back(rex);
    
    if (prefix != 0x00) output.push_back(prefix);
    output.push_back(static_cast<uint8_t>(opcode16 & 0xFF));
    output.push_back(static_cast<uint8_t>(opcode16 >> 8));
    
    uint8_t modrm = 0xC0;
    modrm |= (EncodeRegister(dst) << 3);
    modrm |= EncodeRegister(src);
    output.push_back(modrm);
  }
  // Handle memory to XMM
  else if (mem_to_reg) {
    Register xmm_reg = std::get<Register>(arg1);
    MemoryAddress mem = std::get<MemoryAddress>(arg2);
    
    if (!IsXMMRegister(xmm_reg)) {
      return std::unexpected(std::runtime_error("SSE2 operation requires XMM register destination"));
    }
    
    uint8_t rex = 0x40;
    if (IsExtendedRegister(xmm_reg)) rex |= 0x04; // REX.R
    if (mem.base && IsExtendedRegister(*mem.base)) rex |= 0x01; // REX.B
    if (mem.index && IsExtendedRegister(*mem.index)) rex |= 0x02; // REX.X
    output.push_back(rex);
    
    if (prefix != 0x00) output.push_back(prefix);
    output.push_back(static_cast<uint8_t>(opcode16 & 0xFF));
    output.push_back(static_cast<uint8_t>(opcode16 >> 8));
    
    // Используем EncodeMemoryAddressWithReg
    uint8_t base_low3 = 0;
    uint8_t index_low3 = 0;
    
    if (mem.base) {
      uint8_t base_code = EncodeRegister(*mem.base);
      base_low3 = base_code & 0x07;
    }
    
    if (mem.index) {
      uint8_t index_code = EncodeRegister(*mem.index);
      index_low3 = index_code & 0x07;
    }
    
    EncodeMemoryAddressWithReg(mem, output, EncodeRegister(xmm_reg), base_low3, index_low3);
  }
  // Handle XMM to memory
  else if (reg_to_mem) {
    MemoryAddress mem = std::get<MemoryAddress>(arg1);
    Register xmm_reg = std::get<Register>(arg2);
    
    if (!IsXMMRegister(xmm_reg)) {
      return std::unexpected(std::runtime_error("SSE2 operation requires XMM register source"));
    }
    
    uint8_t rex = 0x40;
    if (IsExtendedRegister(xmm_reg)) rex |= 0x04; // REX.R
    if (mem.base && IsExtendedRegister(*mem.base)) rex |= 0x01; // REX.B
    if (mem.index && IsExtendedRegister(*mem.index)) rex |= 0x02; // REX.X
    output.push_back(rex);
    
    if (prefix != 0x00) output.push_back(prefix);
    output.push_back(static_cast<uint8_t>(opcode16 & 0xFF));
    output.push_back(static_cast<uint8_t>(opcode16 >> 8));
    
    // Используем EncodeMemoryAddressWithReg
    uint8_t base_low3 = 0;
    uint8_t index_low3 = 0;
    
    if (mem.base) {
      uint8_t base_code = EncodeRegister(*mem.base);
      base_low3 = base_code & 0x07;
    }
    
    if (mem.index) {
      uint8_t index_code = EncodeRegister(*mem.index);
      index_low3 = index_code & 0x07;
    }
    
    EncodeMemoryAddressWithReg(mem, output, EncodeRegister(xmm_reg), base_low3, index_low3);
  }
  else {
    return std::unexpected(std::runtime_error("Unsupported SSE2 operand combination"));
  }

  return {};
}

void AsmToBytes::EncodeLabel(const AssemblyInstruction& instr, std::vector<uint8_t>& output) {
  // Labels don't generate code - they're just markers
  // The label address is already stored in label_addresses_ during first pass
}

} // namespace ovum::vm::jit