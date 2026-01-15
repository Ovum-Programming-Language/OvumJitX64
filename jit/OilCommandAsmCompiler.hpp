#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <jit/AsmCompiler.hpp>
#include <jit/machine-code-runner/AsmDataBuffer.hpp>
#include <jit/oil-to-asm-realisation/AsmComplexOperationManager.hpp>
#include "AsmData.hpp"

namespace ovum::vm::jit {

const uint64_t ShadowSpaceSizeBytes = 32;

std::vector<AssemblyInstruction> CreateOperationCaller(CalledOperationCode op_code);
std::vector<AssemblyInstruction> CreateArgumentPlacer(std::vector<std::string> args);

class OilCommandAsmCompiler {
private:
  static const size_t s_all_command_num = 125;

public:
  OilCommandAsmCompiler() = delete;
  OilCommandAsmCompiler(const OilCommandAsmCompiler&) = delete;
  OilCommandAsmCompiler(OilCommandAsmCompiler&&) = delete;
  OilCommandAsmCompiler& operator=(const OilCommandAsmCompiler&) = delete;
  OilCommandAsmCompiler& operator=(OilCommandAsmCompiler&&) = delete;

  [[nodiscard]] static const std::vector<AssemblyInstruction> Compile(std::vector<PackedOilCommand>& packed_oil_body);

  [[nodiscard]] static const std::vector<AssemblyInstruction>& GetAssemblyForCommand(
      std::string_view command_name) noexcept {
    static const std::vector<AssemblyInstruction> empty_vector;

    const auto it = s_command_assemblers.find(command_name);
    if (it != s_command_assemblers.end()) {
      return it->second;
    }
    return empty_vector;
  }

  [[nodiscard]] static const std::vector<AssemblyInstruction> GetAssemblyForCommandWithArgs(
      std::string_view command_name, std::vector<std::string>& command_args) noexcept {
    static const std::vector<AssemblyInstruction> empty_vector;
    std::vector<AssemblyInstruction> result;

    const auto it = s_command_assemblers.find(command_name);
    if (it != s_command_assemblers.end()) {
      auto arg_placer = CreateArgumentPlacer(command_args);
      result.insert(result.end(), arg_placer.begin(), arg_placer.end());
      result.insert(result.end(), it->second.begin(), it->second.end());
      return result;
    }
    return empty_vector;
  }

  [[nodiscard]] static bool HasAssemblyForCommand(std::string_view command_name) noexcept {
    return s_command_assemblers.find(command_name) != s_command_assemblers.end();
  }

  [[nodiscard]] static const std::array<std::string_view, s_all_command_num>& GetAllCommandNames() noexcept {
    return s_all_command_names;
  }

  static bool RegisterCustomAssembly(std::string_view command_name, std::vector<AssemblyInstruction>&& instructions) {
    return s_command_assemblers.emplace(command_name, std::move(instructions)).second;
  }

  static void InitializeStandardAssemblers();

private:
  static const std::array<std::string_view, s_all_command_num> s_all_command_names;

  static std::unordered_map<std::string_view, std::vector<AssemblyInstruction>> s_command_assemblers;

  static void InitializeStackOperations();

  static void InitializeIntegerOperations();

  static void InitializeFloatOperations();

  static void InitializeByteOperations();

  static void InitializeBooleanOperations();

  static void InitializeStringOperations();

  static void InitializeConversionOperations();

  static void InitializeControlFlowOperations();

  static void InitializeInputOutputOperations();

  static void InitializeSystemOperations();

  static void InitializeFileOperations();

  static void InitializeTimeOperations();

  static void InitializeProcessOperations();

  static void InitializeOSOperations();

  static void InitializeRandomOperations();

  static void InitializeMemoryOperations();

  static void InitializeLocalDataOperations();

  static void AddStandardAssembly(std::string_view command_name, std::vector<AssemblyInstruction>&& instructions);
};

} // namespace ovum::vm::jit
