
if ( ! $?PTEST_NO_SOURCE_ENV && ! $?PTEST_ENV_SOURCED ) then
	setenv PTEST_ENV_SOURCED 1
	setenv PATH "$PSETHOME/bin:/usr/ucb:/usr/bsd:/usr/bin:/bin:/usr/bin/X11:/usr/openwin/bin:${PROJ_ROOT}/tools/ptest:/usr/atria/bin"

	setenv Arch "`DISarch`"
	echo ""
	echo "ptest_set_env.csh: 		Setting ptest environment for $Arch"
	echo ""

	setenv LM_LICENSE_FILE $LM_LICENSE_FILE
	echo "  LM_LICENSE_FILE=	$LM_LICENSE_FILE"

	setenv BADARCH 0

	if ( "$Arch" == "hp700" ) then

	  setenv OS_ROOTDIR /usr/local/CC
	  echo "  OS_ROOTDIR=		$OS_ROOTDIR"
	  setenv CP_LANG_INCL $OS_ROOTDIR/include/CC
	  echo "  CP_LANG_INCL=		$CP_LANG_INCL"
	  setenv PATH $PATH':'$OS_ROOTDIR/bin

	else if ( "$Arch" == "hp10" ) then

	  setenv OS_ROOTDIR /opt/aCC
	  echo "  OS_ROOTDIR=		$OS_ROOTDIR"
	  setenv CP_LANG_INCL $OS_ROOTDIR/include
	  echo "  CP_LANG_INCL=		$CP_LANG_INCL"
	  setenv PATH $PATH':'$OS_ROOTDIR/bin

	else if ( "$Arch" == "sun5" ) then

	  setenv OS_ROOTDIR /solar/opt/SUNWspro/SC2.0.1
	  echo "  OS_ROOTDIR=		$OS_ROOTDIR"
	  setenv CP_LANG_INCL $OS_ROOTDIR/include/CC
	  echo "  CP_LANG_INCL=		$CP_LANG_INCL"
	  setenv PATH $PATH':'/solar/opt/SUNWspro/bin
            
          setenv JAVA_HOME /gdb/jre/sun5/jre 
          echo "  JAVA_HOME=            $JAVA_HOME"
          setenv CLASSPATH $JAVA_HOME/lib/rt.jar:$JAVA_HOME/lib/i18n.jar:$JAVA_HOME/lib/swingall.jar:$CLASSPATH
          echo "  CLASSPATH=            $CLASSPATH"
          setenv PATH $PATH':'$JAVA_HOME/bin


	else if ( "$Arch" == "sun4" ) then

	  setenv OS_ROOTDIR /users/objectstore/sun4
	  echo "  OS_ROOTDIR=		$OS_ROOTDIR"
	  setenv CP_LANG_INCL $OS_ROOTDIR/include/objectstore/CC
	  echo "  CP_LANG_INCL=		$CP_LANG_INCL"
	  setenv PATH $PATH':'$OS_ROOTDIR/bin

	else if ( "$Arch" == "irix6" ) then

	  setenv OS_ROOTDIR /usr
	  echo "  OS_ROOTDIR=		$OS_ROOTDIR"
	  setenv CP_LANG_INCL $OS_ROOTDIR/include/CC
	  echo "  CP_LANG_INCL=		$CP_LANG_INCL"
	  setenv PATH $PATH':'$OS_ROOTDIR/bin

	else

	  setenv BADARCH 1

	endif
	
	setenv PATH $PATH':'/usr/openwin/bin
	echo "  PATH=			$PATH"

	setenv OPENWINHOME /usr/openwin
	echo "  OPENWINHOME=		$OPENWINHOME"

	setenv LD_LIBRARY_PATH $OS_ROOTDIR/lib:/usr/openwin/lib:/usr/lib
	echo "  LD_LIBRARY_PATH=	$LD_LIBRARY_PATH"

	setenv PTEST_LOCATION ${PROJ_ROOT}/job1/tests
	echo "  PTEST_LOCATION=    	$PTEST_LOCATION"

	setenv PSET_SHOW_ADMIN_MENU 1
	setenv PSET_SHOW_DEBUG_MENU 1

	if ( "$BADARCH" == "1" ) then

	  echo ""
	  echo "ptest_set_env.csh: 		Ptest environment for $Arch could not be set"
	  echo ""

	else

	  echo ""
	  echo "ptest_set_env.csh: 		Ptest environment for $Arch is set"
	  echo ""

	endif
endif

