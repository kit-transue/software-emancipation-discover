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
use FileHandle;
use Cwd;
use File::Path;

require ("parsercmds.pl");

# print STDERR "@ARGV\n";
$filename = "$ARGV[0]";
unless (-e "$filename") {
    die "$filename does not exist.\n";
}
print STDOUT "$filename\n";

# from command line, get dsp file name

$rslt1 = `scandsp.exe "$filename" --verbose > sources_to_compile`;
print STDOUT "$rslt1\n";

parsercmds::filter("sources_to_compile", "sources_to_parse");

# Note the current directory.
$owd = cwd();

open SOURCES, "<sources_to_parse" or die "Can't open sources: $!";
while ($line = <SOURCES>) {
    print STDOUT "----------------------------------------------------\n";
    ($line) = split("\n", $line);
    ($project, $wd, $sourceName, $parser, $options) = split("\t", $line);
    $cmdFileName = "$sourceName";
    chdir $wd;
    if ($parser eq "CPP") {
	print STDOUT "aset_CCcc $options --indent \"$cmdFileName\" \n";
	$rslt3 = `aset_CCcc $options --indent "$cmdFileName"`;
        if ($? == 0) {
	    print STDOUT "aset_CCcc succeeded.\n";
	}
    }
    elsif ($parser eq "C") {
	print STDOUT "aset_CCcc $options --indent  \"$cmdFileName\" \n";
	$rslt3 = `aset_CCcc $options --indent "$cmdFileName"`;
        if ($? == 0) {
	    print STDOUT "aset_CCcc succeeded.\n";
	}
    }
    elsif ($parser eq "java") {
	print STDOUT "aset_jfe $options --indent  \"$cmdFileName\" \n";
	$rslt3 = `aset_jfe $options --indent "$cmdFileName"`;
        if ($? == 0) {
	    print STDOUT "aset_jfe succeeded.\n";
	}
    }
    elsif ($parser eq "rc") {
	print STDOUT "# rcparser $options \"$cmdFileName\" \"$cmdIfName\" \n";
    }
    elsif ($parser eq "H") {
	print STDOUT "# C/C++ header in project: \"$cmdFileName\"\n";
    }
    else {
	print STDOUT "Cannot parse $sourceName using parser \"$parser\".\n";
    }
    chdir $owd;
}
close SOURCES;
