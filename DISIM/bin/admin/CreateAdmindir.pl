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
# Usage:  	perl CreateAdmindir.pl -v<VERSION> -tTEMP_BUILD_LOCATION
# Description:	Create an ADMINDIR from the corresponding template

require ("$ENV{PSETHOME}/mb/IMUtil.pl");
require ("$ENV{PSETHOME}/mb/dismb_lib.pl");
require ("$ENV{PSETHOME}/mb/dismb_msg.pl");

if ($0 =~ m@CreateAdmindir[^/\\]*@) {
    $FlagsFormat="-v<MODELNAME/VERSION> -m<full|incr> [-b<BASELINE_VERSION>] [-t<BUILD_LOCATION>] [-n] [-V]";
    $THIS_FILE="CreateAdmindir.pl";
}

sub CreateAdmindir(@_)
{
	print_always("Creating new ADMINDIR of model, \$1, at \$2", "$ENV{MODELNAME}/$ENV{VERSION}", $ENV{TEMPBUILD});
	if(! -w "$ENV{TEMPBUILD}")
	{
		ShowUsageError("\$1 is not a writable directory.", $ENV{TEMPBUILD});
	}
	if(! -w "$ENV{TEMPBUILD}/$ENV{MODELNAME}")
	{
		print_always("Making directory \$1 for temporary build.", "$ENV{TEMPBUILD}/$ENV{MODELNAME}");
		if(!mkdir("$ENV{TEMPBUILD}/$ENV{MODELNAME}",0755)){ShowUsageError("Failed to create \$1/\$2", $ENV{TEMPBUILD}, $ENV{MODELNAME});}
	}
	if(! -w "$ENV{DISIM}/model/$ENV{MODELNAME}")
	{
		print_always("Making directory \$1 for model publishing.", "$ENV{DISIM}/model/$ENV{MODELNAME}");
		if(!mkdir("$ENV{DISIM}/model/$ENV{MODELNAME}",0755)){ShowUsageError("Failed to create \$1/model/\$2", $ENV{DISIM}, $ENV{MODELNAME});}
	}
	if(-r "$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}")
	{
		print_always("Removing previous copy of \$1 in \$2.", "$ENV{MODELNAME}/$ENV{VERSION}", $ENV{TEMPBUILD});
		if(!run_command("RMRF", "$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}")
		                or (-d "$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}")) {
		    &dismb_msg::die("Failed to remove previous copy of \$1", 
				            "$ENV{MODELNAME}$ENV{VERSION}");
		}
	}
	if(!run_command("CPRF", "$ENV{DISIM}/template/$ENV{MODELNAME}Template", "$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}")){&dismb_msg::die("Failed to create \$1/\$2/\$3", $ENV{TEMPBUILD}, $ENV{MODELNAME}, $ENV{VERSION});}
	if(exists($ENV{BASELINE_MODEL_LOCATION}))
	{
		print_always("Copying baseline model from \$1 to \$2.", $ENV{BASELINE_MODEL_LOCATION}, "$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}");
		if(!run_command("CPRF", "$ENV{BASELINE_MODEL_LOCATION}/model", "$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}/model")){&dismb_msg::die("Failed to copy baseline model");}
		if(!run_command("COPY", "$ENV{BASELINE_MODEL_LOCATION}/config/file_attributes.lst", "$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}/config")){&dismb_msg::die("Failed to copy baseline file attributes");}
                if(-f "$ENV{BASELINE_MODEL_LOCATION}/config/other_dependencies.lst") 
                {
                        run_command("COPY", "$ENV{BASELINE_MODEL_LOCATION}/config/other_dependencies.lst", "$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}/config");
		}
		if(!run_command("COPY", "$ENV{BASELINE_MODEL_LOCATION}/pdf/build.pdf", "$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}/pdf")){&dismb_msg::die("Failed to copy baseline pdf");}
	}
	if(! -e "$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}/log")
	{
		if(!mkdir("$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}/log",0777)){ShowUsageError("Failed to create \$1/\$2/\$3/log", $ENV{TEMPBUILD}, $ENV{MODELNAME}, $ENV{VERSION});}
	}
	if(! -e "$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}/tmp")
	{
		if(!mkdir("$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}/tmp",0777)){ShowUsageError("Failed to create \$1/\$2/\$3/tmp", $ENV{TEMPBUILD}, $ENV{MODELNAME}, $ENV{VERSION});}
	}
	if(! -e "$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}/model")
	{
		if(!mkdir("$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}/model",0777)){ShowUsageError("Failed to create \$1/\$2/\$3/model", $ENV{TEMPBUILD}, $ENV{MODELNAME}, $ENV{VERSION});}
	}
	if(!run_command("ECHO", "$ENV{StartDate}", ">", "$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}/tmp/Building")){&dismb_msg::die("Failed to lock");}

	if(exists($ENV{TEST_PSETHOME}))
	{
		run_command("ECHO", "VAR=$ENV{HostType}_PSETHOME:$ENV{TEST_PSETHOME}", ">", ">", "$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}/config/Setup.txt");
	}
	&dismb_msg::report_progress("CreateAdmindir");
	return(1);
}

if ($0 =~ m@CreateAdmindir[^/\\]*@) {
    GetInstallationValues();
    CheckArguments("MODELNAME","VERSION","MODE");
    $ENV{ADMINDIR}="$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}";
    CreateAdmindir();
}
else {
    1;
}
