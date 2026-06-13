add_library(ring_buffer STATIC ring_buffer.c)
target_include_directories(ring_buffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ring_buffer PRIVATE patterns_project_options)

if(BUILD_TESTING)
  add_pattern_test(ring_buffer test_ring_buffer.cpp)
endif()
