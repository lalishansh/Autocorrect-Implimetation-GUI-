-- Autocorrect
project "Autocorrect"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("../builds/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../builds/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
	}
    
	includedirs
	{
        -- "../%{IncludeDir.??}",
        "../%{IncludeDir.ImGui}",
        "../%{IncludeDir.GLM}",
        "../%{IncludeDir.spdlog}",
        "../%{IncludeDir.Glad}",
        "../%{IncludeDir.Glad}/khr",
        "../OpenGL-Laboratory/src",
        "../~vendor"
	}

	links
	{
		"OpenGL-Laboratory"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			-- #defines
		}

	filter "configurations:Debug"
		defines "MODE_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "MODE_RELEASE"
		runtime "Release"
        optimize "on"