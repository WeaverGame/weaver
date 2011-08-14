

project "base_cgame"
	targetname  "cgame"
	targetdir 	"../.."
	language    "C++"
	kind        "SharedLib"
	flags       { "ExtraWarnings" }
	files
	{
		"../../../code/shared/**.c",
		"../../../code/shared/q_shared.h",
		"../../../code/shared/cg_public.h",
		"../../../code/shared/tr_types.h",
		"../../../code/shared/keycodes.h",
		"../../../code/shared/surfaceflags.h",
		
		"**.c", "**.cpp", "**.h",
		
		"../game/bg_**.c", "../game/bg_**.cpp", "../game/bg_**.h",
		
		"../game/objective_common.h"
		
		"../game/spell_shared.c"
		"../game/spell_common.h"
		"../game/spell_info.def"
		"../game/spell_shared.h"
		
		"../game/lua_qmath.c",
		"../game/lua_vector.c",
		
		"../../../code/libs/lua/src/lapi.c",
		"../../../code/libs/lua/src/lcode.c",
		"../../../code/libs/lua/src/ldebug.c",
		"../../../code/libs/lua/src/ldo.c",
		"../../../code/libs/lua/src/ldump.c",
		"../../../code/libs/lua/src/lfunc.c",
		"../../../code/libs/lua/src/lgc.c",
		"../../../code/libs/lua/src/llex.c",
		"../../../code/libs/lua/src/lmem.c",
		"../../../code/libs/lua/src/lobject.c",
		"../../../code/libs/lua/src/lopcodes.c",
		"../../../code/libs/lua/src/lparser.c",
		"../../../code/libs/lua/src/lstate.c",
		"../../../code/libs/lua/src/lstring.c",
		"../../../code/libs/lua/src/ltable.c",
		"../../../code/libs/lua/src/ltm.c",
		"../../../code/libs/lua/src/lundump.c",
		"../../../code/libs/lua/src/lvm.c",
		"../../../code/libs/lua/src/lzio.c",
		"../../../code/libs/lua/src/lauxlib.c",
		"../../../code/libs/lua/src/lbaselib.c",
		"../../../code/libs/lua/src/ldblib.c",
		"../../../code/libs/lua/src/liolib.c",
		"../../../code/libs/lua/src/lmathlib.c",
		"../../../code/libs/lua/src/ltablib.c",
		"../../../code/libs/lua/src/lstrlib.c",
		"../../../code/libs/lua/src/loadlib.c",
		"../../../code/libs/lua/src/linit.c",
		"../../../code/libs/lua/src/loslib.c",
	}
	excludes
	{
		"cg_unlagged.c",
		"cg_newdraw.c",
	}
	includedirs
	{
		"../../../code/shared",
		"../../../code/libs/lua/src",
	}
	defines
	{ 
		"LUA",
	}
	
	--
	-- Platform Configurations
	--
	configuration "x32"
		targetname  "cgamex86"
	
	configuration "x64"
		targetname  "cgamex86_64"
				
	-- 
	-- Project Configurations
	-- 
	configuration "vs*"
		linkoptions
		{
			"/DEF:cgame.def",
		}
		defines
		{
			"WIN32",
			"_CRT_SECURE_NO_WARNINGS",
		}
	
	configuration { "linux", "x32" }
		targetname  "cgamex86"
		targetprefix ""
	
	configuration { "linux", "x64" }
		targetname  "cgamex86_64"
		targetprefix ""
	
	
