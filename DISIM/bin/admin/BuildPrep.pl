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
# Usage:  	perl PrepareAdmindir.pl -v<VERSION> -t<TEMP_BUILD_LOCATION> -m<full|incr> [-b<BASELINE_VERSION>]
# Description:	Prepare the specified ADMINDIR for building using 'dismb'

require ("$ENV{PSETHOME}/mb/IMUtil.pl");
require ("$ENV{PSETHOME}/mb/dismb_lib.pl");
require ("$ENV{PSETHOME}/mb/dismb_msg.pl");

if ($0 =~ m@BuildPrep[^/\\]*@) {
    $FlagsFormat="-v<MODELNAME/VERSION> -m<full|incr> [-b<BASELINE_VERSION>] [-t<BUILD_LOCATION>] [-n] [-V]";
    $THIS_FILE="BuildPrep.pl";
}
sub CheckBuildLines
{
	if (defined @BuildLogList) {
		require (FindFile("$ENV{PSETHOME}/mb/GetBuildLog.pl"));
		my($index) = 0;
		for $FunArgsRef ( @BuildLogList ) {
			my($FunctionName, @Args) = @$FunArgsRef;
			my($FunctionLine) = @BuildLogRef[$index];
			my($f) = "GetBuildLog::$FunctionName";
			if ($FunctionName ne "Custom" and not (defined &$f)) {
				ShowUsageError("Invalid build log type, \$1, at \$2.", $FunctionName, $FunctionLine);
			}
			$index += 1;
		}
	}
}

sub PrepareAdmindir(@_)
{
	CheckBuildLines();
	print_always("Preparing ADMINDIR, \$1, for dismb build", $ENV{ADMINDIR});
	my(@ExtraFlags)=();
	local($allLogIsFiles);
	if($Verbosity=~m@^high$@){@ExtraFlags=(@ExtraFlags, "-V")}
	if($ExecutionFlag=~m@^off$@){@ExtraFlags=(@ExtraFlags, "-n")}

	ChangeBuildPrefs();
	$ENV{ShowReleased}="off";

        if (!(($ENV{MODE}=~m@^incr$@) && exists($ENV{UserSpecifiedList}))) {
		&dismb_msg::define_progress_subtasks("PrepareAdmindir",
			["GetBuildLog", 1],
			["FilterBuildLog", 1],
			["FilterBuildTable", 1]);
		&dismb_msg::push_new_message_group("compilation units");
		$dismb_msg::message_group_id{"compilation units"} = &dismb_msg::current_auto_message_group;
		&dismb_msg::pop_new_message_group;
		&dismb_msg::push_new_message_group("get build log");
		push @ExtraFlags, &dismb_msg::lp_flags();
		push @ExtraFlags, "-compilation_units_message_group", $dismb_msg::message_group_id{"compilation units"}; 
		my($GetBuildLog_pl) = FindFile("$ENV{PSETHOME}/mb/GetBuildLog.pl");
	        if(exists($ENV{BUILD_VIEW_NAME}))
	        {
		        if(!run_command("cleartool", "setview", "-exec",
					&dismb_lib::arglist_to_str("$ENV{PSETHOME}/bin/disperl",
					"-I$ENV{PSETHOME}/mb",
					$GetBuildLog_pl,
					@ExtraFlags), $ENV{BUILD_VIEW_NAME})){&dismb_msg::die("Running GetBuildLog.pl in a ClearCase view failed.");}
	        }
	        else
	        {
		        if(!run_command("$ENV{PSETHOME}/bin/disperl", "-I$ENV{PSETHOME}/mb",
					$GetBuildLog_pl, @ExtraFlags)){&dismb_msg::die("Running GetBuildLog.pl failed.");}
	        }
		&dismb_msg::pop_new_message_group;
		&dismb_msg::report_progress("GetBuildLog");
		&dismb_msg::push_new_message_group("build log");
		if(!(require (FindFile("$ENV{PSETHOME}/mb/FilterBuildLog.pl")))){&dismb_msg::die("Running FilterBuildLog.pl failed.");}
		FilterBuildLog::Filter();
		&dismb_msg::pop_new_message_group;
		&dismb_msg::report_progress("FilterBuildLog");
		&dismb_msg::push_new_message_group("compiler command lines");
		if(!(require (FindFile("$ENV{PSETHOME}/mb/FilterBuildTable.pl")))){&dismb_msg::die("Running FilterBuildTable.pl failed.");}
		FilterBuildTable::Filter();
		&dismb_msg::pop_new_message_group;
		&dismb_msg::report_progress("FilterBuildTable");

		# Check that there is something to make a model from.
		my($TranslatorListFileName)="$ENV{ADMINDIR}/config/translator.flg";
		if(!open(TranslatorListFile,"$TranslatorListFileName")){
		    &dismb_msg::die("Could not open flags file, \$1.", $TranslatorListFileName);
		}
		if(!<TranslatorListFile>) {&dismb_msg::die("No compile files after filtering the build log.");}
		close(TranslatorListFile);
       }
       else 
       {
                if(!-r "$ENV{UserSpecifiedList}") {
                        ShowUsageError("Couldn't find user specified outdated list file, \$1", $ENV{UserSpecifiedList});  
                }
                if(-z "$ENV{UserSpecifiedList}") {
                        ShowUsageError("Empty user specified outdated list file, \$1", $ENV{UserSpecifiedList});  
                }
		if(!run_command("COPY", $ENV{UserSpecifiedList}, "$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}/config/user_specified.lst")){&dismb_msg::die("Copying user-specified outdated list failed");}
       }
       &dismb_msg::report_progress("PrepareAdmindir");
       return 1;
}

if ($0 =~ m@BuildPrep[^/\\]*@) {
    GetInstallationValues();
    CheckArguments("MODELNAME","VERSION","MODE");
    $ENV{ADMINDIR}="$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}";
    SetupEnvironment("CompileFiles","BUILD=");
    PrepareAdmindir();
}
else {
    1;
}
