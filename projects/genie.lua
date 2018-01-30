local ide_dir = iif(_ACTION == nil, "vs2015", _ACTION)
local LOCATION = ide_dir
local OBJ_DIR = LOCATION .. "/obj/"
local BINARY_DIR = LOCATION .. "/bin/"
local WORKING_DIR = "../../NewEngineData"
local SRC_DIR = "../src/"

newoption {
	trigger = "workdir",
	description = "Defines debugger working directory"
}
if(_OPTIONS["workdir"]) then
	WORKING_DIR = _OPTIONS["workdir"]
end


solution "NewEngine"
	configurations {
		"Debug",
		"Release"
	}
	
	platforms {
		"x64"
	}
	
	flags { 
		"FatalWarnings",
		"NoPCH",
		"NoExceptions",
		"NoRTTI",
		"WinMain"
	}
	
	language "C++"
	startproject "main"
	location(LOCATION)
	
	includedirs {
		SRC_DIR
	}
	
	configuration "Debug"
		objdir(OBJ_DIR .. "Debug")
		targetdir(BINARY_DIR .. "Debug")
		flags { "Symbols" }
		
	configuration "Release"
		objdir(OBJ_DIR .. "Release")
		targetdir(BINARY_DIR .. "Release")
		flags { "Optimize" }
		
	configuration {}
	
	
project "core"
	kind "StaticLib"
	
	includedirs {
	}
	
	files {
		SRC_DIR .. "core/**.h",
		SRC_DIR .. "core/**.inl",
		SRC_DIR .. "core/**.cpp"
	}
	
	configuration {}
	
	
project "renderer"
	kind "StaticLib"
	
	links { "core" }
	
	includedirs {
		"../external/bgfx/include"
	}
	
	files {
		SRC_DIR .. "renderer/**.h",
		SRC_DIR .. "renderer/**.inl",
		SRC_DIR .. "renderer/**.cpp"
	}
	
	configuration {}
	
	
project "main"
	kind "WindowedApp"
	
	links { "core", "renderer" }
	
	debugdir(WORKING_DIR)
	
	includedirs {
	}
	
	files {
		SRC_DIR .. "main/**.h",
		SRC_DIR .. "main/**.inl",
		SRC_DIR .. "main/**.cpp"
	}

	libdirs { "../external/bgfx/lib/win64" }
	
	configuration { "Debug", "x64" }
		links { "bgfxDebug", "bimgDebug", "bxDebug" }
	configuration { "Release", "x64" }
		links { "bgfxRelease", "bimgRelease", "bxRelease" }
	configuration {}
	
	
	
	