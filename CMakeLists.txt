cmake_minimum_required(VERSION 3.24)
project(SampleBox VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/SampleBoxOptions.cmake)

add_subdirectory(src)

if(SAMPLEBOX_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
