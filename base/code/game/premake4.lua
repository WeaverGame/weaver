

project "base_game"
	targetname  ("qagame"..proc)
	targetprefix ""
	targetdir 	"../.."
	language    "C++"
	kind        "SharedLib"
	flags       { "ExtraWarnings" }
	files
	{
		"../../../code/shared/**.c",
		"../../../code/shared/q_shared.h",
		"../../../code/shared/g_public.h",
		"../../../code/shared/surfaceflags.h",

		--"**.c", "**.cpp", "**.h",
		"*.h",
		"acebot_ai.c",
		"acebot_cmds.c",
		"acebot_items.c",
		"acebot_movement.c",
		"acebot_nodes.c",
		"acebot_spawn.c",
		"bg_misc.c",
		"bg_pmove.c",
		"bg_slidemove.c",
		"g_active.c",
		"g_arenas.c",
		"g_bot.c",
		"g_client.c",
		"g_cmds.c",
		"g_combat.c",
		"g_explosive.c",
		"g_items.c",
		"g_lua.c",
		"g_main.c",
		"g_mem.c",
		"g_misc.c",
		"g_missile.c",
		"g_mover.c",
		"g_session.c",
		"g_spawn.c",
		"g_spell_**.c",
		"g_statfields.def",
		"g_statistics.c",
		"g_svcmds.c",
		"g_sword.c",
		"g_syscalls.c",
		"g_target.c",
		"g_team.c",
		"g_trigger.c",
		"g_unlagged.c",
		"g_utils.c",
		"g_weapon.c",
		"lua_*.c",

		"spell_info.def",
		"spell_shared.c",

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
	--	"g_rankings.c",
	--	"g_bullet.cpp",
	--}
	includedirs
	{
		"../../../code/shared",
		"../../../code/libs/lua/src",
	}
	defines
	{
		"QAGAME",
		"LUA"
	}

	--
	-- Project Configurations
	--
	configuration "vs*"
		linkoptions
		{
			"/DEF:game.def",
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
