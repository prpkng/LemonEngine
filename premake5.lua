workspace "Lemon"
	architecture "x64"
	startproject "Sandbox"
	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Lemon"
	location "Lemon"
	kind "SharedLib"
	language "C++"
	targetdir ("bin/".. outputdir .."/%{prj.name}")
	objdir ("bin-int/".. outputdir .."/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/vendor/spdlog/include"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"
		defines
		{
			"LM_PLATFORM_WINDOWS",
			"LM_BUILD_DLL"
		}
	
	postbuildcommands
	{
		("{COPY} %{cfg.buildtarget.relpath} ../bin/"..outputdir.."/Sandbox")
	}

	filter "configurations:Debug"
		defines "LM_DEBUG"
		symbols "On"
	filter "configurations:Release"
		defines "LM_RELEASE"
		optimize "On"
	filter "configurations:Dist"
		defines "LM_DIST"
		optimize "On"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/".. outputdir .."/%{prj.name}")
	objdir ("bin-int/".. outputdir .."/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}
	
	includedirs
	{
		"Lemon/vendor/spdlog/include",
		"Lemon/src"
	}

	links
	{
		"Lemon"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"
		defines
		{
			"LM_PLATFORM_WINDOWS",
		}
	
	postbuildcommands
	{
	}

	filter "configurations:Debug"
		defines "LM_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "LM_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "LM_DIST"
