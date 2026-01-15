#include "PushPopOptimiser.hpp"

namespace ovum::vm::jit {

std::vector<AssemblyInstruction> optimize_push_pop_pairs(
  const std::vector<AssemblyInstruction>& instructions) {
  
  std::vector<AssemblyInstruction> optimized;
  std::vector<std::pair<size_t, size_t>> push_pop_pairs; // пары индексов (push, pop)
  
  // Сначала находим все пары PUSH RAX / POP RAX
  for (size_t i = 0; i < instructions.size(); ++i) {
    if (instructions[i].command == AsmCommand::PUSH &&
      !instructions[i].arguments.empty() &&
      instructions[i].get_argument<Register>(0) == Register::RAX) {
      
      // Ищем соответствующий POP RAX
      for (size_t j = i + 1; j < instructions.size(); ++j) {
        if (instructions[j].command == AsmCommand::POP &&
          !instructions[j].arguments.empty() &&
          instructions[j].get_argument<Register>(0) == Register::RAX) {
          
          // Проверяем, что между ними нет других операций с RAX
          // которые бы нарушали возможность удаления
          bool can_remove = true;
          for (size_t k = i + 1; k < j; ++k) {
            const auto& instr = instructions[k];
            // Если между push и pop есть операции, которые читают/пишут RAX,
            // то удалять нельзя
            if (!instr.arguments.empty()) {
              for (int arg = 0; arg < instr.arguments.size(); ++arg) {
                if (instr.get_argument<Register>(arg) == Register::RAX) {
                  can_remove = false;
                  break;
                }
              }
            }
            if (!can_remove) break;
          }
          
          if (can_remove) {
            push_pop_pairs.emplace_back(i, j);
            i = j; // пропускаем до конца этой пары
            break;
          }
        }
      }
    }
  }
  
  // Теперь строим оптимизированный список, пропуская удаляемые инструкции
  std::vector<bool> to_remove(instructions.size(), false);
  for (const auto& pair : push_pop_pairs) {
    to_remove[pair.first] = true;
    to_remove[pair.second] = true;
  }
  
  for (size_t i = 0; i < instructions.size(); ++i) {
    if (!to_remove[i]) {
      optimized.push_back(instructions[i]);
    }
  }
  
  return optimized;
}

} // namespace ovum::vm::jit
