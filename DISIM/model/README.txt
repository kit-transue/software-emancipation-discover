All models are stored here.  Any given admindir is located at:
	$DISIM/model/$MODELNAME/$VERSION

In some cases it it necessary to store models on separate 
partitions/drives, in which case, the subdirectories of this 
directory can be linked to the desired partitions.

ex:	store modelname 'AA' on /net/mach1/local0/AA_models
	store modelname 'BB' on /net/mach2/local0/BB_models

cd $DISIM/model
ln -s /net/mach1/local0/AA_models ./AA
ln -s /net/mach2/local0/BB_models ./BB

