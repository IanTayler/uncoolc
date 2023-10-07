cmake_minimum_required(VERSION 3.12.0)
project(coolc)
add_executable(coolc 
  src/main.cc
  src/tokenizer.cc
)
target_include_directories(coolc
  PUBLIC
  src
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
