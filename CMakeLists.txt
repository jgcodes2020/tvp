set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
cmake_policy(VERSION 3.20)
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(tvp
  LANGUAGES CXX
  DESCRIPTION "A video player. In your terminal. Pretty ridiculous, if I do say so."
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(CMakeRC)

find_package(fmt REQUIRED)
find_package(Vulkan REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(libav REQUIRED IMPORTED_TARGET
  libavformat
  libavcodec
  libavutil
  libswscale
)

add_executable(tvp
  "src/main.cpp"
  "src/video.hpp"
  "src/video.cpp"
  "src/ffmpeg.hpp"
  "src/terminal.hpp"
  "src/vkcomp/vkcomp.cpp"
  "src/vkcomp/vkcomp.hpp"
)

target_compile_features(tvp PRIVATE cxx_std_20)
target_link_libraries(tvp PRIVATE 
  PkgConfig::libav 
  Vulkan::Vulkan
  fmt::fmt
  tvp-resources
)
