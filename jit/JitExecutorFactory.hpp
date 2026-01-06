#ifndef JIT_JITEXECUTORFACTORY_HPP
#define JIT_JITEXECUTORFACTORY_HPP

#include <string>

#include "lib/executor/IJitExecutorFactory.hpp"

namespace ovum::vm::jit {

class JitExecutorFactory : public executor::IJitExecutorFactory {
public:
  [[nodiscard]] std::unique_ptr<executor::IJitExecutor> Create(const std::string&) const override;
};

} // namespace ovum::vm::jit

#endif // JIT_JITEXECUTORFACTORY_HPP
