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
# Usage:  	perl RunDismb.pl -v<VERSION> -t<TEMP_BUILD_LOCATION> -m<full|incr> [-b<BASELINE_VERSION>]
# Description:	run 'dismb' in specified ADMINDIR

require ("$ENV{PSETHOME}/mb/IMUtil.pl");
require ("$ENV{PSETHOME}/mb/dismb_msg.pl");

if ($0 =~ m@RunDismb[^/\\]*@) {
    $FlagsFormat="-v<MODELNAME/VERSION> -m<full|incr> [-b<BASELINE_VERSION>] [-t<BUILD_LOCATION>] [-n] [-V]";
    $THIS_FILE="RunDismb.pl";
}

sub RunDismb(@_)
{
	print_always("Running dismb on model at \$1", $ENV{ADMINDIR});
	&dismb_msg::define_progress_subtasks("RunDismb",
		["analysis pass", 5],
		["parse pass", 74],
		["update pass", 40],
		["finalize pass", 2]);
	&dismb_msg::define_progress_subtasks("parse pass",
		["parse preparations", 1],
		["parse core", 30]);

	my @lp_flags = (&dismb_msg::lp_flags(), "-stdout");
	my($cumg) = $dismb_msg::message_group_id{"compilation units"};
	my @cumg_flags = ("$cumg" ne "" ? ("-compilation_units_message_group", $cumg) : ());
        my @resume_flags = ();
        if ($ENV{MODE}=~m@^resume$@) {
            my $val = (defined($ENV{RESUME_PASS})) ? $ENV{RESUME_PASS} : "yes";
            $val =~ tr [A-Z] [a-z];
	    @resume_flags = ("-MBcontinue", $val);
        }

	if(!run_command("$ENV{PSETHOME}/bin/dismb", "-MBdoPassCache", "no", "-MBincludeAllHdrs", "yes", "-MBgeneratePdf", "yes", "-prefs", "$ENV{ADMINDIR}/prefs/build.prefs", @lp_flags, @cumg_flags, @resume_flags)){&dismb_msg::die("Invocation of dismb failed.");}
	&dismb_msg::report_progress("RunDismb");
	return(1);
}

sub CheckBuild(@_)
{
	my($Finished)=0;
	my($NoOutdatedFiles)=0;
        my($ret) = 1;
	if(!open(ModelBuildLogFile,"$ModelBuildLogFileName")){&dismb_msg::die("Could not open log file, \$1, for reading", $ModelBuildLogFileName);}
	while(<ModelBuildLogFile>)
	{
		if(m@^No new/outdated files detected@){$NoOutdatedFiles=1;}
		if(m@=+\s+FINALIZE\s+end@){$Finished=1;}
	}
	close(ModelBuildLogFile);
	if($NoOutdatedFiles==1){
                my($msg) = "Incremental build with no new files (new model identical to the baseline model!)";
                if (exists($ENV{PublishIfIdentical}) && "$ENV{PublishIfIdentical}" eq "yes" ) {
                        $msg .= ", publishing model anyway.";
	        } else {
		        $ret = 0;
		        $msg .= ", not publishing model.";
                        #report message that "model build" is done even
                        #though all the regular phases of model build are
                        #not being done
                        &dismb_msg::report_progress("model build");
                }
                &dismb_msg::wrn("$msg");
        } else { 
		if($Finished==0){
			&dismb_msg::inf("dismb did not complete all passes.");
			exit(1);
		}
        }
        
        my($summary_file) = "$ENV{ADMINDIR}/tmp/disbuild/summary.txt";
        if ( exists($ENV{BASELINE_VERSION}) && -f $summary_file ) {
	    if ( open (SUMM_F, ">>$summary_file") ) {
                print SUMM_F "baseline:$ENV{BASELINE_VERSION}";
                close SUMM_F;
            }
        }
	&dismb_msg::report_progress("CheckBuild");
	return($ret);
}

if ($0 =~ m@RunDismb[^/\\]*@) {
    GetInstallationValues(@RequiredInstallationVariables,"VFB_DISPLAY");
    CheckArguments("MODELNAME","VERSION","MODE");
    $ENV{ADMINDIR}="$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}";
    SetupEnvironment();

    #$ENV{LM_LICENSE_FILE}="$ENV{DISIM}/lib/build_license.dat";	#This is a HACK
    if (not RunDismb()) {exit(1);}
    if (not CheckBuild()) {exit(1);}
}
else {
    1;
}
