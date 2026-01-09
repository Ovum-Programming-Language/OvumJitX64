add_library(jit STATIC
        JitExecutorFactory.cpp
        JitExecutor.cpp
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
