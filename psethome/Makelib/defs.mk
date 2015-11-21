PUTDIR=/paraset/build/software/put_directory.sh

buildmode=-system
logdir=/paraset/log
binroot=/paraset/obj/$(Arch)-O/bin/`cat /paraset/admin/version`
installdir=/paraset/psethome/$(Arch)

include /paraset/psethome/Makelib/defs.mk.$(Arch)
