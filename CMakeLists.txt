cmake_minimum_required(VERSION 3.0.0)

project(dumb_meta)

find_package(Clang REQUIRED CONFIG)
find_package(LLVM REQUIRED CONFIG)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(third_party)
add_subdirectory(src)
add_subdirectory(test)

set(LLVM_VISUALIZERS llvm.natvis)
add_custom_target(LLVMVisualizers SOURCES ${LLVM_VISUALIZERS})

set(CLANG_VISUALIZERS clang.natvis)
add_custom_target(ClangVisualizers SOURCES ${CLANG_VISUALIZERS})