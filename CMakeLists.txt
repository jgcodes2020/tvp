set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
cmake_policy(VERSION 3.20)
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(tvp
  LANGUAGES CXX
  DESCRIPTION "A video player. In your terminal. Pretty ridiculous, if I do say so."
)

find_package(PkgConfig REQUIRED)
find_package(fmt REQUIRED)
pkg_check_modules(libav REQUIRED IMPORTED_TARGET
  libavformat
  libavcodec
  libavutil
  libswscale
)

add_executable(tvp
  "src/main.cpp"
  
  "src/vdec.hpp"
  "src/vdec.cpp"
  
  "src/ffmpeg.hpp"
  
  "src/terminal.hpp"
)
target_compile_features(tvp PRIVATE cxx_std_20)
target_link_libraries(tvp PRIVATE PkgConfig::libav fmt::fmt)