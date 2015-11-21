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
# Usage:  	perl CreateSummary.pl -v<MODELNAME/VERSION> [-V] [-D<repository>]for version
#			or
#			perl CreateSummary.pl -v<MODELNAME> [-V] [-D<repository>]for model
# Description:	Creates an HTML build summary for a model or version

require ("$ENV{PSETHOME}/mb/IMUtil.pl");

$usage="-v<MODELNAME[/VERSION]> [-V]";

sub DiagnoseAndCleanUpAndExit {
    if (defined $@) {print $@;}
    CleanUpAndExit(@_);
}

sub DescribeBuild(@_) {
	my($outFile,$BuildName,@ArgList)=@_;
	print_verbose("Describing $BuildName...\n", );
	my($buildDescr)="";
	if($BuildName=~m@ClearCase@) {
	    my($ccView)="current";
	    my($ccPattern)="all derived objects ";
		if(not(@ArgList[1] eq "")) {
			$ccView = "'".@ArgList[1]."'";
  		}
		if(not(@ArgList[0] eq "")) {
	    	$ccPattern="derived objects matched '@ArgList[0]' pattern ";
  		}
		$buildDescr = "Model ${ccPattern}in the $ccView ClearCase view.";
	} elsif($BuildName=~m@FixedLocationLog@) {
	    my($bwd)="";
	    if(not(@ArgList[1] eq "")) {
	    	$bwd="<br>The build working directory is '@ArgList[1]'.";
     	}
		$buildDescr = "Model all compilations logged in '@ArgList[0]' during a custom build.${bwd}";
	} elsif($BuildName=~m@ScanDSX@) {
		$buildDescr = "Model the software built by Microsoft Visual C++ using '@ArgList[0]' file.";
	} elsif($BuildName=~m@FileListFromFileSystem@) {
	    my($bwd)=".";
	    my($exts)="";
	    my($opts)=".";
	    if(not(@ArgList[5] eq "")) {
		   $bwd="<br>The build working directory is '@ArgList[5]'.";
     	}
	    if(not(@ArgList[4] eq "")) {
     		$exts="<br>Build all source files with '@ArgList[4]' extensions.";
     	}
	    if(not(@ArgList[3] eq "")) {
	    	$opts=" with '@ArgList[3]' options.";
     	}
		$buildDescr = "Model the source files within '@ArgList[2]' directory using '@ArgList[0]' compiler${opts} ${exts} ${bwd}";
	} elsif($BuildName=~m@FixedLocationTable@) {
		$buildDescr = "Model a set of files listed in the '@ArgList[0]' table which supplies specific attributes for each file.";
	} elsif($BuildName=~m@Custom@) {
		$buildDescr = "Model the source files using user defined method.";
 	}
 	if(not($buildDescr eq "")){
 		print($outFile "			<li>$buildDescr</li>\n");
  	}
}

sub SetupSummary(@_) {
    my($outFile,$setupTxt,$header)=@_;
    my($BuildNum)=0;
    if(open(setupFile,$setupTxt)) {
        while(<setupFile>) {
            chomp();
			if(m@^BUILD=@) {
				my($BuildName);
				my(@ArgList)=();
				if(m@^BUILD=(\w+)$@) {
					$BuildName=$1;
				} elsif(m@^BUILD=(\w+)\((.*)\)$@) {
					$BuildName=$1;
					# Arguments are interpreted as in the shell.
					@ArgList = map InterpretBuildArg($_), ParseBuildArgs($2);
				} else {
					ShowUsageError("Syntax error in \$1: \$2", $setupTxt, $_);
				}
				if(defined $BuildName) {
				    if($BuildNum==0) {
				    	print($outFile $header);
				    	print($outFile "		<ul>\n");
        			}
        			$BuildNum += 1;
					print_verbose("BUILD        $BuildName(" . (join ",", @ArgList) . ")\n");
					DescribeBuild($outFile,$BuildName,@ArgList);
			    }
			}
        }
		if($BuildNum > 0) {
			print($outFile "		</ul>\n");
  		}
        close(setupFile);
    } else {
		print_always("Cannot open ${setupTxt}.\n", );
    }
}

sub BuildSummary(@_) {
    my($outFile,$summaryName)=@_;
    my($attemptedFiles)="N/A";
    my($succeededFiles)="N/A";
    my($cleanFiles)="N/A";
    my($curCleanFiles)="N/A";
    my($curAttemptedFiles)="N/A";
    my($curSucceededFiles)="N/A";
    my($baseline)="N/A";
    my($incremental)=0;
    if(open(summaryFile,$summaryName)) {
        while(<summaryFile>) {
            chomp();
			@fields = split(/:/, $_);
			# trim a keyword and a value
			@fields[0] =~ s/^\s+//;
			@fields[0] =~ s/\s+$//;
			@fields[1] =~ s/^\s+//;
			@fields[1] =~ s/\s+$//;
			if(@fields[0] eq "attempted") {
			    $attemptedFiles=@fields[1];
   			} elsif(@fields[0] eq "succeeded") {
   			    $succeededFiles=@fields[1];
   			} elsif(@fields[0] eq "clean") {
   			    $cleanFiles=@fields[1];
   			} elsif(@fields[0] eq "current attempted") {
   				$curAttemptedFiles=@fields[1];
				$incremental = 1;
      		} elsif(@fields[0] eq "current succeeded") {
      		    $curSucceededFiles=@fields[1];
				$incremental = 1;
        	} elsif(@fields[0] eq "current clean") {
        	    $curCleanFiles=@fields[1];
				$incremental = 1;
        	} elsif(@fields[0] eq "baseline") {
        	    $baseline=@fields[1];
				$incremental = 1;
         	}
        }
        close(summaryFile);
    } else {
    	print($outFile "		<h3 style=\"color: #c90000;\">The build summary is not available.</h3>\n");
		return;
    }
    if($incremental > 0) {
		print($outFile "		<h3>The model was built incrementally. Baseline version is $baseline</h3>\n");
    }
    print($outFile "		<h3>The build summary:</h3>
		<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\n");
    if($incremental > 0) {
		print($outFile "		<tr align=\"center\">
			<td></td>
			<td>Total <br> in the model</td>
			<td>Current Build</td>
	</tr>\n");
    }
    print($outFile "		<tr>
			<td>Attempted</td>
			<td align=\"center\">${attemptedFiles}</td>\n");
    if($incremental > 0) {
		print($outFile "			<td align=\"center\">$curAttemptedFiles</td>\n");
    }
    print($outFile "		</tr>\n");
    print($outFile "		<tr>
			<td>In the model</td>
			<td align=\"center\">${succeededFiles}</td>\n");
    if($incremental > 0) {
		print($outFile "			<td align=\"center\">${curSucceededFiles}</td>\n");
    }
    print($outFile "		</tr>\n");
    print($outFile "		<tr>
			<td>Cleanly parsed</td>
			<td align=\"center\">${cleanFiles}</td>\n");
    if($incremental > 0) {
		print($outFile "			<td align=\"center\">${curCleanFiles}</td>\n");
    }
    print($outFile "		</tr>\n");
    print($outFile "		</table>\n");
}

sub CreateSummary() {
	my($ModelPath)="";
	my($methodsHeader)="";
	my($title)="";
	my($verStr)="";
	my($htmlSummaryName)="";
	my($imgSrc)="$ENV{PSETHOME}/resources/common/";
	if($dismb::is_NT) {
		$imgSrc="/".$imgSrc;
		# replace back slashes to slashes
		$imgSrc =~ s%\\%/%g;
 	}
	if(defined($ENV{VERSION})) {
		$ModelPath = "$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}";
		$methodsHeader="		<h3>The following build methods were used by the build:</h3>\n";
		$imgSrc=$imgSrc."versionheader.gif";
		$title="Version summary";
		$verStr = "$ENV{MODELNAME}/$ENV{VERSION}";
		$htmlSummaryName = $ModelPath . "/log";
	} else {
		$ModelPath = "$ENV{DISIM}/template/$ENV{MODELNAME}Template";
		$title="Model summary";
		$imgSrc=$imgSrc."modelheader.gif";
		$methodsHeader="		<h3>The following methods are used to gather information about source files:</h3>\n";
		$verStr = "$ENV{MODELNAME}";
		$htmlSummaryName = $ModelPath . "/config";
	}

	$htmlSummaryName=$htmlSummaryName."/summary.html";
	print_verbose("Opening file $htmlSummaryName...");
	if(open(outFile,">$htmlSummaryName")) {
		print(outFile "<html>
	<head>
	<title>${title}</title>
	</head>
	<body>
		<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">
		<tr>
			<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">
			<tr>
				<td colspan=\"2\"><img src=\"file://${imgSrc}\"></td>
			<tr>
				<td>&nbsp;</td>
				<td width=\"96%\">
					<font face=\"Tahoma\" size=\"10\">
						<b>${verStr}</b>
					</font>
				</td>
			</tr>
			<tr>
				<td valign=top colspan=\"2\"><hr size=\"2\" color=\"blue\" width=\"100%\"></hr></td>
				<td valign=top><hr size=\"2\" color=\"blue\" width=\"100%\"></hr></td>
			</tr>
			</table>
		</tr>
		</table>\n");
		if(defined($ENV{VERSION})) {
			BuildSummary(outFile,"$ModelPath/tmp/disbuild/summary.txt");
		}
		SetupSummary(outFile,"$ModelPath/config/Setup.txt",$methodsHeader);
		print(outFile "
	</body>
</html>\n");
		close(outFile);
	}
}

if ($0 =~ m@CreateSummary[^/\\]*@) {
	GetInstallationValues(@RequiredInstallationVariables,"VFB_DISPLAY");
	CheckArguments("MODELNAME");
	&dismb_msg::define_progress_task("summarizing model build");
	CreateSummary();
	&dismb_msg::report_progress("summarizing model build");
	CleanUpAndExit(0);
}
else {
    1;
}
