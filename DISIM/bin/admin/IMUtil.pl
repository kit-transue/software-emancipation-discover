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
# Common utility routines for DISIM scripts

unshift @INC, ("$ENV{PSETHOME}/mb");
require ("$ENV{PSETHOME}/mb/dismb_lib.pl");
require ("$ENV{PSETHOME}/mb/dismb_msg.pl");

%NTCommandTranslationHash=();
%UNIXCommandTranslationHash=();

$NTCommandTranslationHash{FORK}="";
$NTCommandTranslationHash{ECHO}="echo";
$NTCommandTranslationHash{RENAME}="copy";
$NTCommandTranslationHash{RMRF}="rd /S /Q";
$NTCommandTranslationHash{DELETE}="del";
$NTCommandTranslationHash{CPRF}="xcopy /E /F /I /Q";
$NTCommandTranslationHash{COPY}="copy";
$NTCommandTranslationHash{PERL}="PSETHOME\\bin\\disperl";

$UNIXCommandTranslationHash{FORK}="";
$UNIXCommandTranslationHash{ECHO}="echo";
$UNIXCommandTranslationHash{RENAME}="mv -f";
$UNIXCommandTranslationHash{RMRF}="rm -rf";
$UNIXCommandTranslationHash{DELETE}="rm -f";
$UNIXCommandTranslationHash{CPRF}="cp -rf";
$UNIXCommandTranslationHash{COPY}="cp";
$UNIXCommandTranslationHash{PERL}="PSETHOME/bin/disperl";

use Socket;

#==========================================================================
#       Routines common to all scripts
#==========================================================================

sub SetupDefaultEnvironmentVariables(@_)
{
        my($HostType)="NONE";
        if($ENV{OS}=~m@^Windows_NT$@)
        {
                $HostType="NT";
        }
        else
        {
                $HostType=`/usr/bin/uname -s`;chomp($HostType);
                if ($HostType eq "") {
			$HostType=`/bin/uname -s`;chomp($HostType);
		}
        }
        @RequiredInstallationVariables=();
        if($HostType=~m@^HP-UX$@)
        {
                push(@RequiredInstallationVariables,"VALID_SHLIB_PATH");
                $ENV{HostType}="HP10";
                $ENV{USER}=`/usr/bin/whoami`; chomp($ENV{USER});
                $ENV{HOST}=`/usr/bin/uname -n`; chomp($ENV{HOST});
                $ENV{NullFile}="/dev/null";
                $ENV{path_separator}=":";
                $ENV{file_separator}="/";
                $ENV{line_separator}="\n";
        }
        elsif($HostType=~m@^Darwin$@)
        {
                #push(@RequiredInstallationVariables,"VALID_LD_LIBRARY_PATH");
                $ENV{HostType}="MACOSX";
                #$ENV{USER}=`/usr/bin/whoami`; chomp($ENV{USER});
                $ENV{HOST}=`/usr/bin/uname -n`; chomp($ENV{HOST});
                $ENV{NullFile}="/dev/null";
                $ENV{path_separator}=":";
                $ENV{file_separator}="/";
                $ENV{line_separator}="\n";
        }
        elsif($HostType=~m@^SunOS$@)
        {
                push(@RequiredInstallationVariables,"VALID_LD_LIBRARY_PATH");
                $ENV{HostType}="SUN5";
                $ENV{USER}=`/usr/ucb/whoami`; chomp($ENV{USER});
                $ENV{HOST}=`/usr/bin/uname -n`; chomp($ENV{HOST});
                $ENV{NullFile}="/dev/null";
                $ENV{path_separator}=":";
                $ENV{file_separator}="/";
                $ENV{line_separator}="\n";
        }
        elsif($HostType=~m@^IRIX64$@)
        {
                push(@RequiredInstallationVariables,"VALID_LD_LIBRARY_PATH");
                $ENV{HostType}="IRIX";
                $ENV{USER}=`/usr/bin/whoami`; chomp($ENV{USER});
                $ENV{HOST}=`/usr/bin/uname -n`; chomp($ENV{HOST});
                $ENV{NullFile}="/dev/null";
                $ENV{path_separator}=":";
                $ENV{file_separator}="/";
                $ENV{line_separator}="\n";
        }
        elsif($HostType=~m@^Linux$@)
        {
                # push(@RequiredInstallationVariables,"VALID_LD_LIBRARY_PATH");
                $ENV{HostType}="Linux";
                $ENV{USER}=`/usr/bin/whoami`; chomp($ENV{USER});
                $ENV{HOST}=`/bin/uname -n`; chomp($ENV{HOST});
                $ENV{NullFile}="/dev/null";
                $ENV{path_separator}=":";
                $ENV{file_separator}="/";
                $ENV{line_separator}="\n";
        }
        elsif($HostType=~m@^NT$@)
        {
                $ENV{HostType}="NT";
                $ENV{USER}=$ENV{USERNAME};
                $ENV{HOST}=`hostname`;
                if ($?) {
                    &dismb_msg::die("Check the %PATH% environment variable" .
                        " for unresolved %systemroot% values.");
                }
                chomp($ENV{HOST});
                if(!exists($ENV{HOME})){$ENV{HOME}="C:\TEMP";}
                $ENV{NullFile}="NULL:";
                $ENV{path_separator}=";";
                $ENV{file_separator}="\\";
                $ENV{line_separator}="\n";
        }
        else
        {
                ShowUsageError("Invalid Host Type \$1", $HostType);
        }       
        $ENV{StartDate}=GetCurrentDate();

        $ArchPsetHome="$ENV{HostType}_PSETHOME";
        my($exeext) = ($dismb::is_NT ? ".exe" : "");
	if(not(defined($ENV{PSETHOME}) and -e "$ENV{PSETHOME}/bin/ci_environment$exeext"))
	{
	        push(@RequiredInstallationVariables,$ArchPsetHome);
	}
        push(@RequiredInstallationVariables,"LM_LICENSE_FILE");

        $ExecutionFlag="on";
        $ENV{ShowReleased}="on";
        @LocalArgsList=();
        %AdmindirSetup=();
	%CompilerCommand=();
	%ExtraParserFlags=();
}

sub GetCurrentDate(@_)
{
	my($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
	my($CurrentDate) = sprintf("%4d%2d%2d",1900 + $year,1 + $mon,$mday);
	$CurrentDate =~ s/\s/0/g;
        return($CurrentDate);
}

sub SetOutput(@_)
{
        my($OutFileName)=">&STDERR";
        $ENV{ExtraFlags}="";
	$WithoutRepository=0;
        foreach my $Argument (@ARGV)
        {
                my($ArgName)=$Argument; $ArgName=~s@^(..).*$@$1@;
                my($ArgValue)=$Argument; $ArgValue=~s@^..(.*)$@$1@;
                if($ArgName=~m@-l@)
                {
                        $ENV{LogFileName}="$ArgValue";
#                       $OutFileName=">$ENV{LogFileName}";
                }
                elsif($ArgName=~m@-V@)
                {
                        $Verbosity="high";
                        $ENV{ExtraFlags}=~s@$@ $ArgName@;
                }
                elsif($ArgName=~m@-n@)
                {
                        $ExecutionFlag="off";
                        $ENV{ExtraFlags}=~s@$@ $ArgName@;
                }
		elsif($ArgName=~m@-D@)
                {
			if ($ArgValue eq "-") {
				$WithoutRepository = 1;
			}
			else {
	                        $ENV{DISIM}=$ArgValue;
			}
                }
        }
        if(!open(OUTFILE,"$OutFileName"))
        {
                ShowUsageError("could not open log file \$1 for writing.", $ENV{LogFileName});
        }
        select(OUTFILE);$|=1;
        return;
}

sub SetRepository(@_)
{
    if ($ENV{DISIM} eq "") {
	# Find DISIM if it's not already in the environment.
	$ENV{DISIM}=`"$ENV{PSETHOME}/bin/ci_environment" --disim`;
	if ($?) {
	    &dismb_msg::die("No default DIScover repository is known for the current user ("
			    . ($dismb::is_NT ? "%USERPROFILE%" : "\$HOME") . "/.codeintegrity).");
	}
    }
}

# Make the user-selected repository persist, via the .codeintegrity file.
sub SaveRepository
{
    my($repos) = @_[0]; # user-selected repository

    # full path of .codeintegrity file
    my($config) = ($dismb::is_NT ? $ENV{USERPROFILE} : $ENV{HOME}) . "/.codeintegrity";

    SaveConfigurationParameter($config, "DISIM", $repos);
}

sub SelectRepository(@_)
{
    my($saved_repository);
    if ($ENV{DISIM} eq "") {
	# Find DISIM if it's not already in the environment.
	$ENV{DISIM}=`"$ENV{PSETHOME}/bin/ci_environment" --disim`;
	$saved_repository="$ENV{DISIM}";
    }
    my($DISIM) = $ENV{DISIM};
    if (! -d $DISIM) {
	undef $DISIM;
    }
    my($dflt) = ($DISIM eq "" ? "" : " [$DISIM]");
    while (1) {
	print("Please select a repository$dflt:  ");
	my($user_repos)="";
	$user_repos=<STDIN>;
	chomp($user_repos);
	my($repos) = ($user_repos eq "" ? $DISIM : $user_repos);
	if (-d $repos and -e "$repos/lib/Configuration.txt") {
	    $ENV{DISIM} = $repos;
	    if (defined($saved_repository) and $saved_repository ne $repos) {
		SaveRepository($repos);
	    }
	    last;
	}
	elsif ($user_repos ne "") {
	    print ("$repos is not a repository.\n");
	}
	else {
	    &dismb_msg::die("A repository (DISIM directory) is required.");
	}
    }
}

# Save a value in a configuration file.
sub SaveConfigurationParameter
{
    my($config_file,$key,$value) = @_;

    my($new_config_line) = "$key=$value\n";  # config line to place in file
    if (! -e $config_file) {
	# Create the file.
	if (open CONFIG, ">$config_file") {
	    print CONFIG $new_config_line;
	    close CONFIG;
	}
    }
    elsif (-T $config_file and -r $config_file and -w $config_file) {
	# Update the file.
	if (open CONFIG, "<$config_file") {
	    # Read the entire file into @conf_lines, removing lines for $key
	    # and replacing the first one.
	    my(@conf_lines);
	    my($did_substitute) = 0;
	    while (<CONFIG>) {
		if (m/^\s*$key\s*=/) {
		    if (not $did_substitute) {
			push @conf_lines, $new_config_line;
			$did_substitute = 1;
		    }
		}
		else {
		    push @conf_lines, $_;
		}
	    }
	    close CONFIG;

	    # If the file didn't already have a $key line, put the new value
	    # at the front.
	    if (not $did_substitute) {
		unshift @conf_lines, $new_config_line;
	    }
	    # Rewrite the file.
	    if (open CONFIG, ">$config_file") {
		foreach $config_line (@conf_lines) {
		    print CONFIG $config_line;
		}
		close CONFIG;
	    }
	}
    }
}

# Get a value from a configuration file.
sub GetConfigurationParameter
{
    my($config_file,$key) = @_;

    if (-T $config_file and -r $config_file) {
	# Read the file.
	if (open CONFIG, "<$config_file") {
	    # Grab the value of the first line matching $key.
	    while (<CONFIG>) {
		if (m/^\s*$key\s*=\s*(|.*\S)\s*$/) {
		    my($value) = $1;
		    close CONFIG;
		    return $value;
		}
	    }
	    close CONFIG;
	}
    }
    return undef;
}

sub print_verbose(@_)
{
        &dismb_msg::vrb(@_);
}

sub print_always(@_)
{
        &dismb_msg::msg(@_);
}

sub print_error(@_)
{
        &dismb_msg::msg(@_);
}

sub run_command(@_)
{
        my(@CommandToExecute)=@_;
        my($ExecutionMethod)="system";
        if(@CommandToExecute[0] eq "FORK")              # if FORK is first word, set flag then
        {                                               # remove keyword "FORK" from command line
                ($ExecutionMethod, @CommandToExecute) = @CommandToExecute;
        }
        my($ProgramName,@ArgumentList)=@CommandToExecute;
        if($ENV{HostType}=~m@^NT$@)
        {
                if(exists($NTCommandTranslationHash{$ProgramName}))
                {
                        @CommandToExecute=&dismb_lib::argstr_to_list($NTCommandTranslationHash{$ProgramName});
                        foreach my $ArgumentValue (@ArgumentList)
                        {
                                $ArgumentValue=~s@/@\\@g;
                                @CommandToExecute=(@CommandToExecute, $ArgumentValue);
                        }
                }
        }
        else
        {
                if(exists($UNIXCommandTranslationHash{$ProgramName}))
                {
                        @CommandToExecute=&dismb_lib::argstr_to_list($UNIXCommandTranslationHash{$ProgramName});
                        @CommandToExecute=(@CommandToExecute, @ArgumentList);
                }
                
        }
        print_verbose("\$1", &dismb_lib::arglist_to_str(@CommandToExecute));
        if($ExecutionFlag=~m@^on$@)
        {
                if($ENV{HostType}!~m@^NT$@ && $ExecutionMethod=~m@FORK@)
#               if($ExecutionMethod=~m@FORK@)
                {
                        if(!SpawnProcess(@CommandToExecute)){return(0);}
                }
                else
                {
                        &dismb_lib::system(@CommandToExecute);
                        if("$?" != "0"){&dismb_msg::err("The following command failed to execute correctly.\n\$1\nReturn Code: \$2", &dismb_lib::arglist_to_str(@CommandToExecute), $?);return(0);}
                }
        }
        return(1);
}

sub SpawnProcess(@_)
{
        my(@CommandToExecute)=@_;
	my($CommandName) = @CommandToExecute[0];
        if($pid=fork())                                 # is parent process
        {
                print_verbose("my ID is: \$1 (my child is \$2)", $$, $pid);
        }
        else                                            # is child process
        {
		my($ecode) = exec { $CommandName } @CommandToExecute;
                if(!$ecode){&dismb_msg::err("The following command failed to execute correctly.\n\$1\nReturn Code: \$2", arglist_to_str(@CommandToExecute), $?);}
        }
        return(1);
}

sub GetCurrentWorkingDirectory(@_)
{
        my($CurrentWorkingDirectory)="NUL:";
        if($ENV{HostType}=~m@^NT$@)
        {
                open(TmpFileHandle,"dir|");
                while(<TmpFileHandle>)
                {
                        if(m@Directory of\s*([^ ]+)\s*$@)
                        {
                                $CurrentWorkingDirectory=$1;
                                close(TmpFileHandle);
                        }
                }
        }
        else
        {
                $CurrentWorkingDirectory=`/usr/bin/pwd`;
        }
        $CurrentWorkingDirectory=~s@\\@/@g;
        return $CurrentWorkingDirectory
}

sub ShowUsageErrorForFile(@_)
{
        (my($ThisFileName,@MESSAGE))=@_;
        if($#MESSAGE >= 0){ &dismb_msg::err(@MESSAGE);}
        print_error("USAGE: \$1 \$2", $ThisFileName, $FlagsFormat);
        if($ENV{ShowReleased}=~m@on@) {ShowReleasedModels();}
        exit(1);
}

sub ShowUsageError(@_)
{
        my(@MESSAGE)=@_;
        if($#MESSAGE >= 0){ &dismb_msg::err(@MESSAGE);}
	if ($THIS_FILE=~m@^(CreateModel).pl$@) {
		print_error("USAGE: \$1 \$2", "BuildModel", $FlagsFormat);
	}
	elsif ($THIS_FILE=~m@^(BuildPrep|CreateAdmindir|CreateModel|Dispatcher|GetBuildErrors|NTPSETServerAdmin|ReleaseModel|RemoveModel|RunDismb|RunQAR|RunSimplify|ServerAdmin|VersionWindow).pl$@) {
		print_error("USAGE: \$1 \$2", $1, $FlagsFormat);
	}
	else
	{
		print_error("USAGE: disperl \$1 \$2", $THIS_FILE, $FlagsFormat);
	}
        if($ENV{ShowReleased}=~m@on@) {ShowReleasedModels();}
        exit(1);
}

sub CleanUpAndExit(@_)
{
        my($ReturnCode)=@_;
        if(-f $TempBuildLockFile) {unlink($TempBuildLockFile);}
        if(-f $PublishBuildLockFile) {unlink($PublishBuildLockFile);}
        exit($ReturnCode);
}

sub CheckValidModelName(@_)
{
        my($ModelName)=@_;
        if(!grep(m@^$ModelName$@,@ModelNameArray))
        {
                ShowUsageError("The specified MODELNAME, \$1, is not valid.", $ModelName);
        }
}

sub CheckValidBaselineVersion(@_)
{
        my($ModelName, $ModelVersion)=@_;
        if (! -d "$ENV{DISIM}/model/$ModelName/$ModelVersion") {
               ShowUsageError("Baseline version, \"$ModelVersion\", does not exist in directory, \"$ENV{DISIM}/model/$ModelName\". Please specify a valid baseline version.");
        }
        else {
               $ENV{BASELINE_MODEL_LOCATION} = "$ENV{DISIM}/model/$ModelName/$ModelVersion";
        }
}

sub ShowReleasedModelsAndChoose(@_)
{
        @ReleasedModelsArray=&GetReleasedModelNames();
        my($ReleasedModelNumber)=0;
        if($#ReleasedModelsArray >= 0)
        {
                print("Valid DIScover model versions are:\n");
        }
	else {
        	return("NoModelName","NoVersion");
	}
        foreach my $ReleasedModel (@ReleasedModelsArray)
        {
                print("   $ReleasedModelNumber  $ReleasedModel\n");
                $ReleasedModelNumber++;
        }
        print("Please select a version:  ");
        my($UserSelection)="";
        $UserSelection=<STDIN>;
        chomp($UserSelection);
        $UserSelection=~s@^\s*([^\s]+).*@$1@;           # just use first word (number or full name)
        if($UserSelection=~m@^[0-9]+$@)
        {
                if($UserSelection <= $#ReleasedModelsArray)
                {
                        $UserSelection=$ReleasedModelsArray[$UserSelection];
                }
        }
        if($UserSelection=~m@^[^/]*$@){return("NoModelName","NoVersion");}
        my($MODELNAME,$VERSION)=split(m@/@,$UserSelection,2);
        return($MODELNAME,$VERSION);
}

sub GetReleasedModelNames(@_)
{
        $ReleasedModelsRef=&dismb_lib::find_files_with_pattern("$ENV{DISIM}/model", "Release");
        my(@ReturnValue)=();
        foreach my $ReleasedModel (@$ReleasedModelsRef)
        {
		if ($ReleasedModel =~ m@/model/([^/]+/[^/]+)/tmp/Release$@) {
			$ReleasedModel=$1;
			push (@ReturnValue, $ReleasedModel);
		}
        }
        return(@ReturnValue);
}

sub ShowReleasedModels(@_)
{
        @ReleasedModelsArray=&GetReleasedModelNames();
        if($#ReleasedModelsArray >= 0)
        {
                print_error("Valid DIScover model versions are:");
        }
        foreach my $ReleasedModel (@ReleasedModelsArray)
        {
                print_error("   \$1", $ReleasedModel);
        }
}
        
sub GetModels(@_)
{
        @ReleasedModelsArray=&GetReleasedModelNames();
        my($ReturnValue)=0;
        if($#ReleasedModelsArray >= 0)
        {
                $ReturnValue=1;
        }
        if($ReturnValue!=0) 
        {
                print STDOUT "@ReleasedModelsArray\n";
        }
        return($ReturnValue);
}


#==========================================================================
#       Utility routines
#==========================================================================

sub ChangePref(@_)
{
        $ENV{ShowReleased}="off";
        my($PrefsFileName,$PrefName,$NewPrefValue,$Instruction)=@_;
        my($FoundPref)="No";
        my($TempPrefsFileName)="$PrefsFileName.tmp";
        if(!open(PrefsFile,"$PrefsFileName")){ShowUsageError("Could not open Prefs File, \$1, for reading", $PrefsFileName);}
        if(!open(TempPrefsFile,">$TempPrefsFileName")){ShowUsageError("Could not open Prefs File, \$1, for writing", $PrefsFile);}
        while(<PrefsFile>)
        {
                chomp();
                if(m@^\#?(\*psetPrefs.${PrefName}:)(\s*)(.*)\s*$@)
                {
                        my($NewPrefLine)="#$_";
                        my($FullPrefName,$TabString,$OldPrefValue)=($1,$2,$3);
                        if($Instruction=~m@^append$@){$NewPrefLine=$FullPrefName . $TabString . $OldPrefValue . " " . $NewPrefValue;}
                        elsif($Instruction=~m@^prepend$@){$NewPrefLine=$FullPrefName . $TabString . $NewPrefValue . " " . $OldPrefValue;}
                        elsif($Instruction=~m@^change$@){$NewPrefLine=$FullPrefName . $TabString . $NewPrefValue;}
                        elsif($Instruction=~m@^delete$@){$NewPrefLine="#$_";}
                        else{ShowUsageError("Invalid ChangePref instruction, \$1", $Instruction);}
                        print(TempPrefsFile $NewPrefLine . $ENV{line_separator});
                        $FoundPref="Yes";
                }
                else
                {
                        print(TempPrefsFile $_ . $ENV{line_separator});
                }
        }
        if($FoundPref=~m@^No$@)
        {
                print(TempPrefsFile "*psetPrefs.${PrefName}:    $NewPrefValue" . $ENV{line_separator});
        }
        close(PrefsFile);
        close(TempPrefsFile);

        if($ENV{HostType}=~m@^NT$@)
        {
                run_command("RENAME", $TempPrefsFileName, $PrefsFileName);
                run_command("DELETE", $TempPrefsFileName);
        }
        else
        {
                run_command("RENAME", $TempPrefsFileName, $PrefsFileName);
        }

        $ENV{ShowReleased}="on";
}

# To update more than one pref at a time, this function is more efficient.  
# It uses only one file open and close
sub ChangePrefHash(@_)                  
{
        $ENV{ShowReleased}="off";
        my(@Local_Prefs_Hash)=@_;
        my($PrefsFileName)="$ENV{ADMINDIR}/prefs/build.prefs";
        my($TempPrefsFileName)="$PrefsFileName.tmp";
        if(!open(PrefsFile,"$PrefsFileName")){ShowUsageError("Could not open Prefs File, \$1, for reading", $PrefsFileName);}
        if(!open(TempPrefsFile,">$TempPrefsFileName")){ShowUsageError("Could not open Prefs File, \$1, for writing", $PrefsFile);}
        my($Prefs_Line)="";
        
        while(<PrefsFile>)
        {
                s@[\015\012]@@g;                        # remove carriage return - line feed
                my($NewPrefLine)=$_;
                my($index)=0;
                my($FoundPref)="No";

                foreach $Prefs_Line (@Local_Prefs_Hash)
                { 
                        my($PrefName,$NewPrefValue,$Instruction)=split(m@\|@, $Prefs_Line);
                        if($NewPrefLine=~m@^\#?(\*psetPrefs.${PrefName}:)(\s*)(.*)\s*$@)
                        {
                                $Local_Prefs_Hash[$index]="Found";      # mark the ones found 
                                $FoundPref="Yes";
                                my($FullPrefName,$TabString,$OldPrefValue)=($1,$2,$3);
                                if($Instruction=~m@^append$@){$NewPrefLine=$FullPrefName . $TabString . $OldPrefValue . " " . $NewPrefValue;}
                                elsif($Instruction=~m@^prepend$@){$NewPrefLine=$FullPrefName . $TabString . $NewPrefValue . " " . $OldPrefValue;}
                                elsif($Instruction=~m@^change$@){$NewPrefLine=$FullPrefName . $TabString . $NewPrefValue;}
                                elsif($Instruction=~m@^delete$@){$NewPrefLine="#$NewPrefLine";}
                                elsif($Instruction=~m@^enable$@){$NewPrefLine=$FullPrefName . $TabString . $OldPrefValue;}
                                else{ShowUsageError("Invalid ChangePref instruction, \$1", $Instruction);}
                        }
                        $index++;
                }
                print(TempPrefsFile $NewPrefLine . $ENV{line_separator});
        }

        foreach $Prefs_Line (@Local_Prefs_Hash)                         # print the ones we didn't find
        {
                if($Prefs_Line!~m@^Found$@)
                {
                        my($PrefName,$NewPrefValue,$Instruction)=split(m@\|@, $Prefs_Line);
                        print(TempPrefsFile "*psetPrefs.${PrefName}:    $NewPrefValue" . $ENV{line_separator});
                }
        }
        close(PrefsFile);
        close(TempPrefsFile);

        if($ENV{HostType}=~m@^NT$@)
        {
                run_command("RENAME", $TempPrefsFileName, $PrefsFileName);
                run_command("DELETE", $TempPrefsFileName);
        }
        else
        {
                run_command("RENAME", $TempPrefsFileName, $PrefsFileName);
        }

        $ENV{ShowReleased}="on";
}

sub GetPref(@_)
{
        $ENV{ShowReleased}="off";
        my($PrefsFileName,$PrefName)=@_;
        my($FoundPref)="";
        if(!open(PrefsFile,"$PrefsFileName")){ShowUsageError("Could not open Prefs File, \$1, for reading", $PrefsFileName);}
        while(<PrefsFile>)
        {
                chomp();
                if(m@^(\*psetPrefs.${PrefName}:)(\s*)(.*)\s*$@)
                {
                        my($FullPrefName,$TabString,$PrefValue)=($1,$2,$3);
                        $FoundPref="$PrefValue";
                }
        }
        close(PrefsFile);
        $ENV{ShowReleased}="on";
        return($FoundPref);
}

sub GetExpandedPref(@_)
{
        my($PrefsFileName,$PrefName)=@_;
        my($FoundPref)=GetPref($PrefsFileName,$PrefName);
	my($ExpandedPref) = eval '&dismb_lib::expand_env_vars($FoundPref)';
	if ($@) {
		&dismb_msg::fixeval;
		&dismb_msg::err ("Pref \$1, error expanding \$2, \$3", $PrefName, $FoundPref, $@);
		$ExpandedPref = "";
	}
        return($ExpandedPref);
}

#Below sub reads # of PARALLEL_PARSE or UPDATE set in template config/Setup.txt and changes it in $ADMINDIR/prefs/build.prefs
#Calling this in BuildPrep.pl
sub ChangeBuildPrefs(@_)
{
        my(@CommonPrefsHash)=();
        my(@ELSList)=GetELSFileList(@ValidSourceExtensionArray);
        my($ELS_List_sql)="off";
        foreach my $ELSfile (@ELSList)
        {
                if($ELSfile=~m@^java$@)
                {
                        #ChangePref($PrefsFileName, "ELS.List", "java", "append"); 
                        push(@CommonPrefsHash, "ELS.List|java|append");
                        push(@CommonPrefsHash, "ELS.nonAscii.Suffix|null|enable");
                        push(@CommonPrefsHash, "ELS.java.Suffix|null|enable");
                        push(@CommonPrefsHash, "ELS.java.FileIcon|null|enable");
                }

                if($ELSfile=~m@^sql$@)
                {
                        if($ELS_List_sql=~m@^off$@)
                        {
                                $ELS_List_sql="on";
                                # ChangePref($PrefsFileName, "ELS.List", "sql", "append"); 
                                push(@CommonPrefsHash, "ELS.List|sql|append");
                        }
                        push(@CommonPrefsHash, "ELS.sql.Suffix|null|enable");
                        push(@CommonPrefsHash, "ELS.sql.FileIcon|null|enable");
                }

                if($ELSfile=~m@^pc$@)
                {
                        if($ELS_List_sql=~m@^off$@)
                        {
                                $ELS_List_sql="on";
                                # ChangePref($PrefsFileName, "ELS.List", "sql", "append"); 
                                push(@CommonPrefsHash, "ELS.List|sql|append");
                        }
                        push(@CommonPrefsHash, "cEsqlSuffix|null|enable");
                        push(@CommonPrefsHash, "cEsqlPp|null|enable");
                        push(@CommonPrefsHash, "cEsqlArgs|null|enable");

                }

                if($ELSfile=~m@^pcc$@)
                {
                        if($ELS_List_sql=~m@^off$@)
                        {
                                $ELS_List_sql="on";
                                # ChangePref($PrefsFileName, "ELS.List", "sql", "append"); 
                                push(@CommonPrefsHash, "ELS.List|sql|append");
                        }
                        push(@CommonPrefsHash, "cPlusEsqlSuffix|null|enable");
                        push(@CommonPrefsHash, "cPlusEsqlPp|null|enable");
                        push(@CommonPrefsHash, "cPlusEsqlArgs|null|enable");
                }
        }

        if(exists($ENV{PARALLEL_PARSE}))
        {
                push(@CommonPrefsHash, "MBparallelHosts.parse|$ENV{PARALLEL_PARSE}|change");
		my($auto_multiple_IFFs) = 1;
		if (exists($ENV{auto_multiple_IFFs})) {
			if ($ENV{auto_multiple_IFFs} == 0 or $ENV{auto_multiple_IFFs} eq "no") {
			    $auto_multiple_IFFs = 0;
			} 
		}
		if ($auto_multiple_IFFs) {
	                my($is_parallel) = 1;
                        my(@hosts) = split(' ', $ENV{PARALLEL_PARSE});
		        if ($#hosts == 0) {  #if only one host name is listed
			    my($host, $cpus) = split(':', $hosts[0]); 
                            $is_parallel = ($cpus > 1) ? 1 : 0; 
                        }
			my($gen_multiple_IFFs) = ($is_parallel) ? "no" : "yes";
			push(@CommonPrefsHash, "MBmultipleIFFs|${gen_multiple_IFFs}|change");
		}
        }
        if(exists($ENV{PARALLEL_UPDATE}))
        {
                push(@CommonPrefsHash, "MBparallelHosts.update|$ENV{PARALLEL_UPDATE}|change");
        }
        if ($ENV{MODE}=~m@^incr$@) {
                push(@CommonPrefsHash, "MBscratch|no|change");
        } else {
                push(@CommonPrefsHash, "MBscratch|yes|change");
        }
        if (exists($ENV{AutomaticAnalysis})) {
                push(@CommonPrefsHash, "MBanalysisMethod|$ENV{AutomaticAnalysis}|change");
        }
        ChangePrefHash(@CommonPrefsHash);
}

# Find a .xml or .pl file, with overriding in $ADMINDIR/config.
sub FindFile(@_) {
    my($file_name) = $_[0];
    if ($file_name =~ m@.*/([^/]*)@) {
	my($overrider) = "$ENV{ADMINDIR}/config/$1";
	if (-r $overrider) {
	    return $overrider;
        }
    }
    if ($file_name =~ m@.*/mb/([^/]*)@) {
	my($overrider) = "$ENV{DISIM}/bin/admin/$1";
	if (-r $overrider) {
	    return $overrider;
        }
    }
    return $file_name;
}

sub DifferentBuildHostOK {
    return (not ($ENV{BuildHost} eq 'this' or $ENV{BuildHost} eq $ENV{HOST}));
}

#Returns 1, if license to do model builds exists, 
#        0, otherwise.
sub CheckModelBuildLicense {
    my($retval) = 1;
    #For now we will use aset_CCcc to check if we have build license
    #(as it is sufficient for the purpose without adding a new executable
    #or figuring out perl interface to FLEXLM library).
    my($exe) = "$ENV{'PSETHOME'}/bin/aset_CCcc";
    $exe .= ".exe" if ($HostType =~ m@^NT$@);
    my(@cmd) = ($exe, "dummyFileToParser.c", "2>&1");
    print_verbose("Checking license: \$1", &dismb_lib::arglist_to_str(@cmd));

    my(@output) = &dismb_lib::backtick(@cmd);

    my($ln);
    foreach $ln (@output) {
	if ($ln =~ /Failed to get license/) {
	    &dismb_msg::die("Failed to get license for building model.\n\$1", "@output");
	    $retval = 0;
            last;
	}
    }
    if ($retval) {
	print_verbose("Found model build license.");
    }
    return $retval;
}

#==========================================================================
#       Routines for initializations
#==========================================================================

sub GetInstallationValues(@_)
{
	IdentifyRepository();

        %WindowPreferenceHash=();
        %BuildServerPreferenceHash=();
        $ConfigutaionFileName="$ENV{DISIM}/lib/Configuration.txt";
        
        print_verbose("================ Installation Values ================");
        my(@Requirements)=@_;
        if($#Requirements<1)
        {
                @Requirements=@RequiredInstallationVariables;
        }
        foreach my $RequiredVariable (@Requirements)
        {
                delete($ENV{$RequiredVariable});
                print_verbose("REQUIREMENT      \$1", $RequiredVariable);
        }
        if(!open(ConfigutaionFile,"$ConfigutaionFileName"))
        {
                ShowUsageError("Could not open config file \$1.", $ConfigutaionFileName);
        }
        while(<ConfigutaionFile>)
        {
                chomp;
                if($ENV{HostType}=~m@^NT$@){s@\\@/@g}
                if(m@^VAR=@)
                {
                        my($KeyWord,$ArgumentList)=split(m@=@,$_,2);
                        my($VariableName,$Value)=split(m@:@,$ArgumentList,2);
                        $Value=~s@\\@/@g;
                        $Value=`echo $Value`;
                        chomp($Value);
                        my($HOSTVariableOverride,$HostTypeVariableOverride)=("$ENV{HOST}_$VariableName","$ENV{HostType}_$VariableName");
			my($RootVariableName);
                        if(!exists($ENV{$HOSTVariableOverride}) && !exists($ENV{$HostTypeVariableOverride}))
                        {
                                $RootVariableName=$VariableName;
                        }
                        if($VariableName=~m@$ENV{HostType}_([^\s]+)$@)
                        {
                                $RootVariableName=$1;
                                my($HOSTVariableOverride)="$ENV{HOST}_$RootVariableName";
                                if(exists($ENV{$HOSTVariableOverride}))
                                {
					undef $RootVariableName;
				}
                        }
                        if($VariableName=~m@$ENV{HOST}_([^\s]+)$@)
                        {
                                $RootVariableName=$1;
                        }
			if ($RootVariableName eq "DISIM") {
				&dismb_msg::wrn("In Configuration.txt, VAR:DISIM has no effect.");
			}
			elsif (defined($RootVariableName)) {
				$ENV{$RootVariableName}=$Value;
				print_verbose(($RootVariableName eq $VariableName ? "VARIABLE" : "AUTOMATIC")
                                              . " \$1=\$2", $RootVariableName, $ENV{$RootVariableName});
			}
                }
                if(m@^\s*WINDOW=@)
                {
                        my($Keyword,$WindowPreferenceString)=split(/=/,$_);
                        my($HostName,$ModelName,$SubroutineName,$ArgumentValue)=split(/:/,$WindowPreferenceString);
                        if(exists($WindowPreferenceHash{$ModelName}))
                        {
                                $WindowPreferenceHash{$ModelName}=~s@$@=$HostName:$SubroutineName:$ArgumentValue@;
                        }
                        else
                        {
                                $WindowPreferenceHash{$ModelName}="$HostName:$SubroutineName:$ArgumentValue";
                        }
                        print_verbose("WINDOW           \$1 at \$2 doing \$3 for \$4", $ModelName, $HostName, $SubroutineName, $ArgumentValue);
                }
                if(m@^\s*BUILDSERVER=@)
                {
                        my($Keyword,$BuildServerPreferenceString)=split(/=/,$_);
                        my($HostName,$SubroutineName,$ArgumentValue)=split(/:/,$BuildServerPreferenceString,3);
                        if(exists($WindowPreferenceHash{$ModelName}))
                        {
                                $BuildServerPreferenceHash{$HostName}=~s@$@=$HostName:$SubroutineName:$ArgumentValue@;
                        }
                        else
                        {
                                $BuildServerPreferenceHash{$HostName}="$SubroutineName:$ArgumentValue";
                        }
                        print_verbose("BUILDSERVER      \$1 doing \$2 with DISPLAY \$3", $HostName, $SubroutineName, $ArgumentValue);
                }
        }
        close(ConfigutaionFile);
        if(exists($ENV{DIS_TMPDIR})){$ENV{TMPDIR}=$ENV{DIS_TMPDIR};}    # if $DIS_TMPDIR exists override $TMPDIR
        foreach my $RequiredVariable (@Requirements)
        {
                if($ENV{$RequiredVariable}=~m@^\s*$@)
                {
                        ShowUsageError("Could not ascertain value of \$1 from Configuration file.", $RequiredVariable);
                }
                if(($RequiredVariable=~m@^VALID_LD_LIBRARY_PATH$@)&&(exists($ENV{VALID_LD_LIBRARY_PATH})))
                {
                        $ENV{LD_LIBRARY_PATH}=$ENV{VALID_LD_LIBRARY_PATH};
                        print_verbose("VARIABLE LD_LIBRARY_PATH=\$1", $ENV{VALID_LD_LIBRARY_PATH});
                }
                if(($RequiredVariable=~m@^VALID_SHLIB_PATH$@)&&(exists($ENV{VALID_LD_LIBRARY_PATH})))
                {
                        $ENV{LD_LIBRARY_PATH}=$ENV{VALID_SHLIB_PATH};
                        print_verbose("VARIABLE LD_LIBRARY_PATH=\$1", $ENV{VALID_SHLIB_PATH});
                }
#               if(($RequiredVariable=~m@^VFB_DISPLAY$@)&&(exists($ENV{VFB_DISPLAY})))
#               {
#                       $ENV{DISPLAY}="$ENV{HOST}:$ENV{VFB_DISPLAY}";
#                       print_verbose("VARIABLE DISPLAY=\$1:\$2", $ENV{HOST}, $ENV{VFB_DISPLAY});
#               }
        }
        my($ModelDirectoryName)="$ENV{DISIM}/template";
        @ModelNameArray=();
        if(!opendir(ModelDirectory,"$ModelDirectoryName"))
        {
                ShowUsageError("Could not open directory \$1.", $ModelDirectoryName);
        }
	my(@modelDirs) = grep !/^ModelTemplate$/, grep /Template$/, readdir ModelDirectory;
	closedir ModelDirectory;
	foreach $model (@modelDirs) {
		if (not (-e "$ModelDirectoryName/$model/config/Disable")) {
			$model =~ s/Template$//;
			push(@ModelNameArray,$model);
		}
	}
        if(exists($ENV{TEST_PSETHOME}))
        {
                $ENV{PSETHOME}=$ENV{TEST_PSETHOME};
                print_verbose("VARIABLE PSETHOME=TEST_PSETHOME=\$1", $ENV{TEST_PSETHOME});
        }
        print_verbose("=====================================================");
        $TempBuildLockFile="$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}/tmp/Building";
        $PublishBuildLockFile="$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}/tmp/Building";
        return;
}

sub IdentifyRepository {
        my($InternalConfigurationFile)="$ENV{DISIM}/repository.rpj";
	$RepositoryID = GetConfigurationParameter($InternalConfigurationFile, "ID");
        if (defined($RepositoryID)) {
		# Strip trailing comment.
		if ($RepositoryID =~ m/^([^\#]*)\#/) {
			# Strip trailing whitespace.
			$1 =~ m/^(|.*\S)\s*$/;
			$RepositoryID = $1;
		}
	}
}

sub CheckArguments(@_)
{
        my(@ArgList)=@ARGV;
        print_verbose("=================== Argument List ===================");
        my(@Requirements)=@_;
        foreach my $RequiredVariable (@Requirements)
        {
                delete($ENV{$RequiredVariable});
                print_verbose("REQUIREMENT      \$1", $RequiredVariable);
        }

        my($ARGVIndex)=-1;
        foreach my $Argument (@ArgList)
        {
                $ARGVIndex=$ARGVIndex+1;
                my($ArgName)=$Argument; $ArgName=~s@^(..).*$@$1@;
                my($ArgValue)=$Argument; $ArgValue=~s@^..(.*)$@$1@;
                if($ENV{HostType}=~m@^NT$@){$ArgValue=~s@\\@/@g}
                if($ArgName=~m@^-a@)
                {
                        push(@LocalArgsList,"$ArgValue");
                        print_verbose("ARGUMENT \$1:Argument=\$2", $ArgName, $ArgValue);
                }
                elsif($ArgName=~m@^-b@)
                {
                        my($BaseVersion)=$ArgValue;
                        if($BaseVersion=~m@^\s*$@)
                        {
                            ShowUsageError();
                        }
                        if($BaseVersion=~m@TODAY$@)
                        {
                                $BaseVersion=~s@TODAY$@$ENV{StartDate}@;
                                print_verbose("AUTOMATIC:       change Base Version=$\1", $BaseVersion);
                        }
                        print_verbose("ARGUMENT \$1:BASELINE=\$2", $ArgName, $BaseVersion);
                        $ENV{BASELINE_VERSION}=$BaseVersion;
                }
                elsif($ArgName=~m@-c@)
                {
                        $ENV{BUILD_VIEW_NAME}=$ArgValue;
                        print_verbose("ARGUMENT \$1:BUILD_VIEW_NAME=\$2", $ArgName, $ENV{BUILD_VIEW_NAME});
                }
                elsif($ArgName=~m@-e@)
                {
                        $ENV{ExecuteCommand}=$ArgValue;
                        print_verbose("ARGUMENT \$1:ExecuteCommand=\$2", $ArgName, $ENV{ExecuteCommand});
                }
                if($ArgName=~m@-l@)
                {
                        $ENV{LogFileName}="$ArgValue";
                        print_verbose("ARGUMENT \$1:LogFileName=\$2", $ArgName, $ENV{LogFileName});
                }
                elsif($ArgName=~m@-m@)
                {
                        $ENV{MODE}=$ArgValue;
                        if($ENV{MODE}=~m@^\s*$@)
                        {
                                ShowUsageError();
                        }
                        if(!(($ENV{MODE}=~m@^start$@)||($ENV{MODE}=~m@^stop$@)||($ENV{MODE}=~m@^check$@)||($ENV{MODE}=~m@^fix$@)||($ENV{MODE}=~m@^full$@)||($ENV{MODE}=~m@^incr$@)||($ENV{MODE}=~m@^resume$@)||($ENV{MODE}=~m@^sort$@)||($ENV{MODE}=~m@^format$@)))
                        {
                                ShowUsageError("\$1 is not a valid mode type.", $ArgValue);
                        }
                        print_verbose("ARGUMENT \$1:MODE=\$2", $ArgName, $ENV{MODE});
                }
                elsif($ArgName=~m@-n@)
                {
                        $ExecutionFlag="off";
                        print_verbose("ARGUMENT \$1:ExecutionFlag=\$2", $ArgName, $ExecutionFlag);
                }
                elsif($ArgName=~m@-t@)
                {
                        $ENV{TEMPBUILD}=$ArgValue;
                        if($ENV{TEMPBUILD}=~m@^\s*$@)
                        {
                                ShowUsageError();
                        }
                        if(! -w $ENV{TEMPBUILD})
                        {
                                ShowUsageError("\$1 is not a writable directory.", $ENV{TEMPBUILD});
                        }
                        print_verbose("ARGUMENT \$1:TEMPBUILD=\$2", $ArgName, $ENV{TEMPBUILD});
                }
                if($ArgName=~m@^-v@)
                {
                        $ENV{ShowReleased}="yes";
                        my($FullVersion)=$ArgValue;
                        ($ENV{MODELNAME},$ENV{VERSION})=split(/\//,$FullVersion);
#                        if(($ENV{MODELNAME}=~m@^\s*$@)||($ENV{VERSION}=~m@^\s*$@))
                        if($ENV{MODELNAME}=~m@^\s*$@)
                        {
                                ShowUsageError("Model name should be specified as:  MODELNAME[/VERSION].");
                        }
                        #------
                        # Issue #43438 Model names and versions with '_' character: modelserver won't start
                        # model name is never empty - validate it
			if($ENV{MODELNAME}=~m@[.\/;:, _]@)
			{
			    ShowUsageError("Model name should not contain . , \\ / ; : ,  or _ characters.");
			}
                        # check if version is empty
			if($ENV{VERSION}!~m@^\s*$@)
			{
			    # check if it's a special _ALL_ version name
			    if($ENV{VERSION}!~m@_ALL_$@) {
				# if it's not - validate it
				if($ENV{VERSION}=~m@[.\/;:, _]@)
				{
				    ShowUsageError("Model version should not contain . , \\ / ; : ,  or _ characters.");
				}
			    }
			}
                        #------
                        print_verbose("ARGUMENT \$1:MODELNAME/VERSION=\$2/\$3", $ArgName, $ENV{MODELNAME}, $ENV{VERSION});
                        CheckValidModelName($ENV{MODELNAME});
                        if($ENV{VERSION}=~m@TODAY$@)
                        {
                                $ENV{VERSION}=~s@TODAY$@$ENV{StartDate}@;       # substitute TODAY with datestamp
                                $ARGV[$ARGVIndex]="-v$ENV{MODELNAME}/$ENV{VERSION}";
                                print_verbose("AUTOMATIC:       change VERSION=\$1", $ENV{VERSION});
                        }
                }
                elsif($ArgName=~m@-V@)
                {
                        $Verbosity="high";
                        print_verbose("ARGUMENT \$1:Verbosity=\$2", $ArgName, $Verbosity);
                }
                elsif($ArgName=~m@-p@)
                {
		        if ($ENV{MODE}=~m@^resume$@)
                        {
			    $ENV{RESUME_PASS}=$ArgValue;
                        }
                }
        }
        foreach my $RequiredVariable (@Requirements)
        {
                if((!exists($ENV{$RequiredVariable})||($ENV{$RequiredVariable}=~m@^\s*$@)))
                {
                        ShowUsageError("Required value \$1 not specified.", $RequiredVariable);
                }
                if($RequiredVariable=~m@^MODE$@)
                {
                        if(($ENV{MODE}=~m@^(start|sort|format)$@)&&(!exists($ENV{VERSION})))
                        {
                                ShowUsageError("VERSION must be specified.");
                        }
                }
        }
        if (exists($ENV{BASELINE_VERSION})) {
                if ($ENV{MODE}=~m@^incr$@) {
                        &CheckValidBaselineVersion($ENV{MODELNAME}, $ENV{BASELINE_VERSION});
                } elsif ($ENV{MODE}=~m@^full$@) {
                        ShowUsageError("Baseline version should not be specified with \"full\" mode.");
                }
        } elsif ($ENV{MODE}=~m@^incr$@) {
                ShowUsageError("Baseline version must be specified with \"\$1\" mode.", $ENV{MODE});
        }
        print_verbose("=====================================================");
        $TempBuildLockFile="$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}/tmp/Building";
        $PublishBuildLockFile="$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}/tmp/Building";
        return;
}

# extensionIn($ext,@exts) returns whether the file extension string $ext
# is a member of the list @exts.  On NT, a case-folded match counts as a member.
sub extensionIn {
    my($x, @xs) = @_;
    my($y);
    foreach $y (@xs) {
        if($ENV{HostType}=~m@^NT$@) {
            if ($x =~ m@^$y$@i) {
                return 1;
            }
        }
        else {
            if ($x eq $y) {
                return 1;
            }
        }
    }
    return 0;
}

# Reprocess an argument which was too long for the Windows 'echo' command.
sub InterpretLongWindowsArg {
    my($arg,$forget) = @_;
    my($result) = "";
    # Split the argument up into fragments, each of which is fed to 'echo'.
    # Fragments are based on %, ', and " pairs, but do not deal with backslashes.
    while($arg ne "") {
	my($this);
	if ($arg =~ m/^(\s+)(.*)$/) {
	    ($this, $arg) = ($1, $2);
	    $result .= $this;
	    next;
	}
	elsif ($arg =~ m/^([^\'\"\%]{1,500})(.*)$/) {
	    # limited to 500 so as not to overflow 'echo'
	    ($this, $arg) = ($1, $2);
	}
	elsif ($arg =~m/^("[^"]*")(.*)$/) {
	    ($this, $arg) = ($1, $2);
	}
	elsif ($arg =~m/^('[^']*')(.*)$/) {
	    ($this, $arg) = ($1, $2);
	}
	elsif ($arg =~m/^(\%[^\%]*\%)(.*)$/) {
	    ($this, $arg) = ($1, $2);
	}
	else {
	    ($this, $arg) = ($arg, "");
	}
	my($that) = `echo $this`;
	if ($?) {
	    &dismb_msg::wrn("echo failed in long windows arg, with \$1", $this);
	    return undef;
	}
	$that =~ s/\s$//;
	$result .= $that;
    }
    return $result;
}

# Break the comma-separated args into a Perl list.
# The input does not have enclosing parentheses.
sub ParseBuildArgs {
	my($args,$forget) = @_;
	my @ArgList = ();
	while(1) {
		print_verbose ("Parsing build arg list \$1", $arg);
		my($arg);
		# Break at the first comma that's not quoted.
		# Between quotes, backslashes and quotes may be escaped with backslash.
        	if ($dismb::is_NT and $args =~ m/^(([^'",]+|"[^"]*"|'[^']*')*),(.*)$/) {
			($arg,$args) = ($1, $3);
		}
                elsif ($args =~ m/^(([^'",\\]+|\\.|"(\\.|[^"\\])*"|'(\\.|[^'\\])*')*),(.*)$/) {
			($arg,$args) = ($1, $5);
		}
		else {
			($arg,$args) = ($args, "");
		}
		push @ArgList, $arg;
		if ($args eq "") {
			last;
		}
	}
	return @ArgList;
}

# Arguments are interpreted as in the shell.
sub InterpretBuildArg {
	my($arg, $forget) = @_;
	print_verbose("echo \$1", $arg);
	if ($arg !~ m/^\s*$/) {
	    # Carefully, don't have a blank $arg which would yield "ECHO is on." on Windows.
	    my($orig_arg) = $arg;
	    if($ENV{HostType}!~m@^NT$@) {
		# Unix: Prevent echo * globbing, by escaping with \.
		$arg =~ s/([^\\\*])\*/$1\\\*/g;
		# Unix: Prevent echo ; running 2 commands, by escaping with \.
		$arg =~ s/;/\\;/g;
	    }
	    $arg = `echo $arg`;
	    if ($? != 0) {
		# Failure calling echo.
		if ($arg eq "") {
		    $arg = undef;
		    if ($dismb::is_NT) {
			# Because Windows 'echo' has trouble with sizes more than about 2000,
			# (input or output), and sizes of more than 5000 are found in classpaths,
			# we provide a backup method.
			$arg = InterpretLongWindowsArg($orig_arg);
		    }
		    if (not defined($arg)) {
			ShowUsageError("Failed to interpret argument to BUILD= line in Setup.txt.  \$1", $orig_arg);
		    }
		}
		else {
		    &dismb_msg::err("Error interpreting argument to BUILD= line in Setup.txt.");
		    &dismb_msg::inf("Given argument: \$1", $orig_arg);
		    &dismb_msg::inf("Interpreted argument: \$1", $arg);
		}
	    }
	}
	chomp $arg;
	return $arg;
}

sub SetupNoAdmindirEnvironment(@_)
{
        ($AdmindirSetupFileName,@Requirements)=@_;
        @ProjectNameList=();
        @SubProjectList=();
	@BuildLogList=();	# A list of BUILD lines from Setup.txt.
	@BuildLogRef=();	# A parallel list of diagnostic lines.
	@TranslatePathsList=();	# A list of path translations, pushed two at a time.
	undef $WindowPreference;

        print_verbose("================= Model Setup =================");
        foreach my $RequiredVariable (@Requirements)
        {
                delete($ENV{$RequiredVariable});
                print_verbose("REQUIREMENT      \$1", $RequiredVariable);
        }
        
        
        if(!open(AdmindirSetupFile,"$AdmindirSetupFileName"))
        {
                ShowUsageError("Could not open setup file \$1.", $AdmindirSetupFileName);
        }
	my($lineno) = 0;
        while(<AdmindirSetupFile>)
        {
    		$lineno += 1;
                chomp();
                if(m@^PROJECT=@)
                {
                        my($KeyWord,$ProjectName)=split(m@=@,$_,2);
                        push(@ProjectNameList,$ProjectName);
                        print_verbose("PROJECT          \$1", $ProjectName);
                }
                if(m@^SUB_PROJECT=@)
                {
                        my($KeyWord,$SubProjectName)=split(m@=@,$_,2);
                        push(@SubProjectList,$SubProjectName);
                        print_verbose("SUB_PROJECT              \$1", $SubProjectName);
                }
                if(m@^PARALLEL_PARSE=@)
                {
                        my($KeyWord, $parseArg)=split(m@=@, $_, 2);
                        $ENV{PARALLEL_PARSE}=$parseArg;
                }
                if(m@^PARALLEL_UPDATE=@)
                {
                        my($KeyWord, $updateArg)=split(m@=@, $_, 2);
                        $ENV{PARALLEL_UPDATE}=$updateArg;
                }
                if(m@^VAR=@)
                {
                        my($KeyWord,$ArgumentList)=split(m@=@,$_,2);
                        my($VariableName,$Value)=split(m@:@,$ArgumentList,2);
                        $Value=~s@\\@/@g;
                        $Value=`echo $Value`;
                        chomp($Value);
		        my($exeext) = ($dismb::is_NT ? ".exe" : "");
                        if($VariableName=~m@^MODELNAME$@ and not (-e "$ENV{PSETHOME}/bin/ci_environment$exeext"))
                        {
                                if(exists($ENV{MODELNAME}))
                                {
                                        if($Value!~m@^$ENV{MODELNAME}$@)
                                        {
                                                &dismb_msg::die("MODELNAME specified in \$1 (\$2) does not match name specifed on command line (\$3)\n.", $AdmindirSetupFileName, $Value, $ENV{MODELNAME});
                                        }
                                }
                        }
                        my($HOSTVariableOverride,$HostTypeVariableOverride)=("$ENV{HOST}_$VariableName","$ENV{HostType}_$VariableName");
                        if(!exists($ENV{$HOSTVariableOverride}) && !exists($ENV{$HostTypeVariableOverride}))
                        {
                                $ENV{$VariableName}=$Value;
                                print_verbose("VARIABLE \$1=\$2", $VariableName, $ENV{$VariableName});
                        }
                        if($VariableName=~m@$ENV{HostType}_([^\s]+)$@)
                        {
                                my($RootVariableName)=$1;
                                my($HOSTVariableOverride)="$ENV{HOST}_$RootVariableName";
                                if(!exists($ENV{$HOSTVariableOverride}))
                                {
                                        $ENV{$RootVariableName}=$Value;
                                        print_verbose("AUTOMATIC        \$1=\$2", $RootVariableName, $ENV{$RootVariableName});
                                }
                        }
                        if($VariableName=~m@$ENV{HOST}_([^\s]+)$@)
                        {
                                my($RootVariableName)=$1;
                                $ENV{$RootVariableName}=$Value;
                                print_verbose("AUTOMATIC        \$1=\$2", $RootVariableName, $ENV{$RootVariableName});
                        }
                }
		if(m@^BUILD=@) {
			my($FunctionName);
			my(@ArgList)=();
			if (m@^BUILD=(\w+)$@) {
				$FunctionName=$1;
			}
			elsif(m@^BUILD=(\w+)\((.*)\)$@) {
				$FunctionName=$1;
				# Arguments are interpreted as in the shell.
				@ArgList = map InterpretBuildArg($_), ParseBuildArgs($2);
			}
			else {
				ShowUsageError("Syntax error in \$1: \$2", $AdmindirSetupFileName, $_);
			}
			if (defined $FunctionName) {
				# Add the function with args to the end of the list of builds.
				my(@fun_args) = ($FunctionName, @ArgList);
				push @BuildLogList, [ @fun_args ];
				push @BuildLogRef, "$AdmindirSetupFileName, line $lineno: $_";
				print_verbose("BUILD        \$1", "$FunctionName(" . (join ",", @ArgList) . ")");
		    	}
		}
		if(m@^TRANSLATE_PATHS=@) {
			my($LogPath,$ActualPath);
			if (m@^TRANSLATE_PATHS=([^;]*);([^;]*)$@) {
				$LogPath=$1;
				$ActualPath=$2;
				if($ENV{HostType}=~m@^NT$@)
				{
				    $LogPath =~ s@\\@/@g;
				    $ActualPath =~ s@\\@/@g;
				}
				$LogPath =~ s@/$@@;
				$ActualPath =~ s@/$@@;
			}
			if ($LogPath ne "") {
				# Add the paths to the end of the list of path translations.
				push @TranslatePathsList, ($LogPath, $ActualPath);
				print_verbose("TRANSLATE_PATHS        \$1;\$2", $LogPath, $ActualPath);
		    	}
			else {
				ShowUsageError("Syntax error in \$1: \$2\nPaths must be separated by one semicolon.", $AdmindirSetupFileName, $_);
			}
		}
                if(m@^EXTENSION=@)
                {
                        my($KeyWord,$ExtensionKey,$Value)=split("[\=: +]",$_,3);
                        $Value=~s@^\s*@@;
                        $Value=~s@\s*$@@;
                        $AdmindirSetup{$ExtensionKey}=$Value;
                        print_verbose("EXTENSION        \$1=\$2", $ExtensionKey, $AdmindirSetup{$ExtensionKey});
                }
		if(m@^COMPILER=@)
		{
		    if (m@^COMPILER=([^:]*):(.*)$@) {
		        my($Command,$CompilerId)=($1,$2);
			$CompilerCommand{$Command}=$CompilerId;
                        print_verbose("COMPILER        \$1=\$2", $Command, $CompilerCommand{$Command});
		    }
		    else {
			ShowUsageError("Invalid COMPILER line: \$1, in \$2.", $_, $AdmindirSetupFileName);
		    }
		}
		if(m@^EXTRA_FLAGS=@)
		{
		    if (m@^EXTRA_FLAGS=([^":]*):(.*)$@) {
		        my($CompilerId,$Flags)=($1,$2);
			$DISIM::ExtraParserFlags{$CompilerId}=$Flags;
                        print_verbose("EXTRA_FLAGS        \$1=\$2", $CompilerId, $DISIM::ExtraParserFlags{$CompilerId});
			if (not (-f &FindFile("$ENV{PSETHOME}/lib/$CompilerId.xml"))) {
			    &dismb_msg::err("EXTRA_FLAGS for nonexistent compiler id \$1.",
				$CompilerId);
			}
		    }
		    elsif (m@^EXTRA_FLAGS=(.*)$@) {
			$DISIM::UniversalExtraParserFlags=$1;
                        print_verbose("EXTRA_FLAGS        \$1", $DISIM::UniversalExtraParserFlags);
		    }
		    else {
			ShowUsageError("Invalid EXTRA_FLAGS line: \$1, in Setup.txt.", $_);
		    }
		}
                if(m@^WINDOW=@)
                {
                        my($Keyword,$WindowPreferenceString)=split(/=/,$_);
                        my($HostName,$SubroutineName,$ArgumentValue)=split(/:/,$WindowPreferenceString);
                        if(defined($WindowPreference))
                        {
                                $WindowPreference=~s@$@=$HostName:$SubroutineName:$ArgumentValue@;
                        }
                        else
                        {
                                $WindowPreference="$HostName:$SubroutineName:$ArgumentValue";
                        }
                        print_verbose("WINDOW           At \$1 doing \$2 for \$3", $HostName, $SubroutineName, $ArgumentValue);
                }
        }
        close(AdmindirSetupFile);
        foreach my $RequiredVariable (@Requirements)
        {

		if ($RequiredVariable=~m@^(\w+)=$@) {
			if ($1 eq "BUILD" and $#BuildLogList < 0) {
				&dismb_msg::die("No valid $1 line in \$1", $AdmindirSetupFileName);
			}
		}
                elsif((!exists($ENV{$RequiredVariable})||($ENV{$RequiredVariable}=~m@^\s*$@))&&(!exists($AdmindirSetup{$RequiredVariable})))
                {
                        ShowUsageError("Required value \$1 not specified.", $RequiredVariable);
                }
        }
        if(exists($ENV{TEST_PSETHOME}))
        {
                $ENV{PSETHOME}=$ENV{TEST_PSETHOME};
                print_verbose("VARIABLE PSETHOME=TEST_PSETHOME=\$1", $ENV{TEST_PSETHOME});
        }

        @ValidSourceExtensionArray=split(m@\s+@,$AdmindirSetup{CompileFiles});

	# Let project name default to model name.
	if (not @ProjectNameList) {
		push(@ProjectNameList,$ENV{MODELNAME});
	}


        # Weed out duplicate source extensions.
        # Duplicates are likely to occur on NT due to case folding.
        my(@DistinctExts, $SourceExtension);
        foreach $SourceExtension (@ValidSourceExtensionArray) {
                if (not &extensionIn($SourceExtension, @DistinctExts)) {
                        push(@DistinctExts, $SourceExtension);
                }
        }
        @ValidSourceExtensionArray = @DistinctExts;

        print_verbose("=====================================================");
        return(1);
}

sub SetupEnvironment(@_)
{
        $ENV{PUBLISH_ADMINDIR}="$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}";
        $ENV{BUILD_ADMINDIR}="$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}";

        $ENV{ADMINDIR}=~m@^.*/([^/]+)/([^/]+)$@;
	$ENV{PMODServiceName}= (defined($RepositoryID) ? "${RepositoryID}:" : "") . "${1}_${2}";
        $ENV{PMODServerLockFile}="$ENV{DISIM}/tmp/lockfiles/$ENV{MODELNAME}/$ENV{VERSION}_$ENV{HOST}.lock";
        $ServerLockFileDir="$ENV{DISIM}/tmp/lockfiles/$ENV{MODELNAME}";
        $BuildLockFile="$ENV{ADMINDIR}/tmp/Building";
        $ModelBuildLogFileName="$ENV{ADMINDIR}/log/$ENV{USER}_dismb.log";
	my($AdmindirSetupFileName) = "$ENV{ADMINDIR}/config/Setup.txt";
        if(! -r "$AdmindirSetupFileName")
        {
                &dismb_msg::err("Specified model at \$1 has no Setup.txt file", $ENV{ADMINDIR});
                return(0);
        }

	SetupNoAdmindirEnvironment($AdmindirSetupFileName,@_);
}

sub SetupPmodList(@_)
{
        my($PmodListFileName)="$ENV{ADMINDIR}/config/PmodList.txt";
        if(!open(PmodListFile,">$PmodListFileName")){&dismb_msg::die("Could not open PmodList file, \$1, for writing", $PmodListFileName);}

        foreach my $SubProjects (@_)
        {
                print(PmodListFile "$SubProjects\n");
        }
        
        close(PmodListFile);
}

sub GetELSFileList(@_)
{
        my($ELSfile)="";
        my(@ELSfileList)=();
        
        foreach $ELSfile (@_)
        {
                if($ELSfile=~m@^(java|sql|pc|pcc)$@)
                {
                        push(@ELSfileList, $ELSfile);
                }
        }

        return(@ELSfileList);
}

#==========================================================================
#       Routines for server maintenance
#==========================================================================

sub Build_Server_Test(@_)
{
  	my($proto) = getprotobyname("tcp");
  	socket(BLDSERV_SOCKET, PF_INET, SOCK_STREAM, $proto) || &dismb_msg::die("Can't create socket: \$1", $!);
 
  	my($iaddr) = inet_aton("127.0.0.1");
  	my($paddr) = pack_sockaddr_in(61796, $iaddr);
  	my($result) = connect(BLDSERV_SOCKET, $paddr);
  	close (BLDSERV_SOCKET);
 
  	return $result;
}

sub CheckBuildServer(@_)
{
	my($standalone) = ($0=~m@IMUtil.pl$@);
        my($BuildServerDisplay)=@_;
        my($OldDisplay)=$ENV{DISPLAY};
        if($BuildServerDisplay=~m@^[^\s]+:[0-9\.]+$@)
        {
                $ENV{DISPLAY}=$BuildServerDisplay;
        }
        print_verbose("Checking/Starting build server.");
	if ($standalone) {
	    &dismb_msg::define_progress_task("checking build server");
	}
        if($ENV{HostType}=~m@^NT$@)
        {
                run_command("$ENV{PSETHOME}/mb/fork.exe", &dismb_lib::arglist_to_fork_str("$ENV{PSETHOME}/bin/tclsh83.exe", "$ENV{PSETHOME}/lib/dislite/appServer.tcl"));
        }
        else
        {
                # run_command("$ENV{PSETHOME}/bin/tclsh", "$ENV{PSETHOME}/lib/dislite/appServer.tcl", "2", ">&", "1", ">", "/dev/null", "&"); # Changed due to a failure of GetBuildLog.pl on fridge -- see that file.
                run_command("$ENV{PSETHOME}/bin/tclsh", "$ENV{PSETHOME}/lib/dislite/appServer.tcl", "2>&", "1", ">", "/dev/null", "&");
        }
	while (not Build_Server_Test()) {
		sleep(1);
	}
	if ($standalone) {
	    &dismb_msg::report_progress("checking build server");
	}
        $ENV{DISPLAY}=$OldDisplay;
}

sub CreateReleasedModelList(@_)
{
        %ReleasedModelHash=("PlaceHolder","");
	$ReleasedModelAdmindirsRef = &dismb_lib::find_files_with_pattern("$ENV{DISIM}/model", "Release");

        ### put all released models into a global hash ###
        foreach my $ReleasedModelAdmindir (@$ReleasedModelAdmindirsRef)
        {
		if ($ReleasedModelAdmindir =~ m@/model/([^/]+)/([^/]+)/tmp/Release$@) {
			my($ModelName, $ModelVersion) = ($1, $2);
			$ReleasedModelHash{$ModelName}=~s@$@$ModelVersion:@;
		}
        }
        foreach my $HashKey (keys(%ReleasedModelHash))
        {
                $ReleasedModelHash{$HashKey}=~s@:$@@;
        }
}

sub CreateFailedModelList(@_)
{
        %FailedModelHash=("PlaceHolder","");
        @ModelDirArray=glob("$ENV{DISIM}/model/*");

        if(! %ReleasedModelHash)
        {
                CreateReleasedModelList();
        }
        ### if model is not in released hash then it has failed ###
	foreach my $ModelDir (@ModelDirArray)
	{
	        @ModelAdmindirArray=glob("${ModelDir}/*");
		foreach my $ModelAdmindir (@ModelAdmindirArray)
		{
			$ModelAdmindir=~s@$ENV{DISIM}/model/([^/]+/[^/]+).*@$1@;
			my($ModelName,$ModelVersion)=split("/",$ModelAdmindir);
			if(exists($ReleasedModelHash{$ModelName}))
			{
				if($ReleasedModelHash{$ModelName}!~m@${ModelVersion}:@)
				{
					$FailedModelHash{$ModelName}=~s@$@$ModelVersion:@;
				}
			}
		}
	}
        foreach my $HashKey (keys(%FailedModelHash))
        {
                $FailedModelHash{$HashKey}=~s@:$@@;
        }
}

sub CreateKeepModelList(@_)
{
        %KeepModelHash=("PlaceHolder","");
        $KeepModelAdmindirsRef = &dismb_lib::find_files_with_pattern("$ENV{DISIM}/model", "Keep");

        ### models with a "Keep" file are exceptions and remain indefinitely ###
        foreach my $KeepModelAdmindir (@$KeepModelAdmindirsRef)
        {
		if ($KeepModelAdmindir =~ m@/model/([^/]+)/([^/]+)/tmp/Keep$@) {
			my($ModelName,$ModelVersion) = ($1, $2);
			$KeepModelHash{$ModelName}=~s@$@$ModelVersion:@;
		}
        }
        foreach my $HashKey (keys(%KeepModelHash))
        {
                $KeepModelHash{$HashKey}=~s@:$@@;
        }
}

sub CreateRunningServerList(@_)
{
        %ServedModelHash=("PlaceHolder","");
        $LockFilesRef = &dismb_lib::find_files_with_pattern("$ENV{DISIM}/tmp/lockfiles", "$ENV{HOST}.lock");
        @RunningModelServerArray=();

        foreach my $LockFile (@$LockFilesRef)
        {
        	if ($LockFile =~ m@/tmp/lockfiles/([^/]+)/([^/]+)_$ENV{HOST}.lock$@) {
			my($ModelName,$ModelVersion)=($1,$2);
			$ServedModelHash{$ModelName}=~s@$@$ModelVersion:@;
			push(@RunningModelServerArray,"$ModelName/$ModelVersion");
		}
        }
        foreach my $HashKey (keys(%ServedModelHash))
        {
                $ServedModelHash{$HashKey}=~s@:$@@;
        }
}

sub CreateAvailableServiceList(@_)
{
        %AvailableServiceHash=("PlaceHolder","");
        @AvailableServiceArray=();
        # run_command("$ENV{PSETHOME}/bin/dish2", "-test", "2", ">&", "1", ">", $ENV{NullFile}); # Changed due to a failure of GetBuildLog.pl on fridge -- see that file.
        run_command("$ENV{PSETHOME}/bin/dish2", "-test", "2>&", "1", ">", $ENV{NullFile});
	local(*InFile);
        &dismb_lib::open_input(\*InFile, "$ENV{PSETHOME}/bin/admintool");
        while(<InFile>)
        {
                chomp($_);
                my($ServiceType,$ServiceName)=split(":",$_,2);
		if ($ServiceName =~ m/^([^\:]*)\:(.*)$/) {
			# Name has a repository ID prefix.  Must match this repository.
			$ServiceName = ($1 eq $RepositoryID) ? $2 : undef;
		}
		elsif (defined($RepositoryID)) {
			# Service has no ID; does not match.
			$ServiceName = undef;
		}
		if (defined($ServiceName)) {
			my($ModelName,$ModelVersion)=split("_",$ServiceName,2);
			$AvailableServiceHash{$ModelName}=~s@$@$ModelVersion:@;
			push(@AvailableServiceArray,"$ModelName/$ModelVersion");
		}
        }
        close(InFile);
        foreach my $HashKey (keys(%ServedModelHash))
        {
                $AvailableServiceHash{$HashKey}=~s@:$@@;
        }
}

sub CreateExceptionServerList(@_)
{
        %ExceptionModelHash=("PlaceHolder","");
        $ExceptionFilesRef=&dismb_lib::find_files_with_pattern("$ENV{DISIM}/model", "Serve_$ENV{HOST}");
        foreach my $ExceptionFile (@ExceptionFileArray)
        {
	        if ($ExceptionFile =~ m@/model/([^/]+)/([^/]+)/tmp/Serve_$ENV{HOST}$@) {
			my($ModelName,$ModelVersion)=($1,$2);
			$ExceptionModelHash{$ModelName}=~s@$@$ModelVersion:@;
		}
        }
        foreach my $HashKey (keys(%ExceptionModelHash))
        {
                $ExceptionModelHash{$HashKey}=~s@:$@@;
        }
}

sub GetReleasedModelList(@_)
{
        my($ModelName)=@_;
        my(@ReleasedModelVersionList)=();
        if(! %ReleasedModelHash)
        {
                CreateReleasedModelList();
        }
        if(exists($ReleasedModelHash{$ModelName}))
        {
                foreach my $ModelVersion (split(":",$ReleasedModelHash{$ModelName}))
                {
                        push(@ReleasedModelVersionList,$ModelVersion);
                }
        }
        my(@SortedModelVersionList)=sort({($b=~m@([0-9]+)$@)[0] <=> ($a=~m@([0-9]+)$@)[0]} @ReleasedModelVersionList);
        return(@SortedModelVersionList);
}

sub GetFailedModelList(@_)
{
        my($ModelName)=@_;
        my(@FailedModelVersionList)=();
        if(! %FailedModelHash)
        {
                CreateFailedModelList();
        }
        if(exists($FailedModelHash{$ModelName}))
        {
                foreach my $ModelVersion (split(":",$FailedModelHash{$ModelName}))
                {
                        push(@FailedModelVersionList,$ModelVersion);
                }
        }
        my(@SortedModelVersionList)=sort({($b=~m@([0-9]+)$@)[0] <=> ($a=~m@([0-9]+)$@)[0]} @FailedModelVersionList);
        return(@SortedModelVersionList);
}

sub GetKeepModelList(@_)
{
        my($ModelName)=@_;
        my(@KeepModelVersionList)=();
        if(! %KeepModelHash)
        {
                CreateKeepModelList();
        }
        if(exists($KeepModelHash{$ModelName}))
        {
                foreach my $ModelVersion (split(":",$KeepModelHash{$ModelName}))
                {
                        push(@KeepModelVersionList,$ModelVersion);
                }
        }
        my(@SortedModelVersionList)=sort({($b=~m@([0-9]+)$@)[0] <=> ($a=~m@([0-9]+)$@)[0]} @KeepModelVersionList);
        return(@SortedModelVersionList);
}

sub GetRunningServerList(@_)
{
        my($ModelName)=@_;
        my(@ServedModelVersionList)=();
        if(! %ServedModelHash)
        {
                CreateRunningServerList();
        }
        if(exists($ServedModelHash{$ModelName}))
        {
                foreach my $ModelVersion (split(":",$ServedModelHash{$ModelName}))
                {
                        push(@ServedModelVersionList,$ModelVersion);
                }
        }
        my(@SortedModelVersionList)=sort({($b=~m@([0-9]+)$@)[0] <=> ($a=~m@([0-9]+)$@)[0]} @ServedModelVersionList);
        return(@SortedModelVersionList);        
}

sub GetAvailableServerList(@_)
{
        my($ModelName)=@_;
        my(@AvailableServiceVersionList)=();
        if(! %AvailableServiceHash)
        {
                CreateAvailableServiceList();
        }
        if(exists($AvailableServiceHash{$ModelName}))
        {
                foreach my $ModelVersion (split(":",$AvailableServiceHash{$ModelName}))
                {
                        push(@AvailableServiceVersionList,$ModelVersion);
                }
        }
        my(@SortedModelVersionList)=sort({($b=~m@([0-9]+)$@)[0] <=> ($a=~m@([0-9]+)$@)[0]} @AvailableServiceVersionList);
        return(@SortedModelVersionList);        
}

sub GetExceptionServerList(@_)
{
        my($ModelName)=@_;
        my(@ExceptionModelVersionList)=();
        if(! %ExceptionModelHash)
        {
                CreateExceptionServerList();
        }
        if(exists($ExceptionModelHash{$ModelName}))
        {
                foreach my $ModelVersion (split(":",$ExceptionModelHash{$ModelName}))
                {
                        push(@ExceptionModelVersionList,$ModelVersion);
                }
        }
        my(@SortedModelVersionList)=sort({($b=~m@([0-9]+)$@)[0] <=> ($a=~m@([0-9]+)$@)[0]} @ExceptionModelVersionList);
        return(@SortedModelVersionList);        
}

sub GetServerState(@_)
{
        my($ServiceName)=@_;
        my($ModelName,$ModelVersion)=split("/",$ServiceName,2);
        if(! %AvailableServiceHash)
        {
                CreateAvailableServiceList();
        }
        if(! %ServedModelHash)
        {
                CreateRunningServerList();
        }
        if(grep(m@^$ModelName/$ModelVersion$@,@AvailableServiceArray))
        {
                if(grep(m@^$ModelName/$ModelVersion$@,@RunningModelServerArray))
                {
                        return("Alive");
                }
                else
                {
                        return("Stray");
                }
        }
        else
        {
                return("Dead");
        }
}

#==========================================================================
#       Routines for Clients
#==========================================================================
sub CreateUserArea(@_)
{
	use File::Path;
		
        if(! -d $ENV{TMPDIR})
	{
            	print_always("Creating temporary directory at \$1", $ENV{TMPDIR});
		if(! mkpath($ENV{TMPDIR},0,0777)){&dismb_msg::err("Failed to make directory \$1.", $ENV{TMPDIR});}
	}
        if(! -d $ENV{TEMPBUILD}) 
	{
            	print_always("Creating temporary build directory at \$1", $ENV{TEMPBUILD});
		if(! mkpath($ENV{TEMPBUILD},0,0777)){&dismb_msg::err("Failed to make directory \$1.", $ENV{TEMPBUILD});}
	}
        if(! -d $ENV{privateModelRoot})
        {
            	print_always("Creating private model root at \$1", $ENV{privateModelRoot});
		if(! mkpath($ENV{privateModelRoot},0,0777)){&dismb_msg::err("Failed to make directory \$1.", $ENV{privateModelRoot});}
        }
	my($groupsDir)="$ENV{privateModelRoot}/groups/ext";
        if(! -d $groupsDir)
        {
            	print_always("Creating groups directory at \$1", $groupsDir);
		if(! mkpath($groupsDir,0,0777)){&dismb_msg::err("Failed to make directory \$1.", $groupsDir);}
        }
        if(! -d $ENV{privateSrcRoot})
        {
                print_always("Creating private source root at \$1", $ENV{privateSrcRoot});
		if(! mkpath($ENV{privateSrcRoot},0,0777)){&dismb_msg::err("Failed to make directory \$1.", $ENV{privateSrcRoot});}
        }
}

sub StartBrowserCommon(@_)
{
        $ENV{ADMINDIR}="$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}";
        if(! -r $ENV{ADMINDIR})
        {
                $FlagsFormat="[MODELNAME/VERSION] [-n] [-V]";
		my ($msg);
		if (scalar(GetReleasedModelNames()) == 0) {
	                $msg = "No model version exists.";
		}
		else {
	                $msg = "The specified model version does not exist.";
		}
	        ShowUsageErrorForFile("StartBrowser", $msg);
                exit(1);
        }
        
        if(SetupEnvironment())
        {
                if(exists($ENV{SRC_ROOT}))
                {
                        $ENV{sharedSrcRoot}=$ENV{SRC_ROOT};
                }
                CreateUserArea();
                run_command("$ENV{PSETHOME}/bin/start_discover", "-prefs", "$ENV{ADMINDIR}/prefs/build.prefs");
        }
}

sub StartBrowser(@_)
{
        if(!exists($ENV{VERSION}))
        {
                ($ENV{MODELNAME},$ENV{VERSION})=ShowReleasedModelsAndChoose();
                $ENV{ShowReleased}="off";
        }
        StartBrowserCommon();
}

sub StartBrowserX(@_)
{
        if($ENV{HostType}=~m@^NT$@)
        {
                run_command("$ENV{PSETHOME}/bin/wish83", "$ENV{PSETHOME}/mb/BrowserX.dis");      
        }
        else
        {
                run_command("$ENV{PSETHOME}/bin/wish", "$ENV{PSETHOME}/mb/BrowserX.dis");
        }

        if(!open(InFile,"$ENV{DISIM}/tmp/model_version.tmp")){&dismb_msg::die("Couldn't receive user input.");}      
        
        my($model_version)="";
        while(<InFile>)
        {

                $model_version=$_;
        }
        
        close(InFile);
        run_command("DELETE", "$ENV{DISIM}/tmp/model_version.tmp");
        
        chomp($model_version);  
        ($ENV{MODELNAME},$ENV{VERSION})=split(m@/@,$model_version,2);
        if ($ENV{MODELNAME} ne "" )
        {
                if($ENV{HostType}=~m@^NT$@){ NTStartBrowser(); } 
                else { StartBrowserCommon(); }
        }
        else
        {
                &dismb_msg::err("No models exist");
        }
}

sub NTStartBrowser(@_)
{
        $ENV{SERVICE_NAME}="";
        $ENV{ADMINDIR}="$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}";
        if(! -r $ENV{ADMINDIR})
        {
                ShowUsageErrorForFile("StartBrowser.bat", "VERSION not specified or no models exist");
                exit(1);
        }
        if(SetupEnvironment())
        {
                if(exists($ENV{SRC_ROOT}))
                {
                        $ENV{sharedSrcRoot}=$ENV{SRC_ROOT};
                }
                
                if(!chdir("$ENV{ADMINDIR}")){&dismb_msg::die("Could not change directory to \$1", $ENV{ADMINDIR});}
                print_verbose("Changed directory to:    \$1", $ENV{ADMINDIR});
                $ENV{defaultPREFS}="prefs\\build.prefs";
                print_verbose("Setting up defaultPREFS: \$1", $ENV{defaultPREFS});
                
                my($PREFSArgument)="$ENV{ADMINDIR}/prefs/build.prefs";
                my($PDFArgument)=GetPref($PREFSArgument, pdfFileUser);
                $PDFArgument=~s@\$ADMINDIR@$ENV{ADMINDIR}@g;    
                my($HomeProj)=GetPref($PREFSArgument, projectHome);
                $ENV{SERVICE_NAME}="Discover:" . $ENV{HOST} . ":" . $PREFSArgument;     

                #Start nameserver here. If it is already running the executable
                #just quits with a "Nameserver already running." message.           
                &dismb_lib::system("cmd", "/c", "start", "/B", "$ENV{PSETHOME}/bin/nameserv.exe");

                &dismb_lib::system("cmd", "/c", "start", "/B", "$ENV{PSETHOME}/bin/pset_server.exe", "-pdf", $PDFArgument, "-prefs", $PREFSArgument, "-server", "-server_name", $ENV{SERVICE_NAME}, "-home", $HomeProj);

                require (FindFile("$ENV{PSETHOME}/mb/NTPSETServerAdmin.pl"));
		ProcessArgList("-eStartPSETServer", $ENV{SERVICE_NAME});
                &dismb_lib::system("$ENV{PSETHOME}/bin/DiscoverMDI.exe", "-service", $ENV{SERVICE_NAME});
                ProcessArgList("-eStopPSETServer", $ENV{SERVICE_NAME});
        }
}

sub StartDeveloperXpress(@_)
{
        my($DeveloperXpressExecutable)="dislite";
        run_command("$ENV{PSETHOME}/bin/$DeveloperXpressExecutable");
}

sub StartCodeRoverViewer(@_)
{
        run_command("$ENV{CODE_ROVER_HOME}/bin/DevXLauncher.exe");
}

#==========================================================================
#       Automatically Executed Area
#==========================================================================

#initialize message group from -message_group option
&dismb_msg::init_message_group_from_option;

SetupDefaultEnvironmentVariables();
SetOutput();

if($0=~m@IMUtil.pl$@)
{
        $FlagsFormat="-e<Subroutine_To_Execute> [-aArgument1 -aArgument2 ...] [-v<MODELNAME/VERSION>]  [-n] [-V]";
        $THIS_FILE="IMUtil.pl";
	if (not $WithoutRepository) {
		SelectRepository();
	        GetInstallationValues();
	}
        CheckArguments();
        if(exists($ENV{ExecuteCommand}))
        {
                my($SubroutineName)=$ENV{ExecuteCommand};
                print_verbose("Running utility subroutine \$1", "$SubroutineName(@LocalArgsList)");
                &$SubroutineName(@LocalArgsList);
        }
}
else
{
	if (not $WithoutRepository) {
		SetRepository();
	}
        return(1);
}
