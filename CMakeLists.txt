cmake_minimum_required(VERSION 3.16)

project(
  embedded_patterns
  VERSION 0.1.0
  DESCRIPTION "Common embedded C patterns, host-tested with GoogleTest"
  LANGUAGES C CXX)

# ---- Standards ----------------------------------------------------------
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- Options (mirrors wordNebula) ---------------------------------------
option(BUILD_TESTING "Build unit tests" ON)
option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)
option(ENABLE_SANITIZERS "Enable AddressSanitizer and UBSan" OFF)
option(ENABLE_CLANG_TIDY "Run clang-tidy during build" OFF)
option(ENABLE_CPPCHECK "Run cppcheck during build" OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)
include(cmake/StaticAnalysis.cmake)

# Interface target that carries warnings/sanitizers to every module.
add_library(patterns_project_options INTERFACE)
target_compile_features(patterns_project_options INTERFACE c_std_11)
set_project_warnings(patterns_project_options)
enable_sanitizers(patterns_project_options)

# ---- Testing ------------------------------------------------------------
if(BUILD_TESTING)
  enable_testing()
  include(cmake/GoogleTest.cmake)
endif()

# ---- Modules ------------------------------------------------------------
# Each pattern is a self-contained static library + test target.
# Add modules here as they are implemented.
add_subdirectory(modules/ring_buffer)
# add_subdirectory(modules/memory_pool)
# add_subdirectory(modules/state_machine)
# add_subdirectory(modules/command_dispatch)
# add_subdirectory(modules/event_queue)
# add_subdirectory(modules/soft_timer)
# add_subdirectory(modules/debounce)
# add_subdirectory(modules/moving_average)
# add_subdirectory(modules/frame_parser)
# add_subdirectory(modules/crc)
# add_subdirectory(modules/fixed_point)
# add_subdirectory(modules/bit_ops)
