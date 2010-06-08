find_package(Boost 1.40 COMPONENTS system serialization date_time thread)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
MathLink_ADD_TM(eureqaml.tm)
include_directories(${MathLink_INCLUDE_DIR} ../eureqa_api ${Boost_INCLUDE_DIR})

link_directories(${MathLink_LIBRARY_DIR})

# add_custom_command(
#         TARGET eureqaml
#         PRE_LINK
#         COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/eureqaml2/other 
# )

add_executable (eureqaml eureqaml.tm.c eureqaml.cpp) #/${MathLink_System_ID})

#add_dependencies(eureqaml/other/eureqaml ${CMAKE_CURRENT_BINARY_DIR}/eureqaml/other)

target_link_libraries(eureqaml ${MathLink_LIBRARIES} ${Boost_LIBRARIES})

INSTALL(DIRECTORY EureqaClient 
                  DESTINATION ${MathLink_USER_BASE_DIR}/Applications)
INSTALL(PROGRAMS eureqaml 
              DESTINATION ${MathLink_USER_BASE_DIR}/Applications/EureqaClient)

add_custom_target(install_workaround
                  COMMAND make install
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src)

add_custom_target(run 
                  COMMAND eureqaml)

add_dependencies(run install_workaround)

add_custom_target(debug 
                  COMMAND gdb --args eureqaml)


add_dependencies(debug install_workaround)

