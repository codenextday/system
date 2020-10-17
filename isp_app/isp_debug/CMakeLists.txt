cmake_minimum_required(VERSION 2.6)

set(MODULE isp_debug)

# include directories
include_directories(
    include
    )

file(GLOB SRC src/*.c)

# app
set(APP isp_debug)

add_library(${MODULE}_static STATIC ${SRC}))

add_executable(${APP} ${SRC})

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
set(APP_DYNAMIC isp_debug_dynamic)

add_executable(${APP_DYNAMIC} ${SRC})

target_link_libraries(
    ${APP_DYNAMIC}
    cam_engine_shared
    som_ctrl_shared
    embUnit_shared
    json_shared
    uvc_display_shared
    )

install(TARGETS ${APP_DYNAMIC} DESTINATION bin)

