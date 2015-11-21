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
# Usage:  	perl ServerAdmin.pl -v<Modelname/Version> -m<start|stop|check|fix> [-n]
# Description:	start serverspy on model in $ADMINDIR

require ("$ENV{PSETHOME}/mb/IMUtil.pl");
require ("$ENV{PSETHOME}/mb/dismb_msg.pl");

if ($0 =~ m@ServerAdmin[^/\\]*@) {
    $FlagsFormat="-v<MODELNAME/VERSION> -m<start|stop|check|fix> [-n] [-V]";
    $THIS_FILE = ServerAdmin.pl;
}

sub StartServer(@_)
{
	print_always("Starting server for \$1/\$2", $ENV{MODELNAME}, $ENV{VERSION});
	print_verbose("START:	\$1/\$2", $ENV{MODELNAME}, $ENV{VERSION});

	$ENV{ADMINDIR}="$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}";
	SetupEnvironment();
	CreateUserArea();
	$ServerState=GetServerState("$ENV{MODELNAME}/$ENV{VERSION}");

	if(! -d "$ServerLockFileDir")
	{
		print_verbose("Creating lockfile directory for model \$1 at \$2.", $ENV{MODELNAME}, $ServerLockFileDir);
		if(! mkdir("$ServerLockFileDir",0777)){ShowUsageError("Failed to make directory \$1/tmp/lockfiles/\$2.", $ENV{DISIM}, $ENV{MODELNAME});}
	}
	if($ServerState!~m@Dead@)
	{
		StopServer($ServerState);
	}
	if(! -r "$ENV{ADMINDIR}/tmp/Release")
	{
		ShowUsageError("This version has not been released.");
	}
	if(! @ProjectNameList)
	{
		ShowUsageError("Could not find any projects listed in setup for this model.");
	}
	if($ENV{privateModelRoot}=~m@^\s*$@) {
		ShowUsageError("Private model root should be specified.");
 	} elsif (! -d "$ENV{privateModelRoot}") {
		print_verbose("Creating private model root directory...");
		use File::Path; 
		if(! mkpath("$ENV{privateModelRoot}",0,0777)){ShowUsageError("Failed to make directory \$1.", $ENV{privateModelRoot});}
  	}
	my($PREFSArgument)="$ENV{ADMINDIR}/prefs/build.prefs";
	if($ENV{HostType}=~m@^NT$@)
	{
		if(!chdir($ENV{ADMINDIR})) {&dismb_msg::err("Failed to change directory to ADMINDIR, \$1", $ENV{ADMINDIR});}
		$PREFSArgument="prefs/build.prefs";
	}
	$PDFArgument=GetPref("$ENV{ADMINDIR}/prefs/build.prefs",defaultPDF);
	$PDFArgument=~s@\$ADMINDIR@$ENV{ADMINDIR}@g;
	$PDFArgument=~s@\%ADMINDIR\%@$ENV{ADMINDIR}@g;
	if($ENV{HostType}!~m@^NT$@) {
	    # if(!run_command("FORK", "$ENV{PSETHOME}/bin/serverspy", "-service", $ENV{PMODServiceName}, "-pdf", $PDFArgument, "-prefs", $PREFSArgument, "-logfile", "$ENV{DISIM}/tmp/DisliteUsage.log", ">", ">", "$ENV{DISIM}/tmp/Server.log", "2", ">&", "1")){&dismb_msg::err("serverspy failed.");} # Changed due to a failure of GetBuildLog.pl on fridge -- see that file.
	    if(!run_command("FORK", "$ENV{PSETHOME}/bin/serverspy", "-service", $ENV{PMODServiceName}, "-pdf", $PDFArgument, "-prefs", $PREFSArgument, "-logfile", "$ENV{DISIM}/tmp/DisliteUsage.log", ">>", "$ENV{DISIM}/tmp/Server.log", "2>&", "1")){&dismb_msg::err("serverspy failed.");}
	}
	else {
	    # ServerSpy on NT has a problem with redirection.
	    # if(!run_command("FORK", "$ENV{PSETHOME}/bin/serverspy", "-service", $ENV{PMODServiceName}, "-pdf", $PDFArgument, "-prefs", $PREFSArgument, "-logfile", "$ENV{DISIM}/tmp/DisliteUsage.log", "2", ">", ">", "$ENV{DISIM}/tmp/Server.log")){&dismb_msg::err("serverspy failed.");} # Changed due to a failure of GetBuildLog.pl on fridge -- see that file.
	    # if(!run_command("FORK", "$ENV{PSETHOME}/bin/serverspy", "-service", $ENV{PMODServiceName}, "-pdf", $PDFArgument, "-prefs", $PREFSArgument, "-logfile", "$ENV{DISIM}/tmp/DisliteUsage.log", "2>>", "$ENV{DISIM}/tmp/Server.log")){&dismb_msg::err("serverspy failed.");}

	    my(@spy_cmd) = ("$ENV{PSETHOME}/mb/fork", &dismb_lib::arglist_to_fork_str("$ENV{PSETHOME}/bin/serverspy.exe", "-service", $ENV{PMODServiceName}, "-pdf", $PDFArgument, "-prefs", $PREFSArgument, "-logfile", "$ENV{DISIM}/tmp/DisliteUsage.log", "2>>", "$ENV{DISIM}/tmp/Server.log"));
	    print_verbose("Starting model_server \"\$1\"", &dismb_lib::arglist_to_str(@spy_cmd));
	    my($startcode) = &dismb_lib::system(@spy_cmd);
	    if($startcode==0)
	    {
		    require "$ENV{PSETHOME}/mb/viewer_client.pl";
		    while (not &dish2_client::is_server_running("Dislite:$ENV{PMODServiceName}")) {
			sleep(1);
		    }
		    print_verbose("Server started. Service name is \$1", $ENV{PMODServiceName});
	    } else {
		    print_verbose("Server failed ($startcode).");
	    }
	}
	if(!run_command("echo", "VERSION=$ENV{MODELNAME}/$ENV{VERSION}", ">", $ENV{PMODServerLockFile})){&dismb_msg::err("Could not create lockfile, \$1, for model server", $ENV{PMODServerLockFile});}
}

sub StopServer(@_)
{
	my($ServerState)=@_;
	print_always("Shutting down \$1/\$2 \(\$3\)", $ENV{MODELNAME}, $ENV{VERSION}, $ServerState);
	print_verbose("STOP:	\$1/\$2=\$3", $ENV{MODELNAME}, $ENV{VERSION}, $ServerState);
	$PMODServiceName=(defined($RepositoryID)?"${RepositoryID}:":"") . "$ENV{MODELNAME}_$ENV{VERSION}";
	$ENV{PMODServerLockFile}="$ENV{DISIM}/tmp/lockfiles/$ENV{MODELNAME}/$ENV{VERSION}_$ENV{HOST}.lock";
	if($ServerState!~m@Dead@)
	{
		local(*admintoolPipe);
		if(!&dismb_lib::open_output(\*admintoolPipe, "$ENV{PSETHOME}/bin/admintool", $PMODServiceName)) {
			&dismb_msg::err("Failed to open admintool pipe.");
		}
		print(admintoolPipe "stop\n");
		close(admintoolPipe);
	}
	if(-r $ENV{PMODServerLockFile})
	{
		print_verbose("Removing lockfile.");
		unlink $ENV{PMODServerLockFile};
	}
}

sub CheckServer(@_)
{
	my($ServerState)=@_;
	print_verbose("CHECK:	\$1/\$2=\$3", $ENV{MODELNAME}, $ENV{VERSION}, $ServerState);
	print_always("\$1/\$2 is \$3", $ENV{MODELNAME}, $ENV{VERSION}, $ServerState);
}

sub FixServer(@_)
{
	my($ServerState)=@_;
	$ENV{PMODServerLockFile}="$ENV{DISIM}/tmp/lockfiles/$ENV{MODELNAME}/$ENV{VERSION}_$ENV{HOST}.lock";
	print_verbose("FIX:	\$1/\$2=\$3", $ENV{MODELNAME}, $ENV{VERSION}, $ServerState);
	if($ServerState=~m@^Dead$@)
	{
		if(-r "$ENV{ADMINDIR}/tmp/Release")
		{
			SetupEnvironment();
			StartServer(@_);
		}
		else
		{
			StopServer("Obsolete");		# if server is not released or does not exist, it shouldn't be running
		}
	}
	elsif($ServerState=~m@^Stray$@)
	{
		if($ENV{MODE}=~m@kill@)			# only stop extra servers if mode is kill
		{
			StopServer(@_);
		}
	}
}

sub ProcessServers(@_)
{
	my($ActionToPerform)=@_;
	my(@PmodServices)=@AvailableServiceArray;
	if($ENV{VERSION}!~m@^\s*$@)
	{
		@PmodServices=("$ENV{MODELNAME}/$ENV{VERSION}");
	}
	else {
		foreach $RunningModelServer (@RunningModelServerArray)
		{
			if(! grep(m@$RunningModelServer@,@PmodServices))
			{
				push(@PmodServices,$RunningModelServer);
			}
		}
	}
	@PmodServices=sort(@PmodServices);
	foreach $PmodService (@PmodServices)
	{
		($ENV{MODELNAME},$ENV{VERSION})=split("/",$PmodService);
		$ENV{ADMINDIR}="$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}";
		$ServerState=GetServerState("$ENV{MODELNAME}/$ENV{VERSION}");
		&$ActionToPerform($ServerState);
	}

	# If it's stopping just one model version, wait for completion.
	if ($ActionToPerform eq "StopServer" and $ENV{VERSION}!~m@^\s*$@)
	{
		require "$ENV{PSETHOME}/mb/viewer_client.pl";
		my($PMODServiceName)=(defined($RepositoryID)?"${RepositoryID}:":"") . "$ENV{MODELNAME}_$ENV{VERSION}";
		while (&dish2_client::is_server_running("Dislite:${PMODServiceName}")) {
			sleep(1);
		}
	}
}

# Note that this function changes environment variables to match the
# ADMINDIR of the implicit or explicit model version.
sub ServerAdmin
{
	my($mode, $ModelName, $ModelVersion) = @_;
	if (defined $ModelVersion) {
	    ($ENV{MODELNAME}, $ENV{VERSION}) = ($ModelName, $ModelVersion);
        }

	CreateAvailableServiceList();
	CreateRunningServerList();

	if($mode=~m@start@)		{StartServer();}
	if($mode=~m@stop@)		{ProcessServers("StopServer");}
	if($mode=~m@kill@)		{ProcessServers("StopServer");}
	if($mode=~m@check@)		{ProcessServers("CheckServer");}
	if($mode=~m@fix@)		{ProcessServers("FixServer");}
}

if ($0 =~ m@ServerAdmin[^/\\]*@) {
    GetInstallationValues(@RequiredInstallationVariables,"VFB_DISPLAY");
    CheckArguments("MODE");

    &dismb_msg::define_progress_task("administering model server");
    ServerAdmin($ENV{MODE});
    &dismb_msg::report_progress("administering model server");
}
else {
    1;
}
