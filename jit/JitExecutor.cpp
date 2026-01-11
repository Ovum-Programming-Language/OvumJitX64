#include "JitExecutor.hpp"

#include <iostream>

namespace ovum::vm::jit {


JitExecutor::JitExecutor(std::shared_ptr<std::vector<TokenPtr>> jit_body) {
  this->oil_body = jit_body;
  
  this->TryCompile();
}

bool JitExecutor::TryCompile() const {
  auto oil_body_vec_ptr = this->oil_body.get();

  if (oil_body_vec_ptr == nullptr) {
    return false;
  }

  auto oil_body_vec = *oil_body_vec_ptr;

  //auto asm_body = SimpleAsmCompile(oil_body_vec);

  for (auto token : oil_body_vec) {
    std::cout << token->GetLexeme() << "    " << token->GetStringType() << "    " << std::endl;
  }

  return false;
}

std::expected<void, std::runtime_error> JitExecutor::Run(
      execution_tree::PassedExecutionData& data) {
  return std::unexpected(std::runtime_error("JitExecutor::Run: not implemented"));
}

} // namespace ovum::vm::jit
