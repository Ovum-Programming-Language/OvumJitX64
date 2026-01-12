#include "AsmToBytes.hpp"

#include <cstring>
#include <stdexcept>

namespace ovum::vm::jit {

AsmToBytes::AsmToBytes() : current_position_(0) {
}

std::expected<std::vector<uint8_t>, std::runtime_error> AsmToBytes::Convert(
    const std::vector<AssemblyInstruction>& instructions) {
  std::vector<uint8_t> output;
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
    // RAX-R15 (64-bit)
    return value & 0x07;
  } else if (value >= 0x20 && value <= 0x2F) {
    // EAX-R15D (32-bit)
    return (value - 0x20) & 0x07;
  } else if (value >= 0x40 && value <= 0x4F) {
    // AX-R15W (16-bit)
    return (value - 0x40) & 0x07;
  } else if (value >= 0x60 && value <= 0x6F) {
    // AL-R15B (8-bit)
    return (value - 0x60) & 0x07;
  }

  return value & 0x07;
}

uint8_t AsmToBytes::GetRegisterSize(Register reg) const {
  uint8_t value = static_cast<uint8_t>(reg);

  if (value >= 0 && value <= 0x1F) {
    return 64; // 64-bit registers
  } else if (value >= 0x20 && value <= 0x3F) {
    return 32; // 32-bit registers
  } else if (value >= 0x40 && value <= 0x5F) {
    return 16; // 16-bit registers
  } else if (value >= 0x60 && value <= 0x7F) {
    return 8; // 8-bit registers
  }

  return 64;
}

bool AsmToBytes::IsExtendedRegister(Register reg) const {
  uint8_t value = static_cast<uint8_t>(reg);
  // R8-R15, R8D-R15D, R8W-R15W, R8B-R15B are extended
  return (value >= 8 && value <= 15) || (value >= 0x28 && value <= 0x2F) || (value >= 0x48 && value <= 0x4F) ||
         (value >= 0x68 && value <= 0x6F);
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

std::expected<void, std::runtime_error> AsmToBytes::EncodeMov(const AssemblyInstruction& instr,
                                                               std::vector<uint8_t>& output) {
  if (instr.arguments.size() < 2) {
    return std::unexpected(std::runtime_error("MOV requires at least 2 arguments"));
  }

  auto arg1 = instr.arguments[0];
  auto arg2 = instr.arguments[1];

  if (std::holds_alternative<Register>(arg1)) {
    Register dst = std::get<Register>(arg1);

    if (std::holds_alternative<Register>(arg2)) {
      // MOV reg, reg
      Register src = std::get<Register>(arg2);
      uint8_t reg_size = GetRegisterSize(dst);

      bool rex_needed = (reg_size == 64) || IsExtendedRegister(dst) || IsExtendedRegister(src);
      if (rex_needed) {
        uint8_t rex = 0x40;
        if (reg_size == 64)
          rex |= 0x08; // REX.W
        if (IsExtendedRegister(dst))
          rex |= 0x01; // REX.B
        if (IsExtendedRegister(src))
          rex |= 0x04; // REX.R
        output.push_back(rex);
      }

      if (reg_size == 8) {
        output.push_back(0x88); // MOV r/m8, r8
      } else if (reg_size == 16) {
        output.push_back(0x66); // 16-bit prefix
        output.push_back(0x89); // MOV r/m16, r16
      } else if (reg_size == 32) {
        output.push_back(0x89); // MOV r/m32, r32
      } else {
        output.push_back(0x89); // MOV r/m64, r64
      }

      uint8_t modrm = 0xC0; // Register mode
      modrm |= (EncodeRegister(dst) << 3);
      modrm |= EncodeRegister(src);
      output.push_back(modrm);

    } else if (std::holds_alternative<int64_t>(arg2)) {
      // MOV reg, imm
      int64_t imm = std::get<int64_t>(arg2);
      uint8_t reg_size = GetRegisterSize(dst);

      bool rex_needed = (reg_size == 64) || IsExtendedRegister(dst);
      if (rex_needed) {
        uint8_t rex = 0x40;
        if (reg_size == 64)
          rex |= 0x08; // REX.W
        if (IsExtendedRegister(dst))
          rex |= 0x01; // REX.B
        output.push_back(rex);
      }

      if (reg_size == 8) {
        output.push_back(0xB0 | EncodeRegister(dst)); // MOV r8, imm8
        EncodeImmediate(imm, 8, output);
      } else if (reg_size == 16) {
        output.push_back(0x66);
        output.push_back(0xB8 | EncodeRegister(dst)); // MOV r16, imm16
        EncodeImmediate(imm, 16, output);
      } else if (reg_size == 32) {
        output.push_back(0xB8 | EncodeRegister(dst)); // MOV r32, imm32
        EncodeImmediate(imm, 32, output);
      } else {
        output.push_back(0xB8 | EncodeRegister(dst)); // MOV r64, imm64
        EncodeImmediate(imm, 64, output);
      }

    } else if (std::holds_alternative<MemoryAddress>(arg2)) {
      // MOV reg, [mem]
      MemoryAddress mem = std::get<MemoryAddress>(arg2);
      uint8_t reg_size = GetRegisterSize(dst);

      bool rex_needed = (reg_size == 64) || IsExtendedRegister(dst) ||
                        (mem.base && IsExtendedRegister(*mem.base)) ||
                        (mem.index && IsExtendedRegister(*mem.index));
      if (rex_needed) {
        uint8_t rex = 0x40;
        if (reg_size == 64)
          rex |= 0x08; // REX.W
        if (IsExtendedRegister(dst))
          rex |= 0x04; // REX.R (destination is in reg field)
        if (mem.base && IsExtendedRegister(*mem.base))
          rex |= 0x01; // REX.B (base is in r/m or SIB base field)
        if (mem.index && IsExtendedRegister(*mem.index))
          rex |= 0x02; // REX.X (index is in SIB index field)
        output.push_back(rex);
      }

      if (reg_size == 8) {
        output.push_back(0x8A); // MOV r8, r/m8
      } else if (reg_size == 16) {
        output.push_back(0x66);
        output.push_back(0x8B); // MOV r16, r/m16
      } else if (reg_size == 32) {
        output.push_back(0x8B); // MOV r32, r/m32
      } else {
        output.push_back(0x8B); // MOV r64, r/m64
      }

      EncodeMemoryAddress(mem, output, EncodeRegister(dst));
    }
  } else if (std::holds_alternative<MemoryAddress>(arg1)) {
    // MOV [mem], reg/imm
    MemoryAddress mem = std::get<MemoryAddress>(arg1);

    if (std::holds_alternative<Register>(arg2)) {
      Register src = std::get<Register>(arg2);
      uint8_t reg_size = GetRegisterSize(src);

      bool rex_needed = (reg_size == 64) || IsExtendedRegister(src) ||
                        (mem.base && IsExtendedRegister(*mem.base)) ||
                        (mem.index && IsExtendedRegister(*mem.index));
      if (rex_needed) {
        uint8_t rex = 0x40;
        if (reg_size == 64)
          rex |= 0x08; // REX.W
        if (IsExtendedRegister(src))
          rex |= 0x04; // REX.R (source is in reg field)
        if (mem.base && IsExtendedRegister(*mem.base))
          rex |= 0x01; // REX.B (base is in r/m or SIB base field)
        if (mem.index && IsExtendedRegister(*mem.index))
          rex |= 0x02; // REX.X (index is in SIB index field)
        output.push_back(rex);
      }

      if (reg_size == 8) {
        output.push_back(0x88); // MOV r/m8, r8
      } else if (reg_size == 16) {
        output.push_back(0x66);
        output.push_back(0x89); // MOV r/m16, r16
      } else if (reg_size == 32) {
        output.push_back(0x89); // MOV r/m32, r32
      } else {
        output.push_back(0x89); // MOV r/m64, r64
      }

      EncodeMemoryAddress(mem, output, EncodeRegister(src));
    } else if (std::holds_alternative<int64_t>(arg2)) {
      // MOV [mem], imm
      int64_t imm = std::get<int64_t>(arg2);
      // Assume 64-bit for memory operations
      output.push_back(0x48); // REX.W
      output.push_back(0xC7); // MOV r/m64, imm32 (sign-extended)
      EncodeMemoryAddress(mem, output, 0);
      EncodeImmediate(imm, 32, output);
    }
  }

  // Handle MOVZX and MOVSX
  if (instr.command == AsmCommand::MOVZX || instr.command == AsmCommand::MOVSX) {
    // Simplified - would need proper handling based on source/dest sizes
    // This is a placeholder
  }

  return {};
}

std::expected<void, std::runtime_error> AsmToBytes::EncodeArithmetic(const AssemblyInstruction& instr,
                                                                       std::vector<uint8_t>& output) {
  if (instr.arguments.size() < 1) {
    return std::unexpected(std::runtime_error("Arithmetic instruction requires at least 1 argument"));
  }

  uint8_t opcode_base = 0;
  switch (instr.command) {
    case AsmCommand::ADD:
      opcode_base = 0x00;
      break;
    case AsmCommand::SUB:
      opcode_base = 0x28;
      break;
    case AsmCommand::AND:
      opcode_base = 0x20;
      break;
    case AsmCommand::OR:
      opcode_base = 0x08;
      break;
    case AsmCommand::XOR:
      opcode_base = 0x30;
      break;
    case AsmCommand::CMP:
      opcode_base = 0x38;
      break;
    case AsmCommand::TEST:
      opcode_base = 0x84;
      break;
    case AsmCommand::INC:
      opcode_base = 0xFE;
      break; // Special
    case AsmCommand::DEC:
      opcode_base = 0xFE;
      break; // Special
    case AsmCommand::NEG:
      opcode_base = 0xF6;
      break; // Special
    case AsmCommand::NOT:
      opcode_base = 0xF6;
      break; // Special
    case AsmCommand::SHL:
      opcode_base = 0xD0;
      break; // Special
    case AsmCommand::SHR:
      opcode_base = 0xD0;
      break; // Special
    case AsmCommand::SAR:
      opcode_base = 0xD0;
      break; // Special
    default:
      break;
  }

  auto arg1 = instr.arguments[0];

  if (std::holds_alternative<Register>(arg1)) {
    Register reg = std::get<Register>(arg1);
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

    if (instr.command == AsmCommand::INC) {
      if (reg_size == 64) {
        output.push_back(0xFF); // INC r/m64
        uint8_t modrm = 0xC0 | (0 << 3) | EncodeRegister(reg);
        output.push_back(modrm);
      } else if (reg_size == 32) {
        output.push_back(0x40 | EncodeRegister(reg)); // INC r32
      } else if (reg_size == 16) {
        output.push_back(0x66);
        output.push_back(0x40 | EncodeRegister(reg)); // INC r16
      } else {
        output.push_back(0xFE); // INC r/m8
        uint8_t modrm = 0xC0 | (0 << 3) | EncodeRegister(reg);
        output.push_back(modrm);
      }
    } else if (instr.command == AsmCommand::DEC) {
      if (reg_size == 64) {
        output.push_back(0xFF); // DEC r/m64
        uint8_t modrm = 0xC0 | (1 << 3) | EncodeRegister(reg);
        output.push_back(modrm);
      } else if (reg_size == 32) {
        output.push_back(0x48 | EncodeRegister(reg)); // DEC r32
      } else if (reg_size == 16) {
        output.push_back(0x66);
        output.push_back(0x48 | EncodeRegister(reg)); // DEC r16
      } else {
        output.push_back(0xFE); // DEC r/m8
        uint8_t modrm = 0xC0 | (1 << 3) | EncodeRegister(reg);
        output.push_back(modrm);
      }
    } else if (instr.command == AsmCommand::NEG) {
      output.push_back(0xF6); // NEG r/m
      uint8_t modrm = 0xC0 | (3 << 3) | EncodeRegister(reg);
      output.push_back(modrm);
    } else if (instr.command == AsmCommand::NOT) {
      output.push_back(0xF6); // NOT r/m
      uint8_t modrm = 0xC0 | (2 << 3) | EncodeRegister(reg);
      output.push_back(modrm);
    } else if (instr.arguments.size() >= 2) {
      auto arg2 = instr.arguments[1];

      if (std::holds_alternative<Register>(arg2)) {
        Register reg2 = std::get<Register>(arg2);

        if (reg_size == 8) {
          output.push_back(opcode_base + 0x00); // ADD/SUB/etc r/m8, r8
        } else if (reg_size == 16) {
          output.push_back(0x66);
          output.push_back(opcode_base + 0x01); // ADD/SUB/etc r/m16, r16
        } else if (reg_size == 32) {
          output.push_back(opcode_base + 0x01); // ADD/SUB/etc r/m32, r32
        } else {
          output.push_back(opcode_base + 0x01); // ADD/SUB/etc r/m64, r64
        }

        uint8_t modrm = 0xC0;
        modrm |= (EncodeRegister(reg) << 3);
        modrm |= EncodeRegister(reg2);
        output.push_back(modrm);

      } else if (std::holds_alternative<int64_t>(arg2)) {
        int64_t imm = std::get<int64_t>(arg2);

        if (reg_size == 8) {
          output.push_back(0x80); // ADD/SUB/etc r/m8, imm8
          uint8_t modrm = 0xC0 | (opcode_base >> 3) | EncodeRegister(reg);
          output.push_back(modrm);
          EncodeImmediate(imm, 8, output);
        } else if (reg_size == 16) {
          output.push_back(0x66);
          output.push_back(0x81); // ADD/SUB/etc r/m16, imm16
          uint8_t modrm = 0xC0 | (opcode_base >> 3) | EncodeRegister(reg);
          output.push_back(modrm);
          EncodeImmediate(imm, 16, output);
        } else if (reg_size == 32) {
          output.push_back(0x81); // ADD/SUB/etc r/m32, imm32
          uint8_t modrm = 0xC0 | (opcode_base >> 3) | EncodeRegister(reg);
          output.push_back(modrm);
          EncodeImmediate(imm, 32, output);
        } else {
          output.push_back(0x81); // ADD/SUB/etc r/m64, imm32 (sign-extended)
          uint8_t modrm = 0xC0 | (opcode_base >> 3) | EncodeRegister(reg);
          output.push_back(modrm);
          EncodeImmediate(imm, 32, output);
        }
      }
    }
  }

  return {};
}

std::expected<void, std::runtime_error> AsmToBytes::EncodeJump(const AssemblyInstruction& instr,
                                                                std::vector<uint8_t>& output) {
  uint8_t opcode = 0;

  switch (instr.command) {
    case AsmCommand::JMP:
      opcode = 0xE9;
      break; // JMP rel32
    case AsmCommand::CALL:
      opcode = 0xE8;
      break; // CALL rel32
    case AsmCommand::JE:
      opcode = 0x84;
      break; // JE rel8 (0F 84 for rel32)
    case AsmCommand::JNE:
      opcode = 0x85;
      break; // JNE rel8 (0F 85 for rel32)
    case AsmCommand::JG:
      opcode = 0x8F;
      break; // JG rel8 (0F 8F for rel32)
    case AsmCommand::JGE:
      opcode = 0x8D;
      break; // JGE rel8 (0F 8D for rel32)
    case AsmCommand::JL:
      opcode = 0x8C;
      break; // JL rel8 (0F 8C for rel32)
    case AsmCommand::JLE:
      opcode = 0x8E;
      break; // JLE rel8 (0F 8E for rel32)
    case AsmCommand::JA:
      opcode = 0x87;
      break; // JA rel8 (0F 87 for rel32)
    case AsmCommand::JAE:
      opcode = 0x83;
      break; // JAE rel8 (0F 83 for rel32)
    case AsmCommand::JB:
      opcode = 0x82;
      break; // JB rel8 (0F 82 for rel32)
    case AsmCommand::JBE:
      opcode = 0x86;
      break; // JBE rel8 (0F 86 for rel32)
    case AsmCommand::RET:
      output.push_back(0xC3);
      return {};
    default:
      break;
  }

  if (instr.command == AsmCommand::JMP || instr.command == AsmCommand::CALL) {
    if (instr.arguments.size() > 0) {
      if (auto label = instr.get_argument<std::string>(0)) {
        // Encode opcode
        output.push_back(opcode);
        // Store position where offset will be patched (after opcode, before offset)
        size_t offset_pos = output.size();
        // Encode placeholder offset (will be patched in second pass)
        EncodeImmediate(static_cast<int64_t>(0), 32, output);
        // Record this position for patching
        jump_patches_.emplace_back(offset_pos, *label);
      } else if (auto imm = instr.get_argument<int64_t>(0)) {
        output.push_back(opcode);
        EncodeImmediate(*imm, 32, output);
      }
    }
  } else {
    // Conditional jumps
    if (instr.arguments.size() > 0) {
      output.push_back(0x0F); // Two-byte opcode prefix
      if (auto label = instr.get_argument<std::string>(0)) {
        output.push_back(opcode);
        // Store position where offset will be patched
        size_t offset_pos = output.size();
        // Encode placeholder offset
        EncodeImmediate(static_cast<int64_t>(0), 32, output);
        // Record this position for patching
        jump_patches_.emplace_back(offset_pos, *label);
      } else if (auto imm = instr.get_argument<int64_t>(0)) {
        output.push_back(opcode);
        EncodeImmediate(*imm, 32, output);
      }
    }
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
  // SSE2 instructions require specific encoding
  // This is a simplified version - full implementation would handle all SSE2 variants

  if (instr.arguments.size() < 2) {
    return std::unexpected(std::runtime_error("SSE2 instruction requires at least 2 arguments"));
  }

  auto arg1 = instr.arguments[0];
  auto arg2 = instr.arguments[1];

  if (std::holds_alternative<Register>(arg1) && std::holds_alternative<Register>(arg2)) {
    Register dst = std::get<Register>(arg1);
    Register src = std::get<Register>(arg2);

    // SSE2 prefix
    output.push_back(0xF2); // REPNE prefix for some SSE2 instructions

    uint8_t opcode = 0;
    switch (instr.command) {
      case AsmCommand::ADDSD:
        opcode = 0x58;
        break;
      case AsmCommand::SUBSD:
        opcode = 0x5C;
        break;
      case AsmCommand::MULSD:
        opcode = 0x59;
        break;
      case AsmCommand::DIVSD:
        opcode = 0x5E;
        break;
      case AsmCommand::SQRTSD:
        opcode = 0x51;
        break;
      case AsmCommand::MOVSD:
        opcode = 0x10;
        break;
      default:
        break;
    }

    if (opcode != 0) {
      output.push_back(0x0F);
      output.push_back(opcode);

      uint8_t modrm = 0xC0;
      modrm |= (EncodeRegister(dst) << 3);
      modrm |= EncodeRegister(src);
      output.push_back(modrm);
    } else {
      return std::unexpected(std::runtime_error("Unsupported SSE2 instruction"));
    }
  } else {
    return std::unexpected(std::runtime_error("SSE2 instruction requires register operands"));
  }

  return {};
}

void AsmToBytes::EncodeLabel(const AssemblyInstruction& instr, std::vector<uint8_t>& output) {
  // Labels don't generate code - they're just markers
  // The label address is already stored in label_addresses_ during first pass
}

} // namespace ovum::vm::jit
