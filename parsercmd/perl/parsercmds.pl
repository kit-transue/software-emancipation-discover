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
# Convert a list of source files for native compilation
# to a list of source files for SET parsers, such as aset_CCcc.

BEGIN { push @INC, "$ENV{PSETHOME}/mb"; }
use File::Path;
use Cwd;
require ("$ENV{PSETHOME}/mb/dismb_msg.pl");

sub parsercmds::is_NT
{
  my $val = 0;
  if ( "$ENV{'OS'}" eq "Windows_NT" ) {
    $val = 1;
  }
  return $val;
}

sub parsercmds::filter {

    # number of arguments must be 2.
    if ($#_ != 1) {
	&dismb_msg::die("usage: perl parsercmds.pl <file of native parses> <output file>");
    }
    $owd = cwd();
    if (not defined $ENV{'ADMINDIR'}) {
	&dismb_msg::die("ADMINDIR not defined.");
    }
    mkpath("$ENV{'ADMINDIR'}/tmp");
    $optionsTempFile = "$ENV{'ADMINDIR'}/tmp/options$$";
    open NATIVES, "<$_[0]" or &dismb_msg::die("Can't open file of native parses \$1: \$2", $_[0], $!);
    open OUTPUT, ">$_[1]" or &dismb_msg::die("Can't open output file \$1: \$2", $_[1], $!);
    while ($line = <NATIVES>) {
	my($proj, $wd, $sourceName, $mgroup_id, $compiler, $compilerOptions);
	$line =~ s/(\r\n|\n).*//;
	if ($line =~ m@^([^\t]*)\t([^\t]*)\t(|\D[^\t]*)\t([^\t]*)\t([^\t]*)$@) {
	    # old format
	    ($proj, $wd, $sourceName, $compiler, $compilerOptions) = ($1,$2,$3,$4,$5);
	}
	elsif ($line =~ m@^([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)$@) {
	    # new format, with parse file message group id
	    ($proj, $wd, $sourceName, $mgroup_id, $compiler, $compilerOptions) = ($1,$2,$3,$4,$5,$6);
	}
	&dismb_msg::init_auto_message_group(defined($mgroup_id) ? $mgroup_id : &dismb_msg::current_auto_message_group);

	# Create new message group for translation of flags for this file
	&dismb_msg::push_new_message_group("parser flags configuration for $sourceName");

	my($logpresent_flags) = &dismb_lib::arglist_to_str(&dismb_msg::lp_flags("--lpservice"));

	$parser = "";
	while ($compiler ne "") {
	    unlink "$optionsTempFile";

	    # Break out arguments from compiler id.
	    # For example, sunccpp(/usr/local/SUNWspro) causes
	    # ($compiler, $envargs) = sunccpp, "/usr/local/SUNWspro" 
	    my $arg;
	    my $envargs = "";
	    while (($compilr, $arg) = ($compiler =~ /(.*)\(([^\(\)]*)\)$/)) {
		$envargs = "\"" . $arg . "\" " . $envargs;
		$compiler = $compilr;
	    }

	    my $comp = "$ENV{'PSETHOME'}/lib/$compiler.xml";
	    if (defined &FindFile) {
		$comp = FindFile($comp);
	    }
	    my $exe = "parsercmd";
	    if (&parsercmds::is_NT) {
		$exe .= ".exe";
	    }
	    else {
		$exe = "$ENV{'PSETHOME'}/bin/" . $exe;
	    }
	    if ("$compiler" eq "Compiler name ERROR!!!") {
		# Received secret code from the Project Editor.
		# Give a useful diagnostic.
		&dismb_msg::pop_new_message_group;
		&dismb_msg::pop_new_message_group;
		my $ext;
		if (! (($ext) = ($sourceName =~ /^.*(\.[^.]*)$/))) {
		    $ext = $sourceName;
		}
		&dismb_msg::die("Unsupported source file type: \$1", $ext);
	    }
	    elsif (-e "$comp") {
		&dismb_msg::msg("Running \$1 with \$2 and args \$3.", $exe, $comp, $envargs);
		if (&parsercmds::is_NT) {
		    open OO, "| $exe \"$comp\" $envargs $logpresent_flags >\"$optionsTempFile\"" or &dismb_msg::die("invoking parsercmd : \$1", $!);
		}
		else {
		    open OO, "| \"$exe\" \"$comp\" $envargs $logpresent_flags >\"$optionsTempFile\"" or &dismb_msg::die("invoking parsercmd : \$1", $!);
		}
		print OO "\"$sourceName\" $compilerOptions\n";
		close OO;
	    }
	    else {
		&dismb_msg::pop_new_message_group;
		&dismb_msg::pop_new_message_group;
		&dismb_msg::die("Configuration file \$1 is missing.", "$comp");
	    }
	    if (-e "$optionsTempFile") {
		open OPTIONS, "<$optionsTempFile";
		$resultLine = <OPTIONS>;
		close OPTIONS;
		# Eliminate leading whitespace and copy only up to newline.
		&dismb_msg::msg("Output from parsercmd: \$1", $resultLine);
		if ($resultLine =~ /^\s*([^\n]*)$/) {
		    $resultLine = $1;
		}
		else {
		    &dismb_msg::pop_new_message_group;
		    &dismb_msg::pop_new_message_group;
		    &dismb_msg::die("Failed to trim result of parsercmd.");
		}
		# Grab first word and remainder.
		if ($resultLine =~ /^(\w+) (.*)$/) {
		    $parser = $1;
		    $parserOptions = $2;
		}
		elsif ($resultLine =~ /^(\w+)$/) {
		    $parser = $1;
		    $parserOptions = "";
		}
		else {
		    $parser = "";
		}
	    }
	    else {
		&dismb_msg::pop_new_message_group;
		&dismb_msg::pop_new_message_group;
		&dismb_msg::die("Invocation of parsercmd \$1 for \$2 did not produce output file \$3.", "$comp", $sourceName, $optionsTempFile);
	    }
	    my($extraOptions) = $DISIM::ExtraParserFlags{$compiler};
	    if (defined $extraOptions) {
		$parserOptions .= (" " . $extraOptions);
	    }
	    $compiler = "";
	    if ($parser eq "reconfigure") {
		# Grab last word of "options".
		my(@opts) = &dismb_lib::argstr_to_list($parserOptions);
		$compiler = pop @opts;
	    }
	    else {
		if (not (defined $extraOptions) and defined $DISIM::UniversalExtraParserFlags) {
		    $parserOptions .= (" " . $DISIM::UniversalExtraParserFlags);
	        }
	    }
	}

	if ($parser eq "C" or $parser eq "CPP") {

	    # Construct new flag for parser, which will say where working directory is
	    my($newParserOptions) = "";
	    $newParserOptions .= "--relative_path_base";
	    $newParserOptions .= " \"$wd\" ";

	    # Replace back slashes with forward slashes
	    $newParserOptions =~ s%\\%/%g;

	    # Remove trailing slashes
	    $newParserOptions =~ s%/+$%%;

	    # Add all old flags
	    $newParserOptions .= "$parserOptions";

	    $parserOptions = "$newParserOptions";
	}

	if ($parser ne "") {
	    if (defined($mgroup_id)) {
		print OUTPUT "$proj\t$wd\t$sourceName\t$mgroup_id\t$parser\t$parserOptions\n";
	    }
	    else {
		print OUTPUT "$proj\t$wd\t$sourceName\t$parser\t$parserOptions\n";
	    }
	}
	else {
	    &dismb_msg::err("Failed to translate options for \$1.", "$line");
	}

	# Pop flags translation message group
	&dismb_msg::pop_new_message_group;
	&dismb_msg::pop_new_message_group;
    }
    close NATIVES;
    close OUTPUT;
}
return 1;
