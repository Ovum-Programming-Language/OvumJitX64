#ifndef JIT_ASMDATA_HPP
#define JIT_ASMDATA_HPP

#include <concepts>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <tokens/Token.hpp>

namespace ovum::vm::jit {

enum class AsmCommand : uint16_t {
  // Integer and general operations
  MOV = 0x100,
  MOVSX,
  MOVZX,
  LEA,
  XCHG,
  MOVQ,      // Move quadword between XMM and integer registers

  ADD = 0x200,
  SUB,
  IMUL,
  MUL,
  IDIV,
  DIV,
  INC,
  DEC,
  NEG,

  AND = 0x300,
  OR,
  XOR,
  NOT,
  TEST,

  SHL = 0x400,
  SHR,
  SAR,
  ROL,
  ROR,
  RCL,
  RCR,

  JMP = 0x500,
  CALL,
  RET,
  JE,
  JNE,
  JG,
  JGE,
  JL,
  JLE,
  JA,
  JAE,
  JB,
  JBE,
  LOOP,
  LOOPE,
  LOOPNE,

  // SETcc commands - conditional set byte
  SETO = 0x580, // Set byte if overflow (OF=1)
  SETNO,        // Set byte if not overflow (OF=0)
  SETB,         // Set byte if below (CF=1) / SETNAE
  SETNB,        // Set byte if not below (CF=0) / SETAE
  SETZ,         // Set byte if zero (ZF=1) / SETE
  SETNZ,        // Set byte if not zero (ZF=0) / SETNE
  SETBE,        // Set byte if below or equal (CF=1 or ZF=1) / SETNA
  SETNBE,       // Set byte if not below or equal (CF=0 and ZF=0) / SETA
  SETS,         // Set byte if sign (SF=1)
  SETNS,        // Set byte if not sign (SF=0)
  SETP,         // Set byte if parity (PF=1) / SETPE
  SETNP,        // Set byte if not parity (PF=0) / SETPO
  SETL,         // Set byte if less (SF≠OF) / SETNGE
  SETNL,        // Set byte if not less (SF=OF) / SETGE
  SETLE,        // Set byte if less or equal (ZF=1 or SF≠OF) / SETNG
  SETNLE,       // Set byte if not less or equal (ZF=0 and SF=OF) / SETG

  // SSE2 Scalar Double-Precision Floating-Point Instructions
  ADDSD = 0x600,  // Add Scalar Double-Precision Floating-Point
  SUBSD,          // Subtract Scalar Double-Precision Floating-Point
  MULSD,          // Multiply Scalar Double-Precision Floating-Point
  DIVSD,          // Divide Scalar Double-Precision Floating-Point
  SQRTSD,         // Square Root Scalar Double-Precision Floating-Point
  COMISD,         // Compare Scalar Ordered Double-Precision Floating-Point
  UCOMISD,        // Unordered Compare Scalar Double-Precision Floating-Point
  CVTSI2SD,       // Convert Dword Integer to Scalar Double-Precision FP
  CVTSD2SI,       // Convert Scalar Double-Precision FP to Dword Integer
  CVTSS2SD,       // Convert Scalar Single-Precision FP to Double-Precision FP
  CVTSD2SS,       // Convert Scalar Double-Precision FP to Single-Precision FP
  
  // SSE2 Data Movement Instructions
  MOVSD,          // Move Scalar Double-Precision Floating-Point
  MOVAPD,         // Move Aligned Packed Double-Precision Floating-Point
  MOVUPD,         // Move Unaligned Packed Double-Precision Floating-Point
  
  // SSE2 Logical Instructions
  ANDPD,          // Bitwise Logical AND of Packed Double-Precision Floating-Point
  ANDNPD,         // Bitwise Logical AND NOT of Packed Double-Precision Floating-Point
  ORPD,           // Bitwise Logical OR of Packed Double-Precision Floating-Point
  XORPD,          // Bitwise Logical XOR of Packed Double-Precision Floating-Point

  // Stack operations
  PUSH = 0x700,
  POP,
  PUSHF,
  POPF,

  // String operations
  MOVSB = 0x800,
  MOVSW,
  MOVSQ,
  CMPSB,
  CMPSW,
  CMPSQ,

  // System operations
  SYSCALL = 0x900,
  INT,
  IRET,

  // Miscellaneous
  NOP = 0xA00,
  HLT,
  CLC,
  STC,
  CMC,
  CMP,
  CQO,           // Convert Quadword to Octword (sign extend RAX into RDX:RAX)

  // Flow control with labels
  LABEL = 0xB00,       // Метка для перехода
  CMOVE,               // Conditional move if equal
  CMOVNE,              // Conditional move if not equal
  CMOVB,               // Conditional move if below
  CMOVBE,              // Conditional move if below or equal
  CMOVA,               // Conditional move if above
  CMOVAE,              // Conditional move if above or equal
  
  // Conversions
  CVTTSD2SI,     // Convert with Truncation Scalar Double-Precision FP to Dword Integer
  CVTTSD2SIQ,    // Convert with Truncation Scalar Double-Precision FP to Quadword Integer
};

// Синонимы для совместимости
constexpr AsmCommand SETE = AsmCommand::SETZ;   // Equal
constexpr AsmCommand SETNE = AsmCommand::SETNZ; // Not equal
constexpr AsmCommand SETA = AsmCommand::SETNBE; // Above
constexpr AsmCommand SETAE = AsmCommand::SETNB; // Above or equal
constexpr AsmCommand SETNA = AsmCommand::SETBE; // Not above
constexpr AsmCommand SETNAE = AsmCommand::SETB; // Not above or equal
constexpr AsmCommand SETG = AsmCommand::SETNLE; // Greater
constexpr AsmCommand SETGE = AsmCommand::SETNL; // Greater or equal
constexpr AsmCommand SETNG = AsmCommand::SETLE; // Not greater
constexpr AsmCommand SETNGE = AsmCommand::SETL; // Not greater or equal

enum class Register : uint8_t {
  // 64-bit registers - lower 3 bits are the actual encoding
  RAX = 0,   // 000
  RCX,       // 001
  RDX,       // 010
  RBX,       // 011
  RSP,       // 100 - Special! Requires SIB byte
  RBP,       // 101 - Special behavior
  RSI,       // 110
  RDI,       // 111
  R8,        // 1000 (with REX.B=1)
  R9,        // 1001
  R10,       // 1010
  R11,       // 1011
  R12,       // 1100
  R13,       // 1101
  R14,       // 1110
  R15,       // 1111
  RIP,       // Special pseudo-register for RIP-relative addressing
  
  // 32-bit registers - same low 3 bits, different size
  EAX = 0x10,   // 0 + size flag
  ECX,          // 1
  EDX,          // 2
  EBX,          // 3
  ESP,          // 4
  EBP,          // 5
  ESI,          // 6
  EDI,          // 7
  R8D,          // 8
  R9D,          // 9
  R10D,         // 10
  R11D,         // 11
  R12D,         // 12
  R13D,         // 13
  R14D,         // 14
  R15D,         // 15
  
  // 16-bit registers
  AX = 0x20,
  CX,
  DX,
  BX,
  SP,
  BP,
  SI,
  DI,
  R8W,
  R9W,
  R10W,
  R11W,
  R12W,
  R13W,
  R14W,
  R15W,
  
  // 8-bit registers - LOW bytes
  AL = 0x30,     // Low byte of RAX
  CL,            // Low byte of RCX
  DL,            // Low byte of RDX
  BL,            // Low byte of RBX
  SPL,           // Low byte of RSP - requires REX
  BPL,           // Low byte of RBP - requires REX
  SIL,           // Low byte of RSI - requires REX
  DIL,           // Low byte of RDI - requires REX
  R8B,
  R9B,
  R10B,
  R11B,
  R12B,
  R13B,
  R14B,
  R15B,
  
  // 8-bit registers - HIGH bytes (cannot be used with REX!)
  AH = 0x40,     // High byte of RAX (bits 8-15)
  CH,            // High byte of RCX
  DH,            // High byte of RDX
  BH,            // High byte of RBX
  
  // Segment registers
  ES = 0x50,
  CS,
  SS,
  DS,
  FS,
  GS,
  
  // Control registers
  CR0 = 0x60,
  CR2,
  CR3,
  CR4,
  CR8,           // Added for x86-64
  
  // Debug registers
  DR0 = 0x70,
  DR1,
  DR2,
  DR3,
  DR6,
  DR7,
  
  // MMX registers (map to lower 64 bits of XMM registers)
  MM0 = 0x80,
  MM1,
  MM2,
  MM3,
  MM4,
  MM5,
  MM6,
  MM7,
  
  // XMM registers (SSE)
  XMM0 = 0x90,
  XMM1,
  XMM2,
  XMM3,
  XMM4,
  XMM5,
  XMM6,
  XMM7,
  XMM8,
  XMM9,
  XMM10,
  XMM11,
  XMM12,
  XMM13,
  XMM14,
  XMM15,
  
  // YMM registers (AVX)
  YMM0 = 0xA0,
  YMM1,
  YMM2,
  YMM3,
  YMM4,
  YMM5,
  YMM6,
  YMM7,
  YMM8,
  YMM9,
  YMM10,
  YMM11,
  YMM12,
  YMM13,
  YMM14,
  YMM15,
  
  // ZMM registers (AVX-512)
  ZMM0 = 0xB0,
  ZMM1,
  ZMM2,
  ZMM3,
  ZMM4,
  ZMM5,
  ZMM6,
  ZMM7,
  ZMM8,
  ZMM9,
  ZMM10,
  ZMM11,
  ZMM12,
  ZMM13,
  ZMM14,
  ZMM15,
  ZMM16,
  ZMM17,
  ZMM18,
  ZMM19,
  ZMM20,
  ZMM21,
  ZMM22,
  ZMM23,
  ZMM24,
  ZMM25,
  ZMM26,
  ZMM27,
  ZMM28,
  ZMM29,
  ZMM30,
  ZMM31
};

template<typename T>
concept AssemblerArgument = requires {
  requires std::same_as<T, Register> || std::same_as<T, int64_t> || std::same_as<T, uint64_t> ||
               std::same_as<T, std::string> || std::same_as<T, float> || std::same_as<T, double>;
};

struct MemoryAddress {
  std::optional<Register> base;
  std::optional<Register> index;
  uint8_t scale = 1;
  int64_t displacement = 0;
  std::optional<Register> segment;

  auto operator<=>(const MemoryAddress&) const = default;
};

using Argument = std::variant<Register, MemoryAddress, int64_t, uint64_t, std::string, float, double>;

struct AssemblyInstruction {
  AsmCommand command;
  std::vector<Argument> arguments;

  constexpr AssemblyInstruction() = default;

  explicit constexpr AssemblyInstruction(AsmCommand cmd) noexcept : command(cmd) {
  }

  constexpr AssemblyInstruction(AsmCommand cmd, std::initializer_list<Argument> args) : command(cmd), arguments(args) {
  }

  constexpr AssemblyInstruction(AsmCommand cmd, std::vector<Argument> args) : command(cmd), arguments(std::move(args)) {
  }

  constexpr size_t argument_count() const noexcept {
    return arguments.size();
  }

  template<AssemblerArgument T>
  constexpr std::optional<T> get_argument(size_t index) const noexcept {
    if (index >= arguments.size())
      return std::nullopt;

    if (const T* value = std::get_if<T>(&arguments[index])) {
      return *value;
    }
    return std::nullopt;
  }

  template<AssemblerArgument T>
  constexpr bool is_argument_type(size_t index) const noexcept {
    if (index >= arguments.size())
      return false;
    return std::holds_alternative<T>(arguments[index]);
  }

  auto operator<=>(const AssemblyInstruction&) const = default;
};

constexpr Argument make_reg_arg(Register reg) noexcept {
  return Argument(reg);
}

constexpr Argument make_imm_arg(int64_t value) noexcept {
  return Argument(value);
}

constexpr Argument make_uimm_arg(uint64_t value) noexcept {
  return Argument(value);
}

constexpr Argument make_mem_arg(MemoryAddress addr) noexcept {
  return Argument(std::move(addr));
}

constexpr Argument make_label_arg(std::string label) {
  return Argument(std::move(label));
}

constexpr Argument make_float_arg(float value) noexcept {
  return Argument(value);
}

constexpr Argument make_double_arg(double value) noexcept {
  return Argument(value);
}

constexpr MemoryAddress create_memory_addr(std::optional<Register> base = std::nullopt,
                                           std::optional<Register> index = std::nullopt,
                                           uint8_t scale = 1,
                                           int64_t displacement = 0,
                                           std::optional<Register> segment = std::nullopt) noexcept {
  return MemoryAddress{.base = base, .index = index, .scale = scale, .displacement = displacement, .segment = segment};
}

constexpr MemoryAddress addr(Register base, int64_t disp = 0) noexcept {
  return create_memory_addr(base, std::nullopt, 1, disp);
}

constexpr MemoryAddress indexed_addr(Register base, Register index, uint8_t scale = 1, int64_t disp = 0) noexcept {
  return create_memory_addr(base, index, scale, disp);
}

class AssemblyInstructionBuilder {
private:
  AssemblyInstruction instr;

public:
  constexpr explicit AssemblyInstructionBuilder(AsmCommand cmd) noexcept : instr(cmd) {
  }

  template<AssemblerArgument T>
  constexpr AssemblyInstructionBuilder& add_arg(T&& arg) {
    if constexpr (std::same_as<std::decay_t<T>, Register>) {
      instr.arguments.push_back(make_reg_arg(std::forward<T>(arg)));
    } else if constexpr (std::same_as<std::decay_t<T>, int64_t>) {
      instr.arguments.push_back(make_imm_arg(std::forward<T>(arg)));
    } else if constexpr (std::same_as<std::decay_t<T>, std::string>) {
      instr.arguments.push_back(make_label_arg(std::forward<T>(arg)));
    } else if constexpr (std::same_as<std::decay_t<T>, uint64_t>) {
      instr.arguments.push_back(make_uimm_arg(std::forward<T>(arg)));
    } else if constexpr (std::same_as<std::decay_t<T>, float>) {
      instr.arguments.push_back(make_float_arg(std::forward<T>(arg)));
    } else if constexpr (std::same_as<std::decay_t<T>, double>) {
      instr.arguments.push_back(make_double_arg(std::forward<T>(arg)));
    }
    return *this;
  }

  AssemblyInstructionBuilder& add_mem(MemoryAddress addr) noexcept {
    instr.arguments.push_back(make_mem_arg(std::move(addr)));
    return *this;
  }

  constexpr AssemblyInstruction build() const& {
    return instr;
  }

  constexpr AssemblyInstruction build() && {
    return std::move(instr);
  }
};

} // namespace ovum::vm::jit

#endif // JIT_ASMDATA_HPP