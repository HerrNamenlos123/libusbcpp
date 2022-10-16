
set(LIBUSB_DIR $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../modules/libusb/>$<INSTALL_INTERFACE:>)
add_library(libusb STATIC)
add_library(libusb::libusb ALIAS libusb)

# Generate the config file
if (MSVC)
    set(IS_MSVC 1)
else()
    set(IS_MSVC 0)
endif()
configure_file(${CMAKE_CURRENT_LIST_DIR}/../libusb/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/libusb/config.h)
target_include_directories(libusb PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/libusb)

# Any platform
target_sources(libusb PRIVATE
        ${LIBUSB_DIR}/libusb/core.c
        ${LIBUSB_DIR}/libusb/descriptor.c
        ${LIBUSB_DIR}/libusb/hotplug.c
        ${LIBUSB_DIR}/libusb/io.c
        ${LIBUSB_DIR}/libusb/libusb-1.0.rc
        ${LIBUSB_DIR}/libusb/strerror.c
        ${LIBUSB_DIR}/libusb/sync.c)

if (WIN32)          # Windows Platform
    target_sources(libusb PRIVATE
            ${LIBUSB_DIR}/libusb/os/threads_windows.c
            ${LIBUSB_DIR}/libusb/os/events_windows.c
            ${LIBUSB_DIR}/libusb/os/windows_common.c
            ${LIBUSB_DIR}/libusb/os/windows_usbdk.c
            ${LIBUSB_DIR}/libusb/os/windows_winusb.c)
elseif (DARWIN)     # DARWIN Platform
    target_sources(libusb PRIVATE
            ${LIBUSB_DIR}/libusb/os/events_posix.c
            ${LIBUSB_DIR}/libusb/os/threads_posix.c
            ${LIBUSB_DIR}/libusb/os/darwin_usb.c)
elseif (HAIKU)     # HAIKU Platform
    target_sources(libusb PRIVATE
            ${LIBUSB_DIR}/libusb/os/events_posix.c
            ${LIBUSB_DIR}/libusb/os/threads_posix.c
            ${LIBUSB_DIR}/libusb/os/haiku_pollfs.c
            ${LIBUSB_DIR}/libusb/os/haiku_usb_backend.c
            ${LIBUSB_DIR}/libusb/os/haiku_usb_raw.c)
elseif (OPENBSD)     # OpenBSD Platform
    target_sources(libusb PRIVATE
            ${LIBUSB_DIR}/libusb/os/events_posix.c
            ${LIBUSB_DIR}/libusb/os/threads_posix.c
            ${LIBUSB_DIR}/libusb/os/openbsd_usb.c)
elseif (NETBSD)     # OpenBSD Platform
    target_sources(libusb PRIVATE
            ${LIBUSB_DIR}/libusb/os/events_posix.c
            ${LIBUSB_DIR}/libusb/os/threads_posix.c
            ${LIBUSB_DIR}/libusb/os/netbsd_usb.c)
elseif (SUNOS)     # SunOS Platform
    target_sources(libusb PRIVATE
            ${LIBUSB_DIR}/libusb/os/events_posix.c
            ${LIBUSB_DIR}/libusb/os/threads_posix.c
            ${LIBUSB_DIR}/libusb/os/sunos_usb.c)
else ()       # Any other (Linux POSIX Platforms)
    target_sources(libusb PRIVATE
            ${LIBUSB_DIR}/libusb/os/events_posix.c
            ${LIBUSB_DIR}/libusb/os/threads_posix.c
            ${LIBUSB_DIR}/libusb/os/linux_netlink.c
            ${LIBUSB_DIR}/libusb/os/linux_udev.c
            ${LIBUSB_DIR}/libusb/os/linux_usbfs.c)
endif ()

target_include_directories(libusb PUBLIC
        $<BUILD_INTERFACE:${LIBUSB_DIR}/libusb>
        $<INSTALL_INTERFACE:/>
        )

install(
        TARGETS libusb
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
        RUNTIME DESTINATION "bin"
        INCLUDES DESTINATION "include"
)