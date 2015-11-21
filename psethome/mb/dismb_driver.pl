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
$dismb_driver::PREINIT = "";
select STDOUT; $| = 1;

sub dismb_driver::check_admindir {
    my ($opt, $arg);
    foreach $arg (@ARGV) {
	if ($opt) {
	    $ENV{'ADMINDIR'} = $arg;
	    return;
	}
	if ($arg =~ /^-MBadmindir/) {
	    $opt = 1
	}
    }
    if ( ! defined $ENV{'ADMINDIR'} ) {
        &dismb_msg::die("Environment variable ADMINDIR is not set");
    }
    $ENV{'ADMINDIR'} =~ s%\\%/%g;
}

# Can't call dismb_msg::die if a problem arises here, because @INC hasn't
# been set yet.
sub dismb_driver::pre_init {
    $dismb::is_NT = ($ENV{'OS'} eq "Windows_NT") ? 1 : 0;
    my($exe)       = ($dismb::is_NT) ? "bin/model_server.exe" : "bin/model_server";

    if ( ! defined $ENV{'PSETHOME'} ) {
        die("Environment variable PSETHOME is not set\n");
    }
    $ENV{'PSETHOME'} =~ s%\\%/%g;
    if ( ! -d $ENV{'PSETHOME'} ) {
        die("PSETHOME=$ENV{'PSETHOME'} PSETHOME is not a directory\n");
    }
    if ( ! -f "$ENV{'PSETHOME'}/$exe" ) {
        die("PSETHOME=$ENV{'PSETHOME'} missing executable $exe\n");
    }

    if ( $ENV{'DISMBDIR'} eq "" ) {
        $ENV{'DISMBDIR'} = "$ENV{'PSETHOME'}/mb";
    }
    $ENV{'DISMBDIR'} =~ s%\\%/%g;
    if ( ! -f "$ENV{'DISMBDIR'}/dismb_driver.pl" ) {
        die("Env variable DISMBDIR is not dismb exec directory\n");
    }
    if ( defined $ENV{'HOME'} ) {
        $ENV{'HOME'} =~ s%\\%/%g;
    }

    $psethome_bin = "$ENV{'PSETHOME'}/bin";

    #Add $PSETHOME/lib to LD_LIBRARY_PATH
    if ( $dismb::is_NT ) {
        $ENV{'LD_LIBRARY_PATH'} .= ";" . "$ENV{'PSETHOME'}/lib";
    } else {
        $ENV{'LD_LIBRARY_PATH'} .= ":" . "$ENV{'PSETHOME'}/lib";
    }
}

sub dismb_driver::check_environment {
    my($tmp_dir)   = ($dismb::is_NT) ? "c:/temp" : "/usr/tmp";
    if (! defined $ENV{'TMPDIR'} ) {
        if ( defined $ENV{'TEMP'} ) {
            $ENV{'TMPDIR'} = "$ENV{'TEMP'}";
        } else {
            if (! -d "$tmp_dir") {
                &dismb_msg::msg("Creating $tmp_dir directory.");
                mkdir ("$tmp_dir", 0777);
            }
            $ENV{'TMPDIR'} = "$tmp_dir";
        }
    }
    $ENV{'TMPDIR'} =~ s%\\%/%g;
    if ( ! -d $ENV{'TMPDIR'} ) {
        &dismb_msg::die("TMPDIR=\$1 is not a directory",$ENV{'TMPDIR'}); 
    }

    &dismb_driver::check_admindir();

    if ( ! defined $ENV{'DISBUILD_TMPDIR'} ) {
        $ENV{'DISBUILD_TMPDIR'} = "$ENV{'ADMINDIR'}/tmp/disbuild";
    }
    $ENV{'DISBUILD_TMPDIR'} =~ s%\\%/%g;
}

#Check if doing incremental model build or full model build
sub dismb_driver::check_incremental_modelbuild 
{
    if ($dismb::standalone_mode) {
        if ( &dismb_prefs::get ("MBscratch") ne "yes" ) {
	    &dismb_prefs::set ('MBscratch', "no");
	    &dismb_msg::inf ("Incremental Model build");
        } else {
	    &dismb_msg::inf ("Scratch Model build; deleting old model files");
        }
    }
    else {
        my($msg);
        if ( &dismb_prefs::get ("MBscratch") ne "yes" ) {
            if ( ! -f $dismb_analysis::prev_attributes_file ) {
                #attributes file does not exist, treat this model build as
                #full build even though prefs point to incremental build
                $dismb::incremental_modelbuild = 0;
                &dismb_msg::wrn("Previous attributes file \$1 does not exist, treating this build as a full model build.", $dismb_analysis::attributes_file);
            } elsif ( ! -d "$ENV{sharedModelRoot}/pmod" || ! -d "$ENV{sharedModelRoot}/pset" ){ 
                #model directory does not exist, treat this model build as
                #full build even though prefs point to incremental build
                $dismb::incremental_modelbuild = 0;
                &dismb_msg::wrn("Previous model dirs \$1 or \$2 does not exist, treating this build as a full model build.", "$ENV{'sharedModelRoot'}/pmod", "$ENV{'sharedModelRoot'}/pset"); 
            } else {
                $dismb::incremental_modelbuild = 1;
	        &dismb_prefs::set ('MBscratch', "no");
                my($user_file) = "$ENV{'ADMINDIR'}/config/user_specified.lst";
                if ( -f $user_file ) {
                    $msg = "user specified list of files";
                    $dismb::user_specified_list = $user_file;
                } else {
                    $msg = "automatic analysis";
                    my($method) = &dismb_prefs::get("MBanalysisMethod");
                    if ( $method eq "TS" ) {
                        $msg .= " using time stamp";
                    }
                    elsif ( $method eq "CS" ) {
                        $msg .= " using checksum";
                    }
                    elsif ( $method eq "TS_CS" ) {
                        $msg .= " using time stamp and checksum";
                    } else {
                        $msg .= " (invalid value \"$method\" for 'MBanalysisMethod' preference) using time stamp";
                        $method = "TS"
                        &dismb_prefs::set ('MBanalysisMethod', "TS");
                    }
                    $dismb_analysis::method = $method;
                    $dismb::user_specified_list = "";
                }
	        &dismb_msg::inf ("Incremental Model build : \$1.", "$msg");
            }
        } else {
            $dismb::incremental_modelbuild = 0;
        }
        if ( !$dismb::incremental_modelbuild && !&dismb_prefs::is_continue_mb()) {
            #clean up for full modelbuild, if we are not resuming model build
	    &dismb_msg::inf ("Full model build, cleaning up any old model files.");
            &dismb_lib::cleanup("$ENV{'sharedModelRoot'}", ".*") if ( -d $ENV{'sharedModelRoot'} );
        }
    }
}

sub dismb_driver::init {
    local($tm) = &dismb_lib::time_stamp;
    # Create init log file
    $dismb_driver::preinit_file = "$ENV{'TMPDIR'}/dismb_init_$$.log";
    open (PREINIT_LOGFILE, ">$dismb_driver::preinit_file") || &dismb_msg::die("Failed to create temporary init file  \$1",$dismb_driver::preinit_file);
    $dismb_driver::PREINIT = PREINIT_LOGFILE;

    #create new message group for dismb initialization
    &dismb_msg::push_new_message_group("disMB initialization");
    &dismb_print ("\n                     ===== disMB init start \$1=====\n", $tm) ;

    &dismb_driver::check_environment();

    &dismb_lib::print_dismb_env ("driver");

    if ( ! &dismb_options::process() ) {
	&dismb_msg::die ("Command line options are wrong.");
    }
    if ( ! &dismb_driver::check_curdir() ) {
	&dismb_msg::die ("Failed to unlink current directory.");
    }
    if ( ! &dismb_driver::check_continue() ) {
	&dismb_msg::die ("Failed to prepare dismb tmpdir") ;
    }
    if ( ! &dismb_prefs::init() ) {
	&dismb_msg::die ("Preferences are not initialized.") ;
    }
    if ( ! &dismb_prefs::init_logfile() ) {
	&dismb_msg::die ("Failed to create logfile.") ;
    }
    if ( ! &dismb_driver::flash_preinit_logfile() ) {
	&dismb_msg::die ("Cannot flash preinit logfile.") ;
    }
    if ( ! &dismb_prefs::init_env_vars() ) {
	&dismb_msg::die ("No PDF environment variables.") ;
    }
    &dismb_driver::check_incremental_modelbuild();
    if ( ! &dismb_prefs::init_passes() ) {
	&dismb_msg::die ("No dismb passes. Quitting.") ;
    }

    $tm = &dismb_lib::time_stamp;
    &dismb_print ("\n                     =====  disMB init end \$1=====\n", $tm);

    #pop message group created for dismb initialization
    &dismb_msg::pop_new_message_group;
}


sub dismb_driver::check_curdir {
    my($tmpd) = $ENV{'DISBUILD_TMPDIR'};
    use Cwd; use Cwd 'chdir';
    $ENV{'PWD'} = cwd();

    if ( ! &dismb_options::is_defined('-MBcontinue') && $ENV{'PWD'} =~ /^$tmpd/ ) {
	&dismb_msg::err ("Current Working Directory is under dismb working directory: current wd= \$1 dismb   wd= \$2",$ENV{'PWD'},$tmpd);
	return 0;
    } else {
	&dismb_msg::vrb ("Current wd= \$1",$ENV{'PWD'});
    }
    return 1;
}


sub dismb_driver::check_continue {
    my($tmpd) = $ENV{'DISBUILD_TMPDIR'};
    if ( ! dismb_options::is_defined ('-MBcontinue') ) {
	if ( -d $tmpd ) {
	    my($ret) = eval '&dismb_lib::unlink($tmpd)';
	    if ($@ || !$ret || -d $tmpd) {
		&dismb_msg::fixeval;
		&dismb_msg::err ("Failed to unlink \$1 \$2",  $tmpd, $@);
		return 0;
	    }
	}
	eval '&dismb_lib::mkdir_path ($tmpd)';
	if ( $@ || ! -d $tmpd ) {
	    &dismb_msg::fixeval;
	    &dismb_msg::err ("Failed to recreate dir \$1 \$2",  $tmpd, $@);
	    return 0;
	}
    } else {
	if ( ! -d $tmpd ) {
	    &dismb_msg::err ("Failed to continue dismb; no dir \$1", $tmpd);
	    return 0;
	}
	&dismb_msg::inf ("Continue dismb.");
	&dismb_msg::vrb ("Use existing working dir \$1", $tmpd);
    }
    return 1;
}

sub dismb_driver::flash_preinit_logfile {
    close $dismb_driver::PREINIT;
    $dismb_driver::PREINIT = "";

    my($status) = &dismb_driver::add_logfile("$dismb_driver::preinit_file");

    if ($status == 0) {
	return 1;
    } else {
	return 0;
    }
#    local(*PREINIT);
#    open (PREINIT,"<$dismb_driver::preinit_file")  || &dismb_msg::die("Failed to read preinit log \$1",$dismb_driver::preinit_file); # msg OK
#    for (; <PREINIT> ;) {
#        &dismb_msg::write_to_logs_and_stdout ("$_"); # Review how can msg system use this with out a better "tag"?
#    }
#    close PREINIT;
}

#
# In dismb_driver::add_logfile, &dismb_msg::write_to_logs_and_stdout 
# function is used because in these functions the log files are being
# read by dismb. Since the messages are being encountered the second
# time they should not be sent to the gui, but it is important to send
# to stdout(log file) since dismb uses log file contents to get the
# results from parallelly forked processes. 
#
# return 0 if succeeded
sub dismb_driver::add_logfile {
    my($logf) = @_; my($fatal) = 0; local (*LOGF);
    if (open(LOGF,"<$logf")) {
	my($title) = "\nlogfile: $logf\n\n";
	local($dismb::suppress_monitor) = "yes"; # gets propagated into &dismb_print
	my($monitor) = 0; my($line); my $upto_date = "$ENV{'DISBUILD_TMPDIR'}/$pass.upto_date";
	local (*UPTO, *PRS_ERRORS, *PRS_FAILED, *PRS_FORGIV);
	my $do_summary = 1;

	if ($pass eq "parse") {
	    open (PRS_ERRORS, ">>$prs_errors") || &dismb_msg::err ("Failed to open \$1: \$2", $prs_errors, $!);
	    open (PRS_FAILED, ">>$prs_failed") || &dismb_msg::err ("Failed to open \$1: \$2",$prs_failed,$!);
	    open (PRS_FORGIV, ">>$prs_forgiv") || &dismb_msg::err ("Failed to open \$1: \$2",$prs_forgiv,$!);
	}

	while ($line = <LOGF>) {
	    if ( $line =~ /\*\*\* FATAL:/ ) {
		$fatal++ ;
	    }
	    if ($pass eq "parse") {
		&dismb_parse::add_summary ($line) ;
	    }
	    if (! $monitor && ($line =~ / Summary ===/) ) {
		undef ($dismb::suppress_monitor);
		if (! open (UPTO, ">$upto_date") ) {
		    &dismb_msg::err ("Failed to open summary file \$1: \$2", $upto_date, $!);
		    $do_summary = 0;
		} else {
		    &dismb_msg::vrb ("Open summary file \$1", $upto_date);
		}

		if ($title) {
		    &dismb_print ("\$1", $title);
		    $title = "";
		}
		$monitor = 1;
	    }
	    &dismb_msg::write_to_logs_and_stdout ($line);
	    if (!defined($dismb::suppress_monitor) && $do_summary) {
		print UPTO $line;
	    }
	    if ($monitor && ($line =~ /===================/)) {
		$dismb::suppress_monitor = "yes" ;
	    }
	}
	undef ($dismb::suppress_monitor);
	close LOGF; 
	if ($pass eq "parse") {
	    close PRS_ERRORS; close PRS_FAILED; close PRS_FORGIV; close FRG_ERRORS; close PRS_TMPERR;
	}
    } else {
	&dismb_msg::err ("Failed to read logfile \$1: \$2", $logf, $!);
	$fatal ++;
    }
    return $fatal;
}


# check whether the file exist and is not empty
# returns 0 - OK
#         1 - empty
#        -1 - does not exist
sub dismb_driver::check_result_file {
    local($f,$msg) = @_;
    local($res) = 1;
    if ( ! -f $f ) {
        &dismb_msg::inf ("Missing file \$1",$f);
    } elsif ( -z $f ) {
	if ( $msg) {
           &dismb_msg::inf ("\$1 \$2",$msg,$f);
       } else {
           &dismb_msg::inf ("Empty \$1",$f);
       }
    } else {
	$msg = "";
	$res = 0;
    }
    return $res;
}


# preset: @hostResources $tmp_logdir @logs $pass
sub dismb_driver::parallel_nofork {
    my($item,$host,$cpu,$hostlog,$lock,$cmd,$key,%running);
    for $item (@hostResources) {
	($host, $cpu) = split(':', $item);
	if ( $cpu eq "" ) {
	    $cpu     = 1 ;
	}
	$key     = "$host"."$cpu";
	$hostlog = "$tmp_logdir/${host}_$cpu.log";
	$lock    = "$hostlog.lck";
        $quoted_lock = &dismb_lib::add_quotes($lock);
	&dismb_msg::push_new_message_group("parallel " . $pass . " " . ${host} . ":". ${cpu});
	my($lp_flags) = &dismb_lib::arglist_to_str(&dismb_msg::lp_flags(), "-stdout");
	$cmd     = "disperl \"$ENV{'DISMBDIR'}/dismb_remote.pl\" $host $cpu $pass NULL NULL $quoted_lock $lp_flags";

	local(*LOCK);
	my($opened) = open (LOCK, ">$lock");
	if (not $opened) {
	    &dismb_msg::pop_new_message_group();
	    &dismb_msg::die("failed to create lock \$1: \$2",$lock,$!);
	}
	close LOCK; # created the lock file

	&dismb_msg::inf ("Parallel \$1 \$2:\$3 cmd: \$4 log: \$5",$pass,${host},${cpu},$cmd,$hostlog);
	$opened = open ("FOO"."$key", "$cmd|");
	if (not $opened) {
	    &dismb_msg::pop_new_message_group();
	    &dismb_msg::die("Failed to start \$1: \$2",$cmd,$!);
	}
	$running{$key} = "$lock";
	&dismb_msg::pop_new_message_group();
    }

    my($elapsed) = 0;
    my($sleep)   = 30;

    sleep $sleep;		# give the guys a chance to create a log

    WAIT: while (1) {
	my($rest) = 0;
	for $item (@hostResources) {
	    ($host, $cpu) = split(':', $item);
	    if ( $cpu eq "" ) {
		$cpu = 1 ;
	    }
	    $key = "$host"."$cpu";
	    $hostlog = "$tmp_logdir/${host}_$cpu.log";
	  
	    if ( defined ($running{$key}) ) {
		$lock = $running{$key};
		if ( -f $hostlog && -f $lock ) { 
		    $rest++ ;
		} else {
		    if ( -f $hostlog ) {
			&dismb_msg::inf ("Done  \$1 \$2:\$3 ; log \$4",$pass,${host},${cpu},$hostlog);
			$logs[$#logs + 1] = $hostlog;
		    } else {
			&dismb_msg::err ("Failed  \$1 \$2:\$3 ; NO LOG \$4",$pass,${host},${cpu},$hostlog);
			unlink ($lock);
		    }
		    undef ($running{$key});
		    close ("FOO"."$key");
		}
	    }
	}
	if ( ! $rest ) {
	    last WAIT ;
	}
	$elapsed += $sleep;
	sleep $sleep;
	next WAIT;
    }
    &dismb_msg::inf ("All \$1 processes are complete.", $pass);
}


sub dismb_driver::reset_heart_beat {
    if ($dismb_driver::beat) {
	alarm 0;
	$SIG{'ALRM'} = 'DEFAULT';
	undef $dismb_driver::beat;
	undef %dismb_driver::remote_pids;
	&dismb_msg::vrb("Cancelled heart beat");
    }
}


sub dismb_driver::alarm_handler {
    my($tm) = &dismb_lib::time_stamp();
    
    if (-f "$ENV{'DISBUILD_TMPDIR'}/heart_beat") {
	my ($cur,@tokill); my @keys =  keys(%dismb_driver::remote_pids);
	if ($#keys >= 0) {
	    #Oops! Some of the guys hung up
	    &dismb_msg::err("heartbeat: killing following hung child processes:");
	    foreach $cur (@keys) {
		push @tokill, $cur;
		my($host, $cpu) = split(':', $dismb_driver::remote_pids{$cur});
		&dismb_print("       pid= \$1, host= \$2, cpu= \$3",$cur,$host,$cpu);
	    }
	    kill 'KILL', @tokill;
	} else {
	    &dismb_msg::err("Caught alarm \$1 Heart beat file exists: \$2 No processes to wait for!",$tm,$ENV{'DISBUILD_TMPDIR'}/heart_beat);
	}
	&dismb_driver::reset_heart_beat();
    } else {
	&dismb_msg::vrb("Caught heart beat: \$1", $tm);
	&dismb_driver::next_beat();	
    }
}


sub dismb_driver::next_beat {
    my $f = "$ENV{'DISBUILD_TMPDIR'}/heart_beat"; local *HHH;
    if (!open(HHH,">$f")) {
	&dismb_msg::err("Skip heart beat, failed to create file: \$1 file name  =\'\$2\' heart beat= \$3 seconds",$!,$f,$dismb_driver::beat);
	&dismb_driver::reset_heart_beat();
	return;
    }
    close HHH;
    &dismb_msg::vrb("\'alarm \$1 seconds\'",$dismb_driver::beat);
    $SIG{ALRM} = 'dismb_driver::alarm_handler';
    alarm $dismb_driver::beat;
}


sub dismb_driver::heart_beat {
    my $min = &dismb_prefs::get("MBheartBeat");
    if ($min eq "no" || (0 + $min) <= 0 || ! $min) {
	&dismb_msg::vrb("Skip heart beat because of Preference MBheartBeat=\'\$1\'",$min);
	return;
    } elsif ($dismb::is_NT) {
        &dismb_msg::vrb("Skip heart beat on Windows NT");
        return;
    }
    $dismb_driver::beat = $min*60;
    &dismb_msg::vrb("Setting heart beat alarm to \$1 seconds",$dismb_driver::beat);
    &dismb_driver::next_beat();
}


# preset: @hostResources $tmp_logdir @logs $pass
sub dismb_driver::parallel_fork {
    local ($fenv)  = &dismb_prefs::remote_env;
    local ($feval) = &dismb_prefs::remote_eval;

    local($item, %pids, $pids_sz); undef %dismb_driver::remote_pids;

    for $item (@hostResources) {
	local ($host, $cpu) = split(':', $item);
	if ( $cpu eq "" ) {
	    $cpu = 1 ;
	}
	local($hostlog) = "$tmp_logdir/${host}_$cpu.log";
	&dismb_msg::push_new_message_group("parallel " . $pass . " " . ${host} . ":". ${cpu});
	&dismb_msg::inf ("Parallel \$1 \$2:\$3 ; log \$4",$pass,${host},${cpu},$hostlog);
	local($pid) = &dismb_lib::do_fork(qq/$host/, $cpu);

	$pids{$pid} = "${host}:$cpu"; $pids_sz++;
	if ($host ne &dismb_lib::get_local_host()) {
	    $dismb_driver::remote_pids{$pid} = "${host}:$cpu" ;
	}
	&dismb_msg::pop_new_message_group();
    }
    if (defined %dismb_driver::remote_pids) {
	&dismb_driver::heart_beat() ;
    }

    my($done_cnt) = 0;

    WAIT: {
	&dismb_msg::vrb ("Waiting for PID ...");
	my($cur) = wait;
	&dismb_msg::vrb ("Got PID =\$1",$cur);

	if ($cur > -1) {
	    if ( ! defined $pids{$cur} ) {
		&dismb_msg::vrb ("Not forked pid= \$1. Skip",$cur);
	    } else {
		local($host, $cpu) = split(':', $pids{$cur});
		undef $dismb_driver::remote_pids{$cur};
		my $sz = 0; my $k;
		foreach $k (keys(%dismb_driver::remote_pids)) {
		    if (defined $dismb_driver::remote_pids{$k}) {
			$sz++ ;
		    }
		}
		if (! $sz) {
		    &dismb_driver::reset_heart_beat() ;
		}
		local($hostlog)  = "$tmp_logdir/${host}_$cpu.log";
		&dismb_msg::inf ("Done \$1 \$2:\$3 ; log \$4",$pass,${host},${cpu},$hostlog);
		$logs[$#logs + 1] = $hostlog;
		$done_cnt++;
	    }
	}

	if ($cur > -1 && $done_cnt < $pids_sz) {
	    redo WAIT ;
	}
 
	&dismb_msg::vrb ("last pid= \$1; done_cnt= \$2; pids_sz= \$3",$cur,$done_cnt,$pids_sz);

	&dismb_msg::inf ("All \$1 processes are done.", $pass);
	&dismb_driver::reset_heart_beat();
	last WAIT;
    }
}


# returns 0 - OK
#         1 - STOP the next pass (zero or No results)
#
sub dismb_driver::check_after_pass {
    if ( &dismb_options::get_pref('MBstopAfter') eq "$pass" ) {
	&dismb_msg::inf ("Command line option MBstopAfter= \$1",$pass);
	return 1;
    }
    local($msg) = ""; local($stop) = 0;
    require ("dismb_parse.pl");
    if ( $pass eq "analysis" ) {
        if ($dismb::standalone_mode) {
	    if (&dismb_driver::check_result_file ($lst_file, "No outdated files in") != 0) {
	        $stop++ ;
	    }
	    if (&dismb_driver::check_result_file ($prs_file, "") != 0) {
	        $stop++ ;
	    }
	    if (&dismb_driver::check_result_file ($nif_file, "") != 0) {
	        $stop++ ;
	    }
        } else {
            if (&dismb_driver::check_result_file ($prs_file, "No files to be parsed") != 0 ) {
                if (!$dismb::incremental_modelbuild) {
                    $stop++;
                }
                elsif (&dismb_driver::check_result_file ($dismb_analysis::remove_file, "No files to be removed") != 0) {
                    &dismb_msg::msg("No new/outdated files detected.");
                    &dismb_analysis::write_summary_info();
                    $stop++;
                }
            }
        }
    } elsif ( $pass eq "parse" ) {
        if (&dismb_driver::check_result_file ($nif_parsed,   "No IFFs after parsing") != 0) {
	    $stop++ ;
	}
	if (&dismb_driver::check_result_file ($parsed_projects, "No parsed projects") != 0) {
	    $stop++ ;
	}
    } elsif ( $pass eq "update" ) {
        if ($dismb::standalone_mode) {
            if (&dismb_driver::check_result_file ($nif_updated,   "No PSETs after update") != 0) {
	        $stop++ ;
	    }
        }
	elsif (&dismb_driver::check_result_file ($nif_updated, "") != 0 && 
              &dismb_driver::check_result_file ($dismb_analysis::remove_file, "") != 0) {

            &dismb_msg::err("No files updated/removed.");
            $stop++ ;
	}
	if (&dismb_driver::check_result_file ($updated_projects, "No updated projects") != 0) {
	    $stop++ ;
	}
    }
    return $stop;
}

sub dismb_driver::skip_parse_pass
{
  my($pass) = $_[0];
  my($skip) = 0;
  if (!$dismb::standalone_mode && $pass eq "parse") {
    if ( !-f $prs_file && -f $dismb_analysis::remove_file ) {
      $skip = 1;
    } 
  }
  return $skip;
}

sub dismb_driver::run_dismb {
    &dismb_msg::print_header ("disMB");

    local($pass,$item);

  LOOP:
    foreach $pass (@dismb_prefs::passes) {
        if ( &dismb_driver::skip_parse_pass($pass) ) {
           &dismb_msg::msg("Skipping parse pass.");
           next;
        }

        #create new group for each pass of dismb
        &dismb_msg::push_new_message_group("$pass pass");

        #create new message group for initialization phase of each pass
        &dismb_msg::push_new_message_group("disMB preparations for $pass pass");

	$ENV{'DISBUILD'} = $pass;
	local($title) = $pass;
	$title =~ tr/[a-z]/[A-Z]/;
	&dismb_msg::print_header ($title);

	local($tmp_logdir) = "$ENV{'DISBUILD_TMPDIR'}/mblog_$pass";
	local($cleanup)    = (&dismb_prefs::is_continue_mb) ? "" : "cleanup";
	&dismb_lib::create_dir ($tmp_logdir, $cleanup);

	&dismb_lib::init_one_pass; # $pass is predefined

	local(@logs,@hostResources);

        if ( ! &dismb_prefs::init_hostResources($pass) ) {
	    &dismb_msg::die ("Wrong host resources. Quitting.");
        }

        #pop message group created for each initialization phase of each pass
        &dismb_msg::pop_new_message_group;

	if ( &dismb_prefs::is_parallel_mb ) {
	    if ( &dismb_prefs::is_fork_supported ) {
                &dismb_msg::inf ("Fork is supported, using fork for parallel build.");
		&dismb_driver::parallel_fork;
	    } else {
                &dismb_msg::inf ("Fork is not supported.");
		&dismb_driver::parallel_nofork;
	    }

	    local($fatal_logs);
	    foreach $hostlog (@logs) {
                if (&dismb_driver::add_logfile ($hostlog)) {
		    $fatal_logs = "$fatal_logs $hostlog";
                }
	    }
            if ($fatal_logs) {
	        &dismb_msg::die ("Failed logs: \$1",$fatal_logs);
            }
	} else {
	    local($host) = &dismb_lib::get_local_host();
	    local($i)    = 1;
	    local($hostlog) = "$tmp_logdir/${host}_$i.log";
	    &dismb_msg::inf ("Start \$1 \$2:\$3 ; log \$4",$pass,${host},${i},$hostlog);

	    eval '&dismb_local::run_pass ($host, $i)';
	    if (&dismb_driver::add_logfile ($hostlog) || $@) {
		&dismb_msg::die ("Failed \$1 \$2:\$3 ; log \$4",$pass,$host,$i,$hostlog) ;
	    }
	}

	eval '&dismb_lib::finish_one_pass'; # $pass is predefined
	&dismb_msg::fixeval;
	&dismb_msg::die("Finishing pass \$1: \$2", $pass, $@) if $@;

	undef $dismb_msg::host; undef $dismb_msg::cpu;
	my($unlink_res) = eval '&dismb_lib::unlink ($tmp_logdir)';
	if (!$unlink_res || $@) {
	    &dismb_msg::fixeval;
	    &dismb_msg::err ("Unlinking \$1 msg: \$2",$tmp_logdir,$@) ;
	}
	&dismb_msg::print_footer ("$title");
	local($upto_date) = "$ENV{'DISBUILD_TMPDIR'}/$pass.upto_date";
	if (! -f $upto_date && open (UPTODATE, ">$upto_date")) {
	    close UPTODATE ; # touch uptodate file
	}
	if (&dismb_driver::check_after_pass) {
	    &dismb_print ("\nSTOP dismb.");
	    last LOOP;
	}
        #pop message group created for each pass of dismb
        &dismb_msg::pop_new_message_group;

        #report progress about finishing of pass
        &dismb_msg::report_progress("$pass pass");
    }
}


sub dismb_driver::main {

    &dismb_driver::pre_init;

    $INC[$#INC] = ( defined $ENV{'DISMBDIR'} ) ? $ENV{'DISMBDIR'} : "$ENV{'PSETHOME'}/mb";

    require ("dismb_lib.pl");
    require ("dismb_prefs.pl");
    require ("dismb_msg.pl");
    require ("dismb_local.pl");
    require ("dismb_options.pl");
    require ("dismb_analysis.pl");

    #initialize message group from -message_group option
    &dismb_msg::init_message_group_from_option;

    eval '&dismb_driver::init()';

    if ( $@ ) {
	&dismb_msg::fixeval;
        &dismb_msg::msg ("Failure intializing driver: \$1", $@); # Review
	if ( $dismb_driver::PREINIT ) {
	    close $dismb_driver::PREINIT ;
	}
	$dismb_driver::PREINIT = "";

	local (*FFF);
	if (open (FFF, "<$dismb_driver::preinit_file")) {
	    while ( <FFF> ) { 
		print;
	    }
	} else {
	    &dismb_msg::die("Failed to open temporary init file \$1: \$2",$dismb_driver::preinit_file,$!);
	}

	close FFF; 
	&dismb_msg::die ("");
    }

    eval '&dismb_driver::run_dismb()';
    &dismb_print ("\$1",$dismb_prefs::logfile_msg);

    if ( $dismb_driver::preinit_file ) {
	unlink $dismb_driver::preinit_file ;
    }

    # Shut down connection to lpservice. Do this at the end of main, after all
    # messages have been sent.  
    &dismb_msg::shutdown;
}


eval '&dismb_driver::main()';
if ( $@ ) {
    die ("$@") ;
}

exit 0;
