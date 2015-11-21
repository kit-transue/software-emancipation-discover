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
# Usage:  	perl VersionWindow.pl
# Description:	monitor windowing scheme as setup in $DISIM/lib/Configuration.txt

require ("$ENV{PSETHOME}/mb/IMUtil.pl");
require ("$ENV{PSETHOME}/mb/dismb_msg.pl");

$FlagsFormat="[-n] [-V]";

sub MaintainBuildServer(@_)
{
	foreach my $HostName (keys(%BuildServerPreferenceHash))
	{
		if($ENV{HOST}=~m@^$HostName$@)
		{
			my($SubroutineName,$ArgumentValue)=split(/:/,$BuildServerPreferenceHash{$HostName},2);
			{
				print_verbose("	\$1(\$2)", $SubroutineName, $ArgumentValue);
				&$SubroutineName($ArgumentValue);
			}
		}
	}
}

sub KeepLatest(@_)
{
	my($ModelName,$WindowSize)=@_;
	my($WindowSizeCount)=0;
	my(@ReleasedModelVersionList)=GetReleasedModelList(@_);
	my(@KeepModelVersionList)=GetKeepModelList(@_);
	foreach my $ModelVersion (@ReleasedModelVersionList)
	{
		if(grep(m@^$ModelVersion$@,@KeepModelVersionList))
		{
			push(@ModelKeepArray,$ModelVersion);
			print_verbose("		\$1/\$2 KEEP	(Exception)", $ModelName, $ModelVersion);
			next;
		}
		$WindowSizeCount++;
		if($WindowSizeCount<=$WindowSize)
		{
			push(@ModelKeepArray,$ModelVersion);
			print_verbose("		\$1/\$2 Keep	(\$3 of \$4)", $ModelName, $ModelVersion, $WindowSizeCount, $WindowSize);
		}
		else
		{
			push(@ModelRemoveArray,$ModelVersion);
			print_verbose("		\$1/\$2 Remove	(\$3 of \$4)", $ModelName, $ModelVersion, $WindowSizeCount, $WindowSize);
		}
	}
}

sub KeepGroup(@_)
{
	my($ModelName,$WindowSize)=@_;
	print_verbose("Keeping \$1 models to limit of \$2 weekly models", $ModelName, $WindowSize);
}

sub ServeLatest(@_)
{
	my($ModelName,$WindowSize)=@_;
	my($WindowSizeCount)=0;
	my(@AvailableServerVersionList)=GetAvailableServerList(@_);
	my(@ExceptionServerVersionList)=GetExceptionServerList(@_);
	my(@ReleasedModelVersionList)=GetReleasedModelList(@_);
	foreach my $ModelVersion (@ReleasedModelVersionList)
	{
		if(grep(m@^$ModelVersion$@,@ExceptionServerVersionList))
		{
			if(grep(m@^$ModelVersion$@,@AvailableServerVersionList))
			{
				push(@ServerMaintainArray,$ModelVersion);
				print_verbose("		\$1/\$2 MAINTAIN	(Exception)", $ModelName, $ModelVersion);
			}
			else
			{
				push(@ServerStartArray,$ModelVersion);
				print_verbose("		\$1/\$2 START	(Exception)", $ModelName, $ModelVersion);
			}
			next;
		}
		$WindowSizeCount++;
		if($WindowSizeCount>$WindowSize){next;}
		if(grep(m@^$ModelVersion$@,@AvailableServerVersionList))
		{
			push(@ServerMaintainArray,$ModelVersion);
			print_verbose("		\$1/\$2 Maintain	(\$3 of \$4)", $ModelName, $ModelVersion, $WindowSizeCount, $WindowSize);
		}
		else
		{
			push(@ServerStartArray,$ModelVersion);
			print_verbose("		\$1/\$2 Start	(\$3 of \$4)", $ModelName, $ModelVersion, $WindowSizeCount, $WindowSize);
		}
	}
	foreach my $ModelVersion (@AvailableServerVersionList)
	{
		if(!(grep(m@^$ModelVersion$@,@ServerMaintainArray)||
			grep(m@^$ModelVersion$@,@ServerStartArray)))
		{
			push(@ServerStopArray,$ModelVersion);
			print_verbose("		\$1/\$2 Stop", $ModelName, $ModelVersion);
		}
	}
}

sub ServeGroup(@_)
{
	my($ModelName,$WindowSize)=@_;
	print_verbose("Keeping \$1 models to limit of \$2 weekly servers", $ModelName, $WindowSize);
}

sub MaintainModels(@_)
{
	my($ModelName)=@_;
	foreach my $ModelVersion (@ModelRemoveArray)
	{
		print_always("Removing model \$1/\$2", $ModelName, $ModelVersion);
		require (FindFile("$ENV{PSETHOME}/mb/RemoveModel.pl"));
		RemoveModelVersion($ModelName, $ModelVersion, "$ENV{DISIM}/model/$ModelName/$ModelVersion");
	}
}

sub MaintainServers(@_)
{
	my($ModelName)=@_;
        require(FindFile("$ENV{PSETHOME}/mb/ServerAdmin.pl"));
	foreach my $ModelVersion (@ServerStartArray)
	{
		ServerAdmin("start", $ModelName, $ModelVersion);
	}
	foreach my $ModelVersion (@ServerStopArray)
	{
		ServerAdmin("stop", $ModelName, $ModelVersion);
	}
}

sub VersionWindow(@_)
{
	my($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
	my($month) = (Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec)[$mon];
	$DateStamp = sprintf("%4d%2d%2d",1900 + $year,1 + $mon,$mday); $DateStamp =~ s/\s/0/g;
	$TimeStamp = sprintf("%2d%2d%2d",$hour,$min,$sec);             $TimeStamp =~ s/\s/0/g;
	print_verbose("========== VersionWindow.pl starting at (\$1 \$2) ==========", $TimeStamp, $DateStamp);
	foreach my $ModelName (@ModelNameArray)
	{
		@ServerMaintainArray=();
		@ServerStartArray=();
		@ServerStopArray=();
		@ModelKeepArray=();
		@ModelRemoveArray=();
		print_verbose("MODELNAME	\$1", $ModelName);

		my($SetupFileName) = "$ENV{DISIM}/template/${ModelName}Template/config/Setup.txt";
		if(! -r "$SetupFileName")
		{
			&dismb_msg::err("Model \$1 has no Setup.txt file", $ModelName);
			next;
		}

		SetupNoAdmindirEnvironment($SetupFileName);

		if(defined($WindowPreference))
		{
			my(@ModelPreferenceArray)=split(/=/,$WindowPreference);
			foreach my $ModelPreference (@ModelPreferenceArray)
			{
				my($HostName,$SubroutineName,$ArgumentValue)=split(/:/,$ModelPreference);
				if($ENV{HostType}=~m@^NT$@ ? $ENV{HOST}=~m@^$HostName$@i : $ENV{HOST}=~m@^$HostName$@)
				{
					print_verbose("	\$1(\$2,\$3)", $SubroutineName, $ModelName, $ArgumentValue);
					&$SubroutineName($ModelName,$ArgumentValue);
				}
			}
			MaintainServers($ModelName);
			MaintainModels($ModelName);
		}
	}
}

$THIS_FILE="VersionWindow.pl";
GetInstallationValues(@RequiredInstallationVariables,"VFB_DISPLAY");
CheckArguments();

VersionWindow();

MaintainBuildServer();

