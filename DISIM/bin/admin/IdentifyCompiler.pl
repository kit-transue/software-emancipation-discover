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
require ("$ENV{PSETHOME}/mb/IMUtil.pl");
require ("$ENV{PSETHOME}/mb/dismb_lib.pl");
require ("$ENV{PSETHOME}/mb/dismb_msg.pl");

my(%HostTranslate) = ("NT","WIN32", "SUN5","SunOS", "HP10","HP-UX", "IRIX","IRIX", "NONE","");

# ==========================================================================
# Return true if the compiler is identified.  If so, id is set with
# the name of the compiler used for configuration purposes, e.g. ntcl.
# Some compiler ids require parameters, e.g. sunccpp(/usr/SUNWspro/SC5.0).
#
sub FilterBuildLog::IdentifyCompiler {
    require Cwd;
    my($compilerPath) = @_;
    my($path, $compilerName) = ($compilerPath =~ m@(^|/|\\\\)([^/\\\\]+)$@);
    my($is_compiler)= ($compilerName !~ m/^\W*(echo$|cd$|rm$|ld$|lex$|Initial|making|[0-9_]?\W*$)/i);

    # From the path, check the existence
    # of the file it refers to, returning whether it exists, and if it exists
    # set $foundPath to the path where it can be invoked.
    my(@foundPaths) = ();  # list of paths for the found file

    # Try the given path for $actualPath.
    if (-x $compilerPath) {
	@foundPaths = ($compilerPath);
    }
    elsif ($compilerName ne "") {
	# Search for $compilerName according to the PATH environment variable.
	require File::Basename;
	my(@paths) = split m@$ENV{path_separator}@, $ENV{PATH};
	if($ENV{HostType} eq "NT") {
	    my($msvcd) = &dismb_lib::backtick("$ENV{'PSETHOME'}\\bin\\scandsp.exe", "--get_msvc_dir");
	    if (not $?) {
		chomp($msvcd);
		my($msvc_bin) = $msvcd . "\\bin";
		push @paths, $msvc_bin;
	    }
	}
	# We follow links and collect all results, because the name of the
	# linked-to file may be either more or less meaningful than the original.
	foreach $path (@paths) {
	    my($compiler) = $path . "/" . $compilerName;
	    while (1) {
		if (-x $compiler) {
		    # Record file name if found using PATH.
		    my($actualPath);
		    if ($compiler=~ m@^(/|\w\:[/\\])@) {
			$actualPath = $compiler;
		    }
		    else {
			$actualPath = Cwd::cwd() . "/" . $compiler;
		    }
		    @foundPaths = (@foundPaths, $actualPath);
		}
		# While searching, append .exe if appropriate.
		elsif($ENV{HostType}=~m@^NT$@) {
		    $compiler = $compiler . ".exe";
		    if (-x $compiler) {
			my($actualPath);
			if ($compiler=~ m@^\w:[/\\]@) {
			    $actualPath = $compiler;
			}
			else {
			    $actualPath = Cwd::cwd() . "/" . $compiler;
			}
			@foundPaths = (@foundPaths, $actualPath);
		    }
		}
		if (-l $compiler) {
		    if (not defined($orig_dir)) {
			$orig_dir = Cwd::cwd();
		    }
		    my($compiler_dir) = File::Basename::dirname($compiler);
		    $compiler = readlink $compiler;
		    chdir $compiler_dir;
		}
		else {
		    last;
		}
	    }
	    if (@foundPaths) {
		last;
	    }
	}
	if (defined($orig_dir)) {
	    chdir $orig_dir;
	}
    }

    my($id);
    my($host) = $HostTranslate{$ENV{HostType}};
    if (@foundPaths) {
        require ("$ENV{PSETHOME}/mb/trygcc.pl");
        require ("$ENV{PSETHOME}/mb/trycompiler.pl");
	foreach $actualPath (@foundPaths) {
	    if (-f $actualPath) {
		if (not ($actualPath =~ m@^(/|\w\:\\)@)) {
		    $actualPath = Cwd::cwd() . "/" . $actualPath;
		}

		# Invoke trygcc.pl with $actualPath to get $id.
		$id = trygcc($host, $actualPath);
		if (not (defined($id))) {
		    # Failing that, invoke trycompiler.pl with platform name and $actualPath to get $id.
		    $id = trycompiler($host, $actualPath);
		}
		if (defined($id)) {
		    # Compiler $id is identified.
		    last;
		}
	    }
	}
	if(not (defined($id))) {
	    if ($is_compiler) {
		($fp) = @foundPaths;
		&dismb_msg::msg("Possible compiler (\$1) could not be identified.", $fp);
	    }
	    return undef;
	}
    }
    else {
        require ("$ENV{PSETHOME}/mb/compilerid.pl");
	# Invoke compilerid.pl with platform name and compiler name to get $id.
	# If the log is difficult, we run into parentheses and it's not worth the trouble to get them into the command argument.
	if (($compilerName !~ m@[\(\)]@) and ($compilerName !~ m@\*@)) {
	    $id = compilerid($host, $compilerName);
	}
	if(not (defined($id))) {
	    if (DifferentBuildHostOK()) {
		if ($is_compiler) {
		    &dismb_msg::msg("Possible compiler (\$1) could not be identified.", $compilerName);
		}
	    }
	    else {
		if ($is_compiler) {
		    &dismb_msg::err("Possible compiler could not be identified, "
		           . "due to \$1 not found on this host (\$2).\n", $compilerPath, $ENV{HOST});
		}
	    }
	    return undef;
	}
    }
    ($id) = split /\n/, $id;
    chomp $id;
    return $id;
}

1;
