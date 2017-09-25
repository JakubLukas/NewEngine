local ide_dir = iif(_ACTION == nil, "vs2015", _ACTION)
local LOCATION = ide_dir
local OBJ_DIR = LOCATION .. "/obj/"
local BINARY_DIR = LOCATION .. "/bin/"
local WORKING_DIR = "."
local SRC_DIR = "../src/"

newoption {
	trigger = "workdir",
	description = "Defines debugger working directory"
}
if(_OPTIONS["working_dir"]) then
	WORKING_DIR = _OPTIONS["working_dir"]
end


solution "VoxelEngine"
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
	
	debugdir(WORKING_DIR)
	
	includedirs {
	}
	
	files {
		SRC_DIR .. "core/**.h",
		SRC_DIR .. "core/**.inl",
		SRC_DIR .. "core/**.cpp"
	}
	
	configuration {}
	
	
project "main"
	kind "WindowedApp"
	
	links { "core" }
	
	debugdir(WORKING_DIR)
	
	includedirs {
	}
	
	files {
		SRC_DIR .. "main/**.h",
		SRC_DIR .. "main/**.inl",
		SRC_DIR .. "main/**.cpp"
	}
	
	configuration {}
	
	
	
	