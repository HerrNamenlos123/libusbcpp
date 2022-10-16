cmake_minimum_required(VERSION 3.16)
project(simple_scan)

add_executable(simple_scan simple_scan.cpp)

target_compile_features(simple_scan PRIVATE cxx_std_17)
set_target_properties(simple_scan PROPERTIES CXX_EXTENSIONS OFF)

if (LIBUSBCPP_STATIC_RUNTIME)
    use_static_runtime(simple_scan)
endif()

target_link_libraries(simple_scan libusbcpp)

set_runtime_output_directory(simple_scan ${CMAKE_BINARY_DIR}/bin)

install(
    TARGETS simple_scan
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    RUNTIME DESTINATION "bin"
    INCLUDES DESTINATION "include"
)