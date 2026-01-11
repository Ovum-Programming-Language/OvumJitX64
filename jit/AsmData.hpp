#ifndef JIT_ASMDATA_HPP
#define JIT_ASMDATA_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <concepts>

#include <tokens/Token.hpp>

namespace ovum::vm::jit {

enum class AsmCommand : uint16_t {
  MOV = 0x100,
  MOVSX,
  MOVZX,
  LEA,
  XCHG,
  
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
  SETO = 0x580,    // Set byte if overflow (OF=1)
  SETNO,           // Set byte if not overflow (OF=0)
  SETB,            // Set byte if below (CF=1) / SETNAE
  SETNB,           // Set byte if not below (CF=0) / SETAE
  SETZ,            // Set byte if zero (ZF=1) / SETE
  SETNZ,           // Set byte if not zero (ZF=0) / SETNE
  SETBE,           // Set byte if below or equal (CF=1 or ZF=1) / SETNA
  SETNBE,          // Set byte if not below or equal (CF=0 and ZF=0) / SETA
  SETS,            // Set byte if sign (SF=1)
  SETNS,           // Set byte if not sign (SF=0)
  SETP,            // Set byte if parity (PF=1) / SETPE
  SETNP,           // Set byte if not parity (PF=0) / SETPO
  SETL,            // Set byte if less (SF≠OF) / SETNGE
  SETNL,           // Set byte if not less (SF=OF) / SETGE
  SETLE,           // Set byte if less or equal (ZF=1 or SF≠OF) / SETNG
  SETNLE,          // Set byte if not less or equal (ZF=0 and SF=OF) / SETG
  
  // Синонимы для совместимости
  SETE = SETZ,     // Equal
  SETNE = SETNZ,   // Not equal
  SETA = SETNBE,   // Above
  SETAE = SETNB,   // Above or equal
  SETNA = SETBE,   // Not above
  SETNAE = SETB,   // Not above or equal
  SETG = SETNLE,   // Greater
  SETGE = SETNL,   // Greater or equal
  SETNG = SETLE,   // Not greater
  SETNGE = SETL,   // Not greater or equal
  
  PUSH = 0x600,
  POP,
  PUSHF,
  POPF,
  
  MOVSB = 0x700,
  MOVSW,
  MOVSD,
  MOVSQ,
  CMPSB,
  CMPSW,
  CMPSD,
  CMPSQ,
  
  SYSCALL = 0x800,
  INT,
  IRET,
  
  NOP = 0x900,
  HLT,
  CLC,
  STC,
  CMC,
  CMP
};

enum class Register : uint8_t {
  RAX = 0,
  RBX,
  RCX,
  RDX,
  RSI,
  RDI,
  RBP,
  RSP,
  R8,
  R9,
  R10,
  R11,
  R12,
  R13,
  R14,
  R15,
  RIP,
  
  EAX = 0x20,
  EBX,
  ECX,
  EDX,
  ESI,
  EDI,
  EBP,
  ESP,
  R8D,
  R9D,
  R10D,
  R11D,
  R12D,
  R13D,
  R14D,
  R15D,
  
  AX = 0x40,
  BX,
  CX,
  DX,
  SI,
  DI,
  BP,
  SP,
  R8W,
  R9W,
  R10W,
  R11W,
  R12W,
  R13W,
  R14W,
  R15W,
  
  AL = 0x60,
  BL,
  CL,
  DL,
  AH,
  BH,
  CH,
  DH,
  SIL,
  DIL,
  BPL,
  SPL,
  R8B,
  R9B,
  R10B,
  R11B,
  R12B,
  R13B,
  R14B,
  R15B,
  
  CS = 0x80,
  DS,
  ES,
  FS,
  GS,
  SS,
  
  CR0 = 0xA0,
  CR1,
  CR2,
  CR3,
  CR4,
  
  DR0 = 0xB0,
  DR1,
  DR2,
  DR3,
  DR6,
  DR7,
  
  MM0 = 0xC0,
  MM1,
  MM2,
  MM3,
  MM4,
  MM5,
  MM6,
  MM7,
  XMM0 = 0xD0,
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
  YMM0 = 0xE0,
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
  ZMM0 = 0xF0,
  ZMM1,
  ZMM2,
  ZMM3,
  ZMM4,
  ZMM5,
  ZMM6,
  ZMM7
};

template<typename T>
concept AssemblerArgument = requires {
  requires std::same_as<T, Register> ||
           std::same_as<T, int64_t> ||
           std::same_as<T, uint64_t> ||
           std::same_as<T, std::string> ||
           std::same_as<T, float> ||
           std::same_as<T, double>;
};

struct MemoryAddress {
  std::optional<Register> base;
  std::optional<Register> index;
  uint8_t scale = 1;
  int64_t displacement = 0;
  std::optional<Register> segment;
  
  auto operator<=>(const MemoryAddress&) const = default;
};

using Argument = std::variant<
  Register,
  MemoryAddress,
  int64_t,
  uint64_t,
  std::string,
  float,
  double
>;

struct AssemblyInstruction {
  AsmCommand command;
  std::vector<Argument> arguments;
  
  constexpr AssemblyInstruction() = default;
  
  explicit constexpr AssemblyInstruction(AsmCommand cmd) noexcept : command(cmd) {}
  
  constexpr AssemblyInstruction(AsmCommand cmd, std::initializer_list<Argument> args) 
    : command(cmd), arguments(args) {}
  
  constexpr AssemblyInstruction(AsmCommand cmd, std::vector<Argument> args) 
    : command(cmd), arguments(std::move(args)) {}
  
  constexpr size_t argument_count() const noexcept { 
    return arguments.size(); 
  }
  
  template<AssemblerArgument T>
  constexpr std::optional<T> get_argument(size_t index) const noexcept {
    if (index >= arguments.size()) return std::nullopt;
    
    if (const T* value = std::get_if<T>(&arguments[index])) {
      return *value;
    }
    return std::nullopt;
  }
  
  template<AssemblerArgument T>
  constexpr bool is_argument_type(size_t index) const noexcept {
    if (index >= arguments.size()) return false;
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

constexpr MemoryAddress create_memory_addr(
  std::optional<Register> base = std::nullopt,
  std::optional<Register> index = std::nullopt,
  uint8_t scale = 1,
  int64_t displacement = 0,
  std::optional<Register> segment = std::nullopt) noexcept {
  
  return MemoryAddress{
    .base = base,
    .index = index,
    .scale = scale,
    .displacement = displacement,
    .segment = segment
  };
}

constexpr MemoryAddress addr(Register base, int64_t disp = 0) noexcept {
  return create_memory_addr(base, std::nullopt, 1, disp);
}

constexpr MemoryAddress indexed_addr(
  Register base, Register index, uint8_t scale = 1, int64_t disp = 0) noexcept {
  
  return create_memory_addr(base, index, scale, disp);
}

class AssemblyInstructionBuilder {
private:
  AssemblyInstruction instr;
  
public:
  constexpr explicit AssemblyInstructionBuilder(AsmCommand cmd) noexcept 
    : instr(cmd) {}
  
  template<AssemblerArgument T>
  constexpr AssemblyInstructionBuilder& add_arg(T&& arg) {
    if constexpr (std::same_as<std::decay_t<T>, Register>) {
      instr.arguments.push_back(make_reg_arg(std::forward<T>(arg)));
    }
    else if constexpr (std::same_as<std::decay_t<T>, int64_t>) {
      instr.arguments.push_back(make_imm_arg(std::forward<T>(arg)));
    }
    else if constexpr (std::same_as<std::decay_t<T>, std::string>) {
      instr.arguments.push_back(make_label_arg(std::forward<T>(arg)));
    }
    else if constexpr (std::same_as<std::decay_t<T>, uint64_t>) {
      instr.arguments.push_back(make_uimm_arg(std::forward<T>(arg)));
    }
    return *this;
  }
  
  AssemblyInstructionBuilder& add_mem(MemoryAddress addr) noexcept {
    instr.arguments.push_back(make_mem_arg(std::move(addr)));
    return *this;
  }
  
  constexpr AssemblyInstruction build() const & {
    return instr;
  }
  
  constexpr AssemblyInstruction build() && {
    return std::move(instr);
  }
};

} // namespace ovum::vm::jit

#endif // JIT_ASMDATA_HPP