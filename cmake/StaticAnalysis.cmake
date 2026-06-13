if(ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY clang-tidy)
  if(CLANG_TIDY)
    set(CMAKE_C_CLANG_TIDY ${CLANG_TIDY} --extra-arg=-Wno-unknown-warning-option)
  else()
    message(WARNING "clang-tidy requested but not found")
  endif()
endif()

if(ENABLE_CPPCHECK)
  find_program(CPPCHECK cppcheck)
  if(CPPCHECK)
    set(CMAKE_C_CPPCHECK
        ${CPPCHECK} --enable=warning,style,performance,portability
        --inline-suppr --suppress=missingIncludeSystem --error-exitcode=1)
  else()
    message(WARNING "cppcheck requested but not found")
  endif()
endif()
