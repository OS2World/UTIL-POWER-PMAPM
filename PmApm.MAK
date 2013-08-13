# PmApm.MAK
# Created by IBM WorkFrame/2 MakeMake at 16:00:44 on 16 Sept 1998
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker
#  Bind::Resource Bind
#  Compile::Resource Compiler

.SUFFIXES:

.SUFFIXES: \
    .C .RC .obj .res 

.C.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /Q /Wdcleffprorearet /Fi /Si /Ti /Gi /C %s

{D:\C\Programme\PmApm}.C.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /Q /Wdcleffprorearet /Fi /Si /Ti /Gi /C %s

.RC.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s %|fF.RES

{D:\C\Programme\PmApm}.RC.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s %|fF.RES

all: \
    .\PmApm.EXE

.\PmApm.EXE: \
    .\PmApm.obj \
    .\PmApm.res \
    {$(LIB)}PmApm.DEF
    @echo " Link::Linker "
    @echo " Bind::Resource Bind "
    icc.exe @<<
     /Q /B" /de /pmtype:pm /optfunc     "
     /FePmApm.EXE 
     PmApm.DEF
     .\PmApm.obj
<<
    rc.exe .\PmApm.res PmApm.EXE

.\PmApm.obj: \
    D:\C\Programme\PmApm\PmApm.C \
    {D:\C\Programme\PmApm;$(INCLUDE);}PmApm.H

.\PmApm.res: \
    D:\C\Programme\PmApm\PmApm.RC \
    {D:\C\Programme\PmApm;$(INCLUDE)}PmApm.DLG \
    {D:\C\Programme\PmApm;$(INCLUDE)}PmApm.H \
    {D:\C\Programme\PmApm;$(INCLUDE)}PmApm.ICO
