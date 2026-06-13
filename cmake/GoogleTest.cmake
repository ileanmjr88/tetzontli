include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2)

# Prevent gtest from overriding our compiler/linker options on Windows.
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# Helper: register a C++ test that links against a pattern's static library.
# Usage: add_pattern_test(ring_buffer test_ring_buffer.cpp)
function(add_pattern_test module_lib test_src)
  get_filename_component(test_name ${test_src} NAME_WE)
  add_executable(${test_name} ${test_src})
  target_link_libraries(${test_name} PRIVATE ${module_lib} GTest::gtest_main)
  gtest_discover_tests(${test_name})
endfunction()
