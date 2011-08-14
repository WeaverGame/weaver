
project "XreaL"
	targetname  "XreaL"
	language    "C++"
	kind        "WindowedApp"
	flags       { "ExtraWarnings", "NoRTTI" }
	files
	{
		"../shared/**.c", "../shared/**.h",
		
		--"botlib/**.c", "botlib/**.h",
		
		"client/**.c", "client/**.h",
		"server/**.c", "server/**.h",
		
		"qcommon/**.h", 
		"qcommon/cmd.c",
		"qcommon/common.c",
		"qcommon/cvar.c",
		"qcommon/files.c",
		"qcommon/huffman.c",
		"qcommon/md4.c",
		"qcommon/md5.c",
		"qcommon/msg.c",
		"qcommon/vm.c",
		"qcommon/net_*.c",
		"qcommon/cm_*.c",
		"qcommon/unzip.c",
		"qcommon/parse.c",
		
		"renderer/**.c", "renderer/**.cpp", "renderer/**.h",
		
		"../libs/glew/src/glew.c",
		"../libs/glew/src/glew.h",
		
		"../libs/jpeg/**.c", "../../libs/jpeg/**.h",
		"../libs/png/**.c", "../../libs/png/**.h",
		"../libs/zlib/**.c", "../../libs/zlib/**.h",
		"../libs/openexr/**.cpp", "../../libs/openexr/**.h",
		
		--"../libs/ft2/**.c", "../../libs/ft2/**.h",
		
		"../libs/freetype/src/autofit/autofit.c",
		"../libs/freetype/src/bdf/bdf.c",
		"../libs/freetype/src/cff/cff.c",
		"../libs/freetype/src/base/ftbase.c",
		"../libs/freetype/src/base/ftbitmap.c",
		"../libs/freetype/src/cache/ftcache.c",
		"../libs/freetype/src/base/ftdebug.c",
		"../libs/freetype/src/base/ftgasp.c",
		"../libs/freetype/src/base/ftglyph.c",
		"../libs/freetype/src/gzip/ftgzip.c",
		"../libs/freetype/src/base/ftinit.c",
		"../libs/freetype/src/lzw/ftlzw.c",
		"../libs/freetype/src/base/ftstroke.c",
		"../libs/freetype/src/base/ftsystem.c",
		"../libs/freetype/src/smooth/smooth.c",
		"../libs/freetype/src/base/ftbbox.c",
		"../libs/freetype/src/base/ftmm.c",
		"../libs/freetype/src/base/ftpfr.c",
		"../libs/freetype/src/base/ftsynth.c",
		"../libs/freetype/src/base/fttype1.c",
		"../libs/freetype/src/base/ftwinfnt.c",
		"../libs/freetype/src/pcf/pcf.c",
		"../libs/freetype/src/pfr/pfr.c",
		"../libs/freetype/src/psaux/psaux.c",
		"../libs/freetype/src/pshinter/pshinter.c",
		"../libs/freetype/src/psnames/psmodule.c",
		"../libs/freetype/src/raster/raster.c",
		"../libs/freetype/src/sfnt/sfnt.c",
		"../libs/freetype/src/truetype/truetype.c",
		"../libs/freetype/src/type1/type1.c",
		"../libs/freetype/src/cid/type1cid.c",
		"../libs/freetype/src/type42/type42.c",
		"../libs/freetype/src/winfonts/winfnt.c",
		
		"../libs/ogg/src/bitwise.c",
		"../libs/ogg/src/framing.c",
		
		"../libs/vorbis/lib/mdct.c",
		"../libs/vorbis/lib/smallft.c",
		"../libs/vorbis/lib/block.c",
		"../libs/vorbis/lib/envelope.c",
		"../libs/vorbis/lib/window.c",
		"../libs/vorbis/lib/lsp.c",
		"../libs/vorbis/lib/lpc.c",
		"../libs/vorbis/lib/analysis.c",
		"../libs/vorbis/lib/synthesis.c",
		"../libs/vorbis/lib/psy.c",
		"../libs/vorbis/lib/info.c",
		"../libs/vorbis/lib/floor1.c",
		"../libs/vorbis/lib/floor0.c",
		"../libs/vorbis/lib/res0.c",
		"../libs/vorbis/lib/mapping0.c",
		"../libs/vorbis/lib/registry.c",
		"../libs/vorbis/lib/codebook.c",
		"../libs/vorbis/lib/sharedbook.c",
		"../libs/vorbis/lib/lookup.c",
		"../libs/vorbis/lib/bitrate.c",
		"../libs/vorbis/lib/vorbisfile.c",
		
		-- "../libs/speex/bits.c",
		-- "../libs/speex/buffer.c",
		-- "../libs/speex/cb_search.c",
		-- "../libs/speex/exc_10_16_table.c",
		-- "../libs/speex/exc_10_32_table.c",
		-- "../libs/speex/exc_20_32_table.c",
		-- "../libs/speex/exc_5_256_table.c",
		-- "../libs/speex/exc_5_64_table.c",
		-- "../libs/speex/exc_8_128_table.c",
		-- "../libs/speex/fftwrap.c",
		-- "../libs/speex/filterbank.c",
		-- "../libs/speex/filters.c",
		-- "../libs/speex/gain_table.c",
		-- "../libs/speex/gain_table_lbr.c",
		-- "../libs/speex/hexc_10_32_table.c",
		-- "../libs/speex/hexc_table.c",
		-- "../libs/speex/high_lsp_tables.c",
		-- "../libs/speex/jitter.c",
		-- "../libs/speex/kiss_fft.c",
		-- "../libs/speex/kiss_fftr.c",
		-- "../libs/speex/lsp_tables_nb.c",
		-- "../libs/speex/ltp.c",
		-- "../libs/speex/mdf.c",
		-- "../libs/speex/modes.c",
		-- "../libs/speex/modes_wb.c",
		-- "../libs/speex/nb_celp.c",
		-- "../libs/speex/preprocess.c",
		-- "../libs/speex/quant_lsp.c",
		-- "../libs/speex/resample.c",
		-- "../libs/speex/sb_celp.c",
		-- "../libs/speex/speex_smallft.c",
		-- "../libs/speex/speex.c",
		-- "../libs/speex/speex_callbacks.c",
		-- "../libs/speex/speex_header.c",
		-- "../libs/speex/speex_lpc.c",
		-- "../libs/speex/speex_lsp.c",
		-- "../libs/speex/speex_window.c",
		-- "../libs/speex/stereo.c",
		-- "../libs/speex/vbr.c",
		-- "../libs/speex/vq.c",
		
		"../libs/theora/lib/dec/apiwrapper.c",
		"../libs/theora/lib/dec/bitpack.c",
		"../libs/theora/lib/dec/decapiwrapper.c",
		"../libs/theora/lib/dec/decinfo.c",
		"../libs/theora/lib/dec/decode.c",
		"../libs/theora/lib/dec/dequant.c",
		"../libs/theora/lib/dec/fragment.c",
		"../libs/theora/lib/dec/huffdec.c",
		"../libs/theora/lib/dec/idct.c",
		"../libs/theora/lib/dec/thinfo.c",
		"../libs/theora/lib/dec/internal.c",
		"../libs/theora/lib/dec/quant.c",
		"../libs/theora/lib/dec/state.c",
	}
	includedirs
	{
		"../shared",
		"../libs/sdl/include",
		"../libs/zlib",
		"../libs/glew/include",
		"../libs/freetype/include",
		"../libs/ogg/include",
		"../libs/vorbis/include",
		"../libs/theora/include",
		"../libs/speex/include",
	}
	defines
	{ 
		"STANDALONE",
		"REF_HARD_LINKED",
		"GLEW_STATIC",
		"BUILD_FREETYPE",
		"FT2_BUILD_LIBRARY",
		"USE_CODEC_VORBIS",
		--"USE_VOIP",
		"USE_CIN_THEORA",
		"USE_ALLOCA",
		"FLOATING_POINT",
		--"USE_CURL", 
		--"USE_MUMBLE",
		--"USE_INTERNAL_GLFW",
		--"USE_INTERNAL_GLEW",
	}
	excludes
	{
		"server/sv_rankings.c",
		"renderer/tr_animation_mdm.c",
		"renderer/tr_model_mdm.c",
	}
	
	--
	-- Platform Configurations
	-- 	
	configuration "x32"
		targetdir 	"../../bin32"
		files
		{ 
			"code/qcommon/vm_x86.c",
			"../libs/theora/lib/dec/x86/mmxidct.c",
			"../libs/theora/lib/dec/x86/mmxfrag.c",
			"../libs/theora/lib/dec/x86/mmxstate.c",
			"../libs/theora/lib/dec/x86/x86state.c"
		}
	
	configuration "x64"
		targetdir 	"../../bin64"
		files
		{ 
			"qcommon/vm_x86_64.c",
			"qcommon/vm_x86_64_assembler.c",
			"../libs/theora/lib/dec/x86/mmxidct.c",
			"../libs/theora/lib/dec/x86/mmxfrag.c",
			"../libs/theora/lib/dec/x86/mmxstate.c",
			"../libs/theora/lib/dec/x86/x86state.c"
		}
		
	--
	-- Options Configurations
	--
	--configuration "with-freetype"
	--	links        { "freetype" }
	--	buildoptions { "`pkg-config --cflags freetype2`" }
	--	defines      { "BUILD_FREETYPE" }

	--configuration "openal-dlopen"
	--	defines      
	--	{
	--		"USE_OPENAL",
	--		"USE_OPENAL_DLOPEN",
	--		"USE_OPENAL_LOCAL_HEADERS"
	--	}
		
	--configuration "openal-link"
	--	links        { "openal " }
	--	defines      { "USE_OPENAL" }

	
	--configuration { "vs*", "Release" }
	-- newaction {
		-- trigger = "prebuild",
		-- description = "Compile libcurl.lib",
		-- execute = function ()
			-- os.execute("cd ../libs/curl-7.12.2;cd lib;nmake /f Makefile.vc6 CFG=release")
		-- end
	-- }
	
	-- 
	-- Project Configurations
	-- 
	configuration "vs*"
		flags       { "WinMain" }
		files
		{
			"sys/sys_main.c",
			"sys/sys_win32.c",
			"sys/con_log.c",
			"sys/con_win32.c",
			"sys/sdl_gamma.c",
			"sys/sdl_glimp.c",
			"sys/sdl_input.c",
			"sys/sdl_snd.c",
			
			"sys/xreal.ico",
			"sys/win_resource.rc",
			
			"../libs/glew/src/wglew.h",
			
			--"code/glfw/lib/win32/*.c", "code/glfw/lib/win32/*.h",
		}
		excludes
		{
			--"win32/win_gamma.c",
			--"win32/win_glimp.c",
			--"win32/win_eh.cpp",
		}
		libdirs
		{
			--"../libs/curl-7.12.2/lib"
		}
		
		links
		{
			"SDL",
			"SDLmain",
			--"dinput8",
			"winmm",
			"wsock32",
			--"Iphlpapi",
			"opengl32",
			"user32",
			"advapi32",
			"ws2_32",
			"Psapi"
		}
		buildoptions
		{
			--"/MT"
		}
		linkoptions 
		{
			--"/NODEFAULTLIB:libcmt.lib",
			--"/NODEFAULTLIB:libcmtd.lib"
			"/NODEFAULTLIB:libc"
		}
		--linkoptions
		--{
		--	"/NODEFAULTLIB:libc"
		--}
		defines
		{
			"WIN32",
			"_CRT_SECURE_NO_WARNINGS",
			--"USE_INTERNAL_SPEEX",
			--"USE_INTERNAL_ZLIB",
			--"FLOATING_POINT",
			--"USE_ALLOCA"
		}
		
		
	configuration { "vs*", "x32" }
	configuration { "vs*", "x32" }
		files
		{
			"gldraw.c",
		}
		libdirs
		{
			"../libs/sdl/lib",
			--"../libs/curl-7.12.2/lib"
		}
		links
		{ 
			--"libcurl",
		}
		
	configuration { "vs*", "x64" }
		--files
		--{
		--	"qcommon/dl_main_stubs.c",
		--}

	configuration { "linux", "gmake" }
		buildoptions
		{
			"`pkg-config --cflags x11`",
			"`pkg-config --cflags xext`",
			"`pkg-config --cflags xxf86dga`",
			"`pkg-config --cflags xxf86vm`",
			"`pkg-config --cflags sdl`",
			"`pkg-config --cflags libcurl`",
		}
		linkoptions
		{
			"`pkg-config --libs x11`",
			"`pkg-config --libs xext`",
			"`pkg-config --libs xxf86dga`",
			"`pkg-config --libs xxf86vm`",
			"`pkg-config --libs sdl`",
			"`pkg-config --libs libcurl`",
		}
	
	configuration "linux"
		targetname  "etxreal"
		files
		{
			"unix/linux_signals.c",
			"unix/unix_main.c",
			"unix/unix_net.c",
			"unix/unix_shared.c",
			"unix/sdl_snd.c",
			"unix/linux_joystick.c",
			"unix/linux_gl3imp.c",
			"../libs/glew/src/glew.c",
		}
		--buildoptions
		--{
		--	"-pthread"
		--}
		links
		{
			"GL",
		}
		defines
		{
            "PNG_NO_ASSEMBLER_CODE",
		}
		
		