#ifndef JIT_JITEXECUTORFACTORY_HPP
#define JIT_JITEXECUTORFACTORY_HPP

#include <string>
#include <vector>

#include <tokens/Token.hpp>

#include <jit/OilCommandAsmCompiler.hpp>
#include "lib/executor/IJitExecutorFactory.hpp"

namespace ovum::vm::jit {

class JitExecutorFactory : public executor::IJitExecutorFactory {
public:
  JitExecutorFactory();
  [[nodiscard]] std::unique_ptr<executor::IJitExecutor> Create(const std::string&,
                                                               std::shared_ptr<std::vector<TokenPtr>>) const override;
};

} // namespace ovum::vm::jit

#endif // JIT_JITEXECUTORFACTORY_HPP
