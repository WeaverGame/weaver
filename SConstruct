import os, string, sys
import SCons
import SCons.Errors


#
# set configuration options
#
opts = Variables('xreal.conf')
opts.Add(EnumVariable('arch', 'Choose architecture to build for', 'linux-i386', allowed_values=('freebsd-i386', 'linux-i386', 'linux-x86_64', 'netbsd-i386', 'win32-mingw', 'darwin-ppc', 'darwin-i386')))
opts.Add(EnumVariable('warnings', 'Choose warnings level', '1', allowed_values=('0', '1', '2')))
opts.Add(EnumVariable('debug', 'Set to >= 1 to build for debug', '0', allowed_values=('0', '1', '2', '3')))
opts.Add(EnumVariable('optimize', 'Set to >= 1 to build with general optimizations', '2', allowed_values=('0', '1', '2', '3', '4', '5', '6')))
opts.Add(EnumVariable('simd', 'Choose special CPU register optimizations', 'none', allowed_values=('none', 'sse', '3dnow')))
#opts.Add(EnumVariable('cpu', 'Set to 1 to build with special CPU register optimizations', 'i386', allowed_values=('i386', 'athlon-xp', 'core2duo')))
opts.Add(BoolVariable('smp', 'Set to 1 to compile engine with symmetric multiprocessor support', 0))
#opts.Add(BoolVariable('purevm', 'Set to 1 to compile the engine with strict checking for vm/*.qvm modules in paks', 0))
opts.Add(BoolVariable('xmap', 'Set to 1 to compile the XMap(2) map compilers', 0))
#opts.Add(BoolVariable('vectorize', 'Set to 1 to compile the engine with auto-vectorization support', 0))
opts.Add(EnumVariable('curl', 'Choose http-download redirection support for the engine', 'compile', allowed_values=('none', 'compile', 'dlopen')))
#opts.Add(BoolVariable('openal', 'Set to 1 to compile the engine with OpenAL support', 0))
opts.Add(BoolVariable('dedicated', 'Set to 1 to only compile the dedicated server', 0))
opts.Add(BoolVariable('master', 'Set to 1 to compile the master server', 0))

#
# initialize compiler environment base
#
env = Environment(ENV = {'PATH' : os.environ['PATH']}, options = opts, tools = ['default'])
#env = Environment(ENV = {'PATH' : os.environ['PATH']}, options = opts, tools = ['mingw'])

Help(opts.GenerateHelpText(env))

#
# set common compiler flags
#
print 'compiling for architecture ', env['arch']

#if env['arch'] == 'win32-mingw':
#	env.Tool('mingw')
#elif env['arch'] == 'win32-xmingw':
#	env.Tool('xmingw', ['SCons/Tools'])


# HACK: see http://www.physics.uq.edu.au/people/foster/amd64_porting.html
if env['arch'] == 'linux-x86_64':
	picLibBuilder = Builder(action = Action('$ARCOM'), emitter = '$LIBEMITTER', prefix = '$LIBPREFIX', suffix = '$LIBSUFFIX', src_suffix = '$OBJSUFFIX', src_builder = 'SharedObject')
	env['BUILDERS']['StaticLibrary'] = picLibBuilder
	env['BUILDERS']['Library'] = picLibBuilder


env.Append(CCFLAGS = '-pipe -fsigned-char')

if env['warnings'] == '1':
	env.Append(CCFLAGS = '-Wall -Wno-unused-parameter')
elif env['warnings'] == '2':
	env.Append(CCFLAGS = '-Wall -Werror')

if env['debug'] != '0':
	env.Append(CCFLAGS = '-ggdb${debug} -D_DEBUG -DDEBUG')
else:
	env.Append(CCFLAGS = '-DNDEBUG')

if env['optimize'] != '0':
	env.Append(CCFLAGS = '-O${optimize} -ffast-math') # -fno-strict-aliasing -funroll-loops')

#if env['cpu'] == 'athlon-xp':
#	env.Append(CCFLAGS = '-march=athlon-xp') # -msse -mfpmath=sse')
#elif env['cpu'] == 'core2duo':
#	env.Append(CCFLAGS = '-march=prescott')

#if env['arch'] == 'linux-i386' and env['vectorize'] == 1:
#	env.Append(CCFLAGS = '-ftree-vectorize -ftree-vectorizer-verbose=1')

if env['simd'] == 'sse':
	env.Append(CCFLAGS = '-DSIMD_SSE -msse')
elif env['simd'] == '3dnow':
	env.Append(CCFLAGS = '-DSIMD_3DNOW')

conf = Configure(env)
env = conf.Finish()

#
# save options
#
opts.Save('xreal.conf', env)

#
# compile targets
#
Export('env')

if env['dedicated'] == 1:
	SConscript('SConscript_xrealded', build_dir='build/xrealded', duplicate=0)
	SConscript('SConscript_base_game', build_dir='build/base/game', duplicate=0)

else:
	SConscript('SConscript_xreal', build_dir='build/xreal', duplicate=0)
	SConscript('SConscript_rendererGL', build_dir='build/rendererGL', duplicate=0)
	SConscript('SConscript_base_cgame', build_dir='build/base/cgame', duplicate=0)
	SConscript('SConscript_base_game', build_dir='build/base/game', duplicate=0)
	SConscript('SConscript_base_ui', build_dir='build/base/ui', duplicate=0)

if env['xmap'] == 1:
	SConscript('SConscript_xmap', build_dir='build/xmap', duplicate=0)
	SConscript('SConscript_xmap2', build_dir='build/xmap2', duplicate=0)

if env['master'] == 1:
	SConscript('SConscript_xrealmaster', build_dir='build/xmass', duplicate=0)
