SET xmap2=%~dp0\..\..\bin\win32\xmap2.exe
SET basepath=%~dp0\..\..

SET starttime=%time%

"%xmap2%" -fs_basepath "%basepath%" -fs_game "base" -game "xreal" -light -v -fast "%1.map"

echo %starttime%
echo %time%

pause