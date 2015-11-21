#!sh

#
# Figure out the correct configuration for a given library
#

Usage()
{
	echo "Usage: get_cfg.sh <Release|Debug> executable [library]" 1>&2
}

cfg=$1
target=$2
lib=$3

if [ "$cfg" != "Release" -a "$cfg" != "Debug" ] ; then
	Usage
	exit 1
fi

if [ "$lib" = "" ] ; then
	case "$target" in
		aset_CCcc)	echo "${cfg}";;
		aset_jfe)	echo "${cfg}";;
		editorserver)	echo "${cfg}";;
		ifext)		echo "${cfg}";;
		parsercmd)	echo "${cfg}";;
		pathcvt)		echo "${target} - Win32 ${cfg}";;
		pdfgen)		echo "${target} - Win32 ${cfg}";;
		CMDriver)		echo "${target} - Win32 ${cfg}";;
		cmdish)		echo "${target} - Win32 ${cfg}";;
		scandsp)		echo "${target} - Win32 ${cfg}";;
		ci_environment)		echo "${target} - Win32 ${cfg}";;
		dis_sql)	echo "${cfg}";;
		model_server)	echo "${cfg}";;
		pset_server)	echo "${cfg}";;
		mrg_ui)		echo "${cfg}";;
		mrg_diff)	echo "${cfg}";;
		mrg_update)	echo "${cfg}";;
		dislite)	echo "${cfg}";;
		*)		echo "${target}_-_Win32_${cfg}";;
	esac
else
	case "$target" in
		discovermdi)	echo "${lib}_-_Win32_${cfg}";;
		c_parser)	echo "${lib}_-_Win32_${cfg}";;
		cp_parser)	echo "${lib}_-_Win32_${cfg}";;
		parentsrvloc)	echo "${lib}_-_Win32_${cfg}";;
		pcparser|vcparser|qvparser)
			if [ "$lib" = "process_base" -o\
			     "$lib" = "process_quantify" -o\
			     "$lib" = "process_purecov" -o\
			     "$lib" = "process_vcpp" ] ; then
				echo "${lib}_-_Win32_${cfg}"
			else
				echo "${cfg}"
			fi;;
		pksym.index*)
			if [ "$lib" = "symbols_src" -o "$lib" = "symbols_rtl" ] ; then
				echo "${lib}_-_Win32_${cfg}"
			else
				echo "${cfg}"
			fi;;
		sevreporter)  echo "$cfg";;
		*)		echo "$cfg";;
	esac
fi
