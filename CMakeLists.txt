cmake_minimum_required(VERSION 3.12)
project(RaceSim VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# All source files
file(GLOB_RECURSE src_files ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/main.cpp)

# All header files
file(GLOB_RECURSE header_files ${CMAKE_SOURCE_DIR}/include/*.hpp)

add_executable(sim main.cpp ${src_files})
target_include_directories(sim PRIVATE ${CMAKE_SOURCE_DIR}/include)
