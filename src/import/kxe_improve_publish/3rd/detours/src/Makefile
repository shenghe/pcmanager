##############################################################################
##
##  Makefile for Detours.
##
##  Microsoft Research Detours Package.  Version 1.5 (Build 46)
##
##  Copyright 1995-2001, Microsoft Corporation
##
##############################################################################

ROOT = ..

INCD = $(ROOT)\include
LIBD = $(ROOT)\lib

##############################################################################
##
.SUFFIXES: .cpp .h .obj
.cpp.obj:
	@$(CC) /nologo $(CFLAGS) /Fo$*.obj /c "$(MAKEDIR)\$*.cpp"

##############################################################################
##
CFLAGS=/nologo /DDEBUG /DWIN32_LEAN_AND_MEAN /W4 /WX /Zi /FR "/Fd$(LIBD)\detours.pdb" \
	/MTd /Gy /Gs /Gi- /Gm- /Zl /O -D_WIN32_WINNT=0x403

OBJS = \
	detours.obj	\
	disasm.obj	\
	image.obj	\
	\
	firstexc.obj	\
	creatwth.obj	\

##############################################################################

all: dirs \
	"$(LIBD)\detours.lib"	\
	"$(INCD)\detours.h"		\
	\
	detours.bsc \

##############################################################################

clean:
	@-del *.obj *.pdb *.ilk *.exe *.sbr *.bsc *~ 2>nul

##############################################################################

dirs:
	if not exist "$(INCD)" mkdir "$(INCD)"
	if not exist "$(LIBD)" mkdir "$(LIBD)"

detours.bsc : $(OBJS:.obj=.sbr)
	bscmake /v /n /o $@ $(OBJS:.obj=.sbr)

"$(ROOT)\lib\detours.lib" : $(OBJS)
	lib /debugtype:cv /nologo /out:$@ $(OBJS)

"$(ROOT)\include\detours.h" : detours.h
	copy detours.h $@

detours.obj detours.sbr : detours.cpp detours.h
disasm.obj disasm.sbr : disasm.cpp disasm.h detours.h
image.obj image.sbr : image.cpp detours.h
firstexc.obj firstexc.sbr : firstexc.cpp detours.h
creatwth.obj creatwth.sbr : creatwth.cpp detours.h

################################################################# End of File.
