-- some utils from Premake 4.4

function os.outputof(cmd)
  local f = assert(io.popen(cmd, 'r'))
  local s = assert(f:read('*a'))
  f:close()
  s = string.gsub(s, '^%s+', '')
  s = string.gsub(s, '%s+$', '')
  s = string.gsub(s, '[\n\r]+', ' ')
  return s
end

local _64BitHostTypes = {
	"x86_64",
	"ia64",
	"amd64",
	"ppc64",
	"powerpc64",
	"sparc64"
}

function os.is64bit()
	-- Call the native code implementation. If this returns true then
	-- we're 64-bit, otherwise do more checking locally
	--if (os._is64bit()) then
	--	return true
	--end

	-- Identify the system
	local arch
	if _OS == "windows" then
		arch = os.getenv("PROCESSOR_ARCHITECTURE")
	elseif _OS == "macosx" then
		arch = os.outputof("echo $HOSTTYPE")
	else
		arch = os.outputof("uname -m")
	end

	-- Check our known 64-bit identifiers
	arch = arch:lower()
	for _, hosttype in ipairs(_64BitHostTypes) do
		if arch:find(hosttype) then
			return true
		end
	end
	return false
end