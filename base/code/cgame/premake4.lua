

project "base_cgame"
	targetname  ("cgame"..proc)
	targetprefix ""
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
		
		--"**.c", "**.cpp", "**.h",
		
		"cg_animation.c",
		"cg_consolecmds.c",
		"cg_draw.c",
		"cg_drawtools.c",
		"cg_effects.c",
		"cg_ents.c",
		"cg_event.c",
		"cg_info.c",
		"cg_local.h",
		"cg_localents.c",
		"cg_lua.c",
		"cg_main.c",
		"cg_marks.c",
		"cg_osd.c",
		"cg_particles.c",
		"cg_players.c",
		"cg_playerstate.c",
		"cg_predict.c",
		"cg_scoreboard.c",
		"cg_servercmds.c",
		"cg_snapshot.c",
		"cg_spell_casts.c",
		"cg_spell_effects.c",
		"cg_spell_ents.c",
		"cg_spell_hud.c",
		"cg_spell_hands.c",
		"cg_spell_sense.c",
		"cg_spell_util.h",
		"cg_spell_weaves.c",
		"cg_sword.c",
		"cg_syscalls.c",
		"cg_tutorial.c",
		"cg_unlagged.c",
		"cg_view.c",
		"cg_weapons.c",
		
		"lua_cgame.c",
		"lua_particle.c",
		
		"../game/bg_**.c", "../game/bg_**.cpp", "../game/bg_**.h",
		
		"../game/objective_common.h",
		
		"../game/spell_shared.c",
		"../game/spell_common.h",
		"../game/spell_shared.h",
		
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
	--excludes
	--{
	--	"cg_newdraw.c",
	--}
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

	--
	-- Platform Configurations
	--
	configuration { "windows", "x32" }
		targetdir  "../../../bin/win32/base"

	configuration { "windows", "x64" }
		targetdir  "../../../bin/win64/base"

	configuration { "linux or solaris or bsd" }
		targetdir 	("../../../bin/" .. os.get() .. "-" .. proc .. "/base")

	configuration { "macosx" }
		targetdir 	("../../../bin/macosx/base")
