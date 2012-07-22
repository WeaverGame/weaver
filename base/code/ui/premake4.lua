

project "base_ui"
	targetname  ("ui"..proc)
	targetprefix ""
	targetdir 	"../.."
	language    "C++"
	kind        "SharedLib"
	flags       { "ExtraWarnings" }
	files
	{
		"../../../code/shared/**.c",
		"../../../code/shared/q_shared.h",
		"../../../code/shared/ui_public.h",
		"../../../code/shared/tr_types.h",
		"../../../code/shared/keycodes.h",
		"../../../code/shared/surfaceflags.h",
		
		"**.c", "**.cpp", "**.h",
	}
	excludes
	{
		"ui_login.c",
		"ui_rankings.c",
		"ui_rankstatus.c",
		"ui_signup.c",
		"ui_specifyleague.c",
		"ui_spreset.c",
	}
	includedirs
	{
		"../../../code/shared",
	}
	defines
	{ 
		--"CGAMEDLL",
	}

	-- 
	-- Project Configurations
	-- 
	configuration "vs*"
		linkoptions
		{
			"/DEF:ui.def",
		}
		defines
		{
			"WIN32",
			"_CRT_SECURE_NO_WARNINGS",
		}

	--
	-- Platform Configurations
	--
	configuration { "windows", "x32" }
		targetdir  ("../../../bin/win32/base")

	configuration { "windows", "x64" }
		targetdir  ("../../../bin/win64/base")

	configuration { "linux or solaris or bsd" }
		targetdir 	("../../../bin/" .. os.get() .. "-" .. proc .. "/base")

	configuration { "macosx" }
		targetdir 	("../../../bin/macosx/base")
