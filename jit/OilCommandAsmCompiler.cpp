#include "OilCommandAsmCompiler.hpp"

namespace ovum::vm::jit {

const std::array<std::string_view, OilCommandAsmCompiler::s_all_command_num>
    OilCommandAsmCompiler::s_all_command_names = {"PushNull",
                                                  "Pop",
                                                  "Dup",
                                                  "Swap",
                                                  "IntAdd",
                                                  "IntSubtract",
                                                  "IntMultiply",
                                                  "IntDivide",
                                                  "IntModulo",
                                                  "IntNegate",
                                                  "IntIncrement",
                                                  "IntDecrement",
                                                  "FloatAdd",
                                                  "FloatSubtract",
                                                  "FloatMultiply",
                                                  "FloatDivide",
                                                  "FloatNegate",
                                                  "FloatSqrt",
                                                  "ByteAdd",
                                                  "ByteSubtract",
                                                  "ByteMultiply",
                                                  "ByteDivide",
                                                  "ByteModulo",
                                                  "ByteNegate",
                                                  "ByteIncrement",
                                                  "ByteDecrement",
                                                  "IntEqual",
                                                  "IntNotEqual",
                                                  "IntLessThan",
                                                  "IntLessEqual",
                                                  "IntGreaterThan",
                                                  "IntGreaterEqual",
                                                  "FloatEqual",
                                                  "FloatNotEqual",
                                                  "FloatLessThan",
                                                  "FloatLessEqual",
                                                  "FloatGreaterThan",
                                                  "FloatGreaterEqual",
                                                  "ByteEqual",
                                                  "ByteNotEqual",
                                                  "ByteLessThan",
                                                  "ByteLessEqual",
                                                  "ByteGreaterThan",
                                                  "ByteGreaterEqual",
                                                  "BoolAnd",
                                                  "BoolOr",
                                                  "BoolNot",
                                                  "BoolXor",
                                                  "IntAnd",
                                                  "IntOr",
                                                  "IntXor",
                                                  "IntNot",
                                                  "IntLeftShift",
                                                  "IntRightShift",
                                                  "ByteAnd",
                                                  "ByteOr",
                                                  "ByteXor",
                                                  "ByteNot",
                                                  "ByteLeftShift",
                                                  "ByteRightShift",
                                                  "StringConcat",
                                                  "StringLength",
                                                  "StringSubstring",
                                                  "StringCompare",
                                                  "StringToInt",
                                                  "StringToFloat",
                                                  "IntToString",
                                                  "FloatToString",
                                                  "IntToFloat",
                                                  "FloatToInt",
                                                  "ByteToInt",
                                                  "CharToByte",
                                                  "ByteToChar",
                                                  "BoolToByte",
                                                  "CallIndirect",
                                                  "Return",
                                                  "Break",
                                                  "Continue",
                                                  "Unwrap",
                                                  "NullCoalesce",
                                                  "IsNull",
                                                  "Print",
                                                  "PrintLine",
                                                  "ReadLine",
                                                  "ReadChar",
                                                  "ReadInt",
                                                  "ReadFloat",
                                                  "UnixTime",
                                                  "UnixTimeMs",
                                                  "UnixTimeNs",
                                                  "NanoTime",
                                                  "FormatDateTime",
                                                  "ParseDateTime",
                                                  "FileExists",
                                                  "DirectoryExists",
                                                  "CreateDir",
                                                  "DeleteFileByName",
                                                  "DeleteDir",
                                                  "MoveFileByName",
                                                  "CopyFileByName",
                                                  "ListDir",
                                                  "GetCurrentDir",
                                                  "ChangeDir",
                                                  "SleepMs",
                                                  "SleepNs",
                                                  "Exit",
                                                  "GetProcessId",
                                                  "GetEnvironmentVar",
                                                  "SetEnvironmentVar",
                                                  "Random",
                                                  "RandomRange",
                                                  "RandomFloat",
                                                  "RandomFloatRange",
                                                  "SeedRandom",
                                                  "GetMemoryUsage",
                                                  "GetPeakMemoryUsage",
                                                  "ForceGarbageCollection",
                                                  "GetProcessorCount",
                                                  "GetOsName",
                                                  "GetOsVersion",
                                                  "GetArchitecture",
                                                  "GetUsername",
                                                  "GetHomeDir",
                                                  "TypeOf",
                                                  "Interop"};

std::unordered_map<std::string_view, std::vector<AssemblyInstruction>> OilCommandAsmCompiler::s_command_assemblers;

void OilCommandAsmCompiler::InitializeStandardAssemblers() {
  InitializeStackOperations();
  InitializeIntegerOperations();
  InitializeFloatOperations();
  InitializeByteOperations();
  InitializeBooleanOperations();
  InitializeStringOperations();
  InitializeConversionOperations();
  InitializeControlFlowOperations();
  InitializeInputOutputOperations();
  InitializeSystemOperations();
  InitializeFileOperations();
  InitializeTimeOperations();
  InitializeProcessOperations();
  InitializeOSOperations();
  InitializeRandomOperations();
  InitializeMemoryOperations();
}

void OilCommandAsmCompiler::AddStandardAssembly(std::string_view command_name,
                                                std::vector<AssemblyInstruction>&& instructions) {
  s_command_assemblers.emplace(command_name, std::move(instructions));
}
/*
void OilCommandAsmCompiler::InitializeStringOperations() {
    std::vector<AssemblyInstruction> string_concat_asm = {
        {".string_concat_start:", {}, "Label for string concatenation", true, 0},
        {"pop", {"rsi"}, "Pop second string", false, 1},
        {"pop", {"rdi"}, "Pop first string", false, 1},
        {"call", {"_string_concat_impl"}, "Call string concatenation", false, 5},
        {"push", {"rax"}, "Push result string", false, 1},
        {"add", {"r11", "1"}, "Increment instruction pointer", false, 4}
    };
    AddStandardAssembly("StringConcat", std::move(string_concat_asm));

    // StringLength, StringSubstring, StringCompare аналогично...
}

void OilCommandAsmCompiler::InitializeConversionOperations() {
    std::vector<AssemblyInstruction> int_to_string_asm = {
        {"pop", {"rdi"}, "Pop integer value", false, 1},
        {"call", {"_int_to_string_impl"}, "Call conversion", false, 5},
        {"push", {"rax"}, "Push result string", false, 1},
        {"add", {"r11", "1"}, "Increment instruction pointer", false, 4}
    };
    AddStandardAssembly("IntToString", std::move(int_to_string_asm));

    // FloatToString, StringToInt, StringToFloat и другие аналогично...
}

void OilCommandAsmCompiler::InitializeControlFlowOperations() {
    std::vector<AssemblyInstruction> return_asm = {
        {"pop", {"rax"}, "Pop return value", false, 1},
        {"mov", {"rsp", "rbp"}, "Restore stack pointer", false, 3},
        {"pop", {"rbp"}, "Restore base pointer", false, 1},
        {"ret", {}, "Return from function", false, 1}
    };
    AddStandardAssembly("Return", std::move(return_asm));

    // CallIndirect, Break, Continue, Unwrap, NullCoalesce, IsNull аналогично...
}

void OilCommandAsmCompiler::InitializeInputOutputOperations() {
    std::vector<AssemblyInstruction> print_asm = {
        {"pop", {"rdi"}, "Pop value to print", false, 1},
        {"call", {"_print_impl"}, "Call print function", false, 5},
        {"add", {"r11", "1"}, "Increment instruction pointer", false, 4}
    };
    AddStandardAssembly("Print", std::move(print_asm));

    // PrintLine, ReadLine, ReadChar, ReadInt, ReadFloat аналогично...
}

void OilCommandAsmCompiler::InitializeSystemOperations() {
    std::vector<AssemblyInstruction> exit_asm = {
        {"pop", {"rdi"}, "Pop exit code", false, 1},
        {"mov", {"rax", "60"}, "syscall number for exit", false, 5},
        {"syscall", {}, "Invoke system call", false, 2}
    };
    AddStandardAssembly("Exit", std::move(exit_asm));

    // SleepMs, SleepNs, GetProcessId и другие аналогично...
}
*/
} // namespace ovum::vm::jit
