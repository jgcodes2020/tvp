cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(tvp
  LANGUAGES CXX
  DESCRIPTION "A video player in your terminal."
)

# Enable compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS yes)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# vcpkg deps
include(DefaultBuildType)
find_package(fmt REQUIRED)
find_package(libdivide REQUIRED)

# FetchContent deps
include(FetchContent)
FetchContent_Declare(avcpp
  GIT_REPOSITORY https://github.com/h4tr3d/avcpp.git
  GIT_TAG 12420c8d3810977f4771a79c039bc612d88378a7
)
FetchContent_GetProperties(avcpp)
if (NOT avcpp_POPULATED)
  FetchContent_Populate(avcpp)
  add_subdirectory(${avcpp_SOURCE_DIR} ${avcpp_BINARY_DIR})
endif()


add_executable(tvp
  "src/main.cpp"
  "src/sixel.cpp"
  "src/term.hpp"
  "src/termutils.cpp"
)

target_compile_features(tvp PUBLIC cxx_std_20)
target_link_libraries(tvp PUBLIC avcpp fmt::fmt libdivide::libdivide)
target_compile_options(tvp PUBLIC "-msse4.2")