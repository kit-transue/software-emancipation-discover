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
# Usage:  	perl GetBuildLog.pl
# Description:	Obtain a list of application source compile lines and indicate/list all source
#		and header files that could have been included

# This file can be executed in 2 ways.
#    1. standalone, to obtain a build log as described above.
#          a. by executing this file body, modified to obtain the build log.
#          b. as per a Setup.txt containing just one BUILD line, BUILD=Custom,
#             with the same effect as (a.) above.
#          c. as per the BUILD= lines in Setup.txt, in which case
#             this file should not be customized in the obsolete fashion
#             wherein executing its body directly obtains the build log.
#    2. via "require", so that the caller can be configured via a Setup.txt which contains
#       BUILD= lines.  Each line is checked to ensure that a corresponding build-log-getting
#       function exists here.

use File::Basename;
use Cwd;
require ("$ENV{PSETHOME}/mb/IMUtil.pl");
require ("$ENV{PSETHOME}/mb/dismb_lib.pl");
require ("$ENV{PSETHOME}/mb/dismb_msg.pl");
if ($0 =~ m@GetBuildLog[^/\\]*@) {
    $FlagsFormat="[-n] [-V]";
}

$GetBuildLog::BuildLogName="$ENV{ADMINDIR}/config/build.log";
$GetBuildLog::BuildTableFileName="$ENV{ADMINDIR}/config/compiler.flg";
$GetBuildLog::FunctionLine;

################################################################
# The following functions support ParseDSX.
#
sub GetBuildLog::ResolvePathName(@_)
{
	my($CurrentWorkingDirectory,$FileName)=@_;
	if($FileName!~m@^/@ && $FileName!~m@^[a-zA-Z]:@)
	{
		$FileName=~s@^@$CurrentWorkingDirectory/@;
	}
	$FileName=~s@\\@/@g;
	$FileName=~s@/./@/@;
	while($FileName=~s@/[^\.][^/ ]+/\.\.@@){}	# change /foo/blah/../flip to /foo/flip
	return($FileName);
}

sub GetBuildLog::DSPDSWFile(@_)
{
	$_ = $_[0];
	if(m@^\s*(.+\.[dD][sS][pP])\s*@)
	{
		push(@GetBuildLog::DSPList,$1);
	}
	if(m@^\s*(.+\.[dD][sS][wW])\s*@)
	{
		push(@GetBuildLog::DSWList,$1);
	}
}

sub GetBuildLog::DSPDSWListFile(@_)
{
	if(!open(DSPDSWListFile,"$DSPDSWListFileName")){ShowUsageError("Cannot open DSW/DSP list file, \$1, for reading", $DSPDSWListFileName);}
	while(<DSPDSWListFile>)
	{
		chomp();
		GetBuildLog::DSPDSWFile($_);
	}
	close(DSPDSWListFile);
}

sub GetBuildLog::DSWFiles(@_)
{
	foreach my $DSWFileName (@GetBuildLog::DSWList)
	{
		my($CurrentWorkingDirectory)=$DSWFileName;
		$CurrentWorkingDirectory=~s@[\\/][^\\/]+$@@;
		if(!open(DSWFile,"$DSWFileName")){ShowUsageError("Cannot open DSW file, \$1, for reading", $DSWFileName);}
		while(<DSWFile>)
		{
			chomp();
			if(m@^Project:\s*[^=]+=(.*\.[dD][sS][pP])\s*-.*$@)
			{
				my($DSPFileName)=GetBuildLog::ResolvePathName($CurrentWorkingDirectory,$1);
				push(@GetBuildLog::DSPList,$DSPFileName);
			}
		}
		close(DSWFile);
	}
}

sub GetBuildLog::GetExtraIncludeArgs
{
    my($args);
    if ($ENV{HostType}=~m@^NT$@) {
	$args = &dismb_lib::backtick("$ENV{'PSETHOME'}\\bin\\scandsp.exe", "--get_include_path");
	if ($?) {
	    if ($args ne "") {
		&dismb_msg::msg ("Could not get additional includes for Microsoft Visual C/C++ user. \$1", $args);
	    }
	    $args = "";
	}
	chomp($args);
    }
    return $args;
}

sub GetBuildLog::DSPFiles(@_)
{
	my($IncludeArgs) = GetBuildLog::GetExtraIncludeArgs();
	foreach my $DSPFileName (@GetBuildLog::DSPList)
	{
		my($CurrentWorkingDirectory)=$DSPFileName;
		my($FlagsList)="NoFlags";
		$CurrentWorkingDirectory=~s@[\\/][^\\/]+$@@;

		$CurrentWorkingDirectory=GetBuildLog::ResolvePathName("", $CurrentWorkingDirectory);

		print(LOG "~DSP~ $DSPFileName\n");
		print(LOG "~cd~ $CurrentWorkingDirectory\n");

		@SourceFileNameList=();
		if(!open(DSPFile,"$DSPFileName")){ShowUsageError("Cannot open DSP file, \$1, for reading", $DSPFileName);}
		while(<DSPFile>)
		{
			chomp();
			if(m@^!(IF|ELSEIF).*Win32 Debug@i .. m@^# ADD LINK32@i)
			{
				if(m@# ADD CPP(.*)$@)
				{
					my($CurrentFlags)=$1;
					if($FlagsList=~m@^NoFlags$@)
					{
						$FlagsList=$CurrentFlags;
					}
				}
			}

			if(m@SOURCE=(.*[^\s])\s*@)
			{
				$matchedFileName = $1;
				if ($matchedFileName=~m/^\"(.*)\"$/) {
				    $matchedFileName=$1;
				}
                                my($SourceFileName)=GetBuildLog::ResolvePathName($CurrentWorkingDirectory,$matchedFileName);
                                if(($SourceFileName=~m@$ENV{sharedSrcRoot}@)
				   or (($ENV{HostType}=~m@^NT$@) and ($SourceFileName=~m@$ENV{sharedSrcRoot}@i))){
					push(@SourceFileNameList,$SourceFileName);
				}
			}
		}
		close(DSPFile);
		if($FlagsList=~m@^NoFlags$@)
		{
			print(LOG "ERROR:  Could not find flags in $DSPFileName\n\tFlagsList=$FlagsList\n");
		}
		foreach my $SourceFileName (@SourceFileNameList)
		{
			if($SourceFileName=~m@\.(C|cc|CC|cxx|c\+\+|cpp|c)$@i)
			{
				if ($SourceFileName =~ m@ @) {
				        $SourceFileName = "\"" . $SourceFileName . "\"";
				}
				print(LOG "cl $FlagsList $IncludeArgs $SourceFileName\n");
			}
		}
	}
}

##################################################
# Pass this an argument, and the arg is taken as a DSW/DSP
# file to use as the build.  Otherwise, looks in
# $ADMINDIR/config/DSXList.txt.
##################################################
sub GetBuildLog::ParseDSX(@_)
{
	$file_name = @_[0];
	@GetBuildLog::DSPList=();
	@GetBuildLog::DSWList=();

	if(!open(LOG, ">$GetBuildLog::BuildLogName")){&dismb_msg::die("Can't open \$1 \$2", $GetBuildLog::BuildLogName, $!)};

	if (defined $file_name) {
	    GetBuildLog::DSPDSWFile($file_name);
	}
	else {
	    $DSPDSWListFileName="$ENV{ADMINDIR}/config/DSXList.txt";

	    GetBuildLog::DSPDSWListFile();
	}
	GetBuildLog::DSWFiles();
	GetBuildLog::DSPFiles();
	close(LOG);
}

sub GetBuildLog::ScanDSXFile
{
    my($ScanOutFileName)="$ENV{ADMINDIR}/config/scandsp.flg";
    $_= $_[0];
    if(m@^\s*(.+\.[dD][sS][pPwW])\s*@)
    {
	    my(@cmd) = ("$ENV{'PSETHOME'}/bin/scandsp", $1,
			"--output", $ScanOutFileName,
			&dismb_msg::lp_flags("--lpservice"));
	    if (defined $dismb_msg::message_group_id{"compilation units"}) {
		my($cumg) = $dismb_msg::message_group_id{"compilation units"};
		@cmd = (@cmd, "--compilation_units_message_group", $cumg);
	    }
	    &dismb_msg::msg("Invoking scandsp: \$1",
			    &dismb_lib::arglist_to_str(@cmd), normal_sev);
	    run_command(@cmd);
    }
    open(InFile,"$ScanOutFileName");
    open(OutFile,">>$GetBuildLog::BuildLogName");
    while(<InFile>)										
    {
	    print(OutFile "~table~$_");
    }
    close(InFile);
    close(OutFile);
}


################################################################
# This is a possible replacement for ParseDSX.
# Pass this an argument, and the arg is taken as a DSW/DSP
# file to use as the build.  Otherwise, looks in
# $ADMINDIR/config/DSXList.txt.
################################################################
sub GetBuildLog::ScanDSX
{
	$file_name = @_[0];

	if (defined $file_name) {
	    GetBuildLog::ScanDSXFile($file_name)
	}
	else {
	    $DSPDSWListFileName="$ENV{ADMINDIR}/config/DSXList.txt";
	    if(!open(DSPDSWListFile,"$DSPDSWListFileName")){ShowUsageError("Cannot open DSW/DSP list file, \$1, for reading", $DSPDSWListFileName);}
	    while(<DSPDSWListFile>)
	    {
		    chomp();
		    GetBuildLog::ScanDSXFile($_);
	    }
	    close(DSPDSWListFile);
	}

	# Delete $BuildLogName so that FilterBuildLog knows to do nothing.
        if(-e $BuildLogName){unlink($BuildLogName);}
}


##############################################################################
sub GetBuildLog::ClearCase(@_)
{
	my($Argument,$View)=@_;

	######################################################################
	# Determine the view.
	my($DOListFileName)="$ENV{ADMINDIR}/config/do_files.txt";
	my($CRFileName)="$ENV{ADMINDIR}/config/config_records.txt";
	my($view_drive);
	my($ViewTagOption);
	my(@DOList)=();

	if (not (defined $View)) {
	    $View = $ENV{BUILD_VIEW_NAME};
	}
	if($ENV{HostType}=~m@^NT$@){
	    # In case of Windows, set a working directory that enables a view,
	    # based on the pattern argument.
	    my($PatternDir);

	    # Scan the pattern for slashes, double-backslashes,
	    # and dirnames that don't involve pattern-matching metacharacters.
	    if ($Argument =~ m@^\^?(([^\\\|\(\)\[\{\^\$\*\+\?\.\/]+|/|\\\\)*)@) {
		$PatternDir = $1;
		$PatternDir =~ s@\\\\@\\@g;
		while (not (-d $PatternDir)) {
		    # Remove final slash, if present.
		    $PatternDir =~ s@(/|\\)$@@;
		    # Remove final name (file or nonexistent directory).
		    if ($PatternDir =~ m@^(.*)(^|/|\\)([^/\\]+)$@) {
			$PatternDir = $1 . $2;
		    }
		    else {
			# Can't find an existing directory.
			$PatternDir = undef;
			last;
		    }
		}
	    }
	    if (not (defined $PatternDir)) {
		ShowUsageError ("The pattern \$1 fails to mention a directory.",
		    $Argument);
	    }
	    &dismb_msg::inf("Extracted directory name \$1 from pattern \$2", $PatternDir, $Argument);

	    chdir($PatternDir);
	    my($ClearCaseView)= `cleartool pwv -s`;
	    if ($ClearCaseView !~ m@([^\-\_\w]|^)NONE([^\-\_\w]|$)@i) {
		chomp($ClearCaseView);
		$ViewTagOption = $ClearCaseView
	    }
	    else {
		ShowUsageError ("The directory \$1 (from the pattern \$2) is not in a ClearCase view.",
		     $PatternDir, $Argument);
	    }
	    if (defined $View and $View ne $ViewTagOption) {
		&dismb_msg::wrn("The pattern \$1 is for a different view (\$2) from the view specified (\$3).", $Argument, $ViewTagOption, $View);
	    }
	    if ($Argument =~ m@^\^?(\w)\:(\\\\|/)$ViewTagOption(|(\\\\|/).*)$@i) {
		# Pattern is primed for the exact name of the derived object.
	    }
	    elsif ($Argument =~ m@^\^?(\w)\:(.+)$@i) {
		# Pattern appears to be for the drive where VOBs are mounted directly.
		$view_drive = $1;
	    }
	}
	else {
	    $ViewTagOption = $View;
	    if (!defined($ViewTagOption)) {
		my($ClearCaseView)=`cleartool pwv -s`;
		if ($ClearCaseView !~ m@([^\-\_\w]|^)NONE([^\-\_\w]|$)@i) {
		    chomp($ClearCaseView);
		    $ViewTagOption = $ClearCaseView;
		}
		elsif (defined $GetBuildLog::FunctionLine) {
		    ShowUsageError("Ensure you are in a ClearCase view, or specify a view as the second argument in the line:\n"
			    . "\$1\n" . "in Setup.txt.", $GetBuildLog::FunctionLine);
		}
		else {
		    ShowUsageError("Ensure you are in a ClearCase view, or specify a view as the second argument to \"ClearCase\" in Setup.txt.");
		}
	    }
	    if ($Argument =~ m@^\^?(/[^/]*)*/$ViewTagOption(|/.*)$@i) {
		# Pattern is primed for the exact name of the derived object.
	    }
	    else {
		# Pattern appears to be for the drive where VOBs are mounted directly.
		$view_drive = '';
	    }
	}
	print_always("Getting Derived Object list from ClearCase view \$1.....", $ViewTagOption);

	######################################################################
	# Create a list of derived objects, based on view and pattern (first arg).
	if ($Argument ne "") {
		print_always("Matching derived objects to Perl pattern \$1", $Argument);
	}
	local(*InFile);
	open(InFile,"cleartool lspriv -do -s -tag $ViewTagOption|");
	my($n_candidates) = 0;
	my($first_candidate, $last_candidate);
	while(<InFile>)
	{
		chomp();
		if(m@\.([cphx]+|a|def|dat|hh)$|stub[^/\s]*$|SunWS_cache@i)	#combined three different "if" statements
		{					# source files don't have compile lines only objects and executables
			next;				# stubs mess up model linking; SunWS_cache is a bunch of bad stuff
		}
#		if(! m@\.o@)				# Everything is first compiled in an object
#		{
#			next;
#		}
		if (not (defined $view_drive)) {
			# Pattern is primed for the exact name of the derived object.
	        }
		elsif ($view_drive eq '' and $_ =~ m@^(/[^/]*)*/$ViewTagOption(/.*)$@i) {
			# Unix, pattern based on root-mounted vob.
			$_ = $2;
		}
		elsif ($_ =~ m@^\w\:[\\/]$ViewTagOption([\\/].*)$@i) {
			# Windows, pattern based on vob mounted on root of a drive.
			$_ = "$view_drive:$1";
		}
		$n_candidates += 1;
		if (not (defined $first_candidate)) {
			$first_candidate = $_;
		}
		$last_candidate = $_;
		if ($Argument ne "") {			# If no argument is passed, then ignore next few lines of code
			my($skip);
			if($ENV{HostType}=~m@^NT$@) {	# else only push those lines which match $Argument to @DOList
				eval { $skip = !m@^${Argument}$@io }	# NT needs a case insensitive match
			}
			else {
				eval { $skip = !m@^${Argument}$@o }	# case sensitive match for unix
			}
			&dismb_msg::fixeval;
			if ($@) {
				&dismb_msg::die("Invalid Perl pattern \$1 in \$2", $Argument, $GetBuildLog::FunctionLine);
			}
			if ($skip) {
				next;
			}
		}
		push(@DOList,$_);
	}
	close(InFile);

	&dismb_msg::inf("Selected \$1 of \$2 derived objects.", scalar(@DOList), $n_candidates);
	if(scalar(@DOList) == 0){
		if ($n_candidates != 0) {
			&dismb_msg::inf("The pattern\n    \$1\ndid not match, for example, the derived objects:\n    \$2\n    \$3", $Argument, $first_candidate, $last_candidate);
		}
		ShowUsageError("Failed to obtain list of Derived Objects.");
	}
	if(!open(DOListFile,">$DOListFileName")){&dismb_msg::die("Failed to open DO list file, \$1, for writing", $DOListFileName);}
	foreach $DerivedObject (@DOList)
	{
		print(DOListFile "$DerivedObject\n");
	}
	close(DOListFile);

	######################################################################
	# Obtain config records.
	print_always("Obtaining \$1 Configuration Records for model.", scalar(@DOList));

	my(@QueryDOList)=();					# initialize DO list with zero elements
	local(*ClearToolPipe);
	# if(!&dismb_lib::open_output(\*ClearToolPipe,"cleartool", ">", $CRFileName, "2", ">&", "1")) {  # failed on fridge
	if(!&dismb_lib::open_output(\*ClearToolPipe,"cleartool", ">", $CRFileName, "2>&", "1")) {
		&dismb_msg::die("Failed to open cleartool pipe for r/w");
	}
	foreach $DerivedObject (@DOList)
	{
		push(@QueryDOList,$DerivedObject);
		if(scalar(@QueryDOList)==100)			# for each 100 DOs, send query to cleartool
		{
			my($DOListSize)=scalar(@QueryDOList);
			print_verbose("grabbing \$1 configuration records", $DOListSize);
			print(ClearToolPipe "catcr -l @QueryDOList\n");
			@QueryDOList=();
		}
	}
	if(scalar(@QueryDOList)>0)				# finish of last DOs
	{
		my($DOListSize)=scalar(@QueryDOList);
		print_verbose("grabbing \$1 configuration records", $DOListSize);
		print(ClearToolPipe "catcr -l @QueryDOList\n");
		@QueryDOList=();
	}
	close(ClearToolPipe);

	print_always("Wrote config records to \$1", $CRFileName);

	######################################################################
	# Exclude lots of lines and write the "build log".
	local(*CRInFile);
	my($target_name);
	my($long_target_name);
	my($in_build_script) = 0;
	my($in_mvfs_object_list) = 0;
	my($refers_to_derived_object) = 0;
	my(%hosts_tracked) = ($ENV{HOST}, 1);
	open(CRInFile,$CRFileName);
	open(OutFile,">$GetBuildLog::BuildLogName");
	while($line = <CRInFile>)
	{
		# Track "Target" lines for reporting purposes.
		if ($line =~ m@^Target ([^ ]*)( .*|)$@) {
			$target_name = $1;
			if ($line =~ m@^Target (.*) built @) {
				$target_name = $1;
			}
			$long_target_name = $target_name;
			$refers_to_derived_object = 0;
			next;
		}
		# Track "Host" lines and report if it's unexpectedly different.
		if ($line =~ m@^Host ([^ ]*)( .*|)$@) {
			my($host_name) = $1;
			$host_name =~s@\"(.*)\"@$1@g;
			if ($line =~ m@^Host \"(.*)\"@) {
				$host_name = $1;
			}
			if (not exists($hosts_tracked{$host_name})) {
				if (not DifferentBuildHostOK()) {
					&dismb_msg::err("The derived object \$1 was built on \$2 rather than on this host (\$3).",
						$target_name, $host_name, $ENV{HOST});
					&dismb_msg::wrn("Normally, the information model should be built on the same host"
							. " where the software is built."
							. " Otherwise, set VAR=BuildHost and COMPILER appropriately in Setup.txt.");
				}
				$hosts_tracked{$host_name} = 1;
			}
			next;
		}
		# Track "Build Scripts".
		if ($line =~ m@^Build Script:\s@) {
			$in_build_script = 1;
			next;
		}
		if ($line =~ m@^MVFS objects:\s@) {
			$in_mvfs_object_list = 1;
			next;
		}
		if ($line =~ m@\-\-\-\-\-\-\-\-\-@) {
			# Unless the object is exempt due to including another derived object,
			# Specify the object's "Build Script" as a group of build log lines which require an output.
			if ($in_build_script == 1) {
				# Entering the build script.
				$in_build_script = 2;
				if (not $refers_to_derived_object) {
					print OutFile "~begin_object~\n";
				}
			}
			elsif ($in_build_script == 2) {
				# Leaving the build script.
				$in_build_script = 0;
				if (not $refers_to_derived_object) {
					print OutFile "~end_object~ ClearCase derived object $long_target_name\n";
				}
			}
			if ($in_mvfs_object_list == 1) {
				# Entering the list of MVFS objects.
				$in_mvfs_object_list = 2;
			}
			elsif ($in_mvfs_object_list == 2) {
				# Leaving the list of MVFS objects.
				$in_mvfs_object_list = 0;
			}
			next;
		}
		if ($line =~ m@^(directory version\s|view directory\s|derived object\s|version\s|symbolic link\s|\-\-\-\-\-\-\-\-\-)@) {
			next;
		}
		if ($line =~ m@^(Reference Time|View was|Host|Target|Variables and Options:|MVFS objects:|Build Script:)\s@) {
			next;
		}
		if ($in_mvfs_object_list == 2) {
			if ($line =~ m/^(.*)@@(\d|\d\d)\-/) {
				# a derived object
				my($path) = $1;
				if ("$target_name" ne "" and $path =~ m@^(|.*/|.*\\)\Q$target_name$@i) {
					# Having self in list is normal, and does not exempt this object.
					$long_target_name = $path;
				}
				else {
					# If the object's list of MVFS objects includes another derived object
					# (whether or not in the model) it's an OK object.
					$refers_to_derived_object = 1;
				}
			}
			next;
		}
		print OutFile $line;
	}
	close(CRInFile);
	close(OutFile);

	######################################################################
	# Finish up.
	$allLogIsFiles = 0;  # Don't complain of non-compile lines.

	print_always("Wrote build log to \$1", $GetBuildLog::BuildLogName);
}

##############################################################################
sub GetBuildLog::FixedLocationLog(@_)
{
	my($FixedLocationBuildLogName,$WorkingDir)=@_;

	if(! -r $FixedLocationBuildLogName)
	{
		ShowUsageError("Cannot access fixed location build log, \$1", $FixedLocationBuildLogName);
	}
	$GetBuildLog::BuildLogName="$ENV{ADMINDIR}/config/build.log";

	&dismb_msg::msg("Copying file \$1 to \$2", $FixedLocationBuildLogName, $GetBuildLog::BuildLogName);

	local(*InFile, *OutFile);
	open(InFile,"$FixedLocationBuildLogName");
	open(OutFile,">>$GetBuildLog::BuildLogName");

	if (defined($WorkingDir)) {
	    # The compiler's working directory is specified.
	    print(OutFile "cd \"$WorkingDir\"\n");
	}
	my($in_line) = 0;
	while(<InFile>)
	{
		$in_line += 1;
		print(OutFile "~bl~ build log $FixedLocationBuildLogName, line $in_line: $_");
		print(OutFile "$_");
	}
	close(InFile);
	close(OutFile);

	$allLogIsFiles = 0;  # Don't complain of non-compile lines.
}

##############################################################################
sub GetBuildLog::FileListFromFileSystem(@_)
{
        my($CompilerName,$FindMatchString,$PathName,$OriginalPathOption,$Extensions,$WorkingDir)=@_;
        my(@FindMatch)=();
	my(@found_count) = 0;
	local(*OutFile);
        if(!open(OutFile,">>$GetBuildLog::BuildLogName")){&dismb_msg::die("Could not open build log, \$1::BuildLogName, for writing.", $GetBuildLog);}

	if (not (defined $PathName)) {
	    if (not (defined $ENV{MainSrcRoot})) {
		if (defined $GetBuildLog::FunctionLine) {
		    ShowUsageError ("Cannot determine root directory from which to find files.\n"
			    . "Specify a third argument to the line:\n"
			    . "\$1\n" . "in Setup.txt.", $GetBuildLog::FunctionLine);
		}
		else {
		    ShowUsageError ("Cannot determine root directory from which to find files.\n"
			 . "Specify a third argument to FileListFromFileSystem in Setup.txt.");
		}
	    }
	    $PathName=$ENV{MainSrcRoot};
	}

	my(@ValidSourceExtensions);
	if (defined $Extensions) {
            @ValidSourceExtensions=split(m@\s+@,$Extensions);
	}
	if (scalar(@ValidSourceExtensions) == 0) {
	    if (defined $GetBuildLog::FunctionLine) {
		ShowUsageError ("Cannot determine file extensions to find.\n"
			. "Specify a fifth argument to the line:\n"
			. "\$1\n" . "in Setup.txt.", $GetBuildLog::FunctionLine);
	    }
	    else {
		ShowUsageError ("Cannot determine file extensions to find.\n"
		    . "Specify a fifth argument to FileListFromFileSystem in Setup.txt.");
	    }
	}
        if ( defined ($WorkingDir) && (! -d $WorkingDir) ) {
	    &dismb_msg::die("Invalid compiler working directory specified: \$1", "$WorkingDir");
	}

	&dismb_msg::msg("Finding files in ROOT:  \$1", $PathName);
	local(*InFile);
	if($ENV{HostType}=~m@^NT$@)
	{
		foreach $SourceExtension (@ValidSourceExtensions) {
			$SourceMatch="*.$SourceExtension ";
			push @FindMatch, $SourceMatch;
			}
		foreach $HeaderExtension (@ValidHeaderExtensions) {
			$HeaderMatch="*.$HeaderExtension ";
			push @FindMatch, $HeaderMatch;
			}
		if(!chdir("$PathName")){&dismb_msg::die("Could not change directory to \$1", $PathName);}
		print_verbose ("dir /s /b \$1", &dismb_lib::arglist_to_str(@FindMatch));
		if(!&dismb_lib::open_input(\*InFile, "dir", "/s", "/b", @FindMatch)) {
			&dismb_msg::die("Could not open FIND pipe:\n\tdir /s /b \$1", &dismb_lib::arglist_to_str(@FindMatch));
		}
	}
	else
	{
		foreach $SourceExtension (@ValidSourceExtensions) {
			@SourceMatch=("-name", "\"*.$SourceExtension\"", "-o");
			push @FindMatch, @SourceMatch;
			}
		foreach $HeaderExtension (@ValidHeaderExtensions) {
			@HeaderMatch=("-name", "\"*.$HeaderExtension\"", "-o");
			push @FindMatch, @HeaderMatch;
			}
		pop @FindMatch; # the last -o
		unshift @FindMatch, "(";
		push @FindMatch, ")"; # Hope this works.
		print_verbose ("find \$1 -type f \$2 -follow -print", $PathName, &dismb_lib::arglist_to_str(@FindMatch));
		if(!&dismb_lib::open_input(\*InFile, "find", $PathName, "-type", "f", @FindMatch, "-follow", "-print")) {
			&dismb_msg::die("Could not open FIND pipe:\n\tfind \$1 -type f \$2 -follow -print", $PathName, &dismb_lib::arglist_to_str(@FindMatch));
		}
	}

	if (defined($WorkingDir)) {
	    # The compiler's working directory is specified.
	}
	else {
	    # For Java compilations, if the working dir is not otherwise specified then
	    my($is_absolute) = ($PathName =~ m@^(|[a-zA-Z]:)[/\\]@) ? 1 : 0;
	    if(($CompilerName eq "javac" or $CompilerName eq "jbuilder")
	       and (not defined($WorkingDir)) and $is_absolute) {
		    # Use the find-root as the compiler's working directory.
		    if (defined $GetBuildLog::FunctionLine) {
			&dismb_msg::wrn ("Initial working directory was not specified in Setup.txt as the sixth argument in the following:\n"
			     . "\$3\n"
			     . "Using \$1 as working directory for \$2.", $PathName, $CompilerName, $GetBuildLog::FunctionLine);
		    }
		    else {
			&dismb_msg::wrn ("Initial working directory was not specified in Setup.txt.\n"
			     . "Using \$1 as working directory for \$2.", $PathName, $CompilerName);
		    }
		    $WorkingDir = $PathName;
	    }
	}
	print(OutFile "~bl~ File found in directory $PathName.\n");
	if (defined($WorkingDir)) {
	    print(OutFile "cd \"$WorkingDir\"\n");
	}
	if (defined $OriginalPathOption and not $dismb::is_NT) {
		# On Unix, a semicolon that's not quoted should not appear in the options.
		# Explanation of regular expresssion: looking for a semicolon which is preceded
		# by zero or more of any of the following: (1) Stuff quoted with ' or "
		# (2) non-quote characters, or (3) Single character quoted with \.
		if ($OriginalPathOption =~ m/^("([^"\\]|\.)*"|'([^'\\]|'.)*'|[^"'\\;]+|\\.)*;/) {
			if (defined $GetBuildLog::FunctionLine) {
			    ShowUsageError ("Semicolon (;) is invalid in compiler options in "
				    . "\$1\n", $GetBuildLog::FunctionLine);
			}
			else {
			    ShowUsageError ("Semicolon (;) is invalid in compiler options \$1 in "
				    . "Setup.txt\n", $OriginalPathOption);
			}
		}
	}
	while(<InFile>)
	{
		my($PathOption) = $OriginalPathOption;
		if (defined $PathOption) {
		}
		elsif(($CompilerName eq "javac") and (exists($ENV{CLASSPATH}))) {
			$PathOption="-classpath $ENV{CLASSPATH}";
		}
		elsif($CompilerName eq "sqlc") {
			# This can be modified to indicate -include flags to dis_sql.
			$PathOption="";
		}
		else {$PathOption="";}

		# $_ indicates a source file.
		# Fabricate a compile line as it would appear in a build log.
		# Modify this to show relevant options.
		chomp();
		if ($_ =~ m@ @) {
			$_ = "\"" . $_ . "\"";
		}
		if (not (defined($WorkingDir))) {
		    my($is_absolute) = ($_ =~ m@^(|[a-zA-Z]:)[/\\]@) ? 1 : 0;
		    if ($is_absolute) {
			my($wd) = dirname($_);
			print(OutFile "cd \"$wd\"\n");
		    }
		}
		print(OutFile "$CompilerName $PathOption $_\n");
		$found_count += 1;
	}
	close(InFile);
        close(OutFile);

	if ($found_count == 0) {
		&dismb_msg::wrn("No files found in \$1 matching extensions (\$2).",
			$PathName, (join ' ', @ValidSourceExtensions));
	}
}

################################################################
# The format of each record in a predigested table of files is
# five tab-delimited fields.
#   1. project name
#   2. working directory at the point where compiler is invoked
#   3. source file name
#   4. id of compiler, e.g. ntcl, selects .xml config file
#   5. command line arguments to compiler
################################################################
sub GetBuildLog::FixedLocationTable(@_)
{
	$FixedLocationBuildTableFileName="$_[0]";
	if (not (defined $FixedLocationBuildTableFileName)) {
	    if (not (defined $ENV{MainSrcRoot})) {
		ShowUsageError ("Cannot determine name of file table.\n"
		     . "Specify an argument to FixedLocationTable in Setup.txt.\n");
	    }
	    $FixedLocationBuildTableFileName="$ENV{MainSrcRoot}/build.table";
	}
	if(! -r $FixedLocationBuildTableFileName)
	{
		ShowUsageError("Cannot access fixed location build table, \$1", $FixedLocationBuildTableFileName);
	}

	&dismb_msg::msg("Importing file \$1 to \$2", $FixedLocationBuildTableFileName, $GetBuildLog::BuildLogName);

	open(InFile,"$FixedLocationBuildTableFileName");
	open(OutFile,">>$GetBuildLog::BuildLogName");
	my($line_no) = 0;
	while(<InFile>)
	{
		$line_no += 1;
		# Create a message group for this parse file, and insert its id into the table.
		my($straight_line) = m@^([^\n\r]*)[\n\r]*$@;    # Trim newline, taking care about tab.
		my($proj, $wd, $sourceName, $compiler, $flags);
		if (($proj, $wd, $sourceName, $compiler, $flags)
		        = ($straight_line =~ m@^([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)$@)) {
		    my($cumg);
		    if (defined $dismb_msg::message_group_id{"compilation units"}) {
			$cumg = $dismb_msg::message_group_id{"compilation units"};
			&dismb_msg::push_new_message_group("parse file", $cumg);
			&dismb_msg::inf("Parse file is \$1.", $sourceName);
		    }
		    my($mgroup_id) = &dismb_msg::current_auto_message_group();
		    print OutFile ("~table~" . (join "\t", ($proj, $wd, $sourceName, $mgroup_id, $compiler, $flags)) . "\n");
		    if (defined $cumg) {
			&dismb_msg::pop_new_message_group();
		    }
		}
		else {
		    &dismb_msg::err("Line \$1 of the build table (\$2) does not have 5 tab-separated fields.",
				$line_no, $FixedLocationBuildTableFileName);
		}
	}
	close(InFile);
	close(OutFile);
}

##############################################################################
sub GetBuildLog::Custom {
    # "BUILD=Custom" is assumed to appear just once in Setup.txt.
    # The administrator can add code in this function body, or can uncomment
    # the appropriate lines in the section below marked "OBSOLETE".
}

##############################################################################
$allLogIsFiles = 1;	# The default is to complain about non-compile-files in the log,
                   	# unless a build-processing-function is called which needs otherwise.

##############################################################################
if ($0 =~ m@GetBuildLog[^/\\]*@) {
    $THIS_FILE="GetBuildLog.pl";
    GetInstallationValues();
    SetupEnvironment();
    $ENV{ShowReleased}="off";

    my($cumg_flag) = 0;
    LOOP: foreach $arg (@ARGV) {
      if ( $cumg_flag == 1) {
        $dismb_msg::message_group_id{"compilation units"} = $arg if (! ($arg =~ /^-/) );
        $cumg_flag = 0;
      }
      if ( $arg =~ /^-compilation_units_message_group/ ) {
        $cumg_flag = 1;
      }
    }

    if (defined @BuildLogList and not ($#BuildLogList == 0 and $BuildLogList[0]->[0] eq "Custom")) {
	# Create empty build log.
	if(!open(OutFile,">$GetBuildLog::BuildLogName")) {
	    &dismb_msg::die("Could not open build log, \$1::BuildLogName, for writing.",
			    $GetBuildLog);
	}
	close(OutFile);

	my($index) = 0;
	for $FunArgsRef ( @BuildLogList ) {
		($FunctionName, @Args) = @$FunArgsRef;
		my($f) = "GetBuildLog::$FunctionName";
	        $GetBuildLog::FunctionLine = @BuildLogRef[$index];
		print_always("Getting build log via \$1.", $GetBuildLog::FunctionLine);
		&$f(@Args);
		undef $GetBuildLog::FunctionLine;
		$index += 1;
	}
    }
}
else {
    return 1;
}

################################################################
# OBSOLETE: Now choose BUILD log type in Setup.txt.
################################################################
# For builds that used clearmake, uncomment any one of the 
# following subroutines.  Uncommenting is still supported if
# BUILD=Custom is set in Setup.txt.
################################################################
# For building all files in the clearmake view
#GetBuildLog::ClearCase();

# For building only those files matching the MainSrcRoot 
# Useful when the MainSrcRoot is few or several levels deeper 
# Or to test build small directories
#GetBuildLog::ClearCase("$ENV{MainSrcRoot}");	

# For grabbing derived objects matching this PATTERN... useful only
# when the derived objects are in a diff directory than MainSrcRoot
#GetBuildLog::ClearCase("PATTERN");

################################################################
# OBSOLETE
# For builds that are fully logged, uncomment the following lines.
# If build.log files exists elsewhere or has a different name
# give the fullpath and name (retain the double quotes).
################################################################
#if(-e $GetBuildLog::BuildLogName){unlink($GetBuildLog::BuildLogName);}
#GetBuildLog::FixedLocationLog("$ENV{MainSrcRoot}/build.log");

################################################################
# OBSOLETE
# For MS VC++, uncomment _one_ of the following two lines
# and create a list of DSWs or DSPs in the file DSXList.txt.
################################################################
#GetBuildLog::ParseDSX();
#GetBuildLog::ScanDSX();

################################################################
# OBSOLETE
# For java files, uncomment the following lines then modify
# the subroutine.
################################################################
#GetBuildLog::FileListFromFileSystem("javac","java");

################################################################
# OBSOLETE
# For sql files, uncomment the following lines then modify
# the subroutine.
################################################################
#GetBuildLog::FileListFromFileSystem("sqlc","sql");

################################################################
# OBSOLETE
# For a predigested tab-delimited list of files with options.
################################################################
#GetBuildLog::FixedLocationTable();
