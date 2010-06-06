find_package(Boost 1.40 COMPONENTS system serialization date_time thread)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
MathLink_ADD_TM(eureqa.tm)
include_directories(${MathLink_INCLUDE_DIR} ../eureqa_api ${Boost_INCLUDE_DIR})

link_directories(${MathLink_LIBRARY_DIR})

# add_custom_command(
#         TARGET eureqa
#         PRE_LINK
#         COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/eureqa2/other 
# )

add_executable (eureqa eureqa.tm.c eureqa.cpp) #/${MathLink_System_ID})

#add_dependencies(eureqa/other/eureqa ${CMAKE_CURRENT_BINARY_DIR}/eureqa/other)

target_link_libraries(eureqa ${MathLink_LIBRARIES} ${Boost_LIBRARIES})

INSTALL(DIRECTORY EureqaClient 
                  DESTINATION ${MathLink_USER_BASE_DIR}/Applications)
INSTALL(PROGRAMS eureqa 
              DESTINATION ${MathLink_USER_BASE_DIR}/Applications/EureqaClient)

add_custom_target(install_workaround
                  COMMAND make install
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src)

add_custom_target(run 
                  COMMAND eureqa)

add_dependencies(run install_workaround)

add_custom_target(debug 
                  COMMAND gdb --args eureqa)

add_dependencies(debug install_workaround)

