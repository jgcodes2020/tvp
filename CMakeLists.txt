set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(tvp
  LANGUAGES CXX
  DESCRIPTION "A video player. In your terminal. Pretty ridiculous, if I do say so."
)

include(FetchContent)
FetchContent_Declare(
  avcpp
  GIT_REPOSITORY https://github.com/h4tr3d/avcpp.git
  GIT_TAG 12420c8d3810977f4771a79c039bc612d88378a7
  GIT_SUBMODULES ""
)
FetchContent_MakeAvailable(avcpp)

add_executable(tvp
  "src/main.cpp"
)
target_link_libraries(tvp PRIVATE avcpp::avcpp)