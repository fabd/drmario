# Microsoft Developer Studio Project File - Name="DxTest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=DxTest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Drmario.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Drmario.mak" CFG="DxTest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DxTest - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "DxTest - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DxTest - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Test"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ddraw.lib dsound.lib dxguid.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "DxTest - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FA /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ddraw.lib dsound.lib dxguid.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "DxTest - Win32 Release"
# Name "DxTest - Win32 Debug"
# Begin Group "crap"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\drmario.rc
# End Source File
# Begin Source File

SOURCE=.\gellule.bmp
# End Source File
# Begin Source File

SOURCE=.\prgicon.ico
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\dx6sdk\lib\ddraw.lib
# End Source File
# Begin Source File

SOURCE=..\..\dx6sdk\lib\dxguid.lib
# End Source File
# End Group
# Begin Source File

SOURCE=.\CArgv.cpp
# End Source File
# Begin Source File

SOURCE=.\CArgv.h
# End Source File
# Begin Source File

SOURCE=.\CAudio.cpp
# End Source File
# Begin Source File

SOURCE=.\CAudio.h
# End Source File
# Begin Source File

SOURCE=.\CLogFile.cpp
# End Source File
# Begin Source File

SOURCE=.\CLogFile.h
# End Source File
# Begin Source File

SOURCE=.\CVideo.cpp
# End Source File
# Begin Source File

SOURCE=.\CVideo.h
# End Source File
# Begin Source File

SOURCE=.\CWad.cpp
# End Source File
# Begin Source File

SOURCE=.\CWad.h
# End Source File
# Begin Source File

SOURCE=.\dlgpaddle.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgpaddle.h
# End Source File
# Begin Source File

SOURCE=.\drm_game.cpp
# End Source File
# Begin Source File

SOURCE=.\drm_game.h
# End Source File
# Begin Source File

SOURCE=.\Drmario.cpp
# End Source File
# Begin Source File

SOURCE=.\drmario.h
# End Source File
# Begin Source File

SOURCE=.\errors.h
# End Source File
# Begin Source File

SOURCE=.\fabgrafx.cpp
# End Source File
# Begin Source File

SOURCE=.\fabgrafx.h
# End Source File
# Begin Source File

SOURCE=.\fabtypes.h
# End Source File
# Begin Source File

SOURCE=.\paddle.bmp
# End Source File
# Begin Source File

SOURCE=.\Strings.cpp
# End Source File
# Begin Source File

SOURCE=.\Strings.h
# End Source File
# Begin Source File

SOURCE=.\win_dbg.cpp
# End Source File
# Begin Source File

SOURCE=.\win_dbg.h
# End Source File
# End Target
# End Project
