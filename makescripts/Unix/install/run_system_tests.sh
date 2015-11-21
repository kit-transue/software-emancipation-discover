#!/bin/sh
# /paraset/build/software/run_system_tests.sh

# adapted from local_install.sh
# 19.Sep.2000 Kit Transue

usage() {
	echo ""
	echo "Purpose: testing of a Discover installation area"
	echo "Usage: -psethome psethome"
	echo "				[ -usage | -h | -help ]"
	echo ""
}

PassedSmoke()
{
        echo "Installation ${PSETHOME} passed smoke test: New install finished."
	echo "The latest version of DISCOVER is now waiting to be tested in ${PSETHOME}."

	if [ -w "${PSETHOME}/../../latest" ] ; then
		latest=`cat "${PSETHOME}/../../latest"`
		latest_family=`echo ${latest} | sed 's/\.[^.]*$//'`
		latest_version=`echo ${latest} | sed 's/^.*\.//'`

		this=`dirname "${PSETHOME}"`
		this=`basename "${this}"`
		this_family=`echo ${this} | sed 's/\.[^.]*$//'`
		this_version=`echo ${this} | sed 's/^.*\.//'`

		if [ "${this_family}" = "${latest_family}" -a "${this_version}" -gt "${latest_version}" ] ; then
			echo "${this}" > "${PSETHOME}/../../latest"
		fi
	fi
}

DISPLAY=""
while [ "${*}" ]
do
	case "${1}" in
		-display)
			DISPLAY="${2}"
			shift 2
			;;
		-psethome)
			PSETHOME="${2}"
			shift 2
			;;
		-usage|-h|-help)
			usage # A call to the usage function
			exit 0
			;;
		*)
			echo ""
			echo "${1} : Unrecognized option."
			usage # A call to the usage function
			exit 1
			;;
	esac
done

for extension in "" .1 .2 .3 .4 .5 .additional; do
	TEST_DIRECTORY=${PSETHOME}/../tests${extension}
	if [ ! -d "${TEST_DIRECTORY}" ] ; then
		mkdir "${TEST_DIRECTORY}"
		break
	fi
done

chmod u+rwx "${TEST_DIRECTORY}"
chmod a+w "${TEST_DIRECTORY}"

# Try to start tests
if [ -x "${PSETHOME}/bin/aset" ] ; then
   export PSETHOME

   if [ -z "$DISPLAY" ] ; then 
      case "${Arch}" in
         sun5)
            DISPLAY="oxen:46.0" ; export DISPLAY ;;
         irix6)
            DISPLAY="oxen:47.0" ; export DISPLAY ;;
         hp10)
	    DISPLAY="oxen:45.0" ; export DISPLAY ;;
         *)
	    echo "No defined DISPLAY for architecture ${Arch}" 1>&2
	    exit 1 ;;
      esac
   else
	export DISPLAY
   fi

   if [ -f  ${LOCAL_SRC_ROOT}/makescripts/Unix/install/checkdisplay ] ; then 
      ${LOCAL_SRC_ROOT}/makescripts/Unix/install/checkdisplay
   else
      ${BUILD_SRC_ROOT}/makescrits/Unix/install/checkdisplay
   fi

   if [ $? -eq 0 ] ; then
      
        TMPDIR=/usr/tmp/$$ ; export TMPDIR
	mkdir -p ${TMPDIR}
        PTEST_TMPDIR=${TEST_DIRECTORY}/smoke  ; export PTEST_TMPDIR
	smoke=${PTEST_TMPDIR} ; export smoke
	mkdir -p ${PTEST_TMPDIR}

	cd ${PROJ_ROOT}/tools/ptest
        ./ptest_smoke

      if [ $? -eq 1 ] ; then
        echo "Test failed for ${PSETHOME}: smoke test failed in ${PTEST_TMPDIR}"
      else
	 # report good results; update latest
	 PassedSmoke

	 # Starting system regressions
	 PTEST_TMPDIR=${TEST_DIRECTORY}/alltests ; export PTEST_TMPDIR
	 mkdir -p ${PTEST_TMPDIR}
	 echo "Starting system regressions....see ${PTEST_TMPDIR}/results.log for results (naturally)"
	 ./ptest_all > ${PTEST_TMPDIR}/results.log < /dev/null 2>&1 &

      fi
   else
      echo "Cannot open display ${DISPLAY}"
      exit 1
   fi

else
	echo "Tests could not start for $1: aset missing in ${PSETHOME}/bin"
	exit 1
fi
