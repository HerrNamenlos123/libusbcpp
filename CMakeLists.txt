cmake_minimum_required(VERSION 3.16)
project(libusbcpp VERSION 0.1.1 LANGUAGES C CXX)

include(cmake/util.cmake)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
endif()

##################################
# Options: Library configuration #
##################################

option(LIBUSBCPP_STATIC_RUNTIME "Use statically linked runtime" off)
option(LIBUSBCPP_STATIC_LIB "Build shared library instead of static" off)
option(LIBUSBCPP_BUILD_EXAMPLES "Build examples" on)
option(LIBUSBCPP_VERBOSE_LOGGING "Enable internal verbose logging for debugging" off)


################
# Dependencies #
################

get_github_dependency(libusb "modules/libusb/Makefile.am" "https://github.com/libusb/libusb.git" v1.0.26)
include(cmake/libusb.cmake)

#####################
# Target definition #
#####################

set(SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/libusbcpp.cpp
        )

if (LIBUSBCPP_STATIC_LIB)
    add_library(libusbcpp STATIC ${SOURCES})
else ()
    add_library(libusbcpp SHARED ${SOURCES})
endif ()

add_library(libusbcpp::libusbcpp ALIAS libusbcpp)
target_compile_features(libusbcpp PRIVATE cxx_std_17)
set_target_properties(libusbcpp PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(libusbcpp PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

if(MSVC)
    string(APPEND CMAKE_CXX_FLAGS " /Zc:__cplusplus /MP")
else()
    target_compile_options(libusbcpp PRIVATE -Wno-psabi)
endif()

if (LIBUSBCPP_VERBOSE_LOGGING)
target_compile_definitions(libusbcpp PRIVATE LIBUSBCPP_VERBOSE_LOGGING)
endif()
if (LIBUSBCPP_STATIC_LIB)
target_compile_definitions(libusbcpp PRIVATE LIBUSBCPP_STATIC_LIB)
endif()

if (LIBUSBCPP_STATIC_RUNTIME)
    use_static_runtime(libusb)
    use_static_runtime(libusbcpp)
endif()

set_runtime_output_directory(libusbcpp ${CMAKE_CURRENT_BINARY_DIR}/bin)
set_archive_output_directory(libusbcpp ${CMAKE_CURRENT_BINARY_DIR}/bin)
set_library_output_directory(libusbcpp ${CMAKE_CURRENT_BINARY_DIR}/bin)

############################
# Preprocessor definitions #
############################

if (WIN32)
    target_compile_definitions(libusbcpp PRIVATE
            WIN32_LEAN_AND_MEAN      # Prevents Windows.h from adding unnecessary includes
            NOMINMAX                 # Prevents Windows.h from defining min/max as macros
            _CRT_SECURE_NO_WARNINGS
            )
    target_compile_definitions(libusbcpp PUBLIC
            UNICODE
            _UNICODE
            )
endif()






###########
# Linking #
###########

target_link_libraries(libusbcpp libusb)



############
# Examples #
############

if (LIBUSBCPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()



###########
# Install #
###########

#include(CMakePackageConfigHelpers)
#write_basic_package_version_file(
#    libusbcppConfigVersion.cmake
#    VERSION ${PACKAGE_VERSION}
#    COMPATIBILITY AnyNewerVersion
#)
#
## Install binaries
#install(
#    TARGETS Battery
#    EXPORT BatteryTargets
#    LIBRARY DESTINATION "lib"
#    ARCHIVE DESTINATION "lib"
#    RUNTIME DESTINATION "bin"
#    INCLUDES DESTINATION "include"
#)
#
## Install headers
#install(
#    DIRECTORY include/
#    DESTINATION "include"
#    FILES_MATCHING PATTERN "*.h*"
#)
#
#install(
#    EXPORT BatteryTargets
#    DESTINATION "lib/cmake/Battery"
#    NAMESPACE Battery::
#)
#
## Install Targets
#configure_file(cmake/BatteryConfig.cmake.in BatteryConfig.cmake @ONLY)
#install(
#    FILES "${CMAKE_CURRENT_BINARY_DIR}/BatteryConfig.cmake"
#          "${CMAKE_CURRENT_BINARY_DIR}/BatteryConfigVersion.cmake"
#    DESTINATION lib/cmake/Battery
#)
