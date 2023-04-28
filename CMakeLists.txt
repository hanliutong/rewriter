cmake_minimum_required(VERSION 3.10)

project(RewriterProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_COMPILER "${LLVM_BUILD_DIR}/bin/clang++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

set(LLVM_DIR "${LLVM_BUILD_DIR}/lib/cmake/llvm/")
set(Clang_DIR "${LLVM_BUILD_DIR}/lib/cmake/clang/")

find_path(STDDEF_INCLUDE_PATH stddef.h)
if(STDDEF_INCLUDE_PATH)
  target_include_directories(ocv_rewriter SYSTEM PRIVATE ${STDDEF_INCLUDE_PATH})
endif()

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

find_package(Clang REQUIRED)

include_directories(${CLANG_INCLUDE_DIRS})

add_executable(ocv_rewriter rewriter.cpp)

# llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(ocv_rewriter
                      PRIVATE
                    #   ${llvm_libs}
                      clangTooling
                      clangBasic
                      clangASTMatchers
                      clangFrontend
                      clangAST
                      clangFrontendTool)
