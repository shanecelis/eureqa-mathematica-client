# Copyright (c) 2009 Kashif Rasul
# 
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# - this module looks for Mathematica's MathLink API
# Defines:
#  MathLink_INCLUDE_DIR:      include path for mathlink.h
#  MathLink_LIBRARIES:        required libraries: libML32i3, etc.
#  MathLink_LIBRARY_DIR:      path for required libraries
#  MathLink_MPREP_EXECUTABLE: path to mprep
#  MathLink_MATH_EXECUTABLE:  path to math
#  MathLink_SYSTEM_ID:        Mathematica's $SystemID
#  MathLink_USER_BASE_DIR:    Mathematica User's base directory

SET(MathLink_FOUND 0)
IF( MathLink_FIND_VERSION )
  set(_mathlink_version "${MathLink_FIND_VERSION_MAJOR}.${MathLink_FIND_VERSION_MINOR}")
ELSE ( MathLink_FIND_VERSION)
  MESSAGE( FATAL_ERROR "Invalid Mathematica version string given." )
ENDIF( MathLink_FIND_VERSION )

if (WIN32)
  IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set (MathLink_SYS Windows)
  ELSE(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(MathLink_SYS Windows-x86-64)
  ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(MathLink_ROOT_DIR "$ENV{ProgramFiles}\\Wolfram Research\\Mathematica\\${MathLink_FIND_VERSION_MAJOR}.${MathLink_FIND_VERSION_MINOR}\\SystemFiles\\Links\\MathLink\\DeveloperKit\\${MathLink_SYS}")
endif(WIN32)

if(UNIX)
  IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set (MathLink_SYS Linux)
  ELSE(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(MathLink_SYS Linux-x86-64)
  ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(MathLink_ROOT_DIR /usr/local/Wolfram/Mathematica/${MathLink_FIND_VERSION_MAJOR}.${MathLink_FIND_VERSION_MINOR}/SystemFiles/Links/MathLink/DeveloperKit/${MathLink_SYS})
endif(UNIX)

if(APPLE)
  set(MathLink_ROOT_DIR /Applications/Mathematica.app/SystemFiles/Links/MathLink/DeveloperKit)
endif(APPLE)

find_path(MathLink_INCLUDE_DIR
  NAMES mathlink.h
  PATHS ${MathLink_ROOT_DIR}/CompilerAdditions
        ${MathLink_ROOT_DIR}/CompilerAdditions/mldev32/include
        ${MathLink_ROOT_DIR}/CompilerAdditions/mldev64/include
      )
        
IF(UNIX)
  set(MathLink_LIBRARY_DIR ${MathLink_ROOT_DIR}/CompilerAdditions )
ELSE(UNIX)
  IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(MathLink_LIBRARY_DIR ${MathLink_ROOT_DIR}\\CompilerAdditions\\mldev32\\lib )
  ELSE(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(MathLink_LIBRARY_DIR ${MathLink_ROOT_DIR}\\CompilerAdditions\\mldev64\\lib )
  ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 4)
ENDIF(UNIX)

find_program(MathLink_MPREP_EXECUTABLE 
  NAMES mprep
  PATHS ${MathLink_ROOT_DIR}/CompilerAdditions
        ${MathLink_ROOT_DIR}\\CompilerAdditions\\mldev32\\bin
        ${MathLink_ROOT_DIR}\\CompilerAdditions\\mldev64\\bin
      )
mark_as_advanced(MathLink_MPREP_EXECUTABLE)
find_program(MathLink_MATH_EXECUTABLE 
  NAMES math
  PATHS ${MathLink_ROOT_DIR}/CompilerAdditions
        ${MathLink_ROOT_DIR}\\CompilerAdditions\\mldev32\\bin
        ${MathLink_ROOT_DIR}\\CompilerAdditions\\mldev64\\bin
      )
mark_as_advanced(MathLink_MATH_EXECUTABLE)

MACRO (MathLink_MATH_EXEC command output_variable )
      EXECUTE_PROCESS(COMMAND ${MathLink_MATH_EXECUTABLE} 
      "-noinit" "-noprompt" "-run" "${command}; Quit[];" 
      OUTPUT_VARIABLE ${output_variable})
      #STRING(REGEX REPLACE "\"" "" MathLink_SYSTEM_ID "${MathLink_SYSTEM_ID}")
ENDMACRO (MathLink_MATH_EXEC)

MACRO (REMOVE_QUOTES variable_name)
      STRING(REGEX REPLACE "\"" "" ${variable_name} "${${variable_name}}")
ENDMACRO (REMOVE_QUOTES)

MACRO (REMOVE_NEWLINES variable_name)
      STRING(REGEX REPLACE "\n" "" ${variable_name} "${${variable_name}}")
ENDMACRO (REMOVE_NEWLINES)


MathLink_MATH_EXEC("Print[\$SystemID]" MathLink_SYSTEM_ID)
REMOVE_QUOTES(MathLink_SYSTEM_ID)
REMOVE_NEWLINES(MathLink_SYSTEM_ID)
MESSAGE("system_id is ${MathLink_SYSTEM_ID}")

#STRING(REGEX REPLACE "\"" "" MathLink_USER_BASE_DIR "${MathLink_USER_BASE_DIR}")
MathLink_MATH_EXEC("Print[\$UserBaseDirectory]" MathLink_USER_BASE_DIR)
REMOVE_QUOTES(MathLink_USER_BASE_DIR)
REMOVE_NEWLINES(MathLink_USER_BASE_DIR)
MESSAGE("user base dir is ${MathLink_USER_BASE_DIR}")


IF(WIN32)
  IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
    FIND_LIBRARY( MathLink_ML_LIBRARY 
      NAMES ml32i3m
      PATHS ${MathLink_LIBRARY_DIR}
            ${MathLink_ROOT_DIR}\\SystemAdditions
          )
  ELSE(CMAKE_SIZEOF_VOID_P EQUAL 4)
    FIND_LIBRARY( MathLink_ML_LIBRARY 
      NAMES ml64i3m
      PATHS ${MathLink_LIBRARY_DIR}
            ${MathLink_ROOT_DIR}\\SystemAdditions
          )
  ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 4)
  SET( MathLink_LIBRARIES ${MathLink_ML_LIBRARY} )
ENDIF(WIN32)

IF(APPLE)
  FIND_LIBRARY( MathLink_ML_LIBRARY 
    NAMES MLi3
    PATHS ${MathLink_LIBRARY_DIR}
  )
  SET( MathLink_LIBRARIES ${MathLink_ML_LIBRARY}
    stdc++
  )
ENDIF(APPLE)

IF(UNIX AND NOT APPLE)
  IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
    FIND_LIBRARY( MathLink_ML_LIBRARY 
      NAMES ML32i3
      PATHS ${MathLink_LIBRARY_DIR}
    )
  ELSE(CMAKE_SIZEOF_VOID_P EQUAL 4)
    FIND_LIBRARY( MathLink_ML_LIBRARY 
      NAMES ML64i3
      PATHS ${MathLink_LIBRARY_DIR}
    )
  ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 4)

  SET( MathLink_LIBRARIES ${MathLink_ML_LIBRARY}
    m
    pthread
    rt
    stdc++
  )
ENDIF(UNIX AND NOT APPLE)

MACRO (MathLink_ADD_TM infile )
 GET_FILENAME_COMPONENT(outfile ${infile} NAME_WE)
 GET_FILENAME_COMPONENT(abs_infile ${infile} ABSOLUTE)
 SET(outfile ${CMAKE_CURRENT_BINARY_DIR}/${outfile}.tm.c)
 ADD_CUSTOM_COMMAND(
   OUTPUT   ${outfile}
   COMMAND  ${MathLink_MPREP_EXECUTABLE}
   ARGS     -o ${outfile} ${abs_infile}
   MAIN_DEPENDENCY ${infile})
ENDMACRO (MathLink_ADD_TM)

IF(MathLink_INCLUDE_DIR AND MathLink_LIBRARIES)
  SET(MathLink_FOUND 1)
ENDIF(MathLink_INCLUDE_DIR AND MathLink_LIBRARIES)

MARK_AS_ADVANCED(
  MathLink_LIBRARIES 
  MathLink_INCLUDE_DIR 
  MathLink_LIBRARY_DIR
  MathLink_FOUND
)
