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
# Usage:  	perl FilterBuildLog.pl <build_log> <parseline_file> <files_list>
# Description:	Based on build log information, create a file consisting of parse
#		lines, one line per source file.  Also extract a list of filenames
#		that are to be included into the model.

require ("$ENV{PSETHOME}/mb/IMUtil.pl");
require ("$ENV{PSETHOME}/mb/dismb_lib.pl");
require ("$ENV{PSETHOME}/mb/dismb_msg.pl");
require (FindFile("$ENV{PSETHOME}/mb/GetBuildLog.pl"));

my($SourceListFileName)="$ENV{ADMINDIR}/config/Sources.txt";
my($TranslatorListFileName)="$ENV{ADMINDIR}/config/translator.flg";

sub FilterBuildTable::Filter(@_)
{
    require ("$ENV{PSETHOME}/mb/parsercmds.pl");
    require File::Basename;

    my(%CompileFileNameHash)=();						# unique list of compilable files

    parsercmds::filter($GetBuildLog::BuildTableFileName, $TranslatorListFileName);

    if(!open(TranslatorListFile,"$TranslatorListFileName")){&dismb_msg::die("Could not open dismb flags file, \$1.", $TranslatorListFileName);}
    if(!open(SourceListFile,">$SourceListFileName")){&dismb_msg::die("Could not open source list file, \$1.", $SourceListFileName);}

    my($line);
    while($line = <TranslatorListFile>)
    {
	my($straight_line);
	($straight_line) = split("\n", $line);
	my($FileLanguageName);			# C,CPP,H ... etc.
	my($parserOptions); my($CompileFile); my($proj); my($working_dir);
	($proj, $working_dir, $CompileFile, $FileLanguageName, $parserOptions) = split("\t", $straight_line);
	# &dismb_msg::msg ("FROMPARSERCMD \$1 \$2 \$3 \$4 \$5", $proj, $working_dir, $CompileFile, $FileLanguageName, $parserOptions);
	$parserOptions=~s@\0$@@;  # A null was sneaking in.
	$_ = $parserOptions;
	my($SourceFileDir)=$CompileFile;
	$SourceFileDir=~s@/[^/]+$@@;
	if(! exists($CompileFileNameHash{$CompileFile}))
	{
	    # Source not encountered yet. Write out to flags files.
	    # if($java_sql_jsflag==0) !!
	    print(TranslatorListFile "$ProjectNameList[0]\t$working_dir\t$CompileFile\t$FileLanguageName\t$_\n");
	}
	$CompileFileNameHash{$CompileFile}="$ProjectNameList[0]\t$working_dir\t$CompileFile\t$FileLanguageName\t$_";
    }
    foreach my $SourceFileName (keys(%CompileFileNameHash))
    {
	print(SourceListFile "$SourceFileName\n");
    }
    close(SourceListFile);
    close(TranslatorListFile);
}

1;
