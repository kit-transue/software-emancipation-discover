# Called by main build script and $SCRIPTDIR/ma to determine the UNIX
# release and version.

case `uname -s` in
	HP-UX) 
		case `uname -r` in
			A.09*)
				ARCH="hp700";;
			B.10*)
				ARCH="hp10"
				ARCHV="2";;
			B.11*)
				# leverage hp10 for now
				ARCH="hp10"
				ARCHV="0";;
			*)
				ARCH="unknown";;
		esac;;
	SunOS)
		case `uname -r` in
			4.*)
				ARCH="sun4";;
			5.3)
				ARCH="sun5"
				ARCHV="3";;
			5.4)
				ARCH="sun5"
				ARCHV="4";;
			5.5*)
				ARCH="sun5"
				ARCHV="5";;
			5.6*)
				ARCH="sun5"
				ARCHV="6";;
			5.7*)
				ARCH="sun5"
				ARCHV="7";;
			5.8*)
				ARCH="sun5"
				ARCHV="8";;
			5.9*)
				ARCH="sun5"
				ARCHV="9";;
			*)
				ARCH="unknown";;
		esac;;
	IRIX*) 
		case `uname -r` in 
			5.2)
				ARCH="irix5"
				ARCHV="2";;
			5.3)	
				ARCH="irix5"
				ARCHV="3";;
			6.2)	
				ARCH="irix6"
				ARCHV="2";;
			6.5)
				ARCH="irix6"
				ARCHV="5";;
		esac;;
	Linux)
		ARCH=linux`uname -r | cut -d. -f1`
		ARCHV=`uname -r | cut -d. -f2`;;

	Windows_NT)
		ARCH="win32"
		ARCHV=`uname -r`;;

	*) 
		ARCH="unknown";;
esac

# We should at least warn if we come up empty
if [ "$ARCH" = "unknown" ]
then
	echo "WARNING: Proceeding with an unknown architecture"
fi

Arch=${ARCH}
export ARCH Arch
