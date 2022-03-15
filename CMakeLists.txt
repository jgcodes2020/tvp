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
conan_configure_packages("opencv/4.5.3" "base64/0.4.0" "fmt/8.1.1" "ogg/1.3.5")

add_executable(tvp
  "src/main.cpp"
  "src/term.hpp"
  "src/fixed_string.hpp"
  "src/sixel.cpp"
)
set_target_properties(tvp PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED yes
)
target_compile_features(tvp PUBLIC cxx_std_20)
target_link_libraries(tvp PUBLIC opencv::opencv base64::base64 fmt::fmt)
target_compile_options(tvp PRIVATE "-msse4.2")