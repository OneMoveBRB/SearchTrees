cmake_minimum_required(VERSION 3.20)

project(search_trees
  VERSION 0.1.0
  DESCRIPTION "Search trees library"
  LANGUAGES CXX
)

# ---------------------------------------------------------------------------
# Global settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

option(SEARCH_TREES_BUILD_TESTS        "Build unit tests"                  ON )
option(SEARCH_TREES_WARNINGS           "Enable compiler warnings"          ON )
option(SEARCH_TREES_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

include(cmake/warnings.cmake)

# ---------------------------------------------------------------------------
# Library (header-only)
# ---------------------------------------------------------------------------
add_library(search_trees INTERFACE)
add_library(search_trees::search_trees ALIAS search_trees)

target_include_directories(search_trees INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_compile_features(search_trees INTERFACE cxx_std_20)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(SEARCH_TREES_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# ---------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------
include(GNUInstallDirs)
install(TARGETS search_trees EXPORT search_treesTargets)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT search_treesTargets
  NAMESPACE search_trees::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/search_trees
)
