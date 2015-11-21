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

$WhatToGet=$1;
%UserLicenseUseHash=();
%UserLicenseDurationHash=();

open(InFile,"rsh twister cat /usr/tmp/discover_lmgr.log|");

while(<InFile>)										
{											
# ==================== ====================
	chomp;
	if(m@ TIMESTAMP ([^\s]+)@)
	{
		$CurrentDate=$1;
	}
	if(m@started on [^\s]+ [^\s]+ \(([^\s]+)\)@)
	{
		$CurrentDate=$1;
	}
	if(m#([^\s]+)\s+[^\s]+\s+(OUT|IN):\s+"(DISCOVER|ModelClient|QualityBase)"\s+([^\s]+)@([^\s]+)#)
	{
		$TimeStamp=$1;$WhichWay=$2;$LicenseName=$3;$WhoIsUsing=$4;$HostName=$5;
		$UserLicenseUseHash{$WhoIsUsing}=~s@$@\n$CurrentDate,$TimeStamp,$LicenseName,$WhichWay@;
	}

}
close InFile;

foreach $User (keys(%UserLicenseUseHash))
{
	%LookForHash=();
	$UserLicenseUseHash{$User}=~s@^\n@@;
	@CurrentLicenseArray=split(/\n/,$UserLicenseUseHash{$User});
	foreach $LicenseUseLine (@CurrentLicenseArray)
	{
		($CurrentDate,$TimeStamp,$LicenseName,$WhichWay)=split(/,/,$LicenseUseLine);
		if($WhichWay=~m@^OUT$@)
		{
			$LookForHash{$LicenseName}="$CurrentDate,$TimeStamp";
		}
		elsif($WhichWay=~m@^IN$@)
		{
			if(exists($LookForHash{$LicenseName}))
			{
				($BeginningOfUseDate,$BeginningOfUseTime)=split(/,/,$LookForHash{$LicenseName});
				($BeginningOfUseMonth,$BeginningOfUseDay,$BeginningOfUseYear)=split(/:/,$BeginningOfUseDate);
				($BeginningOfUseHours,$BeginningOfUseMinutes,$BeginningOfUseSeconds)=split(/:/,$BeginningOfUseTime);
				$EndOfUseDate=$CurrentDate;$EndOfUseTime=$TimeStamp;
				($EndOfUseMonth,$EndOfUseDay,$EndOfUseYear)=split(/:/,$EndOfUseDate);
				($EndOfUseHours,$EndOfUseMinutes,$EndOfUseSeconds)=split(/:/,$EndOfUseTime);

				$DurationOfUseSeconds=$EndOfUseSeconds - $BeginningOfUseSeconds;
				if($DurationOfUseSeconds<0) {$DurationOfUseSeconds=60+$DurationOfUseSeconds;$DurationOfUseMinutes--;}
				$DurationOfUseMinutes=$EndOfUseMinutes - $BeginningOfUseMinutes;
				if($DurationOfUseMinutes<0) {$DurationOfUseMinutes=60+$DurationOfUseMinutes;$DurationOfUseHours--;}
				$DurationOfUseHours=$EndOfUseHours - $BeginningOfUseHours;
				if($DurationOfUseHours<0) {$DurationOfUseHours=24+$DurationOfUseHours;$DurationOfUseDay=$EndOfUseDay--;}
				$DurationOfUseDay=$EndOfUseDay - $BeginningOfUseDay;
				if($DurationOfUseDay<0) {$DurationOfUseDay=30+$DurationOfUseDay;$DurationOfUseMonth--;}
				$DurationOfUseMonth=$EndOfUseMonth - $BeginningOfUseMonth;
				if($DurationOfUseMonth<0) {$DurationOfUseMonth=12+$DurationOfUseMonth;$DurationOfUseYear--;}
				$DurationOfUseYear=$EndOfUseYear - $BeginningOfUseYear;
				if($DurationOfUseYear<0) {$DurationOfUseYear=2000+$DurationOfUseYear;}

				$DurationOfUse="$DurationOfUseYear:$DurationOfUseMonth:$DurationOfUseDay:$DurationOfUseHours:$DurationOfUseMinutes:$DurationOfUseSeconds";

				$UserLicenseDurationHash{User}=~s@$@\n$User,$LicenseName,$LookForHash{$LicenseName},$Duration@;
				print "$User,$LicenseName,$LookForHash{$LicenseName},$DurationOfUse\n";
				delete($LookForHash{$LicenseName});
			}
		}
		else
		{
			print "ERROR:  no IN/OUT indication\n";
		}
	}
}
