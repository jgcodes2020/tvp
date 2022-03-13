cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(tvp
  LANGUAGES CXX
  DESCRIPTION "A video player in your terminal."
)

# Enable compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Check build type
# https://www.kitware.com/cmake-and-the-default-build-type/
set(default_build_type "Release")

if(EXISTS "${CMAKE_PROJECT_DIR}/.git")
	set(default_build_type "Debug")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
	set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
		STRING "Choose the type of build." FORCE)

	# Set the possible values of build type for cmake-gui
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
		"Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Download conan-cmake
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
	message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
	file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.17.0/conan.cmake"
		"${CMAKE_BINARY_DIR}/conan.cmake"
		TLS_VERIFY ON)
endif()
include("${CMAKE_BINARY_DIR}/conan.cmake")

# Conan deps
conan_cmake_configure(
  REQUIRES "opencv/4.5.3" "base64/0.4.0" "fmt/8.1.1"
  GENERATORS "cmake_find_package"
)

# Check if generator is multi-config
get_property(is_multiconfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

# Setup packages using conan-cmake
message("Generator is multi-config: ${is_multiconfig}")

if(${is_multiconfig})
	foreach(TYPE ${CMAKE_CONFIGURATION_TYPES})
		message(STATUS "Installing Conan data for build type ${TYPE}")
		conan_cmake_autodetect(conan_detected_settings BUILD_TYPE ${TYPE})
		conan_cmake_install(PATH_OR_REFERENCE .
			BUILD missing
			SETTINGS ${conan_detected_settings}
			INSTALL_FOLDER "${CMAKE_BINARY_DIR}/CMakeFiles/conan_deps"
		)
	endforeach()
else()
	conan_cmake_autodetect(conan_detected_settings)
  message(STATUS "Autodetected Conan settings: ${conan_detected_settings}")
	conan_cmake_install(PATH_OR_REFERENCE .
		BUILD missing
		SETTINGS ${conan_detected_settings}
		INSTALL_FOLDER "${CMAKE_BINARY_DIR}/CMakeFiles/conan_deps"
	)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/CMakeFiles/conan_deps")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/CMakeFiles/conan_deps")

foreach (i IN ITEMS OpenCV base64 fmt)
  find_package(${i} REQUIRED)
endforeach()

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