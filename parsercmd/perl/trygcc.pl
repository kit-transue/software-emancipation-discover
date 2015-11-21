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

# Determine whether the given compiler path name is gcc.
# If so, print the compiler id (gcc) and exit with a code of 0.
# The compiler is assumed to exist and be executable.

sub trygcc {
    my($platform,$compilerPath) = @_;
    my($name,$line);

    # Limit what to attempt this way, because you don't know what
    # commands might stray into the log file.

    if ($compilerPath =~ m@[\\/]([^\\/]+)$@) {
	$name = $1;
    }
    elsif ($compilerPath =~ m@^([^\\/]+)$@) {
	$name = $1;
    }
    if ($name !~ m@(cc|egcs|g\+\+)@i) {
	# Not something we want to risk executing.
	return undef;
    }

    open GCCOUT, ($compilerPath . " -v 2>&1 |") or return undef;
    my($specs);
    my($is_gcc) = 0;
    my($line_no) = 0;
    while (1) {
	$line_no = $line_no+1;
	$line = <GCCOUT>;
	if (not $line) {
	    last;
	}
	elsif ($line =~ /gcc version/i) {
	    $is_gcc = 1;
	}
	elsif (($line_no == 1 or ($line =~ "specs")) and ($line =~ m@ (/\S*)$@)) {
	    $specs = $1;
	}
    }
    close GCCOUT or return undef;
    if ($is_gcc and ($specs =~ m@^(.*)/lib/gcc-lib/([^/]+)/([\d\.]+)/[^/]+$@)) {
	my($base_dir) = $1;
	my($gcc_platform) = $2;
	my($version) = $3;
	$version =~ m@^([^\.]*)(\..*|)$@;
	my($major) = $1;
        my($args) = "(" . $base_dir . ")(" . $gcc_platform . ")(" . $version . ")"
		  . "(" . $major . ")(" . $platform . ")";
	if ($name =~ m@(g\+\+)@i) {
	    return "gcccpp" . $args;
	}
	else {
	    return "gcc" . $args;
	}
    }
    return undef;
}

if ($0 =~ m@trygcc[^/\\]*@) {
    my($result);
    ($result = trygcc(@ARGV)) or exit 1;
    print $result;
    exit 0;
}
else {
    1;
}
