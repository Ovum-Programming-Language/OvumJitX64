#ifndef JIT_ASMCOMPILER_HPP
#define JIT_ASMCOMPILER_HPP

#include <expected>
#include <stdexcept>
#include <string>
#include <vector>

#include <tokens/Token.hpp>
#include "jit/AsmData.hpp"

namespace ovum::vm::jit {

struct PackedOilCommand {
  std::string command_name;
  std::vector<std::string> arguments;
};

std::expected<std::vector<PackedOilCommand>, std::runtime_error> PackOilCommands(std::vector<TokenPtr>& oil_body);

} // namespace ovum::vm::jit

#endif // JIT_ASMCOMPILER_HPP
