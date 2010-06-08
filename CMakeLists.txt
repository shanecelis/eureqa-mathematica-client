cmake_minimum_required(VERSION 2.6.2)

project(eureqa-mathematica-client)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake/mathlink" ${CMAKE_MODULE_PATH})

find_package(MathLink 7.0.1)

set(CMAKE_VERBOSE_MAKEFILE ON)

find_program(MARKDOWN_EXECUTABLE NAMES mymarkdown markdown)

message("MARKDOWN_EXECUTABLE ${MARKDOWN_EXECUTABLE}")

add_custom_command(OUTPUT README.html
                  COMMAND ${MARKDOWN_EXECUTABLE} README.md > README.html
                  DEPENDS README.md)

add_custom_target(doc DEPENDS README.html)

subdirs(
  src
  )

