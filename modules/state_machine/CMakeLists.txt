add_library(state_machine STATIC state_machine.c)
target_include_directories(state_machine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(state_machine PRIVATE patterns_project_options)

if(BUILD_TESTING)
  add_pattern_test(state_machine test_state_machine.cpp)
endif()
