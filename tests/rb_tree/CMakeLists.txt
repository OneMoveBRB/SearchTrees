find_package(GTest QUIET)

if(NOT GTest_FOUND)
  include(FetchContent)
  FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
    GIT_SHALLOW    TRUE
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
endif()

include(GoogleTest)

foreach(src IN ITEMS
    basic_test
    insert_fixup_test
    erase_test
    delete_fixup_test
    queries_test
    stress_test
    templated_type_test
)
  add_executable(rb_tree_${src} ${src}.cpp)
  target_link_libraries(rb_tree_${src} PRIVATE
    search_trees::search_trees
    GTest::gtest GTest::gtest_main
  )
  target_include_directories(rb_tree_${src} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  search_trees_set_warnings(rb_tree_${src})

  #gtest_discover_tests(rb_tree_${src}
  #  TEST_PREFIX "rb_tree.${src}."
  #)

  gtest_discover_tests(rb_tree_${src})
  #add_test(NAME "rb_tree.${src}" COMMAND rb_tree_${src})
endforeach()
