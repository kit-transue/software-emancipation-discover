


ARCH_INSTALL_BASE=${DEST}/install_bases/${Arch}
V2TIME_DAT_FILE=/users/builder/integ/v2time.dat
VERSION_FILE="${BUILD_SRC_ROOT}/makescripts/version"

build_host () {
	case $1 in
		sun5)
			echo tweety
			;;
		hp10)
			echo fridge
			;;
		irix6)
			echo bigfoot
			;;
		*)
			echo "Don't know build machine for architecture $1" 1>&2
			exit 1;
			;;
	esac
}
