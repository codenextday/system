cmake_minimum_required(VERSION 2.6)

set (module isp_daemon)

include_directories(
  include
  )

file(GLOB sources source/*.c )

#APP
set(Demo_App isp_daemon)

add_library(${module}_static STATIC ${sources})

add_executable( ${Demo_App} ${sources})

target_link_libraries(
                      ${Demo_App}
                      hal_static
					  isi_static
                      cam_engine_static
                      som_ctrl_static
					  embUnit_static
					  dl
                      )

install(TARGETS ${Demo_App} DESTINATION bin)

#####################################################
set(Demo_APP_DYNAMIC isp_daemon_dynamic)

#add_library(${module}_static STATIC ${sources})

add_executable( ${Demo_APP_DYNAMIC} ${sources})

target_link_libraries(
                      ${Demo_APP_DYNAMIC}
                      cam_engine_shared
                      som_ctrl_shared
					  embUnit_shared
					  dl
                      )

install(TARGETS ${Demo_APP_DYNAMIC} DESTINATION bin)
