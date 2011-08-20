project "XreaL-master"
	targetname  "XreaL-master"
	language    "C++"
	kind        "ConsoleApp"
	targetdir 	"../.."
	flags       { "ExtraWarnings" }
	files
	{
		"**.c", "**.h",
	}
	includedirs
	{
		--"../libs/zlib",
		--"../shared",
	}
	defines
	{ 
		--"DEDICATED",
	}
	
	--
	-- Platform Configurations
	-- 	
	configuration "x32"
		targetdir 	"../../../bin32"
	
	configuration "x64"
		targetdir 	"../../../bin64"
	
	-- 
	-- Project Configurations
	-- 
	configuration "vs*"
		--flags       { "WinMain" }
		links
		{ 
			"winmm",
			"wsock32",
		}
		defines
		{
			"WIN32",
			"_CRT_SECURE_NO_WARNINGS",
			--"USE_INTERNAL_SPEEX",
			--"USE_INTERNAL_ZLIB",
			--"FLOATING_POINT",
			--"USE_ALLOCA"
		}

	configuration { "linux", "gmake" }
		buildoptions
		{
			--"`pkg-config --cflags sdl`",
		}
		linkoptions
		{
			--"`pkg-config --libs sdl`",
		}
	
	configuration "linux"
		targetname  "xreal-master"
		links
		{
			"m",
		}
		
		