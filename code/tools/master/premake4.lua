

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
	-- Project Configurations
	-- 
	configuration { "windows" }
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

	configuration { "linux or solaris or bsd os macosx", "gmake" }
		buildoptions
		{
			--"`pkg-config --cflags sdl`",
		}
		linkoptions
		{
			--"`pkg-config --libs sdl`",
		}

	configuration { "linux or solaris or bsd os macosx" }
		links
		{
			"m",
		}

	--
	-- Platform Configurations
	--
	configuration { "windows", "x32" }
		targetdir 	("../../../bin/win32")

	configuration { "windows", "x64" }
		targetdir 	("../../../bin/win64")

	configuration { "linux or solaris or bsd" }
		targetname  "xreal-master"
		targetdir 	("../../../bin/" .. os.get() .. "-" .. proc)

	configuration { "macosx" }
		targetname  "xreal-master"
		targetdir 	("../../../bin/macosx")
