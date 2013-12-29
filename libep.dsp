# Microsoft Developer Studio Project File - Name="libep" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libep - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libep.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libep.mak" CFG="libep - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libep - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libep - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libep - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libep___Win32_Release"
# PROP BASE Intermediate_Dir "libep___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "libep___Win32_Release"
# PROP Intermediate_Dir "libep___Win32_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "CHI" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libep - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libep___Win32_Debug"
# PROP BASE Intermediate_Dir "libep___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "libep___Win32_Debug"
# PROP Intermediate_Dir "libep___Win32_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ  /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "CHI" /YX /FD /GZ  /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libep - Win32 Release"
# Name "libep - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=".\b-svm.cpp"
# End Source File
# Begin Source File

SOURCE=.\BagGener.cpp
# End Source File
# Begin Source File

SOURCE=.\Bagging.cpp
# End Source File
# Begin Source File

SOURCE=.\bpnn.cpp
# End Source File
# Begin Source File

SOURCE=.\C45.cpp
# End Source File
# Begin Source File

SOURCE=.\cluster.cpp
# End Source File
# Begin Source File

SOURCE=.\CPM.cpp
# End Source File
# Begin Source File

SOURCE=.\DataFile.cpp
# End Source File
# Begin Source File

SOURCE=.\EnsPruner.cpp
# End Source File
# Begin Source File

SOURCE=.\FS.cpp
# End Source File
# Begin Source File

SOURCE=.\Gasen.cpp
# End Source File
# Begin Source File

SOURCE=.\MDSQ.cpp
# End Source File
# Begin Source File

SOURCE=.\NaiveBayes.cpp
# End Source File
# Begin Source File

SOURCE=.\OrientOrder.cpp
# End Source File
# Begin Source File

SOURCE=.\result.cpp
# End Source File
# Begin Source File

SOURCE=.\SelBest.cpp
# End Source File
# Begin Source File

SOURCE=.\Statistic.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=".\b-svm.h"
# End Source File
# Begin Source File

SOURCE=.\BagGener.h
# End Source File
# Begin Source File

SOURCE=.\Bagging.h
# End Source File
# Begin Source File

SOURCE=.\bpnn.h
# End Source File
# Begin Source File

SOURCE=.\C45.h
# End Source File
# Begin Source File

SOURCE=.\Classifier.h
# End Source File
# Begin Source File

SOURCE=.\Cluster.h
# End Source File
# Begin Source File

SOURCE=.\CPM.h
# End Source File
# Begin Source File

SOURCE=.\DataFile.h
# End Source File
# Begin Source File

SOURCE=.\EnsPruner.h
# End Source File
# Begin Source File

SOURCE=.\FS.h
# End Source File
# Begin Source File

SOURCE=.\Gasen.h
# End Source File
# Begin Source File

SOURCE=.\MDSQ.h
# End Source File
# Begin Source File

SOURCE=.\NaiveBayes.h
# End Source File
# Begin Source File

SOURCE=.\OrientOrder.h
# End Source File
# Begin Source File

SOURCE=.\result.h
# End Source File
# Begin Source File

SOURCE=.\SelBest.h
# End Source File
# Begin Source File

SOURCE=.\Statistic.h
# End Source File
# End Group
# End Target
# End Project
