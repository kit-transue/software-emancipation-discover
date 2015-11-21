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
# Description:	Based on build log information, create a file consisting of parse
#		lines, one line per source file.  Also extract a list of filenames
#		that are to be included into the model.

require ("$ENV{PSETHOME}/mb/IMUtil.pl");
require ("$ENV{PSETHOME}/mb/dismb_lib.pl");
require ("$ENV{PSETHOME}/mb/dismb_msg.pl");
require (FindFile("$ENV{PSETHOME}/mb/GetBuildLog.pl"));
require (FindFile("$ENV{PSETHOME}/mb/IdentifyCompiler.pl"));
require (FindFile("$ENV{PSETHOME}/mb/TrackWorkingDirectory.pl"));
use File::Basename;

my(%DirRefs);
my($BadDirCount);

sub FilterBuildLog::InitDirRefs
{
    %DirRefs = ();
    $BadDirCount = 0;
}
sub FilterBuildLog::ConcludeDirRefs
{
    if (not DifferentBuildHostOK()) {
	$TotalDirCount = keys %DirRefs;
        $nbad = $BadDirCount;

	# A fuzzy test for whether the number of non-found directories
	# seems to deserve a warning.
	$ok = ($nbad + 1) * ($nbad + 1) <= $TotalDirCount + 1;
	if (not $ok) {
	    &dismb_msg::err("\$1 directory paths (out of \$2) do not exist on this host (\$3).", $nbad, $TotalDirCount, $ENV{HOST});
	    &dismb_msg::wrn("Normally, the information model should be built on the same host where the software is built. Otherwise, set VAR=BuildHost and COMPILER appropriately in Setup.txt.");
	}
    }
}
sub FilterBuildLog::NoteDirRef
{
    if (not DifferentBuildHostOK()) {
	my($path) = @_[0];
	if (not (exists $DirRefs{$path})) {
	    $DirRefs{$path} = 0;
	    if (not (-d $path)) {
		&dismb_msg::err("Build log refers to \$1, not found on this host (\$2).", $path, $ENV{HOST});
		$BadDirCount += 1;
	    }
    	}
    }
}


sub FilterBuildLog::Filter(@_)
{
    print_verbose("Filtering build log.");
    my($CURRENT_DRIVE);
    my(%CompileFileNameHash)=();				# unique list of compilable files
    my($CURRENT_DIR);						# Current directory undefined.
    my(@DirectoryStack)=($CURRENT_DIR);				# Initialize first pushd/popd directory.
    my($build_log_origin);
    my($need_file) = 0;						# Whether we're looking for a file for an object.
    my($FullLine)="";
    FilterBuildLog::InitDirRefs();
    if(!open(BuildLog,$GetBuildLog::BuildLogName)){
	if (-e $GetBuildLog::BuildTableFileName) {
	    # No build log digestion required.	    
	    return;
	}
	else {
	    &dismb_msg::die("Could not open build log, \$1.", $GetBuildLog::BuildLogName);
	}
    }
    if(!open(BuildTableFile,">$GetBuildLog::BuildTableFileName")) {
	&dismb_msg::die("Could not open dismb compiler flags file, \$1.", $GetBuildLog::BuildTableFileName);
    }

    while(<BuildLog>)
    {
	if (m@^$@) { next; }					# If line is empty, ignore it.
	chomp();						# Remove trailing LineFeed from end of line.
	if (m@^~table~(.*)$@) {
	    # Line is predigested in build table format.
	    if (! print(BuildTableFile ($1 . "\n")) ) {
		&dismb_msg::die("Failure writing to file, \$1 : \$2", $GetBuildLog::BuildTableFileName, $!);
	    }
	    next;
	}
	if (m@^~\w+~@) {
	    $FinalLine=$_;
	}
	elsif(m@\\$@)						# If line ends with a backslash
	{							# save it and continue.
	    s@\\$@@;
	    $FullLine="$FullLine $_";
	    next;
	}
	else							# Otherwise, set up final line.
	{
	    $FinalLine="$FullLine $_";
	    $FullLine="";
	}

	if($ENV{HostType}=~m@^NT$@) {
	    split(m@(\|\||\|])@,$FinalLine);
	}
	else {
	    split(m@(;|\|\||\|])@,$FinalLine);
	}

	foreach $_ (@_)
	{
	    #================================== Clean up log. ====================================
	    my($original_line) = $_;
	    s@\s+@ @g;						# Two+ consecutive tabs/spaces = 1 space.
	    s@^ +@@;						# Remove spaces from beginning of line.
	    s/^(|\t)@(.*)$/$1$2/;				# Remove initial @.

	    #======================= Set up triggers for compile lines. ==========================
	    # Match for compiler path in $_.
	    my($compilerPath) = "";
	    my($afterCompilerPath);
	    if ($_ =~ m@^((\w:)?[^ =:]*) +(.*)$@) {
		$compilerPath = $1;
		$afterCompilerPath = $3;
	    }
	    my($compilerId) = "";
	    if ($compilerPath =~ m@^~@) {
		# A trigger, not a compiler.
	    }
	    elsif ($compilerPath =~ m@^\s*$@) {
		# Nothing really there; an artifact of our processing the line.
	    }
	    elsif ((keys %CompilerCommand) > 0) {
		$compilerId = $CompilerCommand{$compilerPath};
	    }
	    elsif ($compilerPath =~ m@^(Initial|making|\*+|cd)$@ or m@^[^\s]+:\s+(Enter|Leav)ing directory@) {
		# Destined to be a working directory trigger.
	    }
	    else {
	        $compilerId = FilterBuildLog::IdentifyCompiler($compilerPath);
	    }
	    if ($compilerId ne "") {
		if (defined($afterCompilerPath)) {
		    # Remove the compiler name.
		    $_ = $afterCompilerPath;
		}
		$_ = "~COMPILE". $compilerId . "~ " . $_;
	    }


	    #=============== Set up triggers to keep track of current working directory. =========
	    $_ = FilterBuildLog::TrackWorkingDirectory($_);

	    #========================= Clean up compile line arguments. ==========================
	    if (m@^~COMPILE\w+(\([^\(\)]*\))*~@)		# Fix compile lines with relative paths.
	    {
		if($ENV{HostType}=~m@^NT$@)
		{
		    s@\\@/@g;					# Change C:\blah\foo to C:/blah/foo.
		}
		s@ @ SPACE @g;					# Double-space, for better matching.
		s@$@ @;					        # Append a space, for easier matching.
		foreach $SourceExtension (@ValidSourceExtensionArray)			# Tag source file names.
		{
		    if($ENV{HostType}=~m@^NT$@)
		    {
			s@ +([/\\][^ ]+\.$SourceExtension) @ SOURCE: $1 @gi;		# for full paths
			s@ +([^/\\\-][^ ]*\.$SourceExtension) @ SOURCE: $1 @gi;		# for relative paths
			s@ +("[/\\][^"]+\.$SourceExtension") @ SOURCE: $1 @gi;		# for full paths
			s@ +("[^/\\\-"][^"]*\.$SourceExtension") @ SOURCE: $1 @gi;	# for relative paths
		    }
		    else {
			s@ +([/\\][^ ]+\.$SourceExtension) @ SOURCE: $1 @g;		# for full paths
			s@ +([^/\\\-][^ ]*\.$SourceExtension) @ SOURCE: $1 @g;		# for relative paths
			s@ +("[/\\][^"]+\.$SourceExtension") @ SOURCE: $1 @g;		# for full paths
			s@ +("[^/\\\-"][^"]*\.$SourceExtension") @ SOURCE: $1 @g;	# for relative paths
		    }
		}
		s@ $@@;						# Remove the appended space.
		s@ SPACE @ @g;					# Undouble the spaces.

		if ($_ !~ m@SOURCE@) {
		    &dismb_msg::msg ("No source file found in line: \$1", $original_line);
		}
	    }

	    #======================= If not a trigger statement, continue. ===================================
	    if(! m@^~\w+(\([^\(\)]*\))*~@) {			# If not ~something~, just get next line.
		if (defined $ENV{LogIsJustFiles} ? $ENV{LogIsJustFiles} : $allLogIsFiles) {
		    &dismb_msg::err ("Compiler not recognized for build line: \$1", $original_line);
	    	}
		next;
	    }

	    #============================== Clean up path names. =============================================
	    s@/\. @ @g;						# Change 'blah/blah/.' to 'blah/blah'.
	    s@/ @ @g;						# Change 'blah/blah/' to 'blah/blah'.
	    while(s@/\./@/@){}					# Change 'blah/./blah' to 'blah/blah'.
	    while(s@/[^\.][^/ ]+/\.\.@@){}			# Change /foo/blah/../flip to /foo/flip.
	    while(s@//@/@){}					# Change /foo///blah to /foo/blah.

	    #================================= Translate paths. ==============================================
	    #s@([^s])/space/apps/@$1/net/psales/space/apps/@;	# sturner removed 011031: over-specific?
	    if (@TranslatePathsList and not m@^~bl~@) {
		if($ENV{HostType}=~m@^NT$@) {
		    s@\\@/@g;					# Change C:\blah\foo to C:/blah/foo.
		}
		my($i) = 0;
		my($log_path);
		while (2*$i < scalar(@TranslatePathsList)) {
		    my($log_path) = @TranslatePathsList[2*$i];
		    my($actual_path) = @TranslatePathsList[2*$i+1];

		    # Match must be bounded on the left by a non-pathname character,
		    # and on the right by a non-filename character.
		    if($ENV{HostType}=~m@^NT$@) {
			s@([^/\w]|-I|/I)$log_path([/\W])@$1$actual_path$2@gi;
		    }
		    else {
			s@([^/\w]|-I)$log_path([//\W])@$1$actual_path$2@g;
		    }
		    $i += 1;
		}
	    }

	    #====================== Write compile statements to flags files. =================================
	    if (m@^~COMPILE(\w+(\([^\(\)]*\))*)~@)		# Match compile lines.
	    {
		my($compilerId) = $1;
		my(@CompileFileNameList)=();
		while(s@SOURCE: "([^"]+)"@@)			# For each source in compile line, remove it ...
		{
		    push(@CompileFileNameList,$1);		# ... and add it to the sources array.
		}
		while(s@SOURCE: ([^ ]+)@@)			# For each source in compile line, remove it ...
		{
		    push(@CompileFileNameList,$1);		# ... and add it to the sources array.
		}
		s@^~([^~]+)~ +(.*)@$2@;				# Remove the ~COMPILExxx~ "trigger".

		my($Java_Classpath)="";

		if(m@ CLASSPATH: @)
		{
		    s@ CLASSPATH: (.*$)@@;
		    $Java_Classpath = $1;
		    # For java, used to elaborate the classpath, and then append $ENV{path_separator}$Java_Classpath@;
		}

		foreach my $CompileFile (@CompileFileNameList)		# For each source file in sources array
		{
		    if(! exists($CompileFileNameHash{$CompileFile})) {	# if source not encountered yet, write out to flags files
			my($cumg, $mgroup_id);
		        if (defined $dismb_msg::message_group_id{"compilation units"}) {
			    $cumg = $dismb_msg::message_group_id{"compilation units"};
			    &dismb_msg::push_new_message_group("parse file", $cumg);
			    $mgroup_id = &dismb_msg::current_auto_message_group();
			}
			if (defined $build_log_origin) {
			    &dismb_msg::inf("Parsing \$1 for: \$2", $CompileFile, $build_log_origin);
			}
			else {
			    &dismb_msg::inf("Parse file is \$1.", $CompileFile);
			}

			my($wd) = $CURRENT_DIR;
			my($is_absolute) = ($CompileFile =~ m@^(|[a-zA-Z]:)[/\\]@) ? 1 : 0;
			if (not (defined $wd)) {
			    # If there's no known working directory for the compilation, use the file's directory,
			    # if absolute.
			    if ($is_absolute) {
				if ($CompileFile =~ m@^(.*)[/\\][^/\\]*@) {
				    $wd = $1;
				}
			    }
		    	}
			if (not (defined $wd)) {
			    &dismb_msg::die ("Cannot determine working directory for compiling \$1.\n"
				 . "Use InitialWorkingDirectory in Setup.txt.", $CompileFile);
			}
			$CompileFileNameHash{$CompileFile}="$ProjectNameList[0]\t$wd\t$CompileFile\t$compilerId\t$_";
			if ($compilerId =~ m@^([^\(]*)(\(.*)$@) {
			    # has arguments
			    &dismb_msg::msg ("Preparing compilation with \$1 \$2 \$3 \$4", "$1.xml", $2, $_, $CompileFile);
			}
			else {
			    &dismb_msg::msg ("Preparing compilation with \$1 \$2 \$3", "$compilerId.xml", $_, $CompileFile);
			}
			if ($is_absolute) {
			    FilterBuildLog::NoteDirRef(dirname($CompileFile));
			}
                        
			if (! print(BuildTableFile "$ProjectNameList[0]\t$wd\t$CompileFile\t$mgroup_id\t$compilerId\t$_\n") ) {
                            &dismb_msg::die("Failure writing to file, \$1 : \$2", $GetBuildLog::BuildTableFileName, $!);
                        }
			if (defined $cumg) {
			    &dismb_msg::pop_new_message_group();
			}
		    }
		    $need_file = 0;  # $CompileFile meets the current need.
		}
		next;
	    }

	    #==================== Keep track of origin in build log. ==========================
	    # This gets incorporated into the parse file's message group.
	    if (m@^~bl~\s*(.*(\S|$))\s*@) {
		if ($1 eq "") {
		    undef $build_log_origin;
		}
		else {
		    $build_log_origin = $1;
		}
	    }

	    #==================== Keep track of working directory for path expansion. ========================
	    if(m@^~(pushd|cd)~ +(/.*\S)\s*$@)				# Match absolute-path cd lines.
	    {
		$CURRENT_DIR="$2";					# Keep track of working directory.
		FilterBuildLog::NoteDirRef($CURRENT_DIR);
	    }
	    elsif(m@^~(pushd|cd)~ +(\\.*\S)\s*$@)			# Match absolute-path cd lines (NT).
	    {
		$CURRENT_DIR="${CURRENT_DRIVE}$2";			# Keep track of working directory.
		FilterBuildLog::NoteDirRef($CURRENT_DIR);
	    }
	    elsif(m@^~(pushd|cd)~ +([A-Za-z])(:.*\S)\s*$@)		# Match absolute-path cd lines (NT).
	    {
		$CURRENT_DIR="$2$3";					# Keep track of working directory.
		$CURRENT_DRIVE="$2:";					# Keep track of working directory.
		FilterBuildLog::NoteDirRef($CURRENT_DIR);
	    }
	    elsif(m@^~(pushd|cd)~ +([^/ ](|.*\S))\s*$@)			# Match relative-path cd lines.
	    {
		$CURRENT_DIR="$CURRENT_DIR/$2";				# Keep track of working directory.
		FilterBuildLog::NoteDirRef($CURRENT_DIR);
	    }
	    elsif(m@^~(pushd|cd)~@) {
		&dismb_msg::err("Failed to process working directory change: \$1", @_);
	    }
	    if (defined($CURRENT_DIR)) {
		$CURRENT_DIR=~s@\\@/@g;
	    }
	    if(m@^~pushd~@)
	    {
		push(@DirectoryStack,$CURRENT_DIR);
	    }
	    elsif(m@^~popd~@)
	    {
		pop(@DirectoryStack);
		$CURRENT_DIR= ($#DirectoryStack >= 0) ? $DirectoryStack[$#DirectoryStack] : undef;
	    }

	    #==================== Keep track of whether an object needs a file. ========================
	    if (m@^~begin_object~\s*$@)
	    {
		$need_file = 1;
	    }
	    elsif (m@^~end_object~ (.*\S)\s*$@)
	    {
		if ($need_file) {
		    # The need is unmet.
		    &dismb_msg::wrn("No compilation unit found for \$1.", $1);
		}
		$need_file = 0;
	    }
	}
    }
    close(BuildLog);
    close(BuildTableFile);
    FilterBuildLog::ConcludeDirRefs();
}

1;
