
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
        _SCRIPT_DIR .. "/include"
    }
    includedirs (_includedirs)

    
    -- Main source files
    files ({ "include/**", "src/**" })




    -- Include and linker information for premake projects using this library
    LIBUSBCPP_INCLUDE_DIRS = {}
    appendTable(LIBUSBCPP_INCLUDE_DIRS, _includedirs)

    LIBUSBCPP_LINK_DIRS = {}
    appendTable(LIBUSBCPP_LINK_DIRS, _SCRIPT_DIR .. "/bin/%{cfg.buildcfg}/")

    LIBUSBCPP_LINKS = { "libusbcpp" }
