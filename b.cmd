@echo off
echo agressive build
cl /Ob1gity /Wall /Wp64 /GL inputune.c /link /nodefaultlib:libc libctiny.lib /subsystem:WINDOWS /entry:WinMainCRTStartup /release /opt:NOWIN98 /opt:REF /opt:ICF=4 /WS:AGGRESSIVE
del *.obj