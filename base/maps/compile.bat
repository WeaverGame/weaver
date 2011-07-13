SET xmap2=%~dp0\..\..\xmap2.exe
SET basepath=%~dp0\..\..
SET backuppath=G:\Code\W Maps

@REM Copy map_date to backup dir to preserve this version.
@REM copy /Y "%1.map" "%backuppath%\%1_%date:~0,4%-%date:~5,2%-%date:~8,2%.map"

SET starttime=%time%

"%xmap2%" -fs_basepath "%basepath%" -fs_game "base" -game "xreal" -meta -v -leaktest "%1.map"
"%xmap2%" -fs_basepath "%basepath%" -fs_game "base" -game "xreal" -vis -v "%1.bsp"
@REM "%xmap2%" -fs_basepath "%basepath%" -fs_game "base" -game "xreal" -light -v -fast -samples 2 -lightmapsize 1024 -lomem "%1.map"
"%xmap2%" -fs_basepath "%basepath%" -fs_game "base" -game "xreal" -light -v -fast -samples 2 -scale 1.6 -pointscale 1.8 -areascale 1.6 -gamma 1.35 -bounce 8 -lightmapsize 1024 -compensate 4 -dirty -dirtscale 0.4 -dirtdepth 64 "%1.map"

echo %starttime%
echo %time%

pause