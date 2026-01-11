#ifndef JIT_ASMCOMPILER_HPP
#define JIT_ASMCOMPILER_HPP

#include <expected>
#include <stdexcept>
#include <string>
#include <vector>

#include <tokens/Token.hpp>
#include "jit/AsmData.hpp"

namespace ovum::vm::jit {

std::expected<std::vector<std::string>, std::runtime_error> SimpleAsmCompile(std::vector<TokenPtr>& oil_body);

} // namespace ovum::vm::jit

#endif // JIT_ASMCOMPILER_HPP
