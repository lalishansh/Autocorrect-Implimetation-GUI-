-- OpenGL-Laboratory
project "OpenGL-Laboratory"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

	targetdir ("../builds/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../builds/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.h"
	pchsource "src/pch.cpp"

    files
	{
		-- "~vendor/[prj.dir]/**.h",
		-- "~vendor/[prj.dir]/**.cpp",
		"../~vendor/glm/glm/**.hpp",
		"../~vendor/glm/glm/**.inl",
		"../~vendor/stb_image/**.h",
		"../~vendor/stb_image/**.cpp",
		"src/**.h",
		"src/**.cpp"
	}
    
	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

    includedirs
	{
        -- "../%{IncludeDir.??}",
        "../%{IncludeDir.spdlog}",
        "../%{IncludeDir.Glad}",
        "../%{IncludeDir.Glad}/khr",
        "../%{IncludeDir.GLFW}",
        "../%{IncludeDir.ImGui}",
        "../%{IncludeDir.GLM}",
        "../%{IncludeDir.stb_image}",
        "../~vendor",
		"src"
	}

    links 
	{ 
		-- [prj],
		"Glad",
		"GLFW",
		"ImGui",
		"opengl32.lib"
	}

    filter "system:windows"
		systemversion "latest"

        defines
        {
            -- #defines
			"GLFW_INCLUDE_NONE"
        }

    filter "configurations:Debug"
		defines "MODE_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "MODE_RELEASE"
		runtime "Release"
		optimize "on"