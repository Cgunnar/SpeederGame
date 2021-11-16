project "DirectXTK"
    location "DirectXTK"
    kind "StaticLib"
    staticruntime "on"
    language "C++"
    cppdialect "C++20"
    systemversion "latest"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "pch.h"
    pchsource "Src/pch.cpp"
    
    files 
    {
        "Src/**.h",
        "Src/**.cpp",
        "Src/**.inc"
        --"src/**.fx",
        --"src/**.hlsl",
        --"src/**.hlsli",
        --"src/**.fxh",
    }
    includedirs
    {
        "Inc",
        "Src/Shaders/Compiled"
    }

    filter { "configurations:Debug" }
        defines 
        { 
            "_WIN32_WINNT=0x0601",
            "_WIN7_PLATFORM_UPDATE",
            "WIN32",
            "_DEBUG",
            "_LIB",
            "_CRT_STDIO_ARBITRARY_WIDE_SPECIFIERS"
        }
        runtime "Debug"
        symbols "on"

    filter { "configurations:Release" }
        defines 
        { 
            "_WIN32_WINNT=0x0601",
            "_WIN7_PLATFORM_UPDATE",
            "WIN32",
            "NDEBUG",
            "_LIB",
            "_CRT_STDIO_ARBITRARY_WIDE_SPECIFIERS"
        }
       runtime "Release"
       optimize "on"
