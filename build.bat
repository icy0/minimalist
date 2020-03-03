@echo off

set CODE_BASE_PATH=%CD%
set BUILD_PATH=%CODE_BASE_PATH%\Build
set INTERMEDIATES_PATH=%BUILD_PATH%\Intermediates
set SOURCE_PATH=%CODE_BASE_PATH%\Source

set INCLUDE_DIRECTORIES=/I %SOURCE_PATH% /I %CD%\Dependencies\FreeType2_2.10.1\include
set COMPILER_OPTIONS=/Fe%BUILD_PATH%\texteditor /Fd%INTERMEDIATES_PATH%\ /Fo%INTERMEDIATES_PATH%\ /nologo /MT %INCLUDE_DIRECTORIES% /ZI /EHsc
set INCLUDED_LIBRARIES=User32.lib Gdi32.lib Shell32.lib msvcrt.lib %CD%\Dependencies\FreeType2_2.10.1\build\Debug\freetyped.lib
set LINKER_OPTIONS=/link /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:msvcrtd.lib /DEBUG

IF NOT DEFINED VC_COMPILER_INITIALIZED (
	set /A VC_COMPILER_INITIALIZED=1
	call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
	echo.
)

IF NOT EXIST %BUILD_PATH% (
	mkdir %BUILD_PATH%
)

IF NOT EXIST %INTERMEDIATES_PATH% (
	mkdir %INTERMEDIATES_PATH%
)

cl %COMPILER_OPTIONS% %SOURCE_PATH%\*.cpp %INCLUDED_LIBRARIES% %LINKER_OPTIONS%