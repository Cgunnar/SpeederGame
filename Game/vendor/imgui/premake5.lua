project "ImGui"
    location "ImGui"
    kind "StaticLib"
    staticruntime "on"
    language "C++"
    cppdialect "C++20"
    systemversion "latest"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files 
    {
	    "imconfig.h",
	    "imgui.h",
	    "imgui.cpp",
	    "imgui_draw.cpp",
	    "imgui_internal.h",
	    "imgui_widgets.cpp",
	    "imstb_rectpack.h",
	    "imstb_textedit.h",
	    "imstb_truetype.h",
        "imgui_tables.cpp",
	    "imgui_demo.cpp",
    }

    filter { "configurations:Debug" }
        defines { "_DEBUG", "DEBUG" }
        runtime "Debug"
        symbols "on"

    filter { "configurations:Release" }
       defines { "_NDEBUG", "NDEBUG" }
       runtime "Release"
       optimize "on"
