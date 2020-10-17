cmake_minimum_required(VERSION 2.6)

# define module name
set (module isi)

# we want to compile all .c files as default
file(GLOB libsources source/*.c)

# set public headers, these get installed
file(GLOB pub_headers include/*.h)

# define include paths
include_directories(
    include
    include_priv
    ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include
    )

# module specific defines
###add_definitions(-Wno-error=unused-function)

# add lib to build env
add_library(${module}_static STATIC ${libsources})

SET_TARGET_PROPERTIES(${module}_static PROPERTIES OUTPUT_NAME     ${module})
SET_TARGET_PROPERTIES(${module}_static PROPERTIES LINK_FLAGS      -static)
SET_TARGET_PROPERTIES(${module}_static PROPERTIES FRAMEWORK       TRUE PUBLIC_HEADER "${pub_headers}")

# define lib dependencies
target_link_libraries(${module}_static
                      ${platform_libs}
                      ${base_libs}
                      )

# define stuff to install
install(TARGETS ${module}_static
        PUBLIC_HEADER   DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${module}
        ARCHIVE         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        )

# create common targets for this module
include(${CMAKE_HOME_DIRECTORY}/targets.cmake)

# add sensor driver modules here
add_subdirectory( drv/AR330 )
add_subdirectory( drv/OV2715 )
###add_subdirectory( drv/OV5630 )
###add_subdirectory( drv/OV8810 )
add_subdirectory( drv/OV14825 )

# add unit tests and example appl subdirs
add_subdirectory(unit_test)
###add_subdirectory(example_app)
