
____  ___                     .____     
\   \/  /______   ____ _____  |    |    
 \     /\_  __ \_/ __ \\__  \ |    |    
 /     \ |  | \/\  ___/ / __ \|    |___ 
/___/\  \|__|    \___  >____  /_______ \
      \_/            \/     \/        \/

_________________________________________


XreaL Readme - http://sourceforge.net/projects/xreal

Thank you for downloading XreaL.



_______________________________________

CONTENTS
_______________________________



This file contains the following sections:

	1) SYSTEM REQUIREMENTS

	2) LICENSE

	3) GENERAL NOTES
	
	4) GETTING THE SOURCE CODE AND MEDIA

	5) COMPILING ON WIN32 WITH VISUAL C++ 2008 EXPRESS EDITION

	6) COMPILING ON GNU/LINUX

	7) USING HTTP/FTP DOWNLOAD SUPPORT (SERVER)
	
	8) USING HTTP/FTP DOWNLOAD SUPPORT (CLIENT)
	
	9) MULTIUSER SUPPORT ON WINDOWS SYSTEMS



___________________________________

1) SYSTEM REQUIREMENTS
__________________________



Minimum system requirements:

	CPU: 2 GHz Intel compatible
	System Memory: 512MB
	Graphics card: GeForce 6600 GT or any other Shader Model 3.0 compatible GFX card 

Recommended system requirements:

	CPU: 3 GHz + Intel compatible
	System Memory: 1024MB+
	Graphics card: Geforce 8800 GT, ATI HD 4850 or higher. 




_______________________________

2) LICENSE
______________________

See COPYING.txt for all the legal stuff.



_______________________________

3) GENERAL NOTES
______________________

A short summary of the file layout:

XreaL/base/					XreaL media directory ( models, textures, sounds, maps, etc. )
XreaL/base/code				XreaL game code ( game, cgame, and ui )
XreaL/blender/				Blender plugins for ase, md3, and md5 models
XreaL/code/					XreaL source code ( renderer, game code, OS layer, etc. )
XreaL/code/common			framework source code for command line tools like xmap
XreaL/code/xmap				map compiler ( .map -> .bsp ) (based on q3map)
XreaL/code/xmap2			map compiler ( .map -> .bsp ) (based on q3map2)
XreaL/code/xmass			master server
XreaL/code/xrealradiant		XreaLRadiant level editor source code
XreaL/xrealradiant/			XreaLRadiant level editor work dir with configuration files



____________________________________________

4) GETTING THE SOURCE CODE AND MEDIA
___________________________________

This project's SourceForge.net Subversion repository can be checked out through SVN with the following instruction set: 

svn co https://svn.sourceforge.net/svnroot/xreal/trunk/xreal XreaL



___________________________________________________________________

5) COMPILING ON WIN32 WITH VISUAL C++ 2008 EXPRESS EDITION
__________________________________________________________

1. Download and install the Visual C++ 2008 Express Edition.
2. Download libSDL from http://libsdl.org/release/SDL-devel-1.2.13-VC8.zip
	and extract it to C:\libSDL-1.2.13.
3. Download and install the OpenAL SDK from http://www.openal.org.
4. Download libcURL from http://curl.hoxt.com/download/libcurl-7.15.5-win32-msvc.zip
	and extract it to C:\libcURL
5. Download and install Gtk+ 2.10.11 development environment from http://gladewin32.sourceforge.net/.
6. Download http://oss.sgi.com/projects/ogl-sample/ABI/glext.h and copy it
	to C:\Program Files\Microsoft SDKs\Windows\v6.0A\Include\gl.

8. Add necessary include Directories in VC9 under Tools -> Options... -> Project and Solutions -> VC++ Directories:
	example:
	C:\libSDL-1.2.13\include
	C:\Program Files\OpenAL 1.1 SDK\include
	C:\libcURL\include

9. Add necessary lib Directories in VC9 under Tools -> Options... -> Project and Solutions -> VC++ Directories:
	example:
	C:\libSDL-1.2.13\lib
	C:\Program Files\OpenAL 1.1 SDK\lib\Win32
	C:\libcURL

10. Use the VC9 solutions to compile what you need:
	XreaL/code/xreal.sln
	XreaL/code/xrealradiant/XreaLRadiant.sln
	XreaL/code/xmap2/xmap2.sln


__________________________________

6) COMPILING ON GNU/LINUX
_________________________

You need the following dependencies in order to compile XreaL with all features:

 * SDL >= 1.2
 * FreeType >= 2.3.5
 * OpenAL >= 0.0.8 (if compiled with scons openal=1)
 * libcURL >= 7.15.5 (if compiled with scons curl=compile)
 * libxml2 >= 2.0.0 (if compiled with scons radiant=1 or scons xmap=1)
 * zlib >= 1.2.0 (if compiled with scons radiant=1 or scons xmap=1)
 * GTK+ >= 2.4.0 (if compiled with scons radiant=1 or scons xmap=1, requires glib, atk, pango, iconv, etc)
 * gtkglext >= 1.0.0 (if compiled with scons radiant=1)
 * gtksourceview >= 2.0.0 (if compiled with scons radiant=1)
 * GLEW >= 1.5.0 (if compiled with scons radiant=1)
 * boost >= 1.3.4 (if compiled with scons radiant=1)
 * vorbis >= 1.2.0 (if compiled with scons radiant=1)
 

Compile XreaL for x86 processors:
	>scons arch=linux-i386

Compile XreaL for x86_64 (AMD64) processors:
	>scons arch=linux-x86_64

Type scons -h for more compile options.



__________________________________________________________

USING HTTP/FTP DOWNLOAD SUPPORT (SERVER)
______________________________________________

You can enable redirected downloads on your server by using the 'sets'
command to put the sv_dlURL cvar into your SERVERINFO string and
ensure sv_allowDownloads is set to 1.
 
sv_dlURL is the base of the URL that contains your custom .pk3 files
the client will append both fs_game and the filename to the end of
this value.  For example, if you have sv_dlURL set to
"http://xreal.sourceforge.net/", fs_game is "base", and the client is
missing "test.pk3", it will attempt to download from the URL
"http://xreal.sourceforge.net/base/test.pk3"

sv_allowDownload's value is now a bitmask made up of the following
flags:
    1 - ENABLE
    2 - do not use HTTP/FTP downloads
    4 - do not use UDP downloads
    8 - do not ask the client to disconnect when using HTTP/FTP

Server operators who are concerned about potential "leeching" from their
HTTP servers from other XreaL servers can make use of the HTTP_REFERER
that XreaL sets which is "XreaL://{SERVER_IP}:{SERVER_PORT}".  For,
example, Apache's mod_rewrite can restrict access based on HTTP_REFERER.


________________________________________________________

USING HTTP/FTP DOWNLOAD SUPPORT (CLIENT)
_____________________________________________

Simply setting cl_allowDownload to 1 will enable HTTP/FTP downloads on 
the clients side assuming XreaL was compiled with USE_CURL=1.
Like sv_allowDownload, cl_allowDownload also uses a bitmask value
supporting the following flags:
    1 - ENABLE
    2 - do not use HTTP/FTP downloads
    4 - do not use UDP downloads



________________________________________________________

MULTIUSER SUPPORT ON WINDOWS SYSTEMS
___________________________________________

On Windows, all user specific files such as autogenerated configuration,
demos, videos, screenshots, and autodownloaded pk3s are now saved in a
directory specific to the user who is running XreaL.

On NT-based systems such as Windows XP, this is usually a directory named:
  "C:\Documents and Settings\%USERNAME%\Application Data\XreaL\"

On Windows Vista, this would be:
  "C:\Users\%USERNAME%\Application Data\XreaL\"

Windows 95, Windows 98, and Windows ME will use a directory like:
  "C:\Windows\Application Data\XreaL"
in single-user mode, or:
  "C:\Windows\Profiles\%USERNAME%\Application Data\XreaL"
if multiple logins have been enabled.

You can revert to the old single-user behaviour by setting the fs_homepath
cvar to the directory where XreaL is installed.  For example:
  xreal.exe +set fs_homepath "c:\xreal"
Note that this cvar MUST be set as a command line parameter.

