#!/bin/sh
# $PROJ_ROOT/makescripts/Unix/install/install-fonts.sh
#
# Called with current directory ./fonts
# Make all necessary subdirectories
# Copy font data from $PROJ_ROOT/data/fonts and store in
# install area

dir=`pwd`
if [ `basename ${dir}` != "fonts" ] ; then
  echo "install-fonts.sh cannot install in $cwd"
  exit 1
fi

# Make sure we can update the install directory
chmod +w .

cp $PROJ_ROOT/data/fonts/Compat.list                                      .
cp $PROJ_ROOT/data/fonts/Synonyms.list                                    .
cp $PROJ_ROOT/data/fonts/aseticons20.bdf                                  .
cp $PROJ_ROOT/data/fonts/aseticons20.pcf                                  .
cp $PROJ_ROOT/data/fonts/aseticons20.snf                                  .
cp -r $PROJ_ROOT/data/fonts/metrics                                       .
cp -r $PROJ_ROOT/data/fonts/dummy                                         .
 
# Link everything else into the platforms
for h in sun4 sun5 hp10 hp700 irix6
do
	mkdir -p ${h}
        cd ${h}

	# First get the platform specific items
        cp -r $PROJ_ROOT/data/fonts/${h}/*                                .

	# Then link the platform specific font files
	if [ "${h}" = "sun5" -o "${h}" = "sun4" ] ; then
		ln -s ../aseticons20.snf					.
	else
		ln -s ../aseticons20.pcf					.
	fi
 
	# Then link files and metrics that are shared on all platforms
        ln -s ../Compat.list                                            .
        ln -s ../Synonyms.list                                          .
        ln -s ../aseticons20.bdf                                        .
        ln -s ../metrics                                                .

        chmod u+w fonts.dir
        cd ..
done
