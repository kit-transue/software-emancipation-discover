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
# Usage:  	perl RunQAR.pl -v<VERSION>
# Description:	run QAR in batch mode

require ("$ENV{PSETHOME}/mb/IMUtil.pl");

$FlagsFormat="-v<MODELNAME/VERSION> [-n] [-V]";

sub RunQAR(@_)
{
	&dismb_msg::define_progress_task("QAR");
	my($TCLScriptName)="$ENV{PSETHOME}/lib/batch_qar.dis";
	my($PrefsFileName)="$ENV{ADMINDIR}/prefs/build.prefs";
	my($UserPDFName)=GetExpandedPref($PrefsFileName,pdfFileUser);

	### Because you just gotta have a home project ###
	my($FirstProjectName)=$ProjectNameList[0];
	my(@CommandLineArguments)=("-pdf", $UserPDFName, "-prefs", $PrefsFileName, "-home", "/Home", "-batch", "-source", $TCLScriptName);

	CreateUserArea();
	if (run_command("$ENV{PSETHOME}/bin/model_server", @CommandLineArguments)) {
		&dismb_msg::report_progress("QAR");
	}
}

$THIS_FILE="RunQAR.pl";
GetInstallationValues(@RequiredInstallationVariables,"VFB_DISPLAY");
CheckArguments("MODELNAME","VERSION");

$ENV{ADMINDIR}="$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}";
SetupEnvironment();
RunQAR();

