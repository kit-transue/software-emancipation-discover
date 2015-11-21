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
# Usage:  	disperl QueryResumability.pl -D<repository> -v<VERSION>
# Description:	Print list of resumability options.

require ("$ENV{PSETHOME}/mb/IMUtil.pl");

$FlagsFormat="-v<MODELNAME/VERSION> [-n] [-V]";

sub QueryResumability(@_)
{
        if (exists($ENV{TEMPBUILD}) and exists($ENV{MODELNAME}) and exists($ENV{VERSION})) {
		# Must be past .flg generation, and have an admindir,
		$ENV{ADMINDIR}="$ENV{TEMPBUILD}/$ENV{MODELNAME}/$ENV{VERSION}";
		my($setup_template) = "$ENV{DISIM}/template/$ENV{MODELNAME}Template/config/Setup.txt";
		my($admindir_setup) = "$ENV{ADMINDIR}/config/Setup.txt";
		my($dev,$ino,$mode,$nlink,$uid,$gid,$rdef,$size,$atime,$template_mtime,@more) = stat $setup_template;
		my($dev,$ino,$mode,$nlink,$uid,$gid,$rdef,$size,$atime,$admindir_mtime,@more) = stat $admindir_setup;
		my($disbuild) = "$ENV{ADMINDIR}/tmp/disbuild";
		if (!-z "$ENV{ADMINDIR}/config/translator.flg" && -d $disbuild && $template_mtime <= $admindir_mtime) {
			print("automatic\n");
			print("analysis\n");
			if (-e "$disbuild/analysis.upto_date") {
			    print ("parse\n");
			}
			if (-e "$disbuild/parse.upto_date") {
				print("update\n");
			}
			if (-e "$disbuild/update.upto_date") {
				print("finalize\n");
			}
		}
	}
}

$THIS_FILE="QueryResumability.pl";
GetInstallationValues(@RequiredInstallationVariables);
CheckArguments("MODELNAME","VERSION");

$ENV{ADMINDIR}="$ENV{DISIM}/model/$ENV{MODELNAME}/$ENV{VERSION}";
QueryResumability();
