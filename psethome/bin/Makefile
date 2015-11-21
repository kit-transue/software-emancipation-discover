include /paraset/psethome/Makelib/defs.mk

default: all

FORCE:

all: discover_execs package server_execs tools misc_execs dubious_execs X_execs dislite_execs frame_execs dat_files PT_files lm_tools galaxy_files diff_execs rcs_execs support_utils config_execs hyperhelp copy_reportplates ptest_checkpmod atria_filter

discover_execs: aset pmod_server pset_server aset_CCcc ifext dis_path disperl dismb sevreporter aset_jfe paracancel dis_sql mrg_update mrg_diff mrg_ui simplify tests/testlink checklist

vr_files: 

package: FORCE
	cd $@ ; $(MAKE)

server_execs: nameserv dish2 serverspy admintool 

tools: apply_patch discover check_opts Tutor.setup discover.reset realname DISarch pdfgen Learn.install Learn.setup DISos slay cexec setwait

misc_execs: pset-which paratotal generic/egrep splash dis_lock discrypt cccc flex

dubious_execs: mapper dis_config template_rules get_build_pdf dynamic_pdf_generator

X_execs: xopen xscan disXvfb

dislite_execs: dislite disviper disvim vim

frame_execs: frame_api frame_srvr

dat_files: qhelp.dat menus.dat compilers.dat

PT_files: dis_PT_get_regions.pl dis_DL_get_regions.pl

lm_tools: lmgrd setechd lmutil lmcksum lmdiag lmdown lmhostid lminstall lmremove lmreread lmstat lmswitchr lmver

galaxy_files: gui/shutdown.tcl aset.vr pset_server.vr mrg_ui.vr splash.vr gala.vr gala vdasserv

diff_execs: cmp sdiff diff diff3

rcs_execs: ci co ident merge rcs rcsdiff rcsmerge rlog

support_utils: pset_file_status pset_loc pset_proj_status pset_status

config_execs: configshell configurator

aset: FORCE
	make_aset $(buildmode) > $(logdir)/make_aset.log 2>&1
	cp $(binroot)/aset .

pmod_server: FORCE
	make_pmod_server $(buildmode) > $(logdir)/make_pmod_server.log 2>&1
	cp $(binroot)/pmod_server .

pset_server: FORCE
	make_pset_server $(buildmode) > $(logdir)/make_pset_server.log 2>&1
	cp $(binroot)/pset_server .

aset_CCcc: FORCE
	make_aset_CCcc $(buildmode) > $(logdir)/make_aset_CCcc.log 2>&1
	cp $(binroot)/aset_CCcc .

ifext: FORCE
	make_ifext $(buildmode) > $(logdir)/make_ifext.log 2>&1
	cp $(binroot)/ifext .

dis_path: FORCE
	make_dis_path $(buildmode) > $(logdir)/make_dis_path.log 2>&1
	cp $(binroot)/dis_path .

disperl: FORCE
	make_disperl $(buildmode) > $(logdir)/make_disperl.log 2>&1
	cp $(binroot)/disperl .

dismb: FORCE
	make_dismb $(buildmode) > $(logdir)/make_dismb.log 2>&1
	cp $(binroot)/dismb .

sevreporter: FORCE
	make_sevreporter $(buildmode) > $(logdir)/make_sevreporter.log 2>&1
	cp $(binroot)/sevreporter .

aset_jfe: FORCE
	make_aset_jfe $(buildmode) > $(logdir)/make_aset_jfe.log 2>&1
	cp $(binroot)/aset_jfe .

paracancel: FORCE
	make_paracancel $(buildmode) > $(logdir)/make_paracancel.log 2>&1
	cp $(binroot)/paracancel .

dis_sql: FORCE
	make_dis_sql $(buildmode) > $(logdir)/make_dis_sql.log 2>&1
	cp $(binroot)/dis_sql .

mrg_update: FORCE
	make_mrg_update $(buildmode) > $(logdir)/make_mrg_update.log 2>&1
	cp $(binroot)/mrg_update .

mrg_diff: FORCE
	make_mrg_diff $(buildmode) > $(logdir)/make_mrg_diff.log 2>&1
	cp $(binroot)/mrg_diff .

mrg_ui: FORCE
	make_mrg_ui $(buildmode) > $(logdir)/make_mrg_ui.log 2>&1
	cp $(binroot)/mrg_ui .

simplify: FORCE
	make_simplify $(buildmode) > $(logdir)/make_simplify.log 2>&1
	cp $(binroot)/simplify .

gala: FORCE
	make_gala $(buildmode) > $(logdir)/make_gala.log 2>&1
	cp $(binroot)/gala .

vdasserv: /paraset/bin/$(Arch)/vdasserv
	cp /paraset/bin/$(Arch)/vdasserv .

tests/testlink: FORCE
	mkdir -p tests
	make_testlink $(buildmode) > $(logdir)/make_testlink.log 2>&1
	cp $(binroot)/testlink ./tests

checklist: FORCE
	make_checklist $(buildmode) > $(logdir)/make_checklist.log 2>&1
	cp $(binroot)/checklist .

aset.vr: aset
	cp $(binroot)/aset.vr .

pset_server.vr: pset_server
	cp $(binroot)/pset_server.vr .

mrg_ui.vr: mrg_ui
	cp $(binroot)/mrg_ui.vr .

gala.vr: gala
	cp $(binroot)/gala.vr .

nameserv: FORCE
	cd /paraset/src/nameserver/src ; $(MAKE) -C gnu -k nameserv > $(logdir)/make_nameserv.log 2>&1
	cp /paraset/src/nameserver/src/nameserv .

dish2: FORCE
	cd /paraset/src/dish/src ; $(MAKE) -C gnu -k dish2 > $(logdir)/make_dish2.log 2>&1
	cp /paraset/src/dish/src/dish2 .

serverspy: FORCE
	cd /paraset/src/serverspy ; $(MAKE) -C gnu -k serverspy > $(logdir)/make_serverspy.log 2>&1
	cp /paraset/src/serverspy/src/serverspy .

admintool: FORCE
	cd /paraset/src/admintool ; $(MAKE) -C gnu -k admintool > $(logdir)/make_admintool.log 2>&1
	cp /paraset/src/admintool/src/admintool .

apply_patch: /paraset/tools/apply_patch
	cp /paraset/tools/apply_patch .

discover: /paraset/tools/discover
	cp /paraset/tools/discover .
	chmod a+x ./discover

check_opts: /paraset/tools/check_opts
	cp /paraset/tools/check_opts .

Tutor.setup: /paraset/tools/Tutor.setup
	cp /paraset/tools/Tutor.setup .

discover.reset: /paraset/tools/discover.reset
	cp /paraset/tools/discover.reset .

realname: /paraset/tools/realname
	cp /paraset/tools/realname .

DISarch: /paraset/tools/DISarch
	cp /paraset/tools/DISarch .

Learn.install: /paraset/tools/Learn.install
	cp /paraset/tools/Learn.install .

Learn.setup: /paraset/tools/Learn.setup
	cp /paraset/tools/Learn.setup .

DISos: /paraset/tools/DISos
	cp /paraset/tools/DISos .

slay: /paraset/tools/slay.sh
	cp /paraset/tools/slay.sh .

cexec: /paraset/tools/cexec
	cp /paraset/tools/cexec .

setwait: /paraset/tools/setwait
	cp /paraset/tools/setwait .

pset-which: /paraset/bin/$(Arch)/pset-which
	cp /paraset/bin/$(Arch)/pset-which .

paratotal: FORCE
	error "need to write build script for this"

xscan:
	cd /paraset/src/xutil ; $(MAKE) -C gnu -k xscan > $(logdir)/make_xscan.log 2>&1
	cp /paraset/src/xutil/xscan .

xopen:
	cd /paraset/src/xutil ; $(MAKE) -C gnu -k xopen > $(logdir)/make_xopen.log 2>&1
	cp /paraset/src/xutil/xopen .

disXvfb:
	cp /paraset/bin/$(Arch)/disXvfb .

generic/egrep:
	mkdir -p generic
	cp /paraset/src/grep-2.1/$(Arch)/src/egrep ./generic

dislite: /paraset/src/paraset/dislite/scripts/dislite
	cp /paraset/src/paraset/dislite/scripts/dislite .

disviper: /paraset/src/paraset/dislite/scripts/disviper
	cp /paraset/src/paraset/dislite/scripts/disviper .

disvim: /paraset/src/paraset/dislite/scripts/disvim
	cp /paraset/src/paraset/dislite/scripts/disvim .

vim: FORCE
	cd /paraset/src/vim-5.3/$(Arch) ; $(MAKE)
	cp /paraset/src/vim-5.3/$(Arch)/vim .

frame_api: /paraset/bin/$(Arch)/frame_api
	cp /paraset/bin/$(Arch)/frame_api .

frame_srvr: /paraset/bin/$(Arch)/frame_srvr
	cp /paraset/bin/$(Arch)/frame_srvr .

splash: FORCE
	error "Need to write makefile for this"

splash.vr: splash
	cp /paraset/src/clients/splash/src/splash.vr .

dis_lock: FORCE
	cd /paraset/src/lock/src ; $(MAKE)
	cp /paraset/src/lock/src/dis_lock .

discrypt: FORCE
	cd /paraset/src/discrypt ; $(MAKE) -C gnu
	cp /paraset/src/discrypt/discrypt .

cccc: FORCE
	cd /paraset/src ; $(MAKE) cccc
	cp /paraset/tools/cccc .

flex: FORCE
	cd /paraset/src/flex/$(Arch) ; $(MAKE) -C gnu
	cp /paraset/src/flex/$(Arch)/flex .

qhelp.dat: /paraset/src/clients/gala/src/qhelp.dat
	cp /paraset/src/clients/gala/src/qhelp.dat .

menus.dat: /paraset/src/clients/gala/src/menus.dat
	cp /paraset/src/clients/gala/src/menus.dat .

compilers.dat: /paraset/src/clients/gala/src/compilers.dat
	cp /paraset/src/clients/gala/src/compilers.dat .

dis_PT_get_regions.pl: /paraset/src/DefectProducts/ProblemTracking/dis_PT_get_regions.pl
	cp /paraset/src/DefectProducts/ProblemTracking/dis_PT_get_regions.pl .

dis_DL_get_regions.pl: /paraset/src/DefectProducts/ProblemTracking/dis_DL_get_regions.pl
	cp /paraset/src/DefectProducts/ProblemTracking/dis_DL_get_regions.pl .

lmgrd: 
	cp /paraset/src/FLEXlm/flexlm/current/$(LMGRARCH)/lmgrd       .

setechd:
	cp /paraset/src/FLEXlm/flexlm/current/$(LMGRARCH)/setechd     .

lmutil:
	cp /paraset/src/FLEXlm/flexlm/current/$(LMGRARCH)/lmutil      .

lmcksum: lmutil
	ln -s ./lmutil lmcksum

lmdiag: lmutil
	ln -s ./lmutil lmdiag

lmdown: lmutil
	ln -s ./lmutil lmdown

lmhostid: lmutil
	ln -s ./lmutil lmhostid

lminstall: lmutil
	ln -s ./lmutil lminstall

lmremove: lmutil
	ln -s ./lmutil lmremove

lmreread: lmutil
	ln -s ./lmutil lmreread

lmstat: lmutil
	ln -s ./lmutil lmstat

lmswitchr: lmutil
	ln -s ./lmutil lmswitchr

lmver: lmutil
	ln -s ./lmutil lmver

gui/shutdown.tcl: /paraset/src/clients/gala/tclcode/shutdown.tcl
	mkdir -p gui
	cp /paraset/src/clients/gala/tclcode/shutdown.tcl ./gui
	error "Galaxy, should be gone real soon"

cmp: FORCE
	cd /paraset/src/diff/$(Arch) ; $(MAKE) cmp
	cp /paraset/src/diff/$(Arch)/cmp .

sdiff: FORCE
	cd /paraset/src/diff/$(Arch) ; $(MAKE) sdiff
	cp /paraset/src/diff/$(Arch)/sdiff .

diff: FORCE
	cd /paraset/src/diff/$(Arch) ; $(MAKE) diff
	cp /paraset/src/diff/$(Arch)/diff .

diff3: FORCE
	cd /paraset/src/diff/$(Arch) ; $(MAKE) diff3
	cp /paraset/src/diff/$(Arch)/diff3 .

ci: /paraset/src/rcs/$(Arch)/src/ci
	cp /paraset/src/rcs/$(Arch)/src/ci .

co: /paraset/src/rcs/$(Arch)/src/co
	cp /paraset/src/rcs/$(Arch)/src/co .

ident: /paraset/src/rcs/$(Arch)/src/ident
	cp /paraset/src/rcs/$(Arch)/src/ident .

merge: /paraset/src/rcs/$(Arch)/src/merge
	cp /paraset/src/rcs/$(Arch)/src/merge .

rcs: /paraset/src/rcs/$(Arch)/src/rcs
	cp /paraset/src/rcs/$(Arch)/src/rcs .

rcsdiff: /paraset/src/rcs/$(Arch)/src/rcsdiff
	cp /paraset/src/rcs/$(Arch)/src/rcsdiff .

rcsmerge: /paraset/src/rcs/$(Arch)/src/rcsmerge
	cp /paraset/src/rcs/$(Arch)/src/rcsmerge .

rlog: /paraset/src/rcs/$(Arch)/src/rlog
	cp /paraset/src/rcs/$(Arch)/src/rlog .

pset_file_status: /paraset/src/support/pset_file_status
	cp /paraset/src/support/pset_file_status .

pset_loc: /paraset/src/support/pset_loc
	cp /paraset/src/support/pset_loc .

pset_proj_status: /paraset/src/support/pset_proj_status
	cp /paraset/src/support/pset_proj_status .

pset_status: FORCE
	cd /paraset/src/support ; $(MAKE) -C gnu pset_status
	cp /paraset/src/support/pset_status .

configshell: FORCE
	error "Need to write a makefile for this;  look in /paraset/obj/sun5-O/config/..."

configurator: FORCE
	error "Need to write a makefile for this;  look in /paraset/obj/sun5-O/config/..."

hyperhelp: $(HHHOME)/bin/hyperhelp
	cp $(HHHOME)/bin/hyperhelp .

copy_reportplates: /paraset/build/software/copy_reportplates
	cp /paraset/build/software/copy_reportplates .

ptest_checkpmod: /paraset/pmod/ptest_checkpmod
	cp /paraset/pmod/ptest_checkpmod .

atria_filter: /paraset/src/config/config/clearcase/src/atria_filter
	cp /paraset/src/config/config/clearcase/src/atria_filter .
