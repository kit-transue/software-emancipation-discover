# $Id: environ.mk 1.14 1999/01/21 15:37:26EST sschmidt Exp  $

##########
#
### SET HP Notes
#
#	1. 07/22/97 - The only way to produce an executable for HP which does
#	   NOT give the galaxy "error 1:/Arg/ ..." error is to build on helios.
#	   You must do the following:
#		A. log onto helios. This makefile will not work on any other system.
#		B. Make sure that /usr/local/CC is NOT in your path.
#	   If you see a line where the c compiler 'cc' is invoked, you still have
#	   the wrong compiler in your path and you will get the error on exit.
#	   Good luck.
#
#
# SET Modifications   02/25/97 tlw
#	1. CCXX should come from $CC_ROOTDIR/bin/CC.
#
# SET Modifications   10/23/96 tlw
#	1. hp700 cannot handle .cxx files so we switched to .C
#	2. We force CCXX to be set based on $Arch when the $CCXX
#          environment variable is not set.
#       3. The "-Aa" option was removed from the HP CPPFLAGS.
#
##########
#
#
# Environ file for Unix, make-based builds
#
# The Options in this file can be overridden by setenv'ing them into
# your environment or adding them to the command line.  For example:
#
# % make VBUILDTYPE=debug
#
# It should not be necesary to edit anything in this file.
# For some of the options, uncommenting or setting them in this file
# won't work.  They must be set on the command line or in the environment.
#
# This file assumes you have GALAXYHOME set in your environment.
# It should be set to point to the directory where Galaxy resides.
# For example:
#
# % setenv GALAXYHOME /usr/local/package/galaxy

############################ Options ###################################

#VBUILDTYPE = debug	# galaxy and compiler debugging
#VBUILDTYPE = dynamic	# no debugging, optimized compile, dynamically linked

#CC = acc	# Sun's ANSI C SparcCompiler
#CC = c89	# DEC's C 89 compiler
#CC = cc	# any ANSI C compiler (not K&R)
#CC = clcc	# CenterLine's C compiler
#CC = gcc	# GNU C compiler
#CC = lcc	# Lucid's C/C++ compiler

# CCXX - Compile C++ file
#
#CCXX = CC	# cfront based or other C++ compiler
#CCXX = xlC	# AIX C++ compiler
#CCXX = g++	# GNU C++ compiler
#CCXX = cxx	# DEC C++ compiler

# POSTPROCESS can be used to process object code and libraries before linking
#
#POSTPROCESS = purify -best-effort	# for Pure's Purify
#POSTPROCESS = purelink			# for Pure's PureLink without Purify
#POSTPROCESS = proof			# for CenterLine's TestCenter

# CFLAGS_HOOK are any extra flags for a particular compiler
#
#CFLAGS_HOOK = -sb		# tell SparcCompilers to talk to SourceBrowser
#CFLAGS_HOOK = -Xez		# tell lcc to talk to Energize 
#CFLAGS_HOOK = -Wall -pedantic	# tell gcc to generate more warnings


############################# Definitions ###############################

SHELL=/bin/sh

INIT_VARS=\
LIBXEXT=-lXext;\
if [ "${GALAXYHOME}" = "" ];then echo "GALAXYHOME not set";exit;fi;\
if [ "${VBIKHOME}" = "" -a "${NEEDVBIK}" != "" ];then echo "VBIKHOME not set";exit;fi;\
if [ "${VDBHOME}" = "" -a "${NEEDVDB}" != "" ];then echo "VDBHOME not set";exit;fi;\
if [ "${DREAMHOME}" = "" -a "${NEEDDREAM}" != "" ];then echo "DREAMHOME not set";exit;fi;\
if [ "$$VBUILDTYPE" != "debug" -a "$$VBUILDTYPE" != "dynamic" -a "$$VBUILDTYPE" != "static" -a "$$VBUILDTYPE" != "" ];then echo "VBUILDTYPE, if set, must be debug or dynamic";exit;fi;\
if [ "$$CC" = "" ];then \
    if [ -r /usr/bin/dxterm ];then CC=c89;\
    else if [ "x`/bin/uname`" = "xSunOS" -a ! -r /kernel/unix ];then CC=gcc;\
    else CC=cc;\
    fi;fi;\
fi;\
if [ "$$CCXX" = "" ];then \
    if [ "$$Arch" = "sun5"  ];then \
	CCXX=$$CC_ROOTDIR/bin/CC; \
	CLDCTRL="-R /usr/openwin/lib:/usr/lib"; \
	LD_STATIC=-Bstatic; \
	LD_DYNAMIC=-Bdynamic; \
	LDL=-ldl; \
	LC=-lc; \
    else if [ "$$Arch" = "hp700" ];then \
	CCXX=/net/oxen/vol01/share/hp700/opt/galaCC/bin/CC; \
	CC=/net/oxen/vol01/share/hp700/opt/galaCC/bin/CC; \
	LDL=-ldld; \
	LC=; \
	LD_STATIC=-Wl,-a,archive; \
	LD_DYNAMIC=-Wl,-a,shared; \
    else if [ "$$Arch" = "irix6" ];then \
	CCXX=$$CC_ROOTDIR/bin/CC; \
	CC=$$CC_ROOTDIR/bin/CC; \
	LDL=; \
        LC=; \
        LD_STATIC=; \
        LD_DYNAMIC=; \
    else CCXX=$$CC_ROOTDIR/CC; \
	 LDL=-ldl; \
	 LC=-lc; \
         LD_STATIC=-Bstatic; \
	 LD_DYNAMIC=-Bdynamic; \
    fi;fi;fi;\
fi;\
CPP="$$CC -E";\
CXXPP="$$CCXX -E";\
GALLIBDIR=$$GALAXYHOME/lib;\
if [ "${OPENWINHOME}" = "" -a -r /usr/openwin/include/X11/Xlib.h -a ! -r /usr/lib/libX11.a ];then OPENWINHOME=/usr/openwin;fi;\
if [ "$$OPENWINHOME" != "" ];then OPENWINIFLAGS=-I$$OPENWINHOME/include;\
OPENWINLDFLAGS=-L$$OPENWINHOME/lib;fi;\
if [ "$$VBUILDTYPE" = "debug" ];then VDEBUG=1;OBF=-g;GALEXT=-debug;\
else VDEBUG=0;OBF=-O;\
if [ "$$VBUILDTYPE" = "static" ];then GALEXT=-static;fi;\
fi;\
if [ \( -r /usr/lib/libsocket.a -o -r /usr/lib/libsocket.so \) -a \( -r /usr/lib/libnsl.a -o -r /usr/lib/libnsl.so \) ];then SVR4LIBS="-lsocket -lnsl";fi;\
if [ -r /hp-ux -o -r /HI-UX ];then OSCPPFLAGS="-I/usr/include/X11R5 -D_HPUX_SOURCE";OSCFLAGS="-Wp,-B16000 -Wl,-a,archive";OSLDFLAGS=-L/usr/lib/X11R5;LIBXEXT="-lXext";fi;\
if [ -r /HI-UX ];then OSCPPFLAGS="-I/usr/include/X11R5";fi;\
if [ -r $$OPENWINHOME/lib/libmle.a -o -r /usr/lib/libmle.a ];then INTLLIBS=-lmle;fi;\
if [ -r $$GALLIBDIR/libvgalaxy$$VCXXEXT-unicode$$GALEXT.a ];then VCHARDEF="-DvportVCHAR_CODESET=vportCODESET_UNICODE";VCHAREXT=-unicode;fi;\
if [ -r $$GALLIBDIR/libvsprview$$VCHAREXT.a ];then VSPRVIEWLIB=-lvsprview$$VCHAREXT;fi;\
if [ -r $$VBIKHOME/include/vre.h ];then VBIKHOMEINCLUDE=-I$$VBIKHOME/include;fi;\
if [ -r $$VDBHOME/include/vdb.h -a -r $$VDBHOME/lib/libvdb.a ];then VDBHOMEINCLUDE=-I$$VDBHOME/include;VDBHOMELIB=-L$$VDBHOME/lib;\
if [ "$$VBUILDTYPE" = "debug" ];then LIBVDB=-lvdb-debug;else LIBVDB=-lvdb;fi;\
fi;\
if [ "$$DREAMHOME" != "" ];then DREAMHOMEINCLUDE=-I$$DREAMHOME/include;DREAMHOMELIB=-L$$DREAMHOME/lib;fi;\
if [ -r $$GALLIBDIR/libvre$$VCHAREXT.a ];then VRELIB=-lvre$$VCHAREXT;fi;\
if [ "$$CCXX" = "xlC" -a "$$VCXXEXT" = "++" ];then FORCECXXFLAGS=-+;fi;\
if [ "$$CCXX" = "ncc" -a "$$VCXXEXT" = "++" ];then FORCE_CXX_LINK=-XF;fi;\
if [ "$$M_ROOT" != "" ];then M_ROOTARCH=-L$$M_ROOT/arch;fi;\
CPPFLAGS="$$CLDCTRL -I${GALAXYHOME}/include $$VBIKHOMEINCLUDE $$VDBHOMEINCLUDE $$DREAMHOMEINCLUDE $$OPENWINIFLAGS $$OSCPPFLAGS -DvdebugDEBUG=$$VDEBUG $$VCHARDEF";\
CFLAGS="$$OBF $$OSCFLAGS ${CFLAGS_HOOK} $$FORCECXXFLAGS";\
LDFLAGS="${LDFLAGS_HOOK} -L$$GALLIBDIR $$OPENWINLDFLAGS $$VDBHOMELIB $$DREAMHOMELIB $$OSLDFLAGS $$FORCE_CXX_LINK $$M_ROOTARCH";\
LDLIBS="${LDLIBS_HOOK} $$VRELIB $$VSPRVIEWLIB $$LIBVDB \
	$$LD_STATIC -lvgalaxy$$VCXXEXT$$VCHAREXT$$GALEXT $$LIBXEXT -lX11 \
	$$LD_DYNAMIC $$INTLLIBS $$SVR4LIBS -lm $$LDL $$LC $$LD_STATIC"

O=.o

E=

COMPILE_AND_LINK=@${INIT_VARS} ; \
echo $$POSTPROCESS $$CC -o $@ $$CPPFLAGS $$CFLAGS $$LDFLAGS $? $$LDLIBS ; \
$$POSTPROCESS $$CC $$CPPFLAGS $$CFLAGS $$LDFLAGS -o $(NOTDEFD)

COMPILE_CXX_AND_LINK=@VCXXEXT=++ ; ${INIT_VARS} ; \
echo $$POSTPROCESS $$CCXX -o $@ $$CPPFLAGS $$CFLAGS $$LDFLAGS $? $$LDLIBS ; \
$$POSTPROCESS $$CCXX $$CPPFLAGS $$CFLAGS $$LDFLAGS -o $(NOTDEFD)

LDLIBS=$$LDLIBS


#################################### Rules ################################

.SUFFIXES:
.SUFFIXES: .o .i .c .C .cxx .cc

.c.i :
	@${INIT_VARS} ; echo $$CPP -o $@ $$CPPFLAGS $< ; $$CPP -o $@ $$CPPFLAGS $<

.C.i :
	@VCXXEXT=++ ; ${INIT_VARS} ; echo $$CXXPP -o $@ $$CPPFLAGS $< ; $$CXXPP -o $@ $$CPPFLAGS $<

.cxx.i :
	@VCXXEXT=++ ; ${INIT_VARS} ; echo $$CXXPP -o $@ $$CPPFLAGS $< ; $$CXXPP -o $@ $$CPPFLAGS $<

.cc.i :
	@VCXXEXT=++ ; ${INIT_VARS} ; echo $$CXXPP -o $@ $$CPPFLAGS $< ; $$CXXPP -o $@ $$CPPFLAGS $<

.c.o :
	@${INIT_VARS} ; echo $$CC $$CPPFLAGS $$CFLAGS -c $< ; $$CC $$CPPFLAGS $$CFLAGS -c $<

.C.o :
	@VCXXEXT=++ ; ${INIT_VARS} ; echo $$CCXX $$CPPFLAGS $$CFLAGS -c $< ; $$CCXX $$CPPFLAGS $$CFLAGS -c $<

.cxx.o :
	@VCXXEXT=++ ; ${INIT_VARS} ; echo $$CCXX $$CPPFLAGS $$CFLAGS -c $< ; $$CCXX $$CPPFLAGS $$CFLAGS -c $<

.cc.o :
	@VCXXEXT=++ ; ${INIT_VARS} ; echo $$CCXX $$CPPFLAGS $$CFLAGS -c $< ; $$CCXX $$CPPFLAGS $$CFLAGS -c $<

.c :
	@${INIT_VARS} ; echo $$POSTPROCESS $$CC -o $@ $$CPPFLAGS $$CFLAGS $$LDFLAGS $< $$LDLIBS ; $$POSTPROCESS $$CC -o $@ $$CPPFLAGS $$CFLAGS $$LDFLAGS $< $$LDLIBS

.C :
	@VCXXEXT=++ ; ${INIT_VARS} ; echo $$POSTPROCESS $$CCXX -o $@ $$CPPFLAGS $$CFLAGS $$LDFLAGS $< $$LDLIBS ; $$POSTPROCESS $$CCXX -o $@ $$CPPFLAGS $$CFLAGS $$LDFLAGS $< $$LDLIBS

.cxx :
	@VCXXEXT=++ ; ${INIT_VARS} ; echo $$POSTPROCESS $$CCXX -o $@ $$CPPFLAGS $$CFLAGS $$LDFLAGS $< $$LDLIBS ; $$POSTPROCESS $$CCXX -o $@ $$CPPFLAGS $$CFLAGS $$LDFLAGS $< $$LDLIBS

.cc :
	@VCXXEXT=++ ; ${INIT_VARS} ; echo $$POSTPROCESS $$CCXX -o $@ $$CPPFLAGS $$CFLAGS $$LDFLAGS $< $$LDLIBS ; $$POSTPROCESS $$CCXX -o $@ $$CPPFLAGS $$CFLAGS $$LDFLAGS $< $$LDLIBS

all : ${ALL_TARGETS}
	-@

clean :
	-\rm -f core *.o *.i ${ALL_TARGETS} 

