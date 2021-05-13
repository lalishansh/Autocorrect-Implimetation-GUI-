-- Glad
project "Glad"
    kind "StaticLib"
    language "C"
    staticruntime "on"
    
    targetdir ("../vendor-builds/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../vendor-builds/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "include/glad/glad.h",
        "include/khr/khrplatform.h",
        "src/glad.c"
    }

    includedirs
    {
        "include",
        "include/glad",
        "include/khr"
    }
    
    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"