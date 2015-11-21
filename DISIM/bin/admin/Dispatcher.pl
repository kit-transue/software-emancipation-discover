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
# Usage:  	perl Dispatcher.pl
# Description:	service requests written in $DISIM/tmp/Requests

require ("$ENV{PSETHOME}/mb/IMUtil.pl");
require ("$ENV{PSETHOME}/mb/dismb_msg.pl");

$FlagsFormat="[-n] [-V]";

$RequestLogFileDir="$DISIM}/tmp/Request/log";
$UniqueRequestID=0;

@RequestStack=();
@BuildRequestStack=();
@SimplifyRequestStack=();
@QARRequestStack=();
@ServeRequestStack=();

sub SetupDefaults(@_)
{
	my($RequestType,$ModelName,$ModelVersion)=@_;
	if($RequestType=~m@^BUILD@)
	{
		$Mode="full";
	}
	elsif($RequestType=~m@^SIMPLIFY@)
	{
		$Mode="full";
	}
	elsif($RequestType=~m@^QAR@)
	{
		$Mode="full";
	}
	elsif($RequestType=~m@^SERVE@)
	{
		$Mode="start";
	}
	else
	{
		return(0);
	}
	@ExtraArguments=();
	$KeepException=0;
	$ServerException=0;
	$Dependency="";
	$BuildLog="";
	$TempBuild="$ENV{TEMPBUILD}";
	my(@ReleasedVersionList)=GetReleasedModelList($ModelName);
	if(@ReleasedVersionList)
	{
		$BaseLineVersion=$ReleasedVersionList[0];
	}
		
	return(1);
}

sub ProcessWaitingRequests(@_)
{
	$ServicedRequestLog="$ENV{DISIM}/dispatch/Waiting/Serviced.log";
	$WaitingRequestLog="$ENV{DISIM}/dispatch/Waiting/Processed.log";
}

sub ProcessAllNewRequests(@_)
{
	my(@AllRequestFilesArray)=glob("$ENV{DISIM}/dispatch/Request/*");
	@RequestFileList=();
	foreach my $RequestFileName (@AllRequestFilesArray)
	{
		if(-r "$RequestFileName")
		{
			push(@RequestFileList,$RequestFileName);
			open(RequestFile,"$RequestFileName");
			while(<RequestFile>)
			{
				chomp($_);
				my($RequestType,$Parameters)=split("=",$_,2);
				my($ModelName,$ModelVersion,@Arguments)=split(":",$Parameters);
				if($ModelVersion=~m@^LATEST$@)
				{
					my(@ReleasedVersionList)=GetReleasedModelList($ModelName);
					if(@ReleasedVersionList)
					{
						$ModelVersion=$ReleasedVersionList[0];
					}
				}
				if(!SetupDefaults($RequestType,$ModelName,$ModelVersion))
				{
					next;
				}
				foreach my $Argument (@Arguments)
				{
					if($Argument=~m@^m=(.*)$@)
					{
						$Mode=$1;
						push(@ExtraArguments,"$Argument");
					}
					if($Argument=~m@^t=(.*)$@)
					{
						$TempBuild=$1;
						push(@ExtraArguments,"$Argument");
					}
					if($Argument=~m@^keep=(.*)$@)
					{
						$KeepException=$1;
						push(@ExtraArguments,"$Argument");
					}
					if($Argument=~m@^serve=(.*)$@)
					{
						$ServerException=$1;
						push(@ExtraArguments,"$Argument");
					}
					if($Argument=~m@^depend=(.*)$@)
					{
						$Dependency=$1;
						if($Dependency=~m@^LAST$@)
						{
							$LastRequest=@RequestStack[$#RequestStack];
							my($LastUniqueID,@JunkArray)=split(":",$LastRequest,2);
							push(@ExtraArguments,"depend=$LastUniqueID");
						}
						elsif($Dependency=~m@^LASTBUILD$@)
						{
							$LastRequest=@BuildRequestStack[$#BuildRequestStack];
							my($LastUniqueBuildID,@JunkArray)=split(":",$LastRequest,2);
							push(@ExtraArguments,"depend=$LastUniqueBuildID");
						}
						else
						{
							push(@ExtraArguments,"$Argument");
						}
					}
					if($Argument=~m@^buildlog=(.*)$@)
					{
						$BuildLog=$1;
					}
				}
				$ExtraArgs=join(":",@ExtraArguments);
				AssembleRequest("$UniqueRequestID:$RequestType:$ModelName:$ModelVersion:$ExtraArgs");
				$UniqueRequestID++;
			}
			close(RequestFile);
		}
	}
}

sub AssembleRequest(@_)
{
	my($UniqueRequestID,$RequestType,$ModelName,$ModelVersion,$ExtraArgs)=split(":",$_[0]);
	if($RequestType=~m@^BUILD@)
	{
		print_verbose("\$1:\$2:\$3:\$4:\$5", $UniqueRequestID, $RequestType, $ModelName, $ModelVersion, $ExtraArgs);				
		push(@RequestStack,"$UniqueRequestID:$RequestType:$ModelName:$ModelVersion:$ExtraArgs\n");				
		push(@BuildRequestStack,"$UniqueRequestID:$RequestType:$ModelName:$ModelVersion:$ExtraArgs\n");				
	}
	elsif($RequestType=~m@^SIMPLIFY@)
	{
		print_verbose("\$1:\$2:\$3:\$4:\$5", $UniqueRequestID, $RequestType, $ModelName, $ModelVersion, $ExtraArgs);				
		push(@RequestStack,"$UniqueRequestID:$RequestType:$ModelName:$ModelVersion:$ExtraArgs\n");				
		push(@SimplifyRequestStack,"$UniqueRequestID:$RequestType:$ModelName:$ModelVersion:$ExtraArgs\n");				
	}
	elsif($RequestType=~m@^QAR@)
	{
		print_verbose("\$1:\$2:\$3:\$4:\$5", $UniqueRequestID, $RequestType, $ModelName, $ModelVersion, $ExtraArgs);				
		push(@RequestStack,"$UniqueRequestID:$RequestType:$ModelName:$ModelVersion:$ExtraArgs\n");				
		push(@QARRequestStack,"$UniqueRequestID:$RequestType:$ModelName:$ModelVersion:$ExtraArgs\n");				
	}
	elsif($RequestType=~m@^KEEP@)
	{
		print_verbose("\$1:\$2:\$3:\$4:\$5", $UniqueRequestID, $RequestType, $ModelName, $ModelVersion, $ExtraArgs);				
		push(@RequestStack,"$UniqueRequestID:$RequestType:$ModelName:$ModelVersion:$ExtraArgs\n");				
		push(@ServeRequestStack,"$UniqueRequestID:$RequestType:$ModelName:$ModelVersion:$ExtraArgs\n");				
	}
	elsif($RequestType=~m@^SERVE@)
	{
		print_verbose("\$1:\$2:\$3:\$4:\$5", $UniqueRequestID, $RequestType, $ModelName, $ModelVersion, $ExtraArgs);				
		push(@RequestStack,"$UniqueRequestID:$RequestType:$ModelName:$ModelVersion:$ExtraArgs\n");				
		push(@ServeRequestStack,"$UniqueRequestID:$RequestType:$ModelName:$ModelVersion:$ExtraArgs\n");				
	}
}

sub DispatchRequests(@_)
{
	
}

sub ServerAdmin(@_)
{
	if(! -d "$ENV{DISIM}/Dispatch")
	{
		mkdir("$ENV{DISIM}/dispatch",0755);
		mkdir("$ENV{DISIM}/dispatch/Waiting",0755);
		mkdir("$ENV{DISIM}/dispatch/Request",0777);
	}
	ProcessWaitingRequests();
	ProcessAllNewRequests();
	DispatchRequests();
}

$THIS_FILE="Dispatcher.pl";
GetInstallationValues();
CheckArguments();

ServerAdmin();

