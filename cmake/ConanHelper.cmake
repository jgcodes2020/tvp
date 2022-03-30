# Download conan-cmake
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.17.0/conan.cmake"
    "${CMAKE_BINARY_DIR}/conan.cmake"
    TLS_VERIFY ON)
endif()

include("${CMAKE_BINARY_DIR}/conan.cmake")

macro(conan_configure_install)
  conan_cmake_install(PATH_OR_REFERENCE .
    BUILD missing ${CCFG_OPT_FORCE_BUILD}
    SETTINGS ${conan_detected_settings}
    INSTALL_FOLDER "${CMAKE_BINARY_DIR}/CMakeFiles/conan_deps"
  )
endmacro()

# Configures a list of Conan packages.
# PACKAGES - list of packages to include and also import
# RESOLVE - list of packages to fix a version to, but not import
# FORCE_BUILD - forces the listed packages to be built from source
function(conan_configure_packages)
  # Parse args
  set(flags)
  set(single_opt)
  set(multi_opt PACKAGES RESOLVE FORCE_BUILD)
  cmake_parse_arguments(PARSE_ARGV 0 CCFG_OPT "${flags}" "${single_opt}" "${multi_opt}")
  
  # Conan deps
  conan_cmake_configure(
    REQUIRES ${CCFG_OPT_PACKAGES} ${CCFG_OPT_RESOLVE}
    GENERATORS "cmake_find_package" "json"
  )

  # Check if generator is multi-config
  get_property(is_multiconfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
  # Install
  if(${is_multiconfig})
    foreach(TYPE ${CMAKE_CONFIGURATION_TYPES})
      message(STATUS "Installing Conan data for build type ${TYPE}")
      conan_cmake_autodetect(conan_detected_settings BUILD_TYPE ${TYPE})
      conan_configure_install()
    endforeach()
  else()
    message(STATUS "Installing Conan data for build type ${CMAKE_BUILD_TYPE}")
    conan_cmake_autodetect(conan_detected_settings)
    message(STATUS "Autodetected Conan settings: ${conan_detected_settings}")
    conan_configure_install()
  endif()
  
  # Setup paths
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/CMakeFiles/conan_deps")
  list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/CMakeFiles/conan_deps")

  # Read conanbuildinfo.json because we need that info
  file(READ "${CMAKE_BINARY_DIR}/CMakeFiles/conan_deps/conanbuildinfo.json" conan_build_info)

  # Generate dependency->index map
  set(dep_map "{}")
  string(JSON deps_len LENGTH "${conan_build_info}" dependencies)
  math(EXPR deps_len "${deps_len} - 1")

  foreach(i RANGE ${deps_len})
    string(JSON dep_name GET "${conan_build_info}"
      dependencies ${i} name
    )
    # check filenames.cmake_find_package first
    string(JSON dep_package ERROR_VARIABLE err GET "${conan_build_info}"
      dependencies ${i} filenames cmake_find_package
    )
    # fallback to names.cmake_find_package
    if(err)
      string(JSON dep_package ERROR_VARIABLE err GET "${conan_build_info}"
        dependencies ${i} names cmake_find_package
      )
    endif()
    # fallback to original dep name
    if(err)
      set(dep_package "${dep_name}")
    endif()

    string(JSON dep_map SET "${dep_map}" "${dep_name}" "\"${dep_package}\"")
  endforeach()
  
  # Find the packages that we specified
  foreach(pkg IN ITEMS ${CCFG_OPT_PACKAGES})
    string(REGEX REPLACE "\/.+$" "" package ${pkg})
    message(STATUS "Finding package ${package}")

    string(JSON package GET "${dep_map}" "${package}")
    find_package(${package} REQUIRED)
  endforeach()
endfunction()