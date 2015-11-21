##########################################################################
# Copyright (c) 2015, Synopsys, Inc.                                     #
# All rights reserved.                                                   #
#                                                                        #
# Redistribution and use in source and binary forms, with or without     #
# modification, are permitted provided that the following conditions are #
# met:                                                                   #
#                                                                        #
# 1. Redistributions of source code must retain the above copyright      #
# notice, this list of conditions and the following disclaimer.          #
#                                                                        #
# 2. Redistributions in binary form must reproduce the above copyright   #
# notice, this list of conditions and the following disclaimer in the    #
# documentation and/or other materials provided with the distribution.   #
#                                                                        #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    #
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      #
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  #
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   #
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, #
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       #
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  #
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  #
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    #
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  #
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   #
##########################################################################
# Usage:  	perl CreateModel.pl -mfull [-t<BUILD_LOCATION>] [-n] [-V]
# Description:	setup, build, and publish a new model version

require ("$ENV{PSETHOME}/mb/IMUtil.pl");
require ("$ENV{PSETHOME}/mb/dismb_lib.pl");
$FlagsFormat="-v<MODELNAME/VERSION> -m<full|incr|resume> [-b<BASELINE_VERSION>] [-t<BUILD_LOCATION>] [-p<PASS>] [-n] [-V]";
$THIS_FILE="CreateModel.pl";

sub DefineExpectedProgress {
	&dismb_msg::define_progress_task("model build");
	&dismb_msg::define_progress_subtasks("model build",
		["CreateAdmindir", 5],
		["PrepareAdmindir", 8],
		["RunDismb", 114],
		["CheckBuild", 2],
		["PublishAndReleaseModel", 11]);
}

sub DiagnoseAndCleanUpAndExit {
    if (defined $@) {print $@;}
    CleanUpAndExit(@_);
}

sub ModelAdmin(@_)
{
	eval {CheckModelBuildLicense()} or DiagnoseAndCleanUpAndExit(1);
	if (!exists($ENV{TEMPBUILD})) {
	    &dismb_msg::msg("catastrophe", "TEMPBUILD is not defined in lib/Configuration.txt.");
	    CleanUpAndExit(1);
	}
	if(! -w $ENV{TEMPBUILD})
	{
	    if (not mkdir $ENV{TEMPBUILD}, 0777) {
		&dismb_msg::msg("catastrophe", "TEMPBUILD (\$1) is not a writable directory.", $ENV{TEMPBUILD});
		CleanUpAndExit(1);
	    }
	}

	&DefineExpectedProgress;

        $ENV{ADMINDIR}="$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}";

        #skip creating/preparing AdminDir in resume mode
        if ($ENV{MODE}!~m@^resume$@) {
	    print_always("Creating new model of \$1/\$2, at \$3", $ENV{MODELNAME}, $ENV{VERSION}, $ENV{TEMPBUILD});
	    require(FindFile("$ENV{PSETHOME}/mb/CreateAdmindir.pl"));
	    eval {CreateAdmindir()} or DiagnoseAndCleanUpAndExit(1);

	    SetupEnvironment("CompileFiles","BUILD=");

	    require(FindFile("$ENV{PSETHOME}/mb/BuildPrep.pl"));
	    eval {PrepareAdmindir()} or DiagnoseAndCleanUpAndExit(1);
        } else {
            SetupEnvironment();
            my($TranslatorListFileName) = "$ENV{ADMINDIR}/config/translator.flg";
            if ( !-z $TranslatorListFileName && -d "$ENV{ADMINDIR}/tmp/disbuild" ) {
		print_always("Resuming model build at \$1. Please note that the existing parser flags (\$2) will be used.", $ENV{ADMINDIR}, $TranslatorListFileName);
            } else {
		&dismb_msg::msg("catastrophe", "Cannot resume model build at this stage. Please use regular build options (-mfull/-mincr).");
		CleanUpAndExit(1);
            }
        }

	require(FindFile("$ENV{PSETHOME}/mb/RunDismb.pl"));
	eval {RunDismb()} or DiagnoseAndCleanUpAndExit(1);
	eval {CheckBuild()} or DiagnoseAndCleanUpAndExit(1);

	require(FindFile("$ENV{PSETHOME}/mb/ReleaseModel.pl"));
	eval {PublishAndReleaseModel()} or DiagnoseAndCleanUpAndExit(1);

	require(FindFile("$ENV{PSETHOME}/mb/CreateSummary.pl"));
	eval {CreateSummary()} or DiagnoseAndCleanUpAndExit(1);
	CleanUpAndExit(0);
}

GetInstallationValues(@RequiredInstallationVariables,"VFB_DISPLAY");
CheckArguments("MODELNAME","VERSION","MODE");
$ENV{ShowReleased}="off";
ModelAdmin();
