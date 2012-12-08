##############################################################################
##
##  Makefile for Detours.
##
##  Microsoft Research Detours Package.  Version 1.5 (Build 46)
##
##  Copyright 1995-2001, Microsoft Corporation
##

all:
	cd "$(MAKEDIR)\src"
	@$(MAKE) /NOLOGO /$(MAKEFLAGS)
	cd "$(MAKEDIR)\samples"
	@$(MAKE) /NOLOGO /$(MAKEFLAGS)
	cd "$(MAKEDIR)"

clean:
	cd "$(MAKEDIR)\src"
	@$(MAKE) /NOLOGO /$(MAKEFLAGS) clean
	cd "$(MAKEDIR)\samples"
	@$(MAKE) /NOLOGO /$(MAKEFLAGS) clean
	cd "$(MAKEDIR)"

realclean: clean
	-rmdir /q /s include 2> nul
	-rmdir /q /s lib 2> nul

################################################################# End of File.
