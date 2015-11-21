include /paraset/psethome/defs.mk

default: all

FORCE:

all: package_execs psr_files xport.sh xDefaults

package_execs: package pksym.index pksym.index.rtl qvparser gxpackage pcparser vcparser helper

psr_files: xagport.psr xcgport.psr xclport.psr

package: FORCE
	make_package $(buildmode) > $(logdir)/make_package.log 2>&1
	ln $(binroot)/package .

pksym.index: FORCE
	make_pksym.index $(buildmode) > $(logdir)/make_pksym.index.log 2>&1
	ln $(binroot)/pksym.index .

pksym.index.rtl: FORCE
	make_pksym.index.rtl $(buildmode) > $(logdir)/make_pksym.index.rtl.log 2>&1
	ln $(binroot)/pksym.index.rtl .

qvparser: FORCE
	make_qvparser $(buildmode) > $(logdir)/make_qvparser.log 2>&1
	ln $(binroot)/qvparser .

gxpackage: FORCE
	make_gxpackage $(buildmode) > $(logdir)/make_gxpackage.log 2>&1
	ln $(binroot)/gxpackage .

pcparser: FORCE
	make_pcparser $(buildmode) > $(logdir)/make_pcparser.log 2>&1
	ln $(binroot)/pcparser .

vcparser: FORCE
	make_vcparser $(buildmode) > $(logdir)/make_vcparser.log 2>&1
	ln $(binroot)/vcparser .

helper: FORCE
	make_helper $(buildmode) > $(logdir)/make_helper.log 2>&1
	ln $(binroot)/helper .

xagport.psr: /paraset/src/package/batch/psr/xagport.psr
	cp /paraset/src/package/batch/psr/xagport.psr .

xcgport.psr: /paraset/src/package/batch/psr/xcgport.psr
	cp /paraset/src/package/batch/psr/xcgport.psr .

xclport.psr: /paraset/src/package/batch/psr/xclport.psr
	cp /paraset/src/package/batch/psr/xclport.psr .

xport.sh: /paraset/src/package/batch/scripts/xport.sh
	cp /paraset/src/package/batch/scripts/xport.sh .

xDefaults: /paraset/src/package/gui/X/xDefaults
	cp /paraset/src/package/gui/X/xDefaults .
