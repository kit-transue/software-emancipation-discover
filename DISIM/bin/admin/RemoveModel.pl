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
# Usage:  	perl StartServer.pl -v<Modelname/Version> -m<start|stop|check> [-n]
# Description:	start pmod_server on model in $ADMINDIR

require ("$ENV{PSETHOME}/mb/IMUtil.pl");
require ("$ENV{PSETHOME}/mb/dismb_msg.pl");

if ($0 =~ m@RemoveModel[^/\\]*@) {
    $FlagsFormat="-v<MODELNAME/VERSION> [-n] [-V]";
    $THIS_FILE="RemoveModel.pl";
}

sub RemoveModelVersion(@_)
{
	my($ModelName,$Version,$Admindir) = @_;
	$RemoveMeFileName="$ENV{DISIM}/model/$ModelName/$Version/tmp/Remove";
	$ServerLockFile="$ENV{DISIM}/tmp/lockfiles/$ModelName/${Version}_$ENV{HOST}.lock";
	if(-r "$ServerLockFile")
	{
		require (FindFile("$ENV{PSETHOME}/mb/ServerAdmin.pl"));
		ServerAdmin("stop", $ModelName, $Version);
	}
	if(0 and ! -r "$RemoveMeFileName")  # intent unknown; disabled
	{
		if(! -e "$ENV{DISIM}/model/$ModelName/$Version"){ShowUsageError("Specified model version, \$1/\$2, does not exist.", $ModelName, $Version);}
		# run_command("/usr/bin/date", ">", $RemoveMeFileName);
		if(!open(RemoveFile,">$RemoveMeFileName")){&dismb_msg::err("can't open remove file \$1", $RemoveMeFileName);}
		print(RemoveFile (GetCurrentDate() . "\n"));
		close(RemoveFile);
		return;
	}
	if ($dismb::is_NT) {
		require (FindFile("$ENV{PSETHOME}/mb/NTPSETServerAdmin.pl"));
		my($service) = PSET_Service_Name($ModelName, $Version);
		my($srvr_port, $srvr_addr) = eval('PSET_Server_Info($service)');
		if ($srvr_port != 0) {
			$ENV{SERVICE_NAME} = $service;
			JustStopModel();
		}
	}
	if(-w "$Admindir/config")
	{
		require File::Path;
		&File::Path::rmtree($Admindir);
		if (-e $Admindir) {
		    &dismb_msg::err("Deletion of \$1 failed.", $Admindir);
		}
	}
	else
	{
		ShowUsageError("\$1 does not exist or you do not have write permission.", $Admindir);
	}
}
sub RemoveEntireModel(@_)
{
	my($ModelName) = @_;
	opendir DIRHANDLE, "$ENV{DISIM}/model/${ModelName}";
	my(@AdmindirArray) = grep { $_ ne '.' and $_ ne '..' } readdir DIRHANDLE;
	closedir DIRHANDLE;
	foreach my $Admindir (@AdmindirArray)
	{
		my($Adminpath) = "$ENV{DISIM}/model/${ModelName}/${Admindir}";
		if (-d $Adminpath) {
			RemoveModelVersion(${ModelName},${Admindir},${Adminpath});
		}
	}
	my($ConfigDir) = "$ENV{DISIM}/template/${ModelName}Template";
	if (-w $ConfigDir)
	{
		require File::Path;
		&File::Path::rmtree($ConfigDir);
		if (-e $ConfigDir) {
		    &dismb_msg::err("Deletion of \$1 failed.", $ConfigDir);
	    	}
	}
	elsif (-e $ConfigDir)
	{
		ShowUsageError("You do not have write permission for \$1.", $ConfigDir);
	}
}

if ($0 =~ m@RemoveModel[^/\\]*@) {
    GetInstallationValues();
    CheckArguments("MODELNAME","VERSION");

    if ($ENV{VERSION} ne "_ALL_") {
	$ENV{ADMINDIR}="$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}";
	&dismb_msg::define_progress_task("removing model version");
	RemoveModelVersion($ENV{MODELNAME},$ENV{VERSION},$ENV{ADMINDIR});
	&dismb_msg::report_progress("removing model version");
    }
    else {
	&dismb_msg::define_progress_task("removing model");
	RemoveEntireModel($ENV{MODELNAME});
	&dismb_msg::report_progress("removing model");
    }
}
else {
    1;
}
