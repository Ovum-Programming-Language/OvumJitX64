#include "jit/AsmCompiler.hpp"

namespace ovum::vm::jit {

struct PackedOilCommand {
  std::string command_name;
  std::vector<std::string> arguments;
};

std::expected<std::string, std::runtime_error> ExtractArgument(std::vector<TokenPtr>& oil_body, size_t& pos) {
  if (!oil_body[pos]->GetStringType().contains("LITERAL")) {
    return std::unexpected(std::runtime_error("ExtractArgument: Argument not found!"));
  }

  std::string result = oil_body[pos++]->GetLexeme();
  
  return result;
}

std::expected<PackedOilCommand, std::runtime_error> ExtractOilCommand(std::vector<TokenPtr>& oil_body, size_t& pos) {
  PackedOilCommand result;
  while (oil_body[pos]->GetStringType() != "IDENT" && oil_body.size() > pos) {
    ++pos;
  }

  if (oil_body.size() <= pos) {
    return std::unexpected(std::runtime_error("ExtractOilCommand: EOF before any command"));
  }

  result.command_name = oil_body[pos]->GetLexeme();

  if (result.command_name == "LoadLocal" ||
      result.command_name == "SetLocal" ||
      result.command_name == "LoadStatic" ||
      result.command_name == "SetStatic" ||
      result.command_name == "GetField" ||
      result.command_name == "SetField") {
    // 1 arg commands (size_t)
    auto arg1 = ExtractArgument(oil_body, pos);
    if (!arg1) {
      return std::unexpected(arg1.error());
    }
    result.arguments.push_back(arg1.value());
    
  } else if (result.command_name == "PushInt" ||
             result.command_name == "PushFloat" ||
             result.command_name == "PushBool" ||
             result.command_name == "PushChar" ||
             result.command_name == "PushByte" ||
             result.command_name == "Rotate") {
    // 1 arg (int64_t, double, bool, char, byte)
    auto arg1 = ExtractArgument(oil_body, pos);
    if (!arg1) {
      return std::unexpected(arg1.error());
    }
    result.arguments.push_back(arg1.value());
    
  } else if (result.command_name == "PushString") {
    // 1 arg (string)
    auto arg1 = ExtractArgument(oil_body, pos);
    if (!arg1) {
      return std::unexpected(arg1.error());
    }
    result.arguments.push_back(arg1.value());
    
  } else if (result.command_name == "Call" ||
             result.command_name == "CallVirtual" ||
             result.command_name == "CallConstructor" ||
             result.command_name == "GetVTable" ||
             result.command_name == "SetVTable" ||
             result.command_name == "SafeCall" ||
             result.command_name == "IsType" ||
             result.command_name == "SizeOf") {
  // 1 arg (void*)
  auto arg1 = ExtractArgument(oil_body, pos);
  if (!arg1) {
    return std::unexpected(arg1.error());
  }
  result.arguments.push_back(arg1.value());
  
  } else if (result.command_name == "PushNull" ||
             result.command_name == "Pop" ||
             result.command_name == "Dup" ||
             result.command_name == "Swap" ||
             result.command_name == "IntAdd" ||
             result.command_name == "IntSubtract" ||
             result.command_name == "IntMultiply" ||
             result.command_name == "IntDivide" ||
             result.command_name == "IntModulo" ||
             result.command_name == "IntNegate" ||
             result.command_name == "IntIncrement" ||
             result.command_name == "IntDecrement" ||
             result.command_name == "FloatAdd" ||
             result.command_name == "FloatSubtract" ||
             result.command_name == "FloatMultiply" ||
             result.command_name == "FloatDivide" ||
             result.command_name == "FloatNegate" ||
             result.command_name == "FloatSqrt" ||
             result.command_name == "ByteAdd" ||
             result.command_name == "ByteSubtract" ||
             result.command_name == "ByteMultiply" ||
             result.command_name == "ByteDivide" ||
             result.command_name == "ByteModulo" ||
             result.command_name == "ByteNegate" ||
             result.command_name == "ByteIncrement" ||
             result.command_name == "ByteDecrement" ||
             result.command_name == "IntEqual" ||
             result.command_name == "IntNotEqual" ||
             result.command_name == "IntLessThan" ||
             result.command_name == "IntLessEqual" ||
             result.command_name == "IntGreaterThan" ||
             result.command_name == "IntGreaterEqual" ||
             result.command_name == "FloatEqual" ||
             result.command_name == "FloatNotEqual" ||
             result.command_name == "FloatLessThan" ||
             result.command_name == "FloatLessEqual" ||
             result.command_name == "FloatGreaterThan" ||
             result.command_name == "FloatGreaterEqual" ||
             result.command_name == "ByteEqual" ||
             result.command_name == "ByteNotEqual" ||
             result.command_name == "ByteLessThan" ||
             result.command_name == "ByteLessEqual" ||
             result.command_name == "ByteGreaterThan" ||
             result.command_name == "ByteGreaterEqual" ||
             result.command_name == "BoolAnd" ||
             result.command_name == "BoolOr" ||
             result.command_name == "BoolNot" ||
             result.command_name == "BoolXor" ||
             result.command_name == "IntAnd" ||
             result.command_name == "IntOr" ||
             result.command_name == "IntXor" ||
             result.command_name == "IntNot" ||
             result.command_name == "IntLeftShift" ||
             result.command_name == "IntRightShift" ||
             result.command_name == "ByteAnd" ||
             result.command_name == "ByteOr" ||
             result.command_name == "ByteXor" ||
             result.command_name == "ByteNot" ||
             result.command_name == "ByteLeftShift" ||
             result.command_name == "ByteRightShift" ||
             result.command_name == "StringConcat" ||
             result.command_name == "StringLength" ||
             result.command_name == "StringSubstring" ||
             result.command_name == "StringCompare" ||
             result.command_name == "StringToInt" ||
             result.command_name == "StringToFloat" ||
             result.command_name == "IntToString" ||
             result.command_name == "FloatToString" ||
             result.command_name == "IntToFloat" ||
             result.command_name == "FloatToInt" ||
             result.command_name == "ByteToInt" ||
             result.command_name == "CharToByte" ||
             result.command_name == "ByteToChar" ||
             result.command_name == "BoolToByte" ||
             result.command_name == "CallIndirect" ||
             result.command_name == "Return" ||
             result.command_name == "Break" ||
             result.command_name == "Continue" ||
             result.command_name == "Unwrap" ||
             result.command_name == "NullCoalesce" ||
             result.command_name == "IsNull" ||
             result.command_name == "Print" ||
             result.command_name == "PrintLine" ||
             result.command_name == "ReadLine" ||
             result.command_name == "ReadChar" ||
             result.command_name == "ReadInt" ||
             result.command_name == "ReadFloat" ||
             result.command_name == "UnixTime" ||
             result.command_name == "UnixTimeMs" ||
             result.command_name == "UnixTimeNs" ||
             result.command_name == "NanoTime" ||
             result.command_name == "FormatDateTime" ||
             result.command_name == "ParseDateTime" ||
             result.command_name == "FileExists" ||
             result.command_name == "DirectoryExists" ||
             result.command_name == "CreateDir" ||
             result.command_name == "DeleteFileByName" ||
             result.command_name == "DeleteDir" ||
             result.command_name == "MoveFileByName" ||
             result.command_name == "CopyFileByName" ||
             result.command_name == "ListDir" ||
             result.command_name == "GetCurrentDir" ||
             result.command_name == "ChangeDir" ||
             result.command_name == "SleepMs" ||
             result.command_name == "SleepNs" ||
             result.command_name == "Exit" ||
             result.command_name == "GetProcessId" ||
             result.command_name == "GetEnvironmentVar" ||
             result.command_name == "SetEnvironmentVar" ||
             result.command_name == "Random" ||
             result.command_name == "RandomRange" ||
             result.command_name == "RandomFloat" ||
             result.command_name == "RandomFloatRange" ||
             result.command_name == "SeedRandom" ||
             result.command_name == "GetMemoryUsage" ||
             result.command_name == "GetPeakMemoryUsage" ||
             result.command_name == "ForceGarbageCollection" ||
             result.command_name == "GetProcessorCount" ||
             result.command_name == "GetOsName" ||
             result.command_name == "GetOsVersion" ||
             result.command_name == "GetArchitecture" ||
             result.command_name == "GetUsername" ||
             result.command_name == "GetHomeDir" ||
             result.command_name == "TypeOf" ||
             result.command_name == "Interop") {
    // 0 arg commands

  } else {
    return std::unexpected(std::runtime_error("Unknown command: " + result.command_name));
  }

  return result;
}

std::vector<AssemblyInstruction> CompilePackedCommand(PackedOilCommand& packed_oil_command) {
  std::vector<AssemblyInstruction> result;
  return result;
}

std::expected<std::vector<std::string>, std::runtime_error> CompilePackedCommands(std::vector<PackedOilCommand>& packed_oil_body) {
  return std::unexpected(std::runtime_error("CompilePackedCommands: not implemented"));
}

std::expected<std::vector<std::string>, std::runtime_error> SimpleAsmCompile(std::vector<TokenPtr>& oil_body) {
  std::vector<std::string> result;
  std::vector<PackedOilCommand> packed_commands;
  size_t pos = 0;

  while (pos < oil_body.size()) {
    auto res = ExtractOilCommand(oil_body, pos);
    if (!res && res.error().what() != std::string("EOF before any command")) {
      return std::unexpected(res.error());
    }
    packed_commands.push_back(res.value());
  }

  //перебрать packed_commands и преобразовать в ассемблер

  return result;
}

} // namespace ovum::vm::jit
