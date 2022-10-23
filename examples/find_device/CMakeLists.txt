cmake_minimum_required(VERSION 3.16)
project(find_device)

add_executable(find_device find_device.cpp)

target_compile_features(find_device PRIVATE cxx_std_17)
set_target_properties(find_device PROPERTIES CXX_EXTENSIONS OFF)

if (LIBUSBCPP_STATIC_RUNTIME)
    use_static_runtime(find_device)
endif()

target_link_libraries(find_device libusbcpp)

set_runtime_output_directory(find_device ${CMAKE_BINARY_DIR}/bin)

install(
        TARGETS find_device
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
        RUNTIME DESTINATION "bin"
        INCLUDES DESTINATION "include"
)