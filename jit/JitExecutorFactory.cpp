#include "JitExecutorFactory.hpp"

#include "JitExecutor.hpp"

#include <iostream>

namespace ovum::vm::jit {

JitExecutorFactory::JitExecutorFactory() {
  OilCommandAsmCompiler::InitializeStandardAssemblers();
}

std::unique_ptr<executor::IJitExecutor> JitExecutorFactory::Create(
    const std::string& function_name, std::shared_ptr<std::vector<TokenPtr>> jit_body) const {
  JitExecutor executor(jit_body);
  return std::make_unique<JitExecutor>(executor);
}

} // namespace ovum::vm::jit
