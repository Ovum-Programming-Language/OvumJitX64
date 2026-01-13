#ifndef JIT_MACHINECODEFUNCTION_HPP
#define JIT_MACHINECODEFUNCTION_HPP

#include <vector>
#include <cstdint>
#include <cstring>

#include "ExecutableMemory.hpp"

namespace ovum::vm::jit {

template<typename Func>
class MachineCodeFunction {
  ExecutableMemory memory;
    
public:
  MachineCodeFunction(const code_vector& code) : memory(code.size()) {
    memcpy(memory.data(), code.data(), code.size());
    memory.make_executable();
  }
  
  Func* get() const {
    return reinterpret_cast<Func*>(memory.data());
  }
  
  auto operator()(auto... args) const {
    return get()(args...);
  }
};

} // namespace ovum::vm::jit

#endif // JIT_MACHINECODEFUNCTION_HPP