include(deps)

set (_app bushing-auto)
set (_srcs ${_app}.cc)

file(GLOB SOURCES ./**/*.cpp)
list(APPEND _srcs ${SOURCES})

add_executable(${_app} ${_srcs})
# target_compile_definitions(${_app} PUBLIC ecat_debug)

target_include_directories(${_app} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>

    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/_deps/soem-src/soem>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/_deps/soem-src/osal>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/_deps/soem-src/osal/linux>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/_deps/soem-src/oshw/linux>)

target_link_libraries(${_app} soem) 

set_property(TARGET ${_app} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${_app} PROPERTY C_STANDARD 11)




