############################################################
	DISIM 2.0
############################################################

Changes from DISIM version 1.3
==============================
* DISIM no longer uses a project definition file (PDF) to
  guide the Parse pass.  As a result, all header files are
  automatically incorporated into the model.

* DISIM uses a new method of dermining parser flags, which
  does not require adjustments to FilterBuildLog.pl.

* When running DISIM scripts, be sure to use
  $PSETHOME/bin/disperl or $PSETHOME/bin/disperl.exe.

* This version of DISIM does _not_ support incremental model
  build or parallel update yet.  Retain your previous
  DISIM if you need either of these features.


Merging from older DISIM templates
==================================
* Stop all servers as explained in Instructions.html

* ALWAYS ALWAYS backup the original DISIM template.
  perhaps just change the name to DISIM_OLD.

* Each line of configuration file located at
  $DISIM/lib/Configuration.txt should be merged with
  the original with the possible exception of the very
  last section.

* Most of the changes for each model type should be
  isolated to 3 or 4 files in the model's ADMINDIR template
  located in $DISIM/template/${MODELNAME}Template 

* Copy the new ModelTemplate to the each of the original
  template names.

* Each of these files should be merged into the new
  template in the following ways:

	config/Setup.txt:
	-----------------
	* Compare the following lines and modify them
	  according to the original settings:

	VAR=MODELNAME:src
	PROJECT=ROOT
	EXTENSION=CompileFiles:c cxx cpp cc C java sql

	* The lines for ROOT=sharedSrcRoot and
	  EXTENSION=NonCompileFiles are no longer relevant.

	config/GetBuildLog.pl:
	----------------------
	* Merge changes.

	* If using the routine, ParseDSX(), make sure to
	  copy the file config/DSXList.txt into the new
	  template.

	config/FilterBuildLog.pl:
	-------------------------
	* This file has been changed to work with no
	  adjustments usually necessary.  If you run into
	  trouble with compiler options, see the revised
	  documentation.


