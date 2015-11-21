#!/bin/sh -f

EXECUTE="$1"
TARGETD="$2"

EXEC_BIN="aset \
	aset_CCcc \
	aset_cpp \
	aset_esql \
	aset_jfe \
	builder \
	c-parser \
	cp-parser \
	dis_path \
	dish \
	gala \
	if_extractor \
	ifext \
	mrg_diff \
	mrg_ui \
	mrg_update \
	paracancel \
	plsql \
	pmod_server \
	pset_server \
	sevreporter \
	simplify"

EXEC_PACKAGE="\
	gxpackage \
	index \
	index.rtl \
	package \
	pcparser \
	qvparser \
	vcparser \
	helper"

EXEC_TESTLINK="\
	testlink"

GALAXYHOME=/net/oxen/vol01/share/hp700-10/galaxy/galaxycxx
export GALAXYHOME
SHLIB_PATH=$GALAXYHOME/lib
export SHLIB_PATH
PATH=$GALAXYHOME/bin:${PATH}
export PATH
LOCAL_ROOT=/local/users/builder/hp10
export LOCAL_ROOT

if [ "$EXECUTE" = "build" ] ; then
	echo "HP10: GALAXY   at $GALAXYHOME"
	echo "HP10: Binaries to $LOCAL_ROOT"
	EXEC="$EXEC_BIN $EXEC_PACKAGE $EXEC_TESTLINK"
	for xfile in $EXEC
	do
		echo "HP10: $xfile, make_$xfile -local -nocompile"
		make_$xfile -local -nocompile >make_$xfile-depend.log 2>make_$xfile-depend.log
	done
elif [ "$EXECUTE" = "install" ] ; then
	SOURCE="$LOCAL_ROOT/bin/hp700-O/`cat /paraset/admin/version`"
	if [ -d "$SOURCE" -a -d "$TARGETD" ] ; then
		echo "HP10: Binaries from $SOURCE"
		echo "HP10: Creating bin, and lib directories ..."
		mkdir -p $TARGETD/bin
		mkdir -p $TARGETD/bin/package
		mkdir -p $TARGETD/bin/tests
		mkdir -p $TARGETD/lib
		echo "HP10: Installing $TARGETD/bin ..."
		for xfile in $EXEC_BIN
		do
			cfile="$SOURCE/$xfile"

			if [ "$xfile" = plsql ]
			then
				cp $cfile $TARGETD/bin/dis_sql
			else
				cp $cfile $TARGETD/bin
				if [ -f "$cfile.vr" ] ; then
					cp $cfile.vr $TARGETD/bin
				fi
				if [ -f "$cfile.dir" -a -f "$cfile.pag" ] ; then
					cp $cfile.dir $TARGETD/bin
					cp $cfile.pag $TARGETD/bin
				fi
			fi
		done
		echo "HP10: Installing $TARGETD/bin/package ..."
		for xfile in $EXEC_PACKAGE
		do
			if [ "$xfile" = "index" -o "$xfile" = "index.rtl" ] ; then
				xfile="pksym.$xfile"
			fi
			cfile="$SOURCE/$xfile"
			cp $cfile $TARGETD/bin/package
			if [ -f "$cfile.vr" ] ; then
				cp $cfile.vr $TARGETD/bin/package
			fi
		done
		echo "HP10: Installing $TARGETD/bin/tests ..."
		for xfile in $EXEC_TESTLINK
		do
			cfile="$SOURCE/$xfile"
			cp $cfile $TARGETD/bin/tests
			if [ -f "$cfile.vr" ] ; then
				cp $cfile.vr $TARGETD/bin/tests
			fi
		done
		echo "HP10: Copying splash_hp700 to ./bin ..."
		cp /paraset/src/clients/splash/src/splash_hp700 $TARGETD/bin
		echo "HP10: Copying BUILDsw.vr to ./lib ..."
		cp $SOURCE/BUILDsw.vr			$TARGETD/lib
		echo "HP10: Installing GALAXY from $GALAXYHOME ..."
		cp $GALAXYHOME/bin/vdasserv		$TARGETD/bin
		cp $GALAXYHOME/lib/vgalaxy2.vr		$TARGETD/lib
		cp $GALAXYHOME/lib/libvgalaxy.sl	$TARGETD/lib
		cp $GALAXYHOME/lib/libvgalaxy++.sl	$TARGETD/lib
	else
		echo "HP10: usage: makeHP10.sh install <psethome>"
	fi
else
	echo "HP10: usage: makeHP10.sh < build | install <psethome> >"
fi
