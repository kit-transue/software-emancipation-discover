#!sh

LIBROOT='.'
EXEROOT='.'
EXEMAKE='serverspy'

LIBDIRS=""

ADMMAKE='../makescripts/Windows'

. $ADMMAKE/make_config.sh

. $ADMMAKE/make_driver.sh $*
