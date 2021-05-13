-- Autocorrect
workspace "Autocorrect"
    architecture "x64"
    startproject "Autocorrect"

    configurations
    {
        "Debug",
        "Release"
    }

    flags
    {
        "MultiProcessorCompile"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to solution
IncludeDir = {}
IncludeDir["Glad"]   = "~vendor/glad-OpenGL_4.4/include"
IncludeDir["GLFW"]   = "~vendor/glfw/include"
IncludeDir["ImGui"]  = "~vendor/imgui"
IncludeDir["GLM"]    = "~vendor/glm"
IncludeDir["spdlog"] = "~vendor/spdlog/include"
IncludeDir["stb_image"] = "~vendor/stb_image"

-- Projects
group "Dependencies"
    -- include [prj.path]
    include "~vendor/glad-OpenGL_4.4" -- includeexternal for upcoming workspace/solutions so that it dosen't needs to recompile
    include "~vendor/glfw"            -- includeexternal for upcoming workspace/solutions
    include "~vendor/imgui"           -- includeexternal for upcoming workspace/solutions
group ""

include "OpenGL-Laboratory" -- includeexternal for upcoming workspace/solutions
include "Autocorrect"