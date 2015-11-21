#!sh

#
# make command
#
echo ADMMAKE= ${ADMMAKE}
MAKEEXE="${ADMMAKE}/../bin/win32/gmake.exe"
MAKEOPT='-k'
MAKEMSG='echo'
GET_CFG="${ADMMAKE}/get_cfg.sh"
PATH2BS=${BUILD_SRC_ROOT}

vcvars32 x86
export MAKEEXE MAKEOPT PATH2BS
