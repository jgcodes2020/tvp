cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(tvp
  LANGUAGES CXX
  DESCRIPTION "A video player in your terminal."
)

# Enable compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS yes)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include(DefaultBuildType)
include(ConanHelper)
conan_configure_packages(
  "base64/0.4.0"
  "libdivide/5.0" 
  "fmt/8.1.1"
  
  "avcpp/cci.20220301"
  "ogg/1.3.5"
)

add_executable(tvp
  "src/main.cpp"
  "src/sixel.cpp"
  "src/term.hpp"
)
target_compile_features(tvp PUBLIC cxx_std_20)
target_link_libraries(tvp PUBLIC avcpp::avcpp fmt::fmt libdivide::libdivide)
target_compile_options(tvp PUBLIC "-msse4.2")