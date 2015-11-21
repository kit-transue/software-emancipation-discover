#######################################################################
# Variables for developing ParaSET
#
# NOTE :: this file is maintained through the vob /paraset .
#         It is copied to /ParaSET as a convenience for times when
#         access outside of a clearcase view is needed.
#
#         DO NOT MAKE CHANGES to /ParaSET contents, they will be lost.
#

# aliases for starting DISCOVER:
alias sdb /net/psales/ModelData1/SUDHA_DISIM/bin/StartBrowser.sh
alias sdx /net/psales/ModelData1/SUDHA_DISIM/bin/StartDevx.sh

alias 2page 'pstext -ld -s 6 -i 24 \!* | lpr'
alias print 'p2ps \!* | lpr -Plp'
alias lsl       "sls -p '%s  %m"\""%d %h %y  %a %H:%M"\""  %t  %N'"
alias lssize    "sls -s %s -p '%s  %u %m"\""%d %h %y"\""  %t  %n'"
alias lsize     lssize
alias lsmod     "sls -p '%4P  %p  %u, %-g  %t  %n'"
alias lsown     "sls -s %u -p '%-u  %s  %t  %n'"

set filec
set history=100
if (! (`uname -s` == HP-UX) ) limit core 0

alias penddir '(to pend;pwd)'

#ClearCase aliases
alias ct cleartool
alias cmake clearmake
alias pwv cleartool pwv -short

#Java stuff
setenv CLASSPATH /usr/local/netscape/java_30

# aliases moved from aset.login to aset.cshrc
# NOTE -
# aliases are quoted, since they depend on env variables set in aset.login
#    aset.login is read AFTER this script

alias to 'setenv target_dir \!*;source ${SET_SHARED_DIR}/bin/.tools/switchdir'

set Arch = `/ParaSET/tools/ParaArch`

if ("$Arch" == "sun5") then
	alias CC '${CC_ROOTDIR}/bin/CC'
else if ("$Arch" == "hp700") then
	alias CC '${CC_ROOTDIR}/bin/CC'
	alias rsh remsh
else if ("$Arch" == "irix5") then
	alias CC '${CC_ROOTDIR}/bin/CC'
else if ("$Arch" == "irix6") then
	alias CC '${CC_ROOTDIR}/bin/CC'
	if ( `uname -r` == "6.5") then
	   source /opt/modules/modules/init/csh
	   module load modules
	   module load MIPSpro
	endif
else if ("$Arch" == "linux2" ) then
	alias CC '${CC_ROOTDIR}/bin/CC'
else
	alias CC OSCC
endif
