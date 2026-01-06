#include "JitExecutorFactory.hpp"

#include "JitExecutor.hpp"

namespace ovum::vm::jit {

std::unique_ptr<executor::IJitExecutor> JitExecutorFactory::Create(const std::string&) const {
  return std::make_unique<JitExecutor>();
}

} // namespace ovum::vm::jit
