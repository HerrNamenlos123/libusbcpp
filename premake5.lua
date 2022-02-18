
-- Utility functions
function appendTable(tableA, tableB)
    for _,v in ipairs(tableB) do 
        table.insert(tableA, v) 
    end
end

-- Main library project
project "libusbcpp"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    location "build"
    targetdir "bin/%{cfg.buildcfg}"
    targetname "%{prj.name}"
    
    filter { "platforms:x86" }
        architecture "x86"

    filter { "platforms:x64" }
        architecture "x86_64"

    filter "configurations:Debug"
        defines { "DEBUG", "_DEBUG", "NDEPLOY" }
        runtime "Debug"
        symbols "On"
        optimize "Off"

    filter "configurations:Release"
        defines { "NDEBUG", "NDEPLOY" }
        runtime "Release"
        symbols "On"
        optimize "On"

    filter "configurations:Deploy"
        defines { "NDEBUG", "DEPLOY" }
        runtime "Release"
        symbols "Off"
        optimize "On"

    filter {}


    -- Include directories
    local _includedirs = { 
        _SCRIPT_DIR .. "/include",
        _SCRIPT_DIR .. "/modules/libusb/msvc",
        _SCRIPT_DIR .. "/modules/libusb/libusb",
        _SCRIPT_DIR .. "/modules/libusb/libusb/os"
    }
    includedirs (_includedirs)
    includedirs ("include_private")

    
    -- Main source files
    files ({ "include/**", "src/**" })

    -- libusb source files
    files ({ "modules/libusb/libusb/*.h" })
    files ({ "modules/libusb/libusb/*.c" })
    files ({ "modules/libusb/msvc/config.h" })
    files ({ "modules/libusb/libusb/os/events_windows.c", "modules/libusb/libusb/os/events_windows.h" })
    files ({ "modules/libusb/libusb/os/threads_windows.c", "modules/libusb/libusb/os/threads_windows.h" })
    files ({ "modules/libusb/libusb/os/windows_common.c", "modules/libusb/libusb/os/windows_common.h" })
    files ({ "modules/libusb/libusb/os/windows_usbdk.c", "modules/libusb/libusb/os/windows_usbdk.h" })
    files ({ "modules/libusb/libusb/os/windows_winusb.c", "modules/libusb/libusb/os/windows_winusb.h" })



    -- Include and linker information for premake projects using this library
    LIBUSBCPP_INCLUDE_DIRS = {}
    appendTable(LIBUSBCPP_INCLUDE_DIRS, _includedirs)

    LIBUSBCPP_LINK_DIRS = {}
    appendTable(LIBUSBCPP_LINK_DIRS, _SCRIPT_DIR .. "/bin/%{cfg.buildcfg}/")

    LIBUSBCPP_LINKS = { "libusbcpp" }
