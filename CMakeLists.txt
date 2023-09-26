# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-tools")
include(CMakeToolsVersionFromGit)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(PROJECT_VER ${CMAKE_TOOLS_GIT_TAG})

project(switch)

idf_build_set_property(COMPILE_OPTIONS "-Wno-error=format-truncation" APPEND)
