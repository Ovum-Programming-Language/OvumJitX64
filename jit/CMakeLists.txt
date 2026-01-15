add_library(jit STATIC
        JitExecutorFactory.cpp
        JitExecutor.cpp
        AsmCompiler.cpp
        OilCommandAsmCompiler.cpp
        ./oil-to-asm-realisation/OilToAsmIntegerOperations.cpp
        ./oil-to-asm-realisation/OilToAsmFloatOperations.cpp
        ./oil-to-asm-realisation/OilToAsmByteOperations.cpp
        ./oil-to-asm-realisation/OilToAsmBooleanOperations.cpp
        ./oil-to-asm-realisation/OilToAsmStackOperations.cpp
        ./oil-to-asm-realisation/OilToAsmComplexOperations.cpp
        ./oil-to-asm-realisation/OilToAsmLocalDataOperations.cpp
        ./oil-to-asm-realisation/AsmComplexOperationManager.cpp
        ./oil-to-asm-realisation/AsmToBytes.cpp
        ./oil-to-asm-realisation/optimisers/PushPopOptimiser.cpp
        ./machine-code-runner/ExecutableMemory.cpp
        ./machine-code-runner/MachineCodeFunction.cpp
        ./machine-code-runner/AsmDataBuffer.cpp
)

target_include_directories(jit
        PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_include_directories(jit PUBLIC ${ovumjitx64_SOURCE_DIR})
target_include_directories(jit PUBLIC ${OvumJitX64_SOURCE_DIR})

target_link_libraries(jit PUBLIC tokens)
