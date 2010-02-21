SET xmap2=%~dp0\..\..\xmap2.exe
SET basepath=%~dp0\..\..
SET backuppath=G:\Code\W Maps

@REM Copy map_date to backup dir to preserve this version.
@REM copy /Y "%1.map" "%backuppath%\%1_%date:~0,4%-%date:~5,2%-%date:~8,2%.map"

SET starttime=%time%

"%xmap2%" -fs_basepath "%basepath%" -fs_game "weaver" -game "xreal" -meta -v "%1.map"
"%xmap2%" -fs_basepath "%basepath%" -fs_game "weaver" -game "xreal" -vis "%1.bsp"
"%xmap2%" -fs_basepath "%basepath%" -fs_game "weaver" -game "xreal" -light "%1.map"

echo %starttime%
echo %time%

pause