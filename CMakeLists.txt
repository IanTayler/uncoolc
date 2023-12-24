cmake_minimum_required(VERSION 3.22.1)
project(coolc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

add_executable(coolc 
  src/main.cc
  src/tokenizer.cc
  src/token.cc
  src/symbol.cc
  src/ast.cc
  src/error.cc
  src/parser.cc
)
target_include_directories(coolc
  PUBLIC
  include
)

add_executable(tests
  test/test_symbol.cc
  test/test_token.cc
  test/test_tokenizer.cc
  src/tokenizer.cc
  src/token.cc
  src/symbol.cc
  src/ast.cc
  src/error.cc
  src/parser.cc
)
target_include_directories(tests
  PUBLIC
  include
  test/include
)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

