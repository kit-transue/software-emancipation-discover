#!sh -v
#
# $Id: install.sh 1.48.1.2 2004/10/01 16:15:44EDT John Verne (jverne) Exp  $
#
#syntax: sh install.sh [staging area dir] [-r|-R]
#staging area dir should be specified according to the unix standard
#-r|-R= Release build (delete debug files)

CleanPJs()
{
	for pj in `find $1 -type f -name '*.pj' -print`
	do
#		echo $pj
		rm $pj
	done
}

# Handy commands
JAR=$JAVA_HOME/bin/jar.exe

if [ "$1" = "-?" -o "$1" = "-h" -o "$1" = "/?" ]
then
	echo "Syntax: sh install .sh [staging area dir] [-r]
	staging area dir is the root directory of the install,
	-r means Release build"
	exit 1
fi


if [ "$2" ]
then
	FLAG=$2
	TARGETDIR=$1
fi

# first break up the arguments

if [ "$1" != "" -a "$2" = "" ]
then
        if [ "$1" = "-d" -o "$1" = "-D" -o "$1" = "-r" -o "$1" = "-R" ]
        then
                FLAG=$1
        else 
                TARGETDIR=$1
        fi
fi

if [ "$1" = "" -o "$TARGETDIR" = "" ]
then
        TARGETDIR="N:/tmp/CDstage"
fi

if [ ! -d "$TARGETDIR" ]
then
        echo "
        $TARGETDIR is not a valid directory
        Please try again.
        remember to escape backslashes..."
        exit 1
else
        echo "Target directory is: $TARGETDIR\n"
fi

if [ "$FLAG" = "" ]
then 
	FLAG="-d"
fi

if [ "$FLAG" = "release" -o "$FLAG" = "Release" ]
then
        echo "Preparing Release build..."
        DEBUG=0
        RTYPE="Release"
else
        echo "Preparing Debug build..."
        DEBUG=1
        RTYPE="Debug"   
fi

# the following line takes a generic unix path and turns it into a
# generic DOS path. read the sed book for more info.
TARGETNTDIR=`echo $TARGETDIR | sed 's/\//\\\\/g'`

echo "Deleting everything\n"
DIRS="bin lib include setup perl webhelp mb reports settings DISIM \
      prebuilt share resources"

for DIR in $DIRS
do
    rm -rf $TARGETDIR/$DIR
done
rm -f  $TARGETDIR/*.*

# Do this to force a clean up of old disk image
#rm -rf $TARGETDIR/Inst

SYSBASE=${BUILD_SRC_ROOT}
PROJ_ROOT="$SYSBASE"
# Make a generic DOS filespec of PROJ_ROOT
PROJ_NT_ROOT=`echo $PROJ_ROOT | sed 's/\//\\\\/g'`
BUILD_BASE="$PROJ_ROOT/makescripts"

cp $BUILD_BASE/version $TARGETDIR/.version

mkdir -p $TARGETDIR/bin/generic
echo "Copying the bin directory"
cp $PROJ_ROOT/dis_dl/*.pl $TARGETDIR/bin/
cp $PROJ_ROOT/admintool/$RTYPE/admintool.exe $TARGETDIR/bin/
cp $PROJ_ROOT/serverspy/$RTYPE/serverspy.exe $TARGETDIR/bin/
cp $PROJ_ROOT/dish/$RTYPE/dish2.exe $TARGETDIR/bin/
cp $PROJ_ROOT/nameserver/$RTYPE/nameserv.exe $TARGETDIR/bin/
cp $PROJ_ROOT/dis_path/$RTYPE/dis_path.exe $TARGETDIR/bin/
cp $PROJ_ROOT/disperl/$RTYPE/disperl.exe $TARGETDIR/bin/
cp $PROJ_ROOT/dismb/$RTYPE/dismb.exe $TARGETDIR/bin/
cp $PROJ_ROOT/flex/$RTYPE/flex.exe $TARGETDIR/bin/dis-flex.exe
cp $PROJ_ROOT/log_presentation/$RTYPE/log_presentation.exe $TARGETDIR/bin/
cp $PROJ_ROOT/mbdriver/$RTYPE/mbdriver.exe $TARGETDIR/bin/
cp $PROJ_ROOT/spawnprocess/Windows/$RTYPE/spawnprocess.exe $TARGETDIR/bin/
cp $PROJ_ROOT/mrg_update/$RTYPE/mrg_update.exe $TARGETDIR/bin/
cp $PROJ_ROOT/mrg_ui/$RTYPE/mrg_ui.exe $TARGETDIR/bin/
cp $PROJ_ROOT/mrg_diff/$RTYPE/mrg_diff.exe $TARGETDIR/bin/
cp $PROJ_ROOT/ifext/$RTYPE/ifext.exe $TARGETDIR/bin/
cp $PROJ_ROOT/simplify/$RTYPE/simplify.exe $TARGETDIR/bin/
cp $PROJ_ROOT/model_server/make/Windows/$RTYPE/pset_server.exe $TARGETDIR/bin/
cp $PROJ_ROOT/model_server/make/Windows/$RTYPE/model_server.exe $TARGETDIR/bin/
# vrx2vr.exe chokes on non-DOS pathnames
$BUILD_BASE/bin/win32/vrx2vr $PROJ_NT_ROOT\\data\\aset.vrx $TARGETNTDIR\\bin\\pset_server.vr
cp $PROJ_ROOT/md5/$RTYPE/md5.exe $TARGETDIR/bin/
cp $PROJ_ROOT/aset_jfe/$RTYPE/aset_jfe.exe $TARGETDIR/bin
cp $PROJ_ROOT/dislite/$RTYPE/dislite.exe $TARGETDIR/bin
cp $PROJ_ROOT/wakeup/$RTYPE/wakeup.exe $TARGETDIR/bin
cp $PROJ_ROOT/checklist/$RTYPE/checklist.exe $TARGETDIR/bin
cp $PROJ_ROOT/model_server/gala/src/menus.dat $TARGETDIR/bin/
cp $PROJ_ROOT/model_server/gala/src/qhelp.dat $TARGETDIR/bin/
cp $PROJ_ROOT/diff/nt/cmp.exe $TARGETDIR/bin
cp $PROJ_ROOT/diff/nt/diff.exe $TARGETDIR/bin
cp $PROJ_ROOT/diff/nt/diff3.exe $TARGETDIR/bin
cp $PROJ_ROOT/data/ntexestub.exe $TARGETDIR/bin
cp $PROJ_ROOT/emacsAdapter/$RTYPE/emacsAdapter.exe $TARGETDIR/bin
cp $PROJ_ROOT/devxAdapter/$RTYPE/devxAdapter.exe $TARGETDIR/bin

cp $BUILD_BASE/bin/win32/tclset.dll $TARGETDIR/bin/
cp $BUILD_BASE/bin/win32/tkntutil.dll $TARGETDIR/bin/

cp $PROJ_ROOT/grep/$RTYPE/grep.exe $TARGETDIR/bin/generic/grep.exe
cp $PROJ_ROOT/grep/$RTYPE/grep.exe $TARGETDIR/bin/generic/egrep.exe

# -- begin: copy xml dlls.
cp $PROJ_ROOT/expat-1.0/dist/bin/*.dll $TARGETDIR/bin
# -- end

# -- begin: copy galaxy installation data
echo "Copying galaxy stuff"
cp $BUILD_BASE/bin/win32/vgalaxy*.vr $TARGETDIR/bin/
cp $BUILD_BASE/bin/win32/vdasserv.exe $TARGETDIR/bin/
cp $BUILD_BASE/bin/win32/vgal5.dll $TARGETDIR/bin/
# -- end

# -- begin: copy xml dlls for submission check
echo "Copying xml stuff"
cp $PROJ_ROOT/expat-1.0/dist/bin/xmlparse.dll $TARGETDIR/bin/
cp $PROJ_ROOT/expat-1.0/dist/bin/xmltok.dll $TARGETDIR/bin/
# -- end

# -- begin: copy "include" files
echo "Copying include files"
mkdir -p $TARGETDIR/include
cp $PROJ_ROOT/data/include/nt/* $TARGETDIR/include/
cp $PROJ_ROOT/data/include/gcc.h $TARGETDIR/include/
# -- end

# -- begin: copy FLEXlm executables
echo "Copying flexlm"
# figure the current DLL version (sets $LMGRDLLVER):
. $PROJ_ROOT/FLEXlm/flexlm/current_defs_nt
FLEX_ROOT=$PROJ_ROOT/FLEXlm/flexlm/current
cp $FLEX_ROOT/i86_n3/installs.exe $TARGETDIR/bin
cp $FLEX_ROOT/i86_n3/${LMGRDLLVER}.dll $TARGETDIR/bin
cp $FLEX_ROOT/i86_n3/lmgrd.exe $TARGETDIR/bin
cp $FLEX_ROOT/i86_n3/lmutil.exe $TARGETDIR/bin
cp $FLEX_ROOT/i86_n3/MKS.exe $TARGETDIR/bin
#cp $FLEX_ROOT/i86_n3/setechd.exe $TARGETDIR/bin
# -- end


if [ "$DEBUG" = "1" ]
then
	# -- begin: copy debugging information (internal installation only)
	echo "Copying debugging info"
	cp $PROJ_ROOT/ifext/$RTYPE/ifext.pdb $TARGETDIR/bin/
	cp $PROJ_ROOT/simplify/$RTYPE/simplify.pdb $TARGETDIR/bin/
	cp $PROJ_ROOT/model_server/make/Windows/$RTYPE/pset_server.pdb $TARGETDIR/bin
	cp $PROJ_ROOT/model_server/make/Windows/$RTYPE/model_server.pdb $TARGETDIR/bin
	cp $PROJ_ROOT/md5/$RTYPE/md5.pdb $TARGETDIR/bin/
	cp $PROJ_ROOT/aset_jfe/$RTYPE/aset_jfe.pdb $TARGETDIR/bin/
	cp $PROJ_ROOT/disperl/$RTYPE/disperl.pdb $TARGETDIR/bin/
	cp $PROJ_ROOT/dismb/$RTYPE/dismb.pdb $TARGETDIR/bin/
	cp $PROJ_ROOT/dislite/$RTYPE/dislite.pdb $TARGETDIR/bin
	cp $PROJ_ROOT/wakeup/$RTYPE/wakeup.pdb $TARGETDIR/bin
	cp $PROJ_ROOT/checklist/$RTYPE/checklist.pdb $TARGETDIR/bin
	# -- end
fi

# -- begin: install EDG parser
echo "Copying EDG Parser"
cp $PROJ_ROOT/aset_CCcc/$RTYPE/aset_CCcc.exe $TARGETDIR/bin

if [ "$DEBUG" = "1" ]
then
	cp $PROJ_ROOT/aset_CCcc/$RTYPE/aset_CCcc.pdb $TARGETDIR/bin
fi
# -- end

# -- begin: copy OLE UI files
# When you add a component to this list (any .ocx file)
# you MUST ensure that it is in the 
# /nt-bin/installshield/client/discomps.txt file as well
# or it won't be registered.

#OLESRC="n:/nt-bin/ui_ole"
# We changed the way discovermdi was built, since Dmitry edited the
#       dsp/dsw files instead of editing the Makefile
OLESRC="$PROJ_ROOT/newntui/activex"

cp $OLESRC/categoriescombo/$RTYPE/CategoriesCombo.ocx           $TARGETDIR/bin
cp $OLESRC/datasource/$RTYPE/DataSource.ocx                     $TARGETDIR/bin
cp $OLESRC/groupscombo/$RTYPE/GroupsCombo.ocx                   $TARGETDIR/bin
cp $OLESRC/CodeRoverIntegrator/$RTYPE/CodeRoverIntegrator.ocx   $TARGETDIR/bin
cp $OLESRC/projectmodules/$RTYPE/ProjectModules.ocx             $TARGETDIR/bin
cp $OLESRC/projecttree/$RTYPE/ProjectTree.ocx                   $TARGETDIR/bin
cp $OLESRC/queriescombo/$RTYPE/QueriesCombo.ocx                 $TARGETDIR/bin
cp $OLESRC/queryresults/$RTYPE/QueryResults.ocx                 $TARGETDIR/bin
cp $OLESRC/reporttemolate/$RTYPE/ReportTemolate.ocx             $TARGETDIR/bin
cp $OLESRC/spreadsheet/$RTYPE/Spreadsheet.ocx                   $TARGETDIR/bin
cp $OLESRC/CMIntegrator/$RTYPE/CMIntegrator.ocx                 $TARGETDIR/bin
cp $OLESRC/Impact/$RTYPE/Impact.ocx                             $TARGETDIR/bin

# -- copy Visual 6 controls that do not exist under 4 and 5
cp $BUILD_BASE/bin/win32/*.ocx                                  $TARGETDIR/bin

cp $OLESRC/../discovermdi/$RTYPE/DiscoverMDI.exe                $TARGETDIR/bin

cp $BUILD_BASE/bin/win32/msxsl.exe                              $TARGETDIR/bin
# when you add new ones, remember to add them to discomps.txt!!!!! (See comment at start)
if [ "$DEBUG" = "1" ]
then
	cp $OLESRC/categoriescombo/$RTYPE/categoriescombo.pdb           $TARGETDIR/bin
	cp $OLESRC/datasource/$RTYPE/datasource.pdb                     $TARGETDIR/bin
	cp $OLESRC/groupscombo/$RTYPE/groupscombo.pdb                   $TARGETDIR/bin
	cp $OLESRC/coderoverintegrator/$RTYPE/CodeRoverIntegrator.pdb   $TARGETDIR/bin
	cp $OLESRC/projectmodules/$RTYPE/projectmodules.pdb             $TARGETDIR/bin
	cp $OLESRC/projecttree/$RTYPE/projecttree.pdb                   $TARGETDIR/bin
	cp $OLESRC/queriescombo/$RTYPE/queriescombo.pdb                 $TARGETDIR/bin
	cp $OLESRC/queryresults/$RTYPE/queryresults.pdb                 $TARGETDIR/bin
	cp $OLESRC/reporttemolate/$RTYPE/reporttemolate.pdb             $TARGETDIR/bin
	cp $OLESRC/spreadsheet/$RTYPE/spreadsheet.pdb                   $TARGETDIR/bin
	cp $PROJ_ROOT/newntui/discovermdi/$RTYPE/DiscoverMDI.pdb        $TARGETDIR/bin
fi
# -- end

# -- begin: construct /lib directory
#rm -rf $TARGETDIR/lib/*
echo "Copying lib Directory"
mkdir $TARGETDIR/lib
cp $PROJ_ROOT/dis_dl/*.dis $TARGETDIR/lib/
cp $PROJ_ROOT/model_server/gala/src/admin.qhelp $TARGETDIR/lib/  
cp $PROJ_ROOT/data/options.map $TARGETDIR/lib
cp $PROJ_ROOT/data/default.ptn $TARGETDIR/lib
cp $PROJ_ROOT/data/home.pdf $TARGETDIR/lib/
cp $PROJ_ROOT/data/messages.dat $TARGETDIR/lib/
cp $PROJ_ROOT/data/msg_database.dat $TARGETDIR/lib/
cp $PROJ_ROOT/data/valid_compiler_options.dat $TARGETDIR/lib/
cp $PROJ_ROOT/data/valid_compiler_options_edg.dat $TARGETDIR/lib/
sed 's/\$PSETHOME/%PSETHOME%/g' $PROJ_ROOT/data/ParaDOCS.pdf > $TARGETDIR/lib/ParaDOCS.pdf
cp -ir $PROJ_ROOT/data/report-templates $TARGETDIR/lib/
cp -ir $PROJ_ROOT/data/sample-boilerplates $TARGETDIR/lib/
cp $PROJ_ROOT/flex/windows/flex.skel $TARGETDIR/lib/
cp $PROJ_ROOT/model_server/interface/src/discover.summary.dis $TARGETDIR/lib/
cp $PROJ_ROOT/data/settings/psetPrefs/psetPrefs.nt $TARGETDIR/lib/psetPrefs.default
cp $PROJ_ROOT/data/settings/subsysPrefs/subsysPrefs.default $TARGETDIR/lib
cp $PROJ_ROOT/model_server/gala/src/admin.menus $TARGETDIR/lib/
cp $PROJ_ROOT/log_presentation/data/filtersev.xml $TARGETDIR/lib/
# vrx2vr.exe chokes on non-DOS pathnames
$BUILD_BASE/bin/win32/vrx2vr $PROJ_NT_ROOT\\model_server\\gala\\src\\admin.vrx $TARGETNTDIR\\lib\\admin.vr
# -- Access scripts
cp $PROJ_ROOT/model_server/api/scripts/*.dis $TARGETDIR/lib
cp $PROJ_ROOT/mrg_ui/scripts/mrg_ui.tcl $TARGETDIR/lib
cp $PROJ_ROOT/model_server/api/scripts/*.flt $TARGETDIR/lib
cp $PROJ_ROOT/model_server/api/scripts/exclcmds.txt $TARGETDIR/lib
rm -f $TARGETDIR/lib/*ui_spec.dis
# -- support for dis/galaxy graphics
cp $PROJ_ROOT/model_server/gra/src/symbols.dat $TARGETDIR/lib
cp $PROJ_ROOT/model_server/graGala/src/symbols.new $TARGETDIR/lib
cp $PROJ_ROOT/data/pset_server.menus $TARGETDIR/lib
cp $PROJ_ROOT/data/pset_server.pmenus $TARGETDIR/lib
cp $PROJ_ROOT/data/pset_server.qhelp $TARGETDIR/lib
cp $PROJ_ROOT/data/default.flt $TARGETDIR/lib/
cp $PROJ_ROOT/newntui/scripts/*.* $TARGETDIR/lib 
# -- els stuff
echo "...ELS"
mkdir $TARGETDIR/lib/browser_data
cp $PROJ_ROOT/model_server/els/browser_data/*.ui $TARGETDIR/lib/browser_data
cp $PROJ_ROOT/model_server/els/browser_data/browser.dat.newui $TARGETDIR/lib/browser_data/browser.dat
echo "...tcl library"
mkdir $TARGETDIR/lib/tcl8.3
cp -r $PROJ_ROOT/dis_tcl8.3/library/* $TARGETDIR/lib/tcl8.3/
echo "...tk library"
mkdir $TARGETDIR/lib/tk8.3
cp -r $PROJ_ROOT/dis_tk8.3/library/* $TARGETDIR/lib/tk8.3/
mkdir $TARGETDIR/lib/gala_scripts
cp $PROJ_ROOT/model_server/gala/gala_scripts/*.tcl $TARGETDIR/lib/gala_scripts
rm -f $TARGETDIR/lib/gala_scripts/DialogLayers.EvaluatorUNIX.Procedures.tcl
echo "...autoflag library"
mkdir $TARGETDIR/lib/autoflag
cp -r $PROJ_ROOT/psethome/lib/autoflag/* $TARGETDIR/lib/autoflag/

# -- pdf_wizard stuff
cp $PROJ_ROOT/pdfwiz/precomp/bin/* $TARGETDIR/bin
cp $PROJ_ROOT/pdfwiz/precomp/lib/* $TARGETDIR/lib

cp $PROJ_ROOT/dis_sql/$RTYPE/dis_sql.exe $TARGETDIR/bin/dis_sql.exe

if [ "$DEBUG" = "1" ]
then
	cp $PROJ_ROOT/dis_sql/$RTYPE/dis_sql.pdb $TARGETDIR/bin/dis_sql.pdb
fi
# -- end

# -- software evaluator stuff
echo "...Software Evaluator"
cp $PROJ_ROOT/sevreporter/scripts/*.* $TARGETDIR/lib
cp $PROJ_ROOT/sevreporter/$RTYPE/sevreporter.exe $TARGETDIR/bin
cp $PROJ_ROOT/canceldialog/$RTYPE/canceldialog.exe $TARGETDIR/bin

if [ "$DEBUG" = "1" ]
then
	cp $PROJ_ROOT/sevreporter/$RTYPE/sevreporter.pdb $TARGETDIR/bin
	cp $PROJ_ROOT/canceldialog/$RTYPE/canceldialog.pdb $TARGETDIR/bin
fi
# -- end

# -- begin: copy online help
# 
# The contents of the helps are subject to change, so I just dump the
# contents into the Image dir, and let InstallShield dynamically link
# them into the disk image via $TARGETDIR
#
echo "Copying Online HTML Help"
mkdir -p $TARGETDIR/webhelp/access
mkdir -p $TARGETDIR/webhelp/model_browser
mkdir -p $TARGETDIR/webhelp/admin_client
echo "...Access help"
(cd $TARGETDIR/webhelp/access; $JAR xf $PROJ_ROOT/Docs/webhelp/win_access_webhelp.zip)
echo "...Model Browser help"
(cd $TARGETDIR/webhelp/model_browser; $JAR xf $PROJ_ROOT/Docs/webhelp/win_model_browser_webhelp.zip)
echo "...Admin Client help"
(cd $TARGETDIR/webhelp/admin_client; $JAR xf $PROJ_ROOT/Docs/webhelp/admin_client_webhelp.zip)
# -- end

# -- begin: copy CodeRover style integration files.  Old integration
#           below is probably obsolete.  Removal is left as an exercise
#           for the reader.
cp $PROJ_ROOT/emacs_integration/server/$RTYPE/editorserver.exe $TARGETDIR/bin
cp $PROJ_ROOT/emacs_integration/client/$RTYPE/emacsclient.exe $TARGETDIR/bin
cp $PROJ_ROOT/emacs_integration/src/roverserv.el $TARGETDIR/bin
cp $PROJ_ROOT/emacs_integration/src/emacsIntegration.el $TARGETDIR/bin
cp $PROJ_ROOT/vs_integration/$RTYPE/VSAddIn.dll $TARGETDIR/lib
cp $PROJ_ROOT/jbuilder_integration/$RTYPE/JBuilderAddIn.jar $TARGETDIR/lib/JBuilderAddIn.jar
cp $PROJ_ROOT/DevXLauncher/$RTYPE/DevXLauncher.exe $TARGETDIR/bin
cp $PROJ_ROOT/newntui/discovermdi/res/CodeRover.ico $TARGETDIR/bin

if [ "$DEBUG" = "1" ] 
then
	cp $PROJ_ROOT/emacs_integration/server/$RTYPE/editorserver.pdb $TARGETDIR/bin
	cp $PROJ_ROOT/emacs_integration/client/$RTYPE/emacsclient.pdb $TARGETDIR/bin
	cp $PROJ_ROOT/DevXLauncher/$RTYPE/DevXLauncher.pdb $TARGETDIR/bin
fi

# -- end: CodeRover style integration

# -- begin: copy perl files
echo "Copying Perl 5"
mkdir $TARGETDIR/perl
tar -xf $PROJ_ROOT/thirdparty/perl/NT/perl.tar -C $TARGETDIR/perl
# -- end

# -- begin: install options filter
cp $PROJ_ROOT/makescripts/Windows/install/option_filter.exe $TARGETDIR/bin/setccfilt.exe
cp $PROJ_ROOT/data/NT/exec_options.dat $TARGETDIR/lib

# copy osport support
echo "Copying osport and scripts"
cp $PROJ_ROOT/setfork/$RTYPE/setfork.exe $TARGETDIR/bin

# -- begin: copy mb scripts
echo "Copying mb"
mkdir -p $TARGETDIR/mb
cp $SYSBASE/psethome/mb/* $TARGETDIR/mb
# -- end: mb

# -- extra dislite stuff
echo "Copying Dislite scripts"
mkdir -p $TARGETDIR/lib/dislite
cp $PROJ_ROOT/dislite/scripts/*.* $TARGETDIR/lib/dislite
#copy all but one file...
rm -f $TARGETDIR/lib/dislite/dislite

echo "Copying Dislite mail client"
cp $PROJ_ROOT/mail/$RTYPE/mail.exe $TARGETDIR/bin
 
mkdir -p $TARGETDIR/lib/vim
cp $PROJ_ROOT/vim-5.3/menu.vim $TARGETDIR/lib/vim
cp $PROJ_ROOT/vim-5.3/discover.vim $TARGETDIR/lib/vim
cp $PROJ_ROOT/vim-5.3/mswin.vim $TARGETDIR/lib/vim
cp $PROJ_ROOT/vim-5.3/bugreport.vim $TARGETDIR/lib/vim
cp $PROJ_ROOT/vim-5.3/gvimrc_example $TARGETDIR/lib/vim
cp $PROJ_ROOT/vim-5.3/README.txt $TARGETDIR/lib/vim
cp -r $PROJ_ROOT/vim-5.3/doc $TARGETDIR/lib/vim
cp -r $PROJ_ROOT/vim-5.3/syntax $TARGETDIR/lib/vim
cp -r $PROJ_ROOT/vim-5.3/tools $TARGETDIR/lib/vim
cp -r $PROJ_ROOT/vim-5.3/tutor $TARGETDIR/lib/vim
cp $PROJ_ROOT/vim-5.3/vimrc_example $TARGETDIR/lib/vim/_vimrc
cp $PROJ_ROOT/dis_tcl8.3/win/release/tcl83.dll $TARGETDIR/bin
cp $PROJ_ROOT/dis_tcl8.3/win/release/tclsh83.exe $TARGETDIR/bin
cp $PROJ_ROOT/dis_tk8.3/win/release/tk83.dll $TARGETDIR/bin
cp $PROJ_ROOT/dis_tk8.3/win/release/wish83.exe $TARGETDIR/bin
cp $PROJ_ROOT/vim-5.3/win/$RTYPE/vimrun.exe $TARGETDIR/bin
cp $PROJ_ROOT/vim-5.3/win/$RTYPE/gvim.exe $TARGETDIR/bin
if [ "$DEBUG" = "1" ]
then
        cp $PROJ_ROOT/vim-5.3/win/$RTYPE/vimrun.pdb $TARGETDIR/bin
        cp $PROJ_ROOT/vim-5.3/win/$RTYPE/gvim.pdb $TARGETDIR/bin
fi
echo "End extra dislite stuff"
# -- end

# -- documentation/misc.
echo "Copying NT README"
cp $PROJ_ROOT/data/NT/README $TARGETDIR/README.TXT
# -- end

# -- reports/templates
echo "Copying report information"
mkdir -p $TARGETDIR/reports
mkdir -p $TARGETDIR/reports/templates
touch $TARGETDIR/reports/templates/ignore.me
echo "End of reports"
# -- end

# -- settings
echo "Copying settings for MDI application"
mkdir -p $TARGETDIR/settings
touch $TARGETDIR/settings/ignore.me
echo "End of settings"
# -- end

# parsercmd
echo "Copying parsercmd components"
cp $PROJ_ROOT/parsercmd/"${RTYPE}"/parsercmd.exe "${TARGETDIR}"/bin
cp $PROJ_ROOT/parsercmd/data/*.xml "${TARGETDIR}"/lib
cp $PROJ_ROOT/parsercmd/data/cmdline.dtd "${TARGETDIR}"/lib
cp $PROJ_ROOT/parsercmd/perl/*.pl "${TARGETDIR}"/mb

# Andrey's CM components
echo "Copying CM Driver components"
cp $PROJ_ROOT/cmdriver/"${RTYPE}"/CMDriver.exe "${TARGETDIR}"/bin
cp $PROJ_ROOT/cmdish/"${RTYPE}"/cmdish.exe "${TARGETDIR}"/bin
cp $PROJ_ROOT/cmdriver/CmXml/Windows/cm.xml "${TARGETDIR}"/lib
cp $PROJ_ROOT/cmdriver/CmXml/Windows/si_get_sandboxes.bat "${TARGETDIR}"/bin

# Scott's pdfgen
echo "Copying pdfgen components"
cp $PROJ_ROOT/pdfgen/"${RTYPE}"/pdfgen.exe "${TARGETDIR}/bin"
cp $PROJ_ROOT/scandsp/"${RTYPE}"/scandsp.exe "${TARGETDIR}/bin"
cp $PROJ_ROOT/ci_environment/"${RTYPE}"/ci_environment.exe "${TARGETDIR}/bin"

# Adding xerces library for CM stuff
cp $PROJ_ROOT/extlib/nt/xerces-c_1_2.dll "${TARGETDIR}/lib"
if [ "$DEBUG" = "1" ]
then
	cp $PROJ_ROOT/thirdparty/xerces/NT/xerces-c_1_2D.dll "${TARGETDIR}/lib"
fi

#
# DISIM
# These operations move DISIM files to the install image for deployment
# as well as setting up the DISIM directory to make the DISIM archive
# which is deployed in share/DISIM.
# $PSETHOME/DISIM is not part of the final disk image.
#
echo "Copying DISIM"
cp -R $PROJ_ROOT/DISIM  "${TARGETDIR}"

# Clean out the fork.exe sources
rm -rf "${TARGETDIR}"/DISIM/bin/admin/fork

# Flip these so they are in DOS format; binary files are ignored
find "${TARGETDIR}/DISIM" -type f -exec flip -d {} \; 2> /dev/null

# Copy some of the DISIM files over to the model browser
# NOTE: Leave the empty DISIM/bin/admin dir in the DISIM.zip archive
mv "${TARGETDIR}"/DISIM/bin/admin/*.pl		  "${TARGETDIR}"/mb
mv "${TARGETDIR}"/DISIM/bin/admin/fork.exe	  "${TARGETDIR}"/mb
mv "${TARGETDIR}"/DISIM/bin/admin/BrowserX.dis    "${TARGETDIR}"/mb
mv "${TARGETDIR}"/DISIM/lib/scripts/BatchSimplify.dis "${TARGETDIR}"/lib
# We need a dummy file to keep the archive we make happy
touch "${TARGETDIR}"/DISIM/bin/admin/ignore.me

cp $PROJ_ROOT/psethome/bin/BuildModel.bat	  "${TARGETDIR}"/bin
cp $PROJ_ROOT/psethome/bin/CreateSummary.bat	  "${TARGETDIR}"/bin
cp $PROJ_ROOT/psethome/bin/NTPSETServerAdmin.bat  "${TARGETDIR}"/bin
cp $PROJ_ROOT/psethome/bin/RemoveModel.bat	  "${TARGETDIR}"/bin
cp $PROJ_ROOT/psethome/bin/RunQAR.bat		  "${TARGETDIR}"/bin
cp $PROJ_ROOT/psethome/bin/RunSimplify.bat	  "${TARGETDIR}"/bin
cp $PROJ_ROOT/psethome/bin/ServerAdmin.bat	  "${TARGETDIR}"/bin
cp $PROJ_ROOT/psethome/bin/VersionWindow.bat	  "${TARGETDIR}"/bin
cp $PROJ_ROOT/psethome/bin/QueryResumability.bat  "${TARGETDIR}"/bin
cp $PROJ_ROOT/DISIM/bin/StartBrowser.bat	  "${TARGETDIR}"/bin
cp $PROJ_ROOT/DISIM/bin/StartDevx.bat		  "${TARGETDIR}"/bin
cp $PROJ_ROOT/DISIM/bin/StartBuildServer.bat	  "${TARGETDIR}"/bin

# pathcvt - NT long/short path translation
cp $PROJ_ROOT/pathcvt/"${RTYPE}"/pathcvt.exe "${TARGETDIR}/bin"

# Time to put the prebuilt stuff in
echo "Copying prebuilt stuff ..."
cp -R $PROJ_ROOT/prebuilt $TARGETDIR
chmod -R a+w $TARGETDIR/prebuilt

#
# CI Admin GUI
#
echo "Copying CI Admin GUI classes and resources..."
cp $PROJ_ROOT/adminui/dest/bin/CIEClient.exe	$TARGETDIR/bin
cp $PROJ_ROOT/adminui/dest/lib/CIAdmin.jar	$TARGETDIR/lib
cp $PROJ_ROOT/adminui/dest/lib/wizard.xml	$TARGETDIR/lib
cp $PROJ_ROOT/adminui/dest/lib/setuptxt.xsl	$TARGETDIR/lib

# TODO: This may be temporary if we can abstract the classes we need
# into CIAdmin.jar
#IMPORTS_DIR=$SOLUTION_BUILD_ROOT/imports/jars
#cp $IMPORTS_DIR/xerces.jar		 	$TARGETDIR/lib
#cp $IMPORTS_DIR/weblogic510client.jar	 	$TARGETDIR/lib
#cp $WEBLOGIC_HOME/lib/weblogicaux.jar	 	$TARGETDIR/lib

cp -R $PROJ_ROOT/adminui/dest/resources		$TARGETDIR/resources

echo "...native SI components"
LIBJAVA_DIR=$SOLUTION_BUILD_ROOT/solution/components/libjava/dest/native/win32-ia32
FRAMEWORK_DIR=$SOLUTION_BUILD_ROOT/solution/components/framework/dest/native/win32-ia32
cp $LIBJAVA_DIR/mks_awt_win32.dll	$TARGETDIR/bin
cp $LIBJAVA_DIR/mksIO.dll		$TARGETDIR/bin
cp $LIBJAVA_DIR/mksUtil.dll		$TARGETDIR/bin
cp $LIBJAVA_DIR/msvcUtil.dll		$TARGETDIR/bin
cp $FRAMEWORK_DIR/cie.exe		$TARGETDIR/bin
cp $FRAMEWORK_DIR/cieg.exe		$TARGETDIR/bin

# Last bits of cleanup to the install Image
echo "Tidying up the installation..."
if [ "$DEBUG" = "0" ]
then
    echo "...remove Debug files"
    rm -f $TARGETDIR/bin/*.pdb
fi

echo "...clean up .pj files"
CleanPJs "$TARGETDIR"

echo "...DOSifying end-user text files"
FILEEXTS="c cpp dat default dis menus pdf pl prefs txt ui xml"
for EXT in $FILEEXTS
do
    find $TARGETDIR/lib		-name "*.$EXT" -exec flip -d {} \;
    find $TARGETDIR/mb		-name "*.$EXT" -exec flip -d {} \;
    find $TARGETDIR/prebuilt	-name "*.$EXT" -exec flip -d {} \;
done

# We do this here so we don't suck in .pj files
# This is not in make.install.sh to separate the Image creation from
# the Installer creation
echo "Creating DISIM archive"
mkdir -p ${TARGETDIR}/share/DISIM
$JAR cMf ${TARGETDIR}/share/DISIM/DISIM.zip -C ${TARGETDIR}/DISIM .

echo "install.sh: done"
