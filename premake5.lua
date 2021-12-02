workspace "SpeederGame"
    architecture "x64"
    configurations { "Debug", "Release" }
    startproject "Game"


outputdir = "%{cfg.buildcfg}-%{cfg.architecture}"


include "Game/vendor/imgui"
include "Game/vendor/directxtk"

project "Game"
    location "Game"
    kind "ConsoleApp"
    staticruntime "on"
    language "C++"
    cppdialect "C++20"
    systemversion "latest"
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    

    pchheader "pch.hpp"
    pchsource "%{prj.name}/Src/pch.cpp"

    files 
    {
        "%{prj.name}/Src/**.h",
        "%{prj.name}/Src/**.hpp",
        "%{prj.name}/Src/**.cpp",
        "%{prj.name}/Src/**.hlsl",
        "%{prj.name}/vendor/singleHeaders/**.h",
        "%{prj.name}/vendor/singleHeaders/**.hpp",
        "%{prj.name}/vendor/imgui/backends/imgui_impl_dx11.h",
        "%{prj.name}/vendor/imgui/backends/imgui_impl_dx11.cpp",
        "%{prj.name}/vendor/imgui/backends/imgui_impl_win32.h",
        "%{prj.name}/vendor/imgui/backends/imgui_impl_win32.cpp",
        
    }

    includedirs
    {
        "%{prj.name}/Src",
        "%{prj.name}/Src/**",
        "%{prj.name}/vendor/**",
    }

    libdirs { "%{prj.name}/vendor/Visual Leak Detector/lib" }

    defines
    {
        "_UNICODE",
        "UNICODE",
    }

    links
    {
        "ImGui",
        "DirectXTK"
    }


    filter { "configurations:Debug" }
        defines { "_DEBUG", "DEBUG" }
        runtime "Debug"
        symbols "on"

    filter { "configurations:Release" }
       defines { "_NDEBUG", "NDEBUG" }
       runtime "Release"
       optimize "on"

    filter {"files:**.hlsl"}
        flags {"ExcludeFromBuild"}

    filter {"files:Game/vendor/**.cpp"}
        flags {"NoPCH"}