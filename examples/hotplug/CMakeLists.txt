cmake_minimum_required(VERSION 3.16)
project(hotplug)

add_executable(hotplug hotplug.cpp)

target_compile_features(hotplug PRIVATE cxx_std_17)
set_target_properties(hotplug PROPERTIES CXX_EXTENSIONS OFF)

if (LIBUSBCPP_STATIC_RUNTIME)
    use_static_runtime(hotplug)
endif()

target_link_libraries(hotplug libusbcpp)

set_runtime_output_directory(hotplug ${CMAKE_BINARY_DIR}/bin)

install(
    TARGETS hotplug
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    RUNTIME DESTINATION "bin"
    INCLUDES DESTINATION "include"
)