cmake_minimum_required(VERSION 3.16)
project(miniredis VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Fetch Catch2 from GitHub (v2.13.10 is lightweight and fully compatible with MinGW GCC)
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v2.13.10
)
FetchContent_MakeAvailable(Catch2)

# 3. Define your Source Files (The actual engine)
add_library(miniredis_engine 
    src/store.cpp
    src/protocol.cpp
    src/command.cpp
    src/server.cpp
)

target_include_directories(miniredis_engine PRIVATE include)

# 4. Define your Executable (The Server)
add_executable(run_server src/main.cpp)
target_include_directories(run_server PRIVATE include)
target_link_libraries(run_server PRIVATE miniredis_engine)

if(WIN32)
    target_link_libraries(miniredis_engine PUBLIC ws2_32)
endif()

# 5. Define your Test Executable
add_executable(run_tests 
    tests/test_store.cpp
    tests/test_protocol.cpp
    tests/test_command.cpp
    src/store.cpp
    src/protocol.cpp
    src/command.cpp
)

target_include_directories(run_tests PRIVATE include)

# 3. Link Catch2 with your test target
target_link_libraries(run_tests PRIVATE Catch2::Catch2)