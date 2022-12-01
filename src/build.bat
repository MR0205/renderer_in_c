@echo off
if not defined DevEnvDir (
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat" > nul
)

set timestamp=%date:~0,2%_%date:~3,3%_%date:~7,2%_%time:~0,2%_%time:~3,2%_%time:~6,2%

pushd build
move engine.dll engine_to_delete_%timestamp%.dll >nul 2>nul
move engine.pdb engine_to_delete_%timestamp%.pdb >nul 2>nul
del *.dll *.pdb >nul 2>nul
echo >still_writing 
cl /nologo /Zi /LD /Feengine.dll ../src/engine.cpp  ../src/opentype.cpp ../src/bmp_parser.cpp ../src/platform.cpp
del still_writing
cl /nologo /Zi /Fec_proj.exe ../src/win32_main.cpp User32.lib Gdi32.lib Winmm.lib 
popd
