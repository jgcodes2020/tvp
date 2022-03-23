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
conan_configure_packages("opencv/4.5.3" "ffmpeg/4.4" "base64/0.4.0" "fmt/8.1.1" "ogg/1.3.5")

add_subdirectory(src/lib)
add_subdirectory(src/bin)