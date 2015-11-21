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
dismb_remote::fenvs;		# array of variables sent from driver

sub dismb_remote::get_pref {
    my $key = @_[0]; my $f = "$ENV{'DISBUILD_TMPDIR'}/mb_envs.prefs";
    
    &dismb_prefs::parse_pref($f) if ( ! defined %mbPref );
    $key = ( $key =~ /^\*psetPrefs\./) ? $key : "*psetPrefs." . $key;
    return $mbPrefs{$key} if ( defined ($mbPrefs{$key}) );
    &dismb_msg::err("No preference \$1 in file \$2",$key,$f);
    return "";
}


sub dismb_remote::load_fenv {
    return 1 if ($fenv eq "NULL"); # It's OK
    eval 'require $fenv';
    &dismb_msg::fixeval;
    &dismb_msg::die ("Loading \$1: \$2",$env,$@) if ($@);
    return 1;
}


sub dismb_remote::load_feval {
    return 1 if ($feval eq "NULL"); # It's OK
    eval 'require $feval';
    &dismb_msg::fixeval;
    &dismb_msg::die("Evaluating \$1: \$2",$feval,$@) if ($@);
}


sub dismb_remote::start_log  {
    $dismb_prefs::initialized = eval '&dismb_prefs::load_mbPrefs()';
    &dismb_msg::fixeval;
    &dismb_msg::die ("Failure loading prefs: \$1", $@) if $@;

    my($log_lock) = ($dismb_remote::log_lock) ? $dismb_remote::log_lock : "NULL";
    if (&dismb_prefs::is_verbose_mb) {
        &dismb_msg::inf("Log lock file: \$1", $log_lock);
        if ($fenv ne "NULL" || $feval ne "NULL") { 
	    &dismb_msg::inf ("      remoteEnvVarList file: \$1, remoteEval file: \$2",$fenv,$feval);
        }
        &dismb_lib::print_dismb_env ("Remote " . $host . ':' . $cpu );
    }

    &dismb_prefs::init_env_vars;
}


sub dismb_remote::reset_heart_beat {
    if ($dismb_remote::beat) {
	alarm 0;
	$SIG{'ALRM'} = 'IGNORE';
	undef $dismb_remote::beat;
	&dismb_msg::vrb("\$1.\$2: cancelled heart beat",${host},${cpu});
    }
}


sub dismb_remote::alarm_handler {
    my($tm) = &dismb_lib::time_stamp();
    &dismb_msg::vrb("\$1.\$2: Caught heart beat: \$3",${host},${cpu},$tm);
    &dismb_lib::unlink("$ENV{'DISBUILD_TMPDIR'}/heart_beat");
    &dismb_remote::next_beat();
}


sub dismb_remote::next_beat {
    $SIG{ALRM} = 'dismb_remote::alarm_handler';
    alarm $dismb_remote::beat;
}


sub dismb_remote::heart_beat {
    my $min = &dismb_remote::get_pref("MBheartBeat");
    if ($min eq "no" || (0 + $min) <= 0 || ! $min) {
	&dismb_msg::vrb("\$1.\$2: skip heart beat because of Preference MBheartBeat=\'\$3\'",${host},${cpu},$min);
	return;
    } elsif ($dismb::is_NT) {
        &dismb_msg::vrb("\$1.\$2: skip heart beat on Windows NT", ${host},${cpu});
        return;
    }
    $dismb_remote::beat = $min*20; # 20=60 seconds divided by 3
    &dismb_msg::vrb("\$1.\$2: setting heart beat alarm to \$3 seconds",${host},${cpu},$dismb_remote::beat);
    &dismb_remote::next_beat();
}


sub dismb_remote::main {
    $dismb_prefs::remote_mb   = 1;
    $dismb::is_NT = ($ENV{'OS'} eq "Windows_NT") ? 1 : 0;

    local($host, $cpu, $pass, $fenv, $feval, $log_lock) = @ARGV;
    if ( $log_lock && $log_lock ne "NULL" ) {
	die("  *** FATAL: log lock file $log_lock does not exist\n") if ( ! -f $log_lock );
	$dismb_remote::log_lock = $log_lock;
    } else {
	$dismb_remote::log_lock = "";
    }

    &dismb_remote::load_fenv;	# ENV variables
    &dismb_remote::load_feval;	# Evaluate the file

    # Note that dismb_msg::die can't be called until after @INC is set.
    die ("${host}:$cpu PSETHOME is not defined\n") if ($ENV{'PSETHOME'} eq "");
    die ("${host}:$cpu ADMINDIR is not defined\n") if ($ENV{'ADMINDIR'} eq "");

    $ENV{'DISMBDIR'} = "$ENV{'PSETHOME'}/mb" if ( !defined $ENV{'DISMBDIR'} );
    $ENV{'DISBUILD'} = $pass;

    $ENV{'DISBUILD_TMPDIR'}   = "$ENV{'ADMINDIR'}/tmp/disbuild" if ( $ENV{'DISBUILD_TMPDIR'} eq "" );
    my($exe)             = ($dismb::is_NT) ? "bin/model_server.exe" : "bin/model_server";
    my($tmp_dir)         = ($dismb::is_NT) ? "c:/temp" : "/usr/tmp";
    $ENV{'TMPDIR'}       = "$tmp_dir"             if (! defined $ENV{'TMPDIR'});
    die ("${host}:$cpu TMPDIR=$ENV{'TMPDIR'} is not a directory\n") if ( ! -d $ENV{'TMPDIR'} );
    die ("${host}:$cpu PSETHOME=$ENV{'PSETHOME'} is not a directory\n") if ( ! -d $ENV{'PSETHOME'} );
    die ("${host}:$cpu PSETHOME=$ENV{'PSETHOME'} missing executable $exe\n") if ( ! -f "$ENV{'PSETHOME'}/$exe" );

    ($psethome_bin = "$ENV{'PSETHOME'}/bin") =~ s%\\%/%g;

    @INC[$#INC] = ( defined $ENV{'DISMBDIR'} ) ? $ENV{'DISMBDIR'} : "$ENV{'PSETHOME'}/mb" ;

    require ("dismb_lib.pl");
    require ("dismb_prefs.pl");
    require ("dismb_msg.pl");
    require ("dismb_local.pl");
    require ("dismb_options.pl");

    #initialize message group from -message_group option
    &dismb_msg::init_message_group_from_option;

    use Cwd; use Cwd 'chdir';
    $ENV{'PWD'} = cwd();
    &dismb_msg::vrb ("Remote current wd= \$1",$ENV{'PWD'});

    &dismb_remote::heart_beat();
    eval '&dismb_local::run_pass ($host, $cpu)';
    &dismb_remote::reset_heart_beat();
}

eval '&dismb_remote::main()';
print $@ if $@;

unlink ($dismb_remote::log_lock) if ($dismb_remote::log_lock);

