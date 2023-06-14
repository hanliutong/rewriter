cmake_minimum_required(VERSION 3.10)

project(ocv_tidy C CXX)

find_package(Clang REQUIRED)

add_library(ocv_intrinsic_tidy MODULE module.cpp
    checks/nlanes_check.cpp
    checks/operator_check.cpp
    checks/get0_check.cpp
    checks/lanetype_check.cpp
)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories( ${CLANG_INCLUDE_DIRS})

# issue when building LLVM and module using GCC
target_compile_options(ocv_intrinsic_tidy PRIVATE "-fno-rtti")

target_link_libraries(ocv_intrinsic_tidy PRIVATE clang-tidy)
