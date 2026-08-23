add_library(memory_pool STATIC memory_pool.c)
target_include_directories(memory_pool PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(memory_pool PRIVATE patterns_project_options)

if(BUILD_TESTING)
  add_pattern_test(memory_pool test_memory_pool.cpp)
endif()
