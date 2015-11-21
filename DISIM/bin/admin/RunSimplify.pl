# Usage:  	perl RunSimplify.pl -v<VERSION>
# Description:	run Simplify in batch mode

require ("$ENV{PSETHOME}/mb/IMUtil.pl");

$FlagsFormat="-v<MODELNAME/VERSION> [-n] [-V]";
%Simplify=();
%ExcludeSimplify=();
sub CreateParseFileList(@_)
{
	my($SimplifyInputFileGroupName)="$ENV{privateModelRoot}/groups/ext/SimplifyInput.grp";
	my($FlagsFileName)="$ENV{ADMINDIR}/tmp/disbuild/new_reuse.flg";
	my(@ParseFileNameArray)=();

	open(FlagsFile,"$FlagsFileName");
	while(<FlagsFile>)
	{
		if(m@^\s*"([^\"]+)"\s@)
		{
			my($ParseFileName)=$1;
			if ($ParseFileName !~ m/.(h|hpp)$/i) {
				push(@ParseFileNameArray,$ParseFileName);
			}
		}
	}
	close(FlagsFile);
	print_verbose("Found \$1 parsable files", scalar(@ParseFileNameArray));
	if(!open(SimplifyInputFileGroup,">$SimplifyInputFileGroupName")){&dismb_msg::die("Could not create file \$1", $SimplifyInputFileGroupName);}
	foreach my $ParseFileName (@ParseFileNameArray)
	{
	    if ( $ParseFileName =~ /\s+/ ) {
		print(SimplifyInputFileGroup "\"$ParseFileName\"\n");
            } else {
		print(SimplifyInputFileGroup "$ParseFileName\n");
            }
	}
	close(SimplifyInputFileGroup);
}

sub RunSimplify(@_)
{
	CreateParseFileList();

	my($TCLScriptName)="$ENV{PSETHOME}/lib/BatchSimplify.dis";
	my($PrefsFileName)="$ENV{ADMINDIR}/prefs/build.prefs";
	my($UserPDFName)=GetExpandedPref($PrefsFileName,pdfFileUser);

	### Because you just gotta have a home project ###
	my($HomeProjectName)=GetExpandedPref($PrefsFileName,projectHome);
	my(@CommandLineArguments)=("-pdf", $UserPDFName, "-prefs", $PrefsFileName, "-home", "/$HomeProjectName", "-batch", "-source", $TCLScriptName);


	run_command("$ENV{PSETHOME}/bin/model_server", @CommandLineArguments);
}

sub ProcessSimplifyOutput(@_)
{
# Subroutine reads the Simplify log and creates a hash of
# the source file and the remove and substitute string.
	my($CurrentFile)="";
	my(@RemoveArray)=();
	my(@SubstituteArray)=();

	#main loop
	while (<LOG>)
	{
		chomp();
		if (m@The following changes will be made to ([^\t\n\r\f]+)@)
		{
			$CurrentFile=$1;
			my($directory)=$1;
			$directory=~s@(.*[/\\])[^\s/\\]*@$1@;
			if ($DirectoryHash{$directory})
			{
				$DirectoryHash{$directory}=~s@$@;$CurrentFile@;
			}
			else
			{
				$DirectoryHash{$directory}=$CurrentFile;
			}
		}
		if (m@DIScover Simplify for @)
		{
			if(@SubstituteArray)
			{
				push(@RemoveArray,join(";",@SubstituteArray));
			}
			if(! $CurrentFile=~m@^\s*$@)
			{
				$Simplify{$CurrentFile}=join("!",@RemoveArray);
			}
			@RemoveArray=();
			@SubstituteArray=();
		}
		if(m@remove (.*$)@)
		{
			$CurrentRemove=$1;
			if(@SubstituteArray)
			{
				push(@RemoveArray,join(";",@SubstituteArray));
			}
			@SubstituteArray=("$CurrentRemove");

		}
		if (m@substitute (.*)@)
		{
			@substituteFiles = split(" for ", $1);
			$CurrentSubstitute=$substituteFiles[0];
			push(@SubstituteArray,$CurrentSubstitute);
		}
	}

	close(LOG);
}

sub PrintFormatedSimplifyOutput(@_)
{
# Prints the contents of the Simplify hash in indented format
	print(OUT "Code Integrity Simplify Report $ENV{MODELNAME}_$ENV{VERSION} \n\n");
	my($SubstituteString)="";
	my($x)="";
	foreach (keys %Simplify) {
		my($RemoveString)=$Simplify{$_};
		if (! $RemoveString) {
			if(! $_) {
				print(OUT "#$_\n");
			}
		} else 	{
			print(OUT "$_\n");
			my(@RemoveArray)=split("!",$RemoveString);
			foreach $SubstituteString(@RemoveArray) {
				my(@SubstituteArray)=split(";",$SubstituteString);
				print(OUT "\t$SubstituteArray[0]\n");
				for $x (1 .. $#SubstituteArray) {
					print(OUT "\t\t$SubstituteArray[$x]\n");
				}
			}
		}
	}
	close(OUT);
}

sub FilterFormatedSimplifyOutput(@_)
{
# Filters through the Simplify hash and removes files matching criteria of 
# the configure file and puts them into the ExcludeSimplify hash
	my($SubstituteString)="";
	my($x)="";	
	my(@ExcludeRemoveArray)=();
	my(@ExcludeRemArray)=();
	my(@ExcludeSubArray)=();
	my($Threshold)=1000;
	my($RemoveString)="";

	while(<CONFIG>)
	{
		chomp();
		if(m@ExcludeRemove:@)
		{
			my($KeyWord,$Remove)=split(":",$_,2);
			$Remove=~s@\\@/@;
			$Remove=~s@^\s*@@;
			$Remove=~s@\s*$@@;
			push(@ExcludeRemArray,$Remove);
		}
		if(m@ExcludeSubstitute:@)
		{
			my($KeyWord,$Substitute)=split(":",$_,2);
			$Substitute=~s@\\@/@;
			$Substitute=~s@^\s*@@;
			$Substitute=~s@\s*$@@;
			push(@ExcludeSubArray,$Substitute);
		}
		if(m@Threshold:@)
		{
			my($KeyWord,$threshold)=split(":",$_,2);
			$threshold=~s@^\s*@@;
			$threshold=~s@\s*$@@;
			$Threshold=$threshold;
		}		
	}

	close(CONFIG);

	foreach (keys %Simplify)
	{	
		$FileString=$_;
		$RemoveString=$Simplify{$FileString};
		@ExcludeRemoveArray=();

		my(@RemoveArray)=split("!",$RemoveString);


		my($x)=0;			
		for $x (0 ..$#RemoveArray)
		{
			$SubstituteString = $RemoveArray[$x];
			my(@SubstituteArray)=split(";",$SubstituteString);
			my($AlreadySpliced)=0;
			
			# If the Remove file matches any of the exclude remove files, remove from Simplify hash. 
			foreach(@ExcludeRemArray)
			{
				$ExcludeRem=$_;
				if($SubstituteArray[0]=~$ExcludeRem)
				{							
					push(@ExcludeRemoveArray,$RemoveArray[$x]);
					splice(@RemoveArray,$x,1);
					$AlreadySpliced=1;
				}

			}
			# If the Substitute files match any of the exclude substitute files, remove from Simplify hash. 
			
			foreach(@ExcludeSubArray)
			{
				$ExcludeSub=$_;				
				
				if($SubstituteString=~m@.*;(.*$ExcludeSub.*)@)
				{
					push(@ExcludeRemoveArray,$RemoveArray[$x]);
					if(! $AlreadySpliced)
					{
						splice(@RemoveArray,$x,1);
						$AlreadySpliced=1;
					}
				}
			}


			# If there are more substitute files than the threshold allows, remove from Simplify hash. 
			my($length)=scalar(@SubstituteArray);
			if ($length>($Threshold+1))
			{
				push(@ExcludeRemoveArray,$SubstituteString);
				if(! $AlreadySpliced)
				{
					splice(@RemoveArray,$x,1);
				}
				 
			}

		}

		$ExcludeSimplify{$FileString}=join("!",@ExcludeRemoveArray);
		$Simplify{$FileString}=join("!",@RemoveArray);
		
	}
}

sub PrintExcludedSimplifyOutput(@_)
{
	# Prints the contents of the ExcludeSimplify hash in indented format
	$ExOut="$ENV{privateModelRoot}/$ENV{MODELNAME}_$ENV{VERSION}_ExcludedFormattedSimplify.log";
	open(EXOUT, ">$ExOut") or &dismb_msg::die("Cannot open \$1 for write :\$2", $ExOut, $!);
	print(EXOUT "Code Integrity Simplify Excluded Output $ENV{MODELNAME}_$ENV{VERSION} \n\n");
	my($SubstituteString)="";
	my($x)="";
	foreach (keys %ExcludeSimplify)
		{
		my($RemoveString)=$ExcludeSimplify{$_};
		if (! $RemoveString)
			{
			if(! $_)
				{
				print(EXOUT "#$_\n");
				}
			}
		else
			{
			print(EXOUT "$_\n");
			my(@RemoveArray)=split("!",$RemoveString);
			foreach $SubstituteString(@RemoveArray)
				{
				my(@SubstituteArray)=split(";",$SubstituteString);
				print(EXOUT "\t$SubstituteArray[0]\n");
				for $x (1 .. $#SubstituteArray)
					{
				        print(EXOUT "\t\t$SubstituteArray[$x]\n");
				        }
				}
			}	
		}
	close(EXOUT);
}

sub OpenFiles(@_)
{
	$SimpLog="$ENV{privateModelRoot}/$ENV{MODELNAME}_$ENV{VERSION}_Simplify.log";
	open(LOG, $SimpLog) or &dismb_msg::die("Cannot open \$1 for read :\$2", $SimpLog, $!);
	$Out="$ENV{privateModelRoot}/$ENV{MODELNAME}_$ENV{VERSION}_FormattedSimplify.log";
	open(OUT, ">$Out") or &dismb_msg::die("Cannot open \$1 for write :\$2", $out, $!);
	$Html="$ENV{privateModelRoot}/$ENV{MODELNAME}_$ENV{VERSION}_Simplify.html";
	open(HTML, ">$Html") or &dismb_msg::die("Cannot open \$1 for write :\$2", $Html, $!);


}
sub PrintOutputHTML(@_)
{
# Prints contents of the Simplify hash to an HTML file
	my ($filename, $dir, $status) = ("");
 	print_header();
	print  HTML "<H2> File by file analysis</H2>\n";

 	foreach(keys %DirectoryHash)
 	{
		my($dir)=$_;
		my(@FileArray)=split(";",$DirectoryHash{$_});
		if($dir=~m@^[^HASH]@)
		{
			print HTML "<P>\n<HR>\n<P>\n";
			print_attached_title (dir_header($dir));
		}
		print HTML "<OL>\n";
		foreach (@FileArray)
		{
			$filename=$_;
			my($RemoveString)=$Simplify{$_};

			# print one paragraph per Source File
			 $filename=~s@File: /([^\s])@$1@;
			 $filename=~s@(.*/)@@;

			print_attached_title (file_header($filename));
			print HTML "<TABLE cellpadding=\"3\" align=\"center\" width=\"90%\" cellspacing=\"0\" border=\"1\">\n";
			print HTML "\t<TR bgcolor=\"lightgreen\">\n";
			print HTML "\t\t<TH>Status</TH>\n";
			print HTML "\t\t<TH>Remove File(s)</TH>\n";
			print HTML "\t\t<TH>Line #</TH>\n";
			print HTML "\t\t<TH>Substitute File(s)</TH>\n";
			print HTML "\t</TR>\n";

			my(@RemoveArray)=split("!",$RemoveString);
			if(! @RemoveArray)
			{
				$status = "<IMG SRC=\"thumbs_up.gif\">";
				print HTML "\t<TR valign=\"top\">\n";
				print HTML "\t\t<TD align=\"center\">$status</TD>\n";
				print HTML "\t\t<TD align=\"center\"><B>none</B></TD>\n";
				print HTML "\t\t<TD align=\"center\">n/a</TD>\n";
				print HTML "\t\t<TD align=\"center\"><B>none</B></TD>\n";
				print HTML "\t</TR>\n";

			}
			foreach $SubstituteString(@RemoveArray)
			{
				$status = "<IMG SRC=\"stop.gif\">";

				my(@SubstituteArray)=split(";",$SubstituteString);
				my($RemoveFile)=$SubstituteArray[0];
				$RemoveFile=~s@R: /([^\s])@$1@;
				$RemoveFile=~s@(.*/)@@;
				$RemoveFile=~s@ at line ([^\s]*)@@;
				my($line)=$1;
				my(@SubstituteFiles)=();
				for $x (1 .. $#SubstituteArray)
					{
					my($substitute)=$SubstituteArray[$x];
					$substitute=~s@S: /([^\s])@$1@;
					$substitute=~s@(.*/)@@;
					push(@SubstituteFiles,"$substitute<BR>");
					}
				if(!@SubstituteFiles)
				{
					@SubstituteFiles="<B>none</B>";
				}
                                print HTML "\t<TR valign=\"top\">\n";
                                print HTML "\t\t<TD align=\"center\">$status</TD>\n";
                                print HTML "\t\t<TD align=\"center\">$RemoveFile</TD>\n";
                                print HTML "\t\t<TD align=\"center\">$line</TD>\n";
                                print HTML "\t\t<TD align=\"center\">@SubstituteFiles</TD>\n";
                                print HTML "\t</TR>\n";
				$status="/";
			}
			print HTML "</TABLE>\n";
		}
		print HTML "</OL>\n";
	}
	my($current_time) = Current_Date();
	print HTML "<BR><HR><BR>Report generation completed on $current_time for $ENV{MODELNAME}/$ENV{VERSION}";
	close(HTML);
}

sub print_attached_title
# creates a two-cell table with title and contents (usually, the contents
# itself is a table). The goal is to prevent stupid cut between title and
# data when sending the file to a printer
# At the end of the data, requires a </table> tag. 
{ 
	print HTML "<TABLE cellpadding=\"0\" width=\"100%\" border=\"0\">\n";
	print HTML "\t<TR>\n";
	print HTML "\t\t<TD><H3>@_</H3></TD>\n";
	print HTML "\t</TR>\n";
	print HTML "</TABLE>\n";
 
} 
 
sub dir_header
{
# Formats header for Directories in HTML
	my ($dir) = @_;

        if ($dir)
        {
		return "<IMG ALIGN=\"bottom\" SRC=\"dir.gif\"><B> $dir </B>";
	}
} 
 

sub file_header
{
# Formats header for Source Files in HTML
	my ($filename) = @_;
    if ($filename){
		return "<IMG ALIGN=\"bottom\" SRC=\"spring.jpg\"><B>Source File: $filename</B>";
	}
} 
 
sub print_header(@_) 
{ 
# Prints a Header for the HTML output.
	print HTML "<CENTER>\n";
	print HTML "<TABLE cellpadding=\"0\" width=\"100%\" cellspacing=\"0\" border=\"0\" >\n";
	print HTML "\t<TR>\n";
	print HTML "\t\t<TD rowspan=\"2\"><IMG border=\"0\" SRC=\"upspr.gif\"></TD>\n";
	print HTML "\t\t<TD align=\"center\" width=\"90%\">\n";
	print HTML "\t\t\t<TABLE cellpadding=\"5\" align=\"center\" cellspacing=\"0\" bgcolor=\"lightgreen\" border=\"1\">\n";
	print HTML "\t\t\t\t<TR>\n";
	print HTML "\t\t\t\t\t<TD><B><FONT SIZE=\"+3\">Code Integrity Simplify Report</FONT></B></TD>\n";
	print HTML "\t\t\t\t</TR>\n";
	print HTML "\t\t\t</TABLE>\n";
	print HTML "\t\t</TD>\n";
	print HTML "\t<TR>\n";
	print HTML "\t\t<TD align=\"center\"><FONT SIZE=\"-1\">Report generated automatically by Code Integrity for $ENV{MODELNAME}/$ENV{VERSION}</FONT>\n"; 
	print HTML "\t</TR>\n";
	print HTML "</TABLE>\n";
	print HTML "</CENTER>\n";

	print HTML "<BR>\n<BR>\n";
	print HTML "<EM>Code Integrity Simplify </EM>will\n";
	print HTML "<OL>� Reduce program complexity, reducing maintenance time</OL>\n";
	print HTML "<OL>� Decrease compilation time</OL>\n";
	print HTML "<OL>� Reduce executable file size</OL>\n";

	print HTML "The \n";
	print HTML "<EM>Code Integrity Simplify </EM>\n";
	print HTML "report lists the header files for each C and C++ program.\n";
	print HTML "<P>Simply identifies header files:\n";
	print HTML "<OL>� Included in the program, but not used</OL>\n";
	print HTML "<OL>� Included in the program, but that could be replaced by more efficient header files</OL>\n";
	print HTML "<P>\n<HR>\n";
} 

sub FormatSimplifyOutput(@_)
{

	OpenFiles();
	ProcessSimplifyOutput();
	
	if (@LocalArgsList)
	{
		$SimpConfig="$LocalArgsList[0]";
		open(CONFIG, $SimpConfig) or &dismb_msg::die("Cannot open \$1 for read :\$2", $simpLog, $!);	
		FilterFormatedSimplifyOutput();
		PrintExcludedSimplifyOutput();
	}

	PrintFormatedSimplifyOutput();
	PrintOutputHTML();
	CopyImages();
}

sub CopyImages(@_)
{
	my($ImageSrcDir)="$ENV{DISIM}/lib/doc/Images";
	my(@ImageSource)=("dir.gif",
			  "thumbs_up.gif",
			  "upspr.gif",
			  "stop.gif",
			  "spring.jpg");
	my($ImageDestination)="$ENV{privateModelRoot}/";
	foreach my $imName (@ImageSource) {
		run_command("DELETE", "$ImageDestination/$imName");
		run_command("CPRF", "$ImageSrcDir/$imName", $ImageDestination);
	}
}

sub Current_Date(@_)
{
        my($CurrentDate)="";
        if($ENV{HostType}=~m@^NT$@)
        {
                $CurrentDate=`cmd /c date /T`; chomp($CurrentDate);
        }
        else
        {
                $CurrentDate=`/usr/bin/date`; chomp($CurrentDate);
        }
        return($CurrentDate);
}



$THIS_FILE="RunSimplify.pl";
GetInstallationValues(@RequiredInstallationVariables,"VFB_DISPLAY");
CheckArguments("VERSION");

$ENV{ADMINDIR}="$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}";
SetupEnvironment();
#CheckVFB("$ENV{HOST}:$ENV{VFB_DISPLAY}");				#Obselete
&dismb_msg::define_progress_task("simplify");
CreateUserArea();
RunSimplify();

FormatSimplifyOutput();
&dismb_msg::report_progress("simplify");
