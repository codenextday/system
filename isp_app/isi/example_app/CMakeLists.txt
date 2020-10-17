cmake_minimum_required(VERSION 2.6)

set (module_exapp ${module}_example_app)

# specify the sources to compile
file(GLOB tp_sources *.c)

add_executable(${module_exapp} ${tp_sources})

target_link_libraries(${module_exapp} ${module}_static)

#include_directories(/usr/include/<whatever>)
include_directories(${LIB_ROOT}/include)
#include_directories(/usr/local/include/<whatever>)

install(TARGETS ${module_exapp} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
