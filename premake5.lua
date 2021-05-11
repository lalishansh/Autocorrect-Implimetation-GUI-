-- OpenGL-Sandbox
workspace "OpenGL-TestSite"
    architecture "x64"
    startproject "OpenGL-Sandbox"

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

-- Projects
group "Dependencies"
    -- [prj.name]
group ""

include "OpenGL-Laboratory" -- includeexternal for upcoming workspace/solutions so that it dosen't needs to recompile
include "OpenGL-Sandbox"