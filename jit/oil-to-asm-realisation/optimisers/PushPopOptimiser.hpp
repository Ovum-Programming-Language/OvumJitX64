#include <algorithm>
#include <vector>

#include <jit/AsmData.hpp>

namespace ovum::vm::jit {

std::vector<AssemblyInstruction> optimize_push_pop_pairs(const std::vector<AssemblyInstruction>& instructions);

} // namespace ovum::vm::jit
