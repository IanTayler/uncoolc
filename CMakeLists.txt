cmake_minimum_required(VERSION 3.12.0)
project(coolc)
add_executable(coolc 
  src/main.cc
  src/tokenizer.cc
  src/token_type.cc
)
target_include_directories(coolc
  PUBLIC
  include
)

add_compile_options(-Wall -Wextra -Wpedantic)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
