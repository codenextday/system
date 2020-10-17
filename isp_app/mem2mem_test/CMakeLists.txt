cmake_minimum_required(VERSION 2.6)

# include directories
include_directories(
    include
    include/json
    )

# libjson
set(JSON json)

file(GLOB JSON_SOURCES src/json/*.c)

file(GLOB pub_headers include/json/*.h)

#add_library(${JSON} ${JSON_SOURCES})
add_library(${JSON}_static STATIC ${JSON_SOURCES})
add_library(${JSON}_shared SHARED ${JSON_SOURCES})

SET_TARGET_PROPERTIES(${JSON}_static PROPERTIES OUTPUT_NAME     ${JSON})
SET_TARGET_PROPERTIES(${JSON}_static PROPERTIES LINK_FLAGS      -static)
SET_TARGET_PROPERTIES(${JSON}_static PROPERTIES FRAMEWORK       TRUE PUBLIC_HEADER "${pub_headers}")

SET_TARGET_PROPERTIES(${JSON}_shared PROPERTIES OUTPUT_NAME     ${JSON})
SET_TARGET_PROPERTIES(${JSON}_shared PROPERTIES FRAMEWORK       TRUE PUBLIC_HEADER "${pub_headers}")

install(TARGETS ${JSON}_static
        PUBLIC_HEADER   DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${JSON}
        ARCHIVE         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        )
install(TARGETS ${JSON}_shared
        PUBLIC_HEADER   DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${JSON}
        LIBRARY         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        )

#include(${CMAKE_HOME_DIRECTORY}/targets.cmake)

##########################################################
# app
set(APP isp_mem2mem_test)

file(GLOB TEST_SOURCES src/*.c)

add_executable(${APP} ${TEST_SOURCES})

target_link_libraries(
    ${APP}
    cam_engine_static
    som_ctrl_static
    embUnit_static
    json_static
    uvc_display_static
    )

install(TARGETS ${APP} DESTINATION bin)


#########dynamic lib link###################################
set(APP_DYNAMIC isp_mem2mem_test_dynamic)

add_executable(${APP_DYNAMIC} ${TEST_SOURCES})

target_link_libraries(
    ${APP_DYNAMIC}
    cam_engine_shared
    som_ctrl_shared
    embUnit_shared
    json_shared
    uvc_display_shared
    )

install(TARGETS ${APP_DYNAMIC} DESTINATION bin)

