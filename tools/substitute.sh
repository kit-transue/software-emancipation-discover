#!/bin/sh 
##########################################################################
# Copyright (c) 2015, Synopsys, Inc.                                     #
# All rights reserved.                                                   #
#                                                                        #
# Redistribution and use in source and binary forms, with or without     #
# modification, are permitted provided that the following conditions are #
# met:                                                                   #
#                                                                        #
# 1. Redistributions of source code must retain the above copyright      #
# notice, this list of conditions and the following disclaimer.          #
#                                                                        #
# 2. Redistributions in binary form must reproduce the above copyright   #
# notice, this list of conditions and the following disclaimer in the    #
# documentation and/or other materials provided with the distribution.   #
#                                                                        #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    #
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      #
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  #
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   #
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, #
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       #
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  #
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  #
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    #
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  #
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   #
##########################################################################

#
# synopsis:
#  substitute [ -D<option>=value ]*
#
# Process input 
#  Options are specfied in the calling line as,
#  for example: -DCompiler=cfront
#
#
echo " Options $*"
while [  "`echo $1 | egrep -i  '^-D' | sed 's/-D//'`" ] 
do
	assign="`echo $1 | sed 's/-D//'`"
	eval $assign
	shift 
	vars=$*
	set -- $vars
done

#
# * * * YOU SHOULD NOT HAVE TO CHANGE THIS section * * * 
# UNLESS you add one more platform/compiler dependent flag(s)
#
createMake() {
	OBJDIR=`pwd|sed "s!/paraset/src!/paraset/obj/$ARCH!"`/${OBJDIR}
	LIBDIR=/paraset/lib.new/$ARCH/paraset/$Compiler
	[ ! -d "$LIBDIR" ] && mkdir -p  $LIBDIR
	echo OBJDIR is $OBJDIR
	#rm $OBJDIR/*.[cC]
	ln -s  `pwd`/*.[cC] $OBJDIR  > /dev/null 2>&1
	rm $OBJDIR/astTree_mx.h.C
#
# Rules:
# Template files are located in /paraset/tools/<sun4|sun5|hp700>.cf
#
	echo
	cat /paraset/tools/`ParaArch`.cf  |
    # 961023 kit transue: the following rewrite of gcc include line was
    # removed in sun4 and sun5 build views 960605 and 960410 respectively.
    # A better option for hp might be to change /paraset/tools/hp700.cf if
    # propagating this change to HP causes problems.
	#if [ $Compiler = gcc ] ; then  sed '/GCC_INCLUDES/s+-I\$+-idirafter \$+g'; else tee; fi |
	sed "s+^CCC =.*$+CCC = ${CCC}+" |
	sed "s+^CC =.*$+CC = ${CC}+" |
	sed "s+^cc =.*$+cc = ${cc}+" |
	sed "s+^MX =.*$+MX = ${MX}+" |
	sed "s+^DEBUG =.*$+DEBUG = ${DEBUG}+" |
	sed "s+^AFILE =.*$+AFILE = ${AFILE}+" |
	sed "s+^GFILE =.*$+GFILE = ${GFILE}+" |
	sed "s+^TARGET_ARCH =.*$+TARGET_ARCH = ${TARGET_ARCH}+" |
	sed "s+^LIBDIR =.*$+LIBDIR = ${LIBDIR}+" |
	sed "s+MIXLIBFLAGS.*=.*$+MIXLIBFLAGS = ${MIXLIBFLAGS}+" |
	sed "s+MIXLIBCFLAGS.*=.*$+MIXLIBCFLAGS = ${MIXLIBCFLAGS}+" |
	sed "s+MIXLIBCCFLAGS.*=.*$+MIXLIBCCFLAGS = ${MIXLIBCCFLAGS}+" |
	sed "s+CCDEFINES.*=.*$+CCDEFINES = ${CCDEFINES}+" > /tmp/Makefile.`ParaArch`.$$
	echo  'SOURCES =' *.[cC] | sed 's/astTree_mx.h.C//' >> /tmp/Makefile.`ParaArch`.$$
	echo  'OBJECTS =' *.[cC] | sed 's/astTree_mx.h.C//' |
		sed 's!\.c!\.o!g'    	|
		sed 's!\.C!\.o!g'   	>> /tmp/Makefile.`ParaArch`.$$
	echo "${TARGET} : $(OBJECTS) " >> /tmp/Makefile.`ParaArch`.$$
	echo '	@rm -f $@' >> /tmp/Makefile.`ParaArch`.$$
	echo "	${LINKLINE}" >> /tmp/Makefile.`ParaArch`.$$

#
# Exceptions:
#
	echo '
fCall_mx.o:	$(FORCE) $(BASE)/src/paraset/mixed/src/fCall_mx.c
	@echo Compiling C file fCall_mx.c
	$(TIME) $(CC) -c $(DEBUG) $(ALTCFLAGS) -o $@ $(BASE)/src/paraset/mixed/src/fCall_mx.c 


varargs_mx.o:	$(FORCE) $(BASE)/src/paraset/mixed/src/varargs_mx.c
	@echo Compiling C file varargs_mx.c
	$(TIME) gcc -c -g $(TARGET_ARCH) $(GCC_INCLUDES) -o $@ $(BASE)/src/paraset/mixed/src/varargs_mx.c 

router_mx.o:	$(FORCE) $(BASE)/src/paraset/mixed/src/router_mx.c
	@echo Compiling C file router_mx.c
	$(TIME) $(CC) -g -c $(CFLAGS)  -o $@ $(BASE)/src/paraset/mixed/src/router_mx.c ' >> /tmp/Makefile.`ParaArch`.$$

echo '
exec-mixed_mx.o:	$(FORCE) $(BASE)/src/paraset/mixed/src/exec-mixed_mx.c
	@echo Compiling C file exec_mixed.c
	$(TIME) $(CC) -D$(MX) -c $(CFLAGS) -o $@ $(BASE)/src/paraset/mixed/src/exec-mixed_mx.c'   >> /tmp/Makefile.`ParaArch`.$$


	if [ ! -f $OBJDIR/Makefile.`ParaArch` ] ; then
		       cp /tmp/Makefile.`ParaArch`.$$ $OBJDIR/Makefile.`ParaArch`
	else
		diff  /tmp/Makefile.`ParaArch`.$$  $OBJDIR/Makefile.`ParaArch` > /dev/null 2>&1 
		if [ $? -ne 0 ] ; then
			echo Copying Makefile to Object directory $OBJDIR
			cp /tmp/Makefile.`ParaArch`.$$ $OBJDIR/Makefile.`ParaArch`
		fi
	fi
	(cd $OBJDIR; clearmake -f $OBJDIR/Makefile.`ParaArch` )
}


#
# END SHOULD NOT CHANGE
#

#
# Set platform dependent flags below
#
export cc CC LDFLAGS DEBUGFLAGS 

case  `ParaArch` in 
	sun4)
		AFILE=libmix.so.1.0
		case $Compiler in 
			sparcworks)
				OBJDIR=sparcworks
				CCC=/share/opt-sparc/SWorks3.0.1/CC
				CC=/share/opt-sparc/SWorks3.0.1/acc
				CCDEFINES='-DGCC2 -Dcplusplus_2_1 -Div2_6_compatible -noex'
				TARGET_ARCH='-Dsun4 -DUSE_SPARCWORKS '
				LIBDIR='$(BASE)/lib.new/$(ARCH)/paraset/sparcworks'
				;;
			cfront)
				OBJDIR=cfront
				CCC=/users/objectstore/sun4/bin/CC
				CC=/usr/ucb/cc
				CCDEFINES='-DGCC2 -Dcplusplus_2_1 -Div2_6_compatible -DMX_ATT'
				TARGET_ARCH='-Dsun4'
				LIBDIR='$(BASE)/lib.new/$(ARCH)/paraset/cfront'
				;;
			gcc)
				OBJDIR=gcc
				CCC=gcc
				CC=gcc
				CCDEFINES='-DGCC2 -Dcplusplus_2_1 -Div2_6_compatible'
				TARGET_ARCH='-Dsun4 -DMX_GNU'
				LIBDIR='$(BASE)/lib.new/$(ARCH)/paraset/gcc'
				;;
		esac
		case $System in
		dynamic)
			OBJDIR=${OBJDIR}/dynamic
			MIXLIBFLAGS=-pic
			if [ $Compiler = gcc ] ; then
				MIXLIBFLAGS=-fpic
			fi
			TARGET='$(LIBDIR)/$(AFILE)'
			LINKLINE='	ld -assert pure-text -o $(LIBDIR)/$(AFILE) $(OBJECTS)'
			;;
		static)
			OBJDIR=${OBJDIR}/static
			MIXLIBFLAGS=
			TARGET='$(LIBDIR)/$(RFILE)'
			LINKLINE='	ld -r -o $(LIBDIR)/$(RFILE) $(OBJECTS)'
			;;
		esac
		if [ "`echo $ARCH | egrep 'g$'`" ] ; then
			DEBUG='-g -D_PSET_DEBUG'
		else
			DEBUG='-O'
		fi
		;;
	sun5)
		echo in sun5
		DEBUG0=""
		case $Compiler in
		sparcworks)
			OBJDIR=sparcworks
			CCC=/net/mothra/opt/SUNWspro/bin/CC
			CC=/net/mothra/opt/SUNWspro/bin/cc
			CCDEFINES='-DGCC2 -Dcplusplus_3_0_1 -Div2_6_compatible -DNEWCPP -DSOLARIS -noex'
			TARGET_ARCH='-Dsun5 -DUSG -DSYSV -D__TIMEVAL__ -DUSE_SPARCWORKS -DMX_ATT'
			LIBDIR='$(BASE)/lib.new/$(ARCH)/paraset/sparcworks'

				;;
		cfront)
			OBJDIR=cfront
			CCC=/solar/opt/SUNWspro/bin/CC
			CC=/solar/opt/SUNWspro/bin/cc
			CCDEFINES='-DGCC2 -Dcplusplus_3_0_1 -Div2_6_compatible -DNEWCPP -DSOLARIS -D__cplusplus'
			TARGET_ARCH='-Dsun5 -DUSG -DSYSV -D__TIMEVAL__ -DMX_ATT'
			LIBDIR='$(BASE)/lib.new/$(ARCH)/paraset/cfront'
			DEBUG0=0
				;;
		gcc)
			OBJDIR=gcc
			CCC=gcc
			CC=gcc
			CCDEFINES='-DGCC2 -Dcplusplus_3_0_1 -Div2_6_compatible -DNEWCPP -DSOLARIS -D__cplusplus'
			TARGET_ARCH='-Dsun5 -DUSG -DSYSV -D__TIMEVAL__ -DMX_GNU'
			LIBDIR='$(BASE)/lib.new/$(ARCH)/paraset/gcc'
	    			;;
		esac
		case $System in
		dynamic)
			OBJDIR=${OBJDIR}/dynamic
			TARGET='$(LIBDIR)/$(AFILE)'
			MIXLIBCFLAGS='-K pic'
			MIXLIBCCFLAGS=-pic
			AFILE=libmix.so
			LINKLINE='	$(CC)  -G -o $(LIBDIR)/$(AFILE) $(OBJECTS)'
			if [ $Compiler = gcc ] ; then
				MIXLIBCFLAGS=-fpic
				MIXLIBCCFLAGS=-fpic
				LINKLINE='	$(CC)  -shared -o $(LIBDIR)/$(AFILE) $(OBJECTS)'
			fi
			;;
		static)
			TARGET='$(LIBDIR)/$(RFILE)'
			OBJDIR=${OBJDIR}/static
			MIXLIBCFLAGS=""
			MIXLIBCCFLAGS=""
			AFILE=libmix.so.1.0
			LINKLINE='	ld  -r -o $(LIBDIR)/$(RFILE) $(OBJECTS)'
			;;
		esac
		if [ "`echo $ARCH | egrep 'g$'`" ] ; then
			DEBUG="-g${DEBUG0} -D_PSET_DEBUG"
		else
			DEBUG='-O'
		fi
		;;
	hp700)
		echo in hp700
		AFILE=libmix.sl
		case $Compiler in
			cfront)
				OBJDIR=cfront
				CCC=/usr/local/CC/bin/CC
                        	CC=/bin/cc
                        	CCDEFINES='-DGCC2 -Dcplusplus_3_0_1 -Div2_6_compatible -DNEWCPP -DX_WCHAR -Dalloca -DONLY_INT_FIELDS'
                        	TARGET_ARCH='-Dhp700 -DUSG -DSYSV -DMX_ATT -D_PA_RISC_32'
                        	LIBDIR='$(BASE)/lib.new/$(ARCH)/paraset/cfront'
				;;
			gcc)
                        	OBJDIR=gcc
                        	CCC=gcc
                        	CC=gcc
                        	CCDEFINES='-DGCC2 -Dcplusplus_3_0_1 -Div2_6_compatible -DNEWCPP -DX_WCHAR -Dalloca -DONLY_INT_FIELDS'
                        	TARGET_ARCH='-Dhp700 -DUSG -DSYSV -DMX_ATT -D_PA_RISC_32'
                        	LIBDIR='$(BASE)/lib.new/$(ARCH)/paraset/gcc'
                                ;;
		esac 
		case $System in
			dynamic)
				OBJDIR=${OBJDIR}/dynamic
				TARGET='$(LIBDIR)/$(AFILE)'
				MIXLIBCFLAGS='\+z'
				MIXLIBCCFLAGS='ccC\=\"cc \+z\"'
				AFILE=libmix.so
				LINKLINE='	ld -b -o $(LIBDIR)/$(AFILE) $(OBJECTS)'
				;;
			static)
				OBJDIR=${OBJDIR}/static
				TARGET='$(LIBDIR)/$(RFILE)'
				MIXLIBCFLAGS=""
				MIXLIBCCFLAGS=""
				AFILE=""
				LINKLINE='      ld  -r -o $(LIBDIR)/$(RFILE) $(OBJECTS)'
				;;
		esac
		if [ "`echo $ARCH | egrep 'g$'`" ] ; then
			DEBUG='-g -D_PSET_DEBUG'
		else
			DEBUG='-O'
		fi
		;;
esac

case $Compiler in
	sparcworks ) 
		MX=MX_SOLARIS
		;;
	cfront     )
		MX=MX_ATT
		;;
	gcc	   )
		MX=MX_GNU
		;;
esac
createMake
