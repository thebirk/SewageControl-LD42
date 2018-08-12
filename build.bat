@echo off
REM cl /nologo /Fegame.exe SDL2.lib  main.c SDL2main.lib  SDL2_mixer.lib SDL2_image.lib /ISDL2-2.0.8\include /link /LIBPATH:SDL2-2.0.8\lib\x64 /SUBSYSTEM:console

REM No console
cl /nologo /Fegame.exe SDL2.lib  main.c SDL2main.lib  SDL2_mixer.lib SDL2_image.lib /ISDL2-2.0.8\include /link /LIBPATH:SDL2-2.0.8\lib\x64 /SUBSYSTEM:windows