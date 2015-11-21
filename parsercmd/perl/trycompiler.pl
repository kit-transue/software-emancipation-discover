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

# Determine what compiler is represented by the given
# compiler path name (already determined not to be gcc).  This takes into
# account the current platform. If it is a known compiler, returns true
# and sets id as per the script, i.e. with the name of the compiler used
# for configuration purposes, e.g. suncc.

my($result);

sub trycompiler {
    my($platform, $compilerPath) = @_;
    my($base,$prefix,$compilerName,$id);
    if (($base) = ($compilerPath =~ /^(.*)\.exe$/i)) {
	$compilerPath = $base;
    }

    # Eventually we could try executing the compiler and looking at what it does.
    # But for now we just support 1 compiler family per platform.

    my(%platformCompilerId)
	= ($compilerPath =~ 'CC[^/\\\\]*$')
	    ? ("WIN32","ntcl","SunOS","sunccpp","HP-UX","acc","IRIX","mipsccpp")
	    : ("WIN32","ntcl","SunOS","suncc","HP-UX.10","hpcc","HP-UX","hpcc11","IRIX","mipscc");
    if (($prefix,$compilerName) = ($compilerPath =~ '^(.*[/\\\\]|)([^/\\\\]+)$')) {
	if ($compilerName =~ /javac/i) {
	    $id = "javac";
	}
	elsif ($compilerName =~ /sqlc/i) {
	    # Modify this to passinformixsql or passsybasesql if needed.
	    $id = "passplsql";
	}
	elsif (($compilerName =~ /^(\w|\+)*$/) and ($compilerName !~ /^(echo|cd|rm|ld|lex|Initial|making)$/i)) {
	    if ($platform eq "HP-UX") {
		my($revs)=`/usr/bin/uname -r`;
		if ($revs =~ m/^.*\.(\d+)\.\d+\s*$/) {
		    if ($1 <= 10) {
			$platform = $platform . ".10";
		    }
		}
	    }
	    $id = $platformCompilerId{$platform};
	}
    }

    # Append install dir to $id, if needed and possible.
    if (($id eq "sunccpp") or ($id eq "acc")
			   or ($id eq "mipsccpp")
			   or ($id eq "suncc")) {
	my($arg);
	if (($arg) = ($compilerPath =~ /(.*)\/bin\/.*$/)) {
	    $id = $id . "(" . $arg . ")";
	    if ($id =~ "^sunccpp\\(" and -l $compilerPath) {
		# The link version of the compiler name does not yield a correct include dir.
		$id = undef;
	    }
	}
	elsif ($id eq "suncc") {
	    # Suncc might be satisfactory without the extra include dir.
	    $id = $id . "()";
	}
	else {
	    # Sunccpp, aCC, and MIPS CC are unusable without location of includes.
	    $id = undef;
	}
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
    elsif ($id eq "ntcl") {
	# In case the INCLUDE environment variable is not already set.
	my($arg);
	if ($ENV{INCLUDE} !~ m/MFC/i and (($arg) = ($compilerPath =~ /(.*)[\/\\]bin[\/\\].*$/i))) {
	    $id = $id . "(" . $arg . ")";
	}
	else {
	    # Can be satisfactory without the extra include dir.
	    $id = $id . "()";
	}
    }
    elsif ($id eq "javac") {
	my($sep) = $platform eq 'WIN32' ? ';' : ':';
	if ($compilerPath =~ m@^(.*)[/\\]bin[/\\][^/\\]+$@) {
	    my($basedir) = $1;
	    if (-d "${basedir}/jre") {
		$id = "javac($sep)(${basedir}/jre)";
	    }
	    else {
		$id = "javac($sep)(${basedir})";
	    }
	}
	else {
	    $id = "javac($sep)($ENV{PSETHOME}/jre)";
	}
    }
    return $id;
}

if ($0 =~ m@trycompiler[^/\\]*@) {
    my($result);
    ($result = trycompiler(@ARGV)) or exit 1;
    print $result;
    exit 0;
}
else {
    1;
}
