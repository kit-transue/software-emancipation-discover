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
# Description:	Run launcher.pl, convert some necessary flags for submission check
#		build server, and delete model from TEMPBUILD area

require ("$ENV{PSETHOME}/mb/IMUtil.pl");
require ("$ENV{PSETHOME}/mb/dismb_msg.pl");

if ($0 =~ m@ReleaseModel[^/\\]*@) {
    $FlagsFormat="-v<MODELNAME/VERSION> -m<full|incr> [-b<BASELINE_VERSION>] [-t<BUILD_LOCATION>] [-n] [-V]";
    $THIS_FILE="ReleaseModel.pl";
}

sub ReleaseModel(@_)
{
	print_always("Releasing model \$1/\$2", $ENV{MODELNAME}, $ENV{VERSION});
	if(!open(ReleaseFile,">$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}/tmp/Release")){&dismb_msg::err("can't open release file \$1/model/\$2/\$3/tmp/Release", $ENV{DISIM}, $ENV{MODELNAME}, $ENV{VERSION});}
	print(ReleaseFile "$ENV{StartDate}\n");
	close(ReleaseFile);
	# Issue #43236 "Published Model Configurations shouldn't be editable"
	my($configDir)="$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}/config";
  	my($configFile);
  	if(!opendir(DIR, $configDir)){&dismb_msg::err("can't read $configDir directory");}
	my(@configFiles) = grep {!/^\.\.?$/ } readdir(DIR);
  	closedir(DIR);
  	foreach (@configFiles) {
	    if(-f ($configFile = "$configDir/$_")) {
		chmod(0444, $configFile);
	    }
  	}
}

sub AddServerAndUserPDFhack(@_)
{
	my($GeneratedBuildPDFFileName)="$ENV{ADMINDIR}/pdf/build.pdf";
	my($HomeProjectPDFFileName)=FindFile("$ENV{DISIM}/lib/UserProject.pdf");
	my($UserPDFFileName)="$ENV{ADMINDIR}/pdf/user.pdf";
	my($ServerPDFFileName)="$ENV{ADMINDIR}/pdf/server.pdf";

	if(!open(GeneratedBuildPDFFile,"$GeneratedBuildPDFFileName")){&dismb_msg::die("Could not open file, \$1, for reading.", $GeneratedBuildPDFFileName);}
	if(!open(HomeProjectPDFFile,"$HomeProjectPDFFileName")){&dismb_msg::die("Could not open file, \$1, for reading.", $HomeProjectPDFFileName);}

	if(!open(ServerPDFFile,">$ServerPDFFileName")){&dismb_msg::die("Could not open file, \$1, for writing.", $ServerPDFFileName);}
	if(!open(UserPDFFile,">$UserPDFFileName")){&dismb_msg::die("Could not open file, \$1, for writing.", $UserPDFFileName);}

	print(UserPDFFile "# For use with DIScover Browser\n\n");
	print(ServerPDFFile "# For use with DIScover Model Server\n\n");
	while(<HomeProjectPDFFile>)
	{
#		s@ProjectName@$ProjectName@g;		# update home project name
		print(UserPDFFile "$_");
	}
	while(<GeneratedBuildPDFFile>)
	{
		s@^(${ProjectName}\s+:\s+/)@$1 [[ R ]] @;	# need this to load prj file
		print(UserPDFFile "$_");
		print(ServerPDFFile "$_");
	}

	close(GeneratedBuildPDFFile);
	close(HomeProjectPDFFile);
	close(ServerPDFFile);
	close(UserPDFFile);

}

sub NTPrefsUpdate(@_)
{
	my(@Prefs_Hash)=@_;
	my($PrefsFileName)="$ENV{ADMINDIR}/prefs/build.prefs";
	my($TmpPrefsFileName)="$PrefsFileName.tmp";

	if(!open(PrefsFile,"$PrefsFileName")){&dismb_msg::die("Could not open prefs file, \$1, for reading", $PrefsFileName);}
	if(!open(TmpPrefsFile,">$TmpPrefsFileName")){&dismb_msg::die("Could not open prefs file, \$1, for writing", $TmpPrefsFileName);}
	while(<PrefsFile>)
	{
		s@[\015\012]@@g;
		print(TmpPrefsFile $_ . $ENV{line_separator});
	}
	close(PrefsFile);
	close(TmpPrefsFile);

	run_command("RENAME", $TmpPrefsFileName, $PrefsFileName);
	run_command("DELETE", $TmpPrefsFileName);

	push(@Prefs_Hash, "sharedSrcRoot|$ENV{sharedSrcRoot}|change");
	push(@Prefs_Hash, "sharedModelRoot|$ENV{sharedModelRoot}|change");
	push(@Prefs_Hash, "privateModelRoot|\$privateModelRoot|change");
	push(@Prefs_Hash, "privateSrcRoot|\$privateSrcRoot|change");
	push(@Prefs_Hash, "ADMINDIR|$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}|change");
	push(@Prefs_Hash, "SubmissionCheck.Policy|%PSETHOME%\\lib\\baselib.sc|change");
	push(@Prefs_Hash, "QACockpit.BaseLibrary|%PSETHOME%\\lib\\baselib.sc|change");
	ChangePrefHash(@Prefs_Hash);
}

sub UNIXPrefsUpdate(@_)
{
	my(@Prefs_Hash)=@_;
	my($PrefsFileName)="$ENV{ADMINDIR}/prefs/build.prefs";
	my($TmpPrefsFileName)="$PrefsFileName.tmp";

	if(!open(PrefsFile,"$PrefsFileName")){&dismb_msg::die("Could not open prefs file, \$1, for reading", $PrefsFileName);}
	if(!open(TmpPrefsFile,">$TmpPrefsFileName")){&dismb_msg::die("Could not open prefs file, \$1, for writing", $TmpPrefsFileName);}
	while(<PrefsFile>)
	{
		s@[\015\012]@@g;
		print(TmpPrefsFile $_ . $ENV{line_separator});
	}
	close(PrefsFile);
	close(TmpPrefsFile);
	
	run_command("RENAME", $TmpPrefsFileName, $PrefsFileName);

	push(@Prefs_Hash, "sharedSrcRoot|$ENV{sharedSrcRoot}|change");
	push(@Prefs_Hash, "sharedModelRoot|$ENV{sharedModelRoot}|change");
	push(@Prefs_Hash, "privateModelRoot|\$privateModelRoot|change");
	push(@Prefs_Hash, "privateSrcRoot|\$privateSrcRoot|change");
	push(@Prefs_Hash, "ADMINDIR|$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}|change");
	push(@Prefs_Hash, "SubmissionCheck.Policy|\$PSETHOME/lib/baselib.sc|change");
	push(@Prefs_Hash, "QACockpit.BaseLibrary|\$PSETHOME/lib/baselib.sc|change");
	ChangePrefHash(@Prefs_Hash);
}

sub PrefsUpdate(@_)
{
	my($PrefsFileName)="$ENV{ADMINDIR}/prefs/build.prefs";
	my($BuildPDF,$UserPDF,$ServerPDF);
	my(@CommonPrefsHash)=();
	if($ENV{HostType}=~m@^NT$@) {
		($BuildPDF,$UserPDF,$ServerPDF)=(
			"%ADMINDIR%/pdf/build.pdf",
			"%ADMINDIR%/pdf/user.pdf",
			"%ADMINDIR%/pdf/server.pdf");
	}
	else {
		($BuildPDF,$UserPDF,$ServerPDF)=(
			"\$ADMINDIR/pdf/build.pdf",
			"\$ADMINDIR/pdf/user.pdf",
			"\$ADMINDIR/pdf/server.pdf");
	}

	push(@CommonPrefsHash, "pdfFileBuild|$BuildPDF|change");
	push(@CommonPrefsHash, "pdfFileUser|$UserPDF|change");
	push(@CommonPrefsHash, "defaultPDF|$ServerPDF|change");
#	push(@CommonPrefsHash, "projectHome|${ProjectName}|change");
	push(@CommonPrefsHash, "projectHome|Home|change");
	push(@CommonPrefsHash, "cCompiler|$ENV{cCompiler}|change");
	push(@CommonPrefsHash, "cPlusCompiler|$ENV{cPlusCompiler}|change");

	if($ENV{HostType}=~m@^NT$@) {
		NTPrefsUpdate(@CommonPrefsHash);
	}
	else {
		UNIXPrefsUpdate(@CommonPrefsHash);
	}
}

sub CreateBuildServerBuildInfoFile(@_)
{
	my($BuildInfoFileName)="$ENV{ADMINDIR}/tmp/disbuild/build_info";
	if(-f $BuildInfoFileName)
	{
		if(!unlink($BuildInfoFileName)){&dismb_msg::die("Could not open build info file, \$1.", $BuildInfoFileName);}
	}
	if(!open(BuildInfoFile,">$BuildInfoFileName")){&dismb_msg::err("can't open build info file, \$1, for writing.", $BuildInfoFileName);}
	print(BuildInfoFile "#HOST=$ENV{BUILDSERVERHOST}\n");
	print(BuildInfoFile "#ADMINDIR=$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}\n");
	print(BuildInfoFile "#PREFS=$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}/prefs/build.prefs\n");
	print(BuildInfoFile "#CLEARTOOL=cleartool\n");
	close(BuildInfoFile);
}

sub PrjFileAdmindirConversionHack(@_)
{
	my($OldADMINDIR,$NewADMINDIR)=@_;
	$prjFilesRef=&dismb_lib::find_files_with_pattern("$NewADMINDIR/model/pmod", "pmod.prj");
	foreach $prjfile (@$prjFilesRef)
	{
		if(!open(PmodPrjFile_new,">$prjfile.new")){&dismb_msg::err("can't open new \$1.new", $prjfile);}
		if(!open(PmodPrjFile,"$prjfile")){&dismb_msg::err("can't open \$1", $prjfile);}
		while (<PmodPrjFile>)
		{
			if (m@^#HOST.*.$@)
			{
				chomp();
				s@^#HOST.*.$@#HOST=$ENV{BUILDSERVERHOST}\n@;
			}
			print(PmodPrjFile_new "$_");
		}
		close(PmodPrjFile);
		close(PmodPrjFile_new);
		run_command("RENAME", $prjfile, "$prjfile.bak");
		run_command("RENAME", "$prjfile.new", $prjfile);
	}
}

sub BuildServerFlagsConversionHack(@_)
{
	my($OldADMINDIR,$NewADMINDIR)=@_;
	my($BuildServerFlagsFileName)="$NewADMINDIR/tmp/disbuild/new_reuse.flg";
	my($TempFileName)="$NewADMINDIR/tmp/disbuild/new_reuse.flg.tmp";
	if(! -r "$BuildServerFlagsFileName")
	{
		&dismb_msg::err("Could not find build server flags file \$1", $BuildServerFlagsFileName);
	}
	print_verbose("Converting flags for build server from \$1/ to \$2/.", $OldADMINDIR, $NewADMINDIR);
	open(BuildServerFlagsFile,"${BuildServerFlagsFileName}");
	open(TempFile,">${TempFileName}");
	while(<BuildServerFlagsFile>)
	{
		chomp($_);
		s@$OldADMINDIR/@$NewADMINDIR/@g;
		print(TempFile "$_\n");
	}
	close(BuildServerFlagsFile);
	close(TempFile);
	run_command("RENAME", $TempFileName, $BuildServerFlagsFileName);
}

sub CheckPublish(@_)
{
	my($ADMINDIR)=@_;
	$LauncherLogFileName="$ADMINDIR/log/dislauncher.log";

	my($Finished)=0;
	if(!open(LauncherLogFile,"$LauncherLogFileName")){&dismb_msg::die("Could not open log file, \$1, for reading", $LauncherLogFileName);}
	while(<LauncherLogFile>)
	{
		if(m@=+\s+Succeeded\s+launch@){$Finished=1;}
	}
	close(LauncherLogFile);
	if($Finished==0){ShowUsageError("dismb did not complete finalize pass.");}
}

sub PublishModel(@_)
{
#	if(tempbuild admindir is the same as publish admindir)
	&dismb_msg::push_new_message_group("publication");
	print_always("Publishing model \$1/\$2\n\tfrom:\t\$3\n\tto:\t\$4/model", $ENV{MODELNAME}, $ENV{VERSION}, $ENV{TEMPBUILD}, $ENV{DISIM});
	if(! -w "$ENV{DISIM}/model/$ENV{MODELNAME}")
	{
		print_verbose("Making directory \$1/model/\$2 for model publishing.", $ENV{DISIM}, $ENV{MODELNAME});
		if(!mkdir("$ENV{DISIM}/model/$ENV{MODELNAME}",0755)){ShowUsageError("Failed to create \$1/model/\$2", $ENV{DISIM}, $ENV{MODELNAME});}
	}

	if(-r "$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}")
	{
		print_always("The specified version, \$1/\$2, already exists.", $ENV{MODELNAME}, $ENV{VERSION});
		require(FindFile("$ENV{PSETHOME}/mb/ServerAdmin.pl"));
		ServerAdmin("stop");
		print_always("Removing previous version \$1/\$2", $ENV{MODELNAME}, $ENV{VERSION});
		run_command("RMRF", "$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}");
                if (-r "$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}")
                {
                    #error if directory still exists
		    ShowUsageError("Failed to remove previous version \$1/\$2. Please make sure the directory or its contents are not being used by any other process.", $ENV{MODELNAME}, $ENV{VERSION});
                }
	}
	print_verbose("Moving \$1/\$2 to publish area.", $ENV{MODELNAME}, $ENV{VERSION});
	run_command("$ENV{PSETHOME}/bin/disperl", "$ENV{PSETHOME}/mb/launcher.pl", "-masterAdminDir", "$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}", "-workAdminDir", "$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}", &dismb_msg::lp_flags(), "-stdout");
	CheckPublish("$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}");
	run_command("RMRF", "$ENV{TEMPBUILD}/$ENV{MODELNAME}");
	if(!chmod(0777,"$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}/log")){ShowUsageError("Failed to change permissions on \$1/model/\$2/\$3/log.", $ENV{DISIM}, $ENV{MODELNAME}, $ENV{VERSION});}
	&dismb_msg::pop_new_message_group;
}

sub PublishAndReleaseModel {
    $ProjectName=$ProjectNameList[0];
    PublishModel();

    $ENV{ADMINDIR}="$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}";
    SetupEnvironment("sharedSrcRoot","sharedModelRoot");  # Reevaluate with new ADMINDIR.
    PrjFileAdmindirConversionHack("$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}",
				  "$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}");
    CreateBuildServerBuildInfoFile();
    AddServerAndUserPDFhack();
    PrefsUpdate();

    ReleaseModel();
    &dismb_msg::report_progress("PublishAndReleaseModel");
    return 1;
}

if ($0 =~ m@ReleaseModel[^/\\]*@) {
    GetInstallationValues(@RequiredInstallationVariables,"VFB_DISPLAY");
    CheckArguments("MODELNAME","VERSION","MODE");
    $ENV{ADMINDIR}="$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}";
    SetupEnvironment();
    PublishAndReleaseModel();
}
else {
    1;
}
