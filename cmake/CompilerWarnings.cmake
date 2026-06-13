function(set_project_warnings target)
  set(CLANG_WARNINGS
      -Wall
      -Wextra
      -Wpedantic
      -Wshadow
      -Wconversion
      -Wsign-conversion
      -Wcast-align
      -Wunused
      -Wnull-dereference
      -Wdouble-promotion
      -Wformat=2
      -Werror)

  set(GCC_WARNINGS ${CLANG_WARNINGS} -Wmisleading-indentation
                   -Wduplicated-cond -Wduplicated-branches -Wlogical-op)

  if(CMAKE_C_COMPILER_ID MATCHES ".*Clang")
    set(WARNINGS ${CLANG_WARNINGS})
  elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(WARNINGS ${GCC_WARNINGS})
  endif()

  # Warnings apply to the C sources; tests are C++ and inherit a sane subset.
  target_compile_options(${target} INTERFACE $<$<COMPILE_LANGUAGE:C>:${WARNINGS}>)
endfunction()
