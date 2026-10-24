function(search_trees_set_warnings target)
  if(NOT SEARCH_TREES_WARNINGS)
    return()
  endif()

  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /permissive-)
    if(SEARCH_TREES_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE /WX)
    endif()
  else()
    target_compile_options(${target} PRIVATE
      -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
      -Wold-style-cast -Wnon-virtual-dtor -Woverloaded-virtual
      -Wdouble-promotion -Wformat=2 -Wundef -Wcast-align -Wcast-qual
      -Wnull-dereference -Wimplicit-fallthrough -Wwrite-strings
    )
    if(SEARCH_TREES_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE -Werror)
    endif()
  endif()
endfunction()
