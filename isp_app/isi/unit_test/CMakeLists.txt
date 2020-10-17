cmake_minimum_required(VERSION 2.6)

set (module_ut ${module}_ut)

file(GLOB ut_sources tests/*.c tests/mock/*.c)
file(GLOB utr_sources runner/*.c)

include_directories(
  ../include
  ../include_priv
  ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include
  )

add_library(${module_ut}_static STATIC ${ut_sources})

target_link_libraries(${module_ut}_static
                      ${module}_static
                      ${base_test_libs}
                      )

add_executable(${module_ut}_runner ${utr_sources})

target_link_libraries(${module_ut}_runner
                      ${module_ut}_static
                      dl
                      )

SET_TARGET_PROPERTIES(${module_ut}_static PROPERTIES OUTPUT_NAME ${module_ut})

install(TARGETS ${module_ut}_static DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(TARGETS ${module_ut}_runner DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# create common targets for this module
include(${CMAKE_HOME_DIRECTORY}/targets_utr.cmake)
