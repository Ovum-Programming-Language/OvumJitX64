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
  // Инициализация всех категорий команд
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
void OilCommandAsmCompiler::InitializeStackOperations() {
    std::vector<AssemblyInstruction> push_null_asm = {
        {"mov", {"rax", "0"}, "Push null value", false, 7},
        {"push", {"rax"}, "", false, 1},
        {"add", {"r11", "1"}, "Increment instruction pointer", false, 4}
    };
    AddStandardAssembly("PushNull", std::move(push_null_asm));

    std::vector<AssemblyInstruction> pop_asm = {
        {"pop", {"rax"}, "Pop value from stack", false, 1},
        {"add", {"r11", "1"}, "Increment instruction pointer", false, 4}
    };
    AddStandardAssembly("Pop", std::move(pop_asm));

    std::vector<AssemblyInstruction> dup_asm = {
        {"mov", {"rax", "[rsp]"}, "Load top value", false, 4},
        {"push", {"rax"}, "Duplicate it", false, 1},
        {"add", {"r11", "1"}, "Increment instruction pointer", false, 4}
    };
    AddStandardAssembly("Dup", std::move(dup_asm));

    std::vector<AssemblyInstruction> swap_asm = {
        {"pop", {"rax"}, "Pop first value", false, 1},
        {"pop", {"rbx"}, "Pop second value", false, 1},
        {"push", {"rax"}, "Push first value", false, 1},
        {"push", {"rbx"}, "Push second value", false, 1},
        {"add", {"r11", "1"}, "Increment instruction pointer", false, 4}
    };
    AddStandardAssembly("Swap", std::move(swap_asm));
}

void OilCommandAsmCompiler::InitializeIntegerOperations() {
    std::vector<AssemblyInstruction> int_add_asm = {
        {"pop", {"rbx"}, "Pop right operand", false, 1},
        {"pop", {"rax"}, "Pop left operand", false, 1},
        {"add", {"rax", "rbx"}, "Add operands", false, 3},
        {"push", {"rax"}, "Push result", false, 1},
        {"add", {"r11", "1"}, "Increment instruction pointer", false, 4}
    };
    AddStandardAssembly("IntAdd", std::move(int_add_asm));

    // IntSubtract, IntMultiply и другие аналогично...
}

void OilCommandAsmCompiler::InitializeFloatOperations() {
    std::vector<AssemblyInstruction> float_add_asm = {
        {"movsd", {"xmm1", "[rsp]"}, "Load right operand", false, 5},
        {"add", {"rsp", "8"}, "Adjust stack", false, 4},
        {"movsd", {"xmm0", "[rsp]"}, "Load left operand", false, 5},
        {"addsd", {"xmm0", "xmm1"}, "Add floating point", false, 4},
        {"movsd", {"[rsp]", "xmm0"}, "Store result", false, 5},
        {"add", {"r11", "1"}, "Increment instruction pointer", false, 4}
    };
    AddStandardAssembly("FloatAdd", std::move(float_add_asm));

    // FloatSubtract, FloatMultiply и другие аналогично...
}

void OilCommandAsmCompiler::InitializeByteOperations() {
    std::vector<AssemblyInstruction> byte_add_asm = {
        {"pop", {"rbx"}, "Pop right operand", false, 1},
        {"pop", {"rax"}, "Pop left operand", false, 1},
        {"add", {"al", "bl"}, "Add bytes", false, 2},
        {"movzx", {"rax", "al"}, "Zero extend result", false, 4},
        {"push", {"rax"}, "Push result", false, 1},
        {"add", {"r11", "1"}, "Increment instruction pointer", false, 4}
    };
    AddStandardAssembly("ByteAdd", std::move(byte_add_asm));

    // ByteSubtract, ByteMultiply и другие аналогично...
}

void OilCommandAsmCompiler::InitializeBooleanOperations() {
    std::vector<AssemblyInstruction> bool_and_asm = {
        {"pop", {"rbx"}, "Pop right operand", false, 1},
        {"pop", {"rax"}, "Pop left operand", false, 1},
        {"and", {"rax", "rbx"}, "Logical AND", false, 3},
        {"push", {"rax"}, "Push result", false, 1},
        {"add", {"r11", "1"}, "Increment instruction pointer", false, 4}
    };
    AddStandardAssembly("BoolAnd", std::move(bool_and_asm));

    // BoolOr, BoolNot, BoolXor аналогично...
}

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
