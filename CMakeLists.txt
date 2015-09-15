cmake_minimum_required(VERSION 2.8)
project(mazu)

add_executable(local_demo src/local_demo.cpp src/naive_client.cpp)
target_include_directories(local_demo PUBLIC include)
target_compile_options(local_demo PUBLIC -std=c++11)
