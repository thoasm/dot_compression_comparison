cmake_minimum_required(VERSION 3.13)

project(CompressionComparison LANGUAGES CXX VERSION 0.1.0 DESCRIPTION "An error comparison of different compression techniques")

# Use *_ROOT environment variables for find_package calls
cmake_policy(SET CMP0074 NEW)

# Let CAS handle the CUDA architecture flags (for now)
# Windows still gives CMP0104 warning if putting it in cuda.
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.18)
    cmake_policy(SET CMP0104 OLD)
endif()


if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

configure_file(include/directories.hpp.in include/directories.hpp @ONLY)

find_package(LibPressio REQUIRED)
find_package(LibPressioTools REQUIRED)
find_package(std_compat REQUIRED)
find_package(frsz REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

add_executable(c_comparison)
target_sources(c_comparison
    PRIVATE
    main.cpp)

target_include_directories(c_comparison
    PRIVATE
    include/)

target_compile_features(c_comparison PUBLIC cxx_std_14)
target_link_libraries(c_comparison
    PUBLIC
      nlohmann_json::nlohmann_json
      frsz::frsz
      LibPressio::libpressio
      LibPressioTools::libpressio_meta
      OpenMP::OpenMP_CXX
    )
