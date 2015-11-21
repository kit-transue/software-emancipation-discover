#!/usr/bin/perl
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

# Last-ditch identification of the compiler which appears in the config record
# by the given name, but which cannot be found on disk.

sub compilerid {
    my($platform, $compilerName) = @_;
    my($id);

    if (($base) = ($compilerName =~ /^(.*)\.exe$/i)) {
	$compilerName = $base;
    }
    if ($compilerName =~ /javac/i) {
	my($sep) = $platform eq 'WIN32' ? ';' : ':';
	$id = "javac(${sep})($ENV{PSETHOME}/jre)";
    }
    elsif ($compilerName =~ /sqlc/i) {
	# Modify this to passinformixsql or passsybasesql if needed.
	$id = "passplsql";
    }
    elsif ($compilerName =~ /(gcc|egcs)/i) {
	# Can't handle gcc's includes unless the compiler is located. :-(
    }
    elsif ($compilerName =~ /(g\+\+)/i) {
	# Can't handle g++'s includes unless the compiler is located. :-(
    }
    else {
	# Can't handle sunccpp's, aCC's, or mipsccpp's includes unless the compiler is located. :-(
	my(%platformCompilerId)
	    = ($compilerName =~ /CC/)
		? ("WIN32","ntcl()")
		: ("WIN32","ntcl()","SunOS","suncc()","HP-UX.10","hpcc","HP-UX","hpcc11","IRIX","mipscc");
	if (($compilerName =~ /^(\w|\+)+$/) and ($compilerName !~ /^(echo|cd|rm|ld|lex|Initial|making)$/)) {
	    if ($platform eq "HP-UX") {
		my($revs)=`/usr/bin/uname -r`;
		if ($revs =~ m/^.*\.(\d+)\.\d+\s*$/) {
		    if ($1 <= 10) {
			$platform = $platform . ".10";
		    }
		}
	    }
	    $id = $platformCompilerId{$platform};
	    if ($id =~m/^suncc/) {
		# Suncc and sunccpp need a munged OS number.
		my($relno) = `/usr/bin/uname -r`;
		if ($?) {
		    # Punt.
		    $relno = "5.8";
		}
		else {
		    chomp($relno);
		}
		$relno =~ s/\./_/g;
		$id .= "($relno)";
	    }
	}
    }
    return $id;
}

if ($0 =~ m@compilerid[^/\\]*@) {
    my($result);
    ($result = compilerid(@ARGV)) or exit 1;
    print $result;
    exit 0;
}
else {
    1;
}
