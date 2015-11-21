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
$dismb_options::correct = 0;


sub dismb_options::add {
    local($cur_opt,$cur_val) = @_;
    return 0 if ( $cur_opt eq "" );

    if ( ! defined $mbOptions{$cur_opt} ) {
	$mbOptions{$cur_opt} = $cur_val;
    } else {
	local($old_val) = $mbOptions{$cur_opt};
	if ( $cur_val eq "" || $old_val eq "" ) {
	    &dismb_msg::die ("Wrong command line option usage: \$1",$cur_opt);
	} else {
	    if ( $old_val =~ /\b$cur_val\b/ ) {
		&dismb_msg::wrn ("Multiple command line option \$1, val \$2. Ignore",$cur_opt,$cur_val);
	    } else {
		$mbOptions{$cur_opt} = $old_val . " " . $cur_val;
	    }
	}
    }
}


sub dismb_options::process {
    #create new message group for command line options
    &dismb_msg::push_new_message_group("disMB command line options");

    local ($cur_opt, $arg, $cur_val);
    foreach $arg (@ARGV) {
	if ($arg =~ /^-/) {
	    &dismb_options::add ($cur_opt, "") if ( $cur_opt && ( $cur_val eq "" )); # previous option without value
	    $cur_opt = $arg; 
	    $cur_val = "";
	} else {
	    &dismb_msg::die ("Wrong command line option: \$1",$arg) if ( $cur_opt eq "" );
	    &dismb_options::add ($cur_opt, $arg);
	    $cur_val = $arg;
	}
    }
    
    &dismb_options::add ($cur_opt, "") if ( $cur_opt && ( $cur_val eq "" )); # previous option without value
    &dismb_options::print;

    $dismb_options::correct = 1;

    #pop new message group created for command line options
    &dismb_msg::pop_new_message_group;

    my($cumg) = &dismb_options::get("-compilation_units_message_group");
    if (defined $cumg) {
	$dismb_msg::message_group_id{"compilation units"} = $cumg;
    }

    return $dismb_options::correct;
}

sub dismb_options::print {
    local(@keys) = keys %mbOptions;
    if ($#keys >= 0) {
	&dismb_msg::inf ("Specified command line options:");
	local(@vals) = values %mbOptions;
	while ( $#keys >= 0 ) {
	    &dismb_msg::inf ("      \$1 \t\$2", pop (@keys),pop(@vals));
	}
    } else {
	&dismb_msg::vrb ("No command line options.");
    }
}


sub dismb_options::is_defined {
    local($key) = "@_";
    &dismb_msg::err ("Cannot query command line option \$1 : not processed",$key) if ( ! $dismb_options::correct );
    local($def) = ( defined ($mbOptions{$key}) ) ? 1 : 0;
    return $def;
}

sub dismb_options::get {
    local($key) = "@_";
    &dismb_msg::err ("Cannot get command line option \$1 : not processed",$key) if ( ! $dismb_options::correct );
    local ($val) = $mbOptions{$key};
    return $val;
}

sub dismb_options::get_pref {
    local($key) = @_;
    $key =~ s/^\*psetPrefs\.//;
    $key = '-' . $key;
    local($val) = &dismb_options::get ($key);
    $val = "yes" if ($val eq "" && &dismb_options::is_defined ($key));
    return $val;
}


sub dismb_options::is_defined_pref {
    local($key) = @_;
    $key =~ s/^\*psetPrefs\.//;
    $key = '-' . $key;
    local($val) = &dismb_options::is_defined ($key);
    return $val;
}








return 1;

