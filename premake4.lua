--
-- XreaL build configuration script
-- 
solution "Weaver"
	--configurations { "Release", "Debug" }
	configurations { "Release", "Debug" }
	platforms {"x32", "x64", "native"}

	--
	-- Release/Debug Configurations
	--
	configuration "Release"
		defines     "NDEBUG"
		flags      
		{
			"OptimizeSpeed",
			"EnableSSE",
			"Symbols",
			--"StaticRuntime"
		}

	configuration "Debug"
		defines     "_DEBUG"
		flags
		{
			"Symbols",
			--"StaticRuntime",
			--"NoRuntimeChecks"
		}
	
--
-- Options

newoption
{
	trigger = "with-webp",
	description = "Compile with webp image format support"
}

newoption
{
	trigger = "with-glsl-opt",
	description = "Compile with glsl-optimizer support"
}

--
--newoption
--{
--	trigger = "with-omnibot",
--	description = "Compile with Omni-bot support"
--}

--newoption
--{
--	trigger = "with-freetype",
--	description = "Compile with freetype support"
--}
		
--newoption
--{
--	trigger = "with-openal",
--	value = "TYPE",
--	description = "Specify which OpenAL library",
--	allowed = 
--	{
--		{ "none", "No support for OpenAL" },
--		{ "dlopen", "Dynamically load OpenAL library if available" },
--		{ "link", "Link the OpenAL library as normal" },
--		{ "openal-dlopen", "Dynamically load OpenAL library if available" },
--		{ "openal-link", "Link the OpenAL library as normal" }
--	}
--}

--		
-- Platform specific defaults
--

-- We don't support freetype on VS platform
--if _ACTION and string.sub(_ACTION, 2) == "vs" then
--	_OPTIONS["with-freetype"] = false
--end

-- Default to dlopen version of OpenAL
--if not _OPTIONS["with-openal"] then
--	_OPTIONS["with-openal"] = "dlopen"
--end
--if _OPTIONS["with-openal"] then
--	_OPTIONS["with-openal"] = "openal-" .. _OPTIONS["with-openal"]
--end

if (tonumber(_PREMAKE_VERSION) < 4.4) then
	dofile("premake4_44.lua")
end

if os.get() == "windows" then
	if os.is64bit() then
		proc = "x86_64"
	else
		proc = "x86"
	end
else
	--configuration { "linux or solaris or bsd or macosx" }
	proc = os.outputof("uname -p")
	if proc == "unknown" then
		-- fallback
		proc = os.outputof("uname -m")
	end
end

include "code/engine"
include "base/code/game"
include "base/code/cgame"
include "base/code/ui"

include "code/tools/xmap2"
include "code/tools/master"

