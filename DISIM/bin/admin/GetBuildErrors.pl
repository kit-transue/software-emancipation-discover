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
# Usage:  	perl GetBuildErrors.pl [dismb log-file1] [dismb log-file2] ...
# Description:	process the dismb build log for EDG errors; provide a count
# 		by file (important for assessing header file impact) and a
#		count by error (both absolute and removal of dulplicates).

require ("$ENV{PSETHOME}/mb/IMUtil.pl");

sub GetLogFileList(@_)
{
	my($PerlScriptName,@LogFileNameList)=@_;
	if(@LogFileNameList > 0)
	{
		return(@LogFileNameList);
		$OutPutFileName=">&STDOUT";
	}
	elsif($ENV{ADMINDIR}=~m@^$@)
#	if($ENV{ADMINDIR}=~m@^$@)
	{
		ShowUsageError("No log files specified and ADMINDIR not set.");
	}
	@LogFileNameList=glob("$ENV{ADMINDIR}/log/*_dismb.log $ENV{ADMINDIR}/tmp/disbuild/*/*");
	if(@LogFileNameList > 0)
	{
		$OutPutFileName="$ENV{ADMINDIR}/log/$ENV{USER}_dismb.err";
		print "Sending output to $OutPutFileName\n";
		return(@LogFileNameList);
	}
	else
	{
		ShowUsageError("No log files specified and defaults not found.");
	}
}

sub make_error_message_generic {
        # strip off things in quotes or parens.
        local ($ret) = @_;
        $ret =~ s/\([^\)]*\)/<paren>/g;
        $ret =~ s/"[^\"]*"/<text>/g;
	$ret =~ s/\s+/ /g;
	return $ret;
}

sub ParseLogFilesForErrors(@LogFileNameList)
{
	foreach $LOGFILENAME (@LogFileNameList)
	{
		open(InFile, "$LOGFILENAME");
		while (<InFile>)
		{
		        chomp;
		        if (/^\"(.*)\", line [0-9]*: error.*:/)
		        {
		                $error_count++;
		                $file = $1;
		                $location = $_;
		                $error_msg = "";
		                $error_text = "";
		                while(<InFile>)
		                {
		                        chop;
		                        if (/^[\s]*\^$/)
		                        {
		                                last;
		                        }
		                        $error_msg .= $error_text;
		                        $error_text = $_;
		                }

		                # add the error marker:
		                $error_text .= "\n" . $_;

		                # clean up the error message:
		                $error_msg =~ s/\s+/ /g;

		                $gen_message = &make_error_message_generic($error_msg);
		                $message_count{$gen_message}++;

		                $file_count{$file}++;
		                $uniq_key = $location . "\n" . $error_msg;
		                if ($uniq_errors{$uniq_key}++ == 0)
		                {
		                        $uniq_message_count{$gen_message}++;
		                }
		                $uniq_text{$uniq_key} = $error_text;
		        }
		}
	close Infile;
	}
}

sub PrintErrorReport()
{
	open(OutPutFile,">$OutPutFileName");
	foreach $x (keys(%message_count))
	{
	        print(OutPutFile "GENERIC: $x: $message_count{$x} ($uniq_message_count{$x})\n");
	}
	foreach $x (keys(%file_count))
	{
	        print(OutPutFile "FILE: $x: $file_count{$x}\n");
	}
	$uniq_err_count = 0;
	foreach $x (sort(keys(%uniq_errors)))
	{
	        print(OutPutFile "ERROR: $x: (occurs $uniq_errors{$x} time(s))\n");
	        print(OutPutFile "$uniq_text{$x}\n");
	        $uniq_err_count++;
	}
	print(OutPutFile "TOTAL ERRORS REPORTED: $error_count\n");
	print(OutPutFile "TOTAL UNIQUE ERRORS: $uniq_err_count\n");
}

sub GetBuildErrors(@_)
{
	@LogFileNameList=GetLogFileList(@_);
	ParseLogFilesForErrors(@LogFileNameList);
	PrintErrorReport();
}

$THIS_FILE="GetBuildErrors.pl";
GetInstallationValues();
GetBuildErrors(@ARGV);

