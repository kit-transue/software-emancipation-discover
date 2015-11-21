include ../new-rules/paths.mk

all: common nonwin32

common:
	(cd aset_CCcc && make BUILD_TYPE=DEBUG)
	(cd dis_path && make BUILD_TYPE=DEBUG)
	(cd dish && make BUILD_TYPE=DEBUG)
	(cd ifext && make BUILD_TYPE=DEBUG)
	(cd lock && make BUILD_TYPE=DEBUG)
	(cd model_server && make BUILD_TYPE=DEBUG)
	(cd nameserver && make BUILD_TYPE=DEBUG)
	(cd cmdish/src && make BUILD_TYPE=DEBUG)
	(cd cmdriver/src && make BUILD_TYPE=DEBUG)
	(cd DevXLauncher/src && make BUILD_TYPE=DEBUG)
	(cd emacsAdapter && make BUILD_TYPE=DEBUG)
	(cd mrg_diff && make BUILD_TYPE=DEBUG)
	(cd mrg_update && make BUILD_TYPE=DEBUG)
	(cd simplify/src && make BUILD_TYPE=DEBUG)
	(cd flex && make BUILD_TYPE=DEBUG)
#	(cd mrg_ui && make BUILD_TYPE=DEBUG)


ifeq ($(mc_platform),mingw)
nonwin32:
else
nonwin32:
	(cd devxAdapter && make BUILD_TYPE=DEBUG)
	(cd model_server/xref/test && make BUILD_TYPE=DEBUG)
	(cd model_server/swig && make BUILD_TYPE=DEBUG)
	(cd if_diff && make BUILD_TYPE=DEBUG)
endif
