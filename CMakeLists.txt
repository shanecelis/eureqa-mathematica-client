cmake_minimum_required(VERSION 2.6.2)

project(MathLink-project)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake/mathlink" ${CMAKE_MODULE_PATH})

find_package(MathLink 7.0.1)

subdirs(
  src
  )
