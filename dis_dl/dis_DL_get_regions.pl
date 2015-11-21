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
#
# This script must be passed to $PSETHOME/bin/disperl on the command line
#   i.e. $PSETHOME/bin/disperl dis_DL_get_regions.pl <args......>
#

sub Usage {
    print "";
    print "Produces sorted list of file regions affected by changes between";
    print "two earlier versions of the file.";
    print "";
    print "         Usage:      $nm <file1> <file2> <file3> <ofile> <ffile>";
    print "";
#    print "ifile  - input file, a file where each line contains 3 filenames separated by spaces";
    print "         file1 - current version";
    print "         file2 - fixed version (complete physical pathname)";
    print "         file3 - predecessor of the fixed version";
    print "";
    print "ofile - output file, a sorted file where each line describes an affected region in a";
    print "        current version (file1) in the following format:";
    print "        file1:n1[,m1]";
    print "        file1:n2[,m2]";
    print "        file1:n3[,m3]";
    print "ffile - failed file, list of failed files (file1 only)";
    print "";
}

sub Just_Usage {
    &Usage;
    exit 0;
}

sub Usage_Error {
    print "$nm ERROR: @_[0]";
    &Usage;
    exit 1;
}

sub Die_Error {
    print "$nm FATAL: @_[0]";
    exit 1;
}

sub numerically { $a <=> $b; }

##############################################################################################
print "";
$\ = "\n";
$nm = $0;
$nm =~ s%.*[\\\/]([^\\\/]*)$%\1%;

&Just_Usage if @ARGV[0] eq '-h' || @ARGV[0] eq '-H' || @ARGV[0] eq '?' || @ARGV[0] eq "help";

&Usage_Error ('Takes five parameters')  if ! @ARGV[0] || !@ARGV[1] || !@ARGV[2] || !@ARGV[3] || !@ARGV[4] || @ARGV[5] ;

#open (LOGF,"> c:/Temp/get_regions.log") || &Die_Error ("cannot write @ARGV[2]");

#print LOGF "$nm: STARTED";
#print LOGF "INF: @ARGV[0]";
#print LOGF "OUTF: @ARGV[1]";
#print LOGF "ERF: @ARGV[2]";

print "$nm: STARTED";
#print LOGF "$nm: STARTED";

$f1 =  @ARGV[0];
$f2 =  @ARGV[1];
$f3 =  @ARGV[2];
open (OUTF,"> @ARGV[3]") || &Die_Error ("cannot write @ARGV[3]");
open (ERRF,">> @ARGV[4]") || &Die_Error ("cannot write @ARGV[4]");

$\ = "";

$succeded = 0;

print "\n";
print "$f1 ... ";
print LOGF "$f1 ... \n";
if ( ! -r $f1 || ! -r $f2 || ! -r $f3 ) {
    print ERRF "$f1\n";
    print "failed\n";
} else {
    open (TTT, "$ENV{'PSETHOME'}/bin/diff3 -e $f1 $f2 $f3 |");

    while (<TTT>) {
	if (/^[0-9][0-9,]*/) {
	    s%^([0-9][0-9,]*).*$%$1%;
	    $aarr{$_} = $_;	
	}			
    }				

    foreach $ii (sort numerically keys(%aarr)) {
	$ln_out = "$f1,$aarr{$ii}";
	$ln_out =~ tr/\\/\//;
	print OUTF $ln_out;
    }

    $succeded++;
    print "done\n";
}
close (TTT);
print "\n";

if ($succeded) {
    print "$nm: Succeeded\n";
} else {
    print "$nm: Failed\n";
}
exit 0;





