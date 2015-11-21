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
#static submission check pass percentage
#cache     5
#analysis  5
#parse     40
#update    10
#finalize  5
#dfa_init  5
#dfa       15
#query     15

$dismb_driver::PREINIT = "";
select STDOUT; $| = 1;

sub disch::pre_init {
    $dismb::is_NT = ($ENV{'OS'} eq "Windows_NT") ? 1 : 0;
    my($exe)             = ($dismb::is_NT) ? "bin/model_server.exe" : "bin/model_server";
    my($tmp_dir)         = ($dismb::is_NT) ? "c:/temp" : "/usr/tmp";
    $ENV{'TMPDIR'}       = "$tmp_dir"             if (! defined $ENV{'TMPDIR'});

    &dismb_msg::die("TMPDIR=\$1 is not a directory",$ENV{'TMPDIR'}) if ( ! -d $ENV{'TMPDIR'} );
    &dismb_msg::die("Environment variable PSETHOME is not set")              if ( ! defined $ENV{'PSETHOME'} );
    &dismb_msg::die("PSETHOME=\$1 PSETHOME is not a directory",$ENV{'PSETHOME'}) if ( ! -d $ENV{'PSETHOME'} );
    &dismb_msg::die("Wrong env variable TMPDIR: \$1",$ENV{'TMPDIR'})             if ( ! -d $ENV{'TMPDIR'} );

    $ENV{'DISMBDIR'} = "$ENV{'PSETHOME'}/mb"                      if ( ! defined $ENV{'DISMBDIR'} );
    &dismb_msg::die("Env variable DISMBDIR is not disch exec directory")     if ( ! -f "$ENV{'DISMBDIR'}/disch.pl" );

    $disch::preinit_file = "$ENV{'TMPDIR'}/disch_init_$$.log";
    open (PREINIT_LOGFILE, ">$disch::preinit_file") || &dismb_msg::die("Failed to create temporary init file  \$1",$disch::preinit_file);
    $dismb_driver::PREINIT = PREINIT_LOGFILE;

    ($psethome_bin = "$ENV{'PSETHOME'}/bin") =~ s%\\%/%g;
}


sub disch::init_logfile {
    my($txt, $nm);
    $disch::logfile = "$disch::localRoot/disch.log";

    $txt = "disch log: $disch::logfile";
    &dismb_msg::inf ("disch log: \$1",$disch::logfile);
    $dismb_prefs::logfile_msg = $txt;

    if ( ! open (LOGFILE,">$disch::logfile") ) {
	&dismb_msg::err ("Could not create logfile \$1: \$2", $disch::logfile,$!);
	return 0;
    }

    select LOGFILE ; $| = 1; select STDOUT;

    $dismb_prefs::logfile = LOGFILE;
    if ( ! &disch::flash_preinit_logfile() ) {
	&dismb_msg::err ("Could not flash preinit logfile.");
	return 0;
    }
    return 1;
}


sub disch::flash_preinit_logfile {
    close $dismb_driver::PREINIT;
    $dismb_driver::PREINIT = "";
    local(*PREINIT);
    open (PREINIT,"<$disch::preinit_file")  || &dismb_msg::die("Failed to read preinit log \$1 ",$disch::preinit_file);
    
    my ($ln); $dismb::force_monitor = 1;
    while ($ln = <PREINIT>) {
	&dismb_print ("\$1", $ln);
    }
    close PREINIT;
}



sub disch::init {
    &dismb_msg::die ("Command line options are wrong.")      if ( ! &dismb_options::process() );
    $disch::is_report = &dismb_options::is_defined('-SCreport');
    $disch::mode = $disch::is_report ? "Report" : "Check";

    local($tm) = &dismb_lib::time_stamp;  
    &dismb_print ("\n===== Submission \$1 init started   \$2 =====\n",$disch::mode,$tm);
    &dismb_lib::print_dismb_env ("driver");

    &dismb_msg::die ("File Table is wrong.")                 if ( ! &disch::process_fileTable() );
    &dismb_msg::die ("dismb AdminDir is wrong.")             if ( ! &disch::check_admindir() );
    if ( !$disch::is_report ) {
	&dismb_msg::die ("dismb shared files are not loaded.")   if ( ! &disch::load_shared_files() );
	&dismb_msg::die ("dismb compiler flags are not loaded.") if ( ! &disch::load_shared_flags() );
	&dismb_msg::die ("Local Root directory is wrong.")       if ( ! &disch::create_localRoot () ); 
	&dismb_msg::die ("Can not copy files into local Root.")  if ( ! &disch::copy_sources_into_localRoot () );
	&dismb_msg::die ("Failed to initialize disch admindir.")  if ( !  &disch::prepare_admindir () );
    } else {
	&dismb_msg::die ("Local Root directory is wrong.")       if ( ! &disch::create_localRoot () ); 
    }
    
    &dismb_msg::die ("Failed to create logfile.")              if ( ! &disch::init_logfile );

    unlink $disch::preinit_file if $disch::preinit_file;
    $tm = &dismb_lib::time_stamp;
    &dismb_print ("\n===== Submission \$1 init succeeded \$2 =====\n",$disch::mode,$tm);
}


sub disch::run () {
    #initialize $dis::build_info
    &dismb_lib::init_build_info("$disch::sh_tmpdir/build_info");

    my @cmd = ("$ENV{'PSETHOME'}/bin/disperl", "$ENV{'DISMBDIR'}/dismb_driver.pl", "-MBgeneratePdf", "no", "-MBincludeAllHdrs", "no", "-prefs", $disch::add_prefs);
    my $res = eval '&dismb_lib::cleartool_system_bkgd (0, @cmd)';

    &dismb_msg::die ("Submission Check failed") if !$res;
    return $res;
}


sub disch::init_query_output {
    $disch::sch_out = "$ENV{'ADMINDIR'}/tmp/disch_query.out";
    $disch::res_out  = "$disch::localRoot/submission_check.out";

    $disch::prs_err = "$ENV{'ADMINDIR'}/tmp/disbuild/prs_errors.log";
    $disch::res_err = "$disch::localRoot/submission_check.err";

    $disch::prs_not = "$ENV{'ADMINDIR'}/tmp/disbuild/prs_missed.lst";
    $disch::res_not = "$disch::localRoot/submission_check.not";

    $disch::bld_log = "$ENV{'ADMINDIR'}/tmp/disbuild/build.log";
    $disch::res_log = "$disch::localRoot/submission_check.log";

    eval '&dismb_lib::unlink ("$disch::prs_err")';
    &dismb_msg::fixeval;
    &dismb_msg::wrn ("Failed to delete \$1: \$2", ${disch::prs_err},$@) if ($@) ;

    eval '&dismb_lib::unlink ("$disch::prs_not")';
    &dismb_msg::fixeval;
    &dismb_msg::wrn ("Failed to delete \$1: \$2",${disch::prs_not},$@) if ($@) ;

    eval '&dismb_lib::unlink ("$disch::bld_log")';
    &dismb_msg::fixeval;
    &dismb_msg::wrn ("Failed to delete \$1: \$2",${disch::bld_log},$@) if ($@) ;

    if ( ! -f $disch::sch_out ) {
	&dismb_msg::inf ("Query output: \$1",$disch::sch_out);
    } else {
	eval '&dismb_lib::unlink ("$disch::sch_out")';
	&dismb_msg::fixeval;
	&dismb_msg::die ("Failed to delete existing query output file: \$1 \$2",$@,$disch::sch_out) if ($@);
	&dismb_msg::inf ("Removed existing query output file: \$1", $disch::sch_out);
    }
    if ( ! -f $disch::res_out ) {
	&dismb_msg::inf ("Query result: \$1",$disch::res_out);
    } else {
	eval '&dismb_lib::unlink ("$disch::res_out")';
	&dismb_msg::fixeval;
	&dismb_msg::die ("Failed to delete existing query result file: \$1 \$2",$@,$disch::res_out) if ($@);
	&dismb_msg::inf ("Removed existing query result file: \$1", $disch::res_out);
    }
}


sub disch::check_query_output {
    if ( -f $disch::sch_out ) {
	&dismb_print("Query output:\$1", $disch::sch_out);
	if (!&dismb_lib::copy ($disch::sch_out, "$disch::res_out")) {
	    &dismb_msg::err ("Failed to create query result \$1",$disch::res_out);
	} else {
            &dismb_print("Query result:\$1", $disch::res_out);
	}
    } else {
	&dismb_msg::err ("Failed to create query output \$1", $disch::sch_out);
    }

    if (!&dismb_lib::copy ($disch::prs_err, $disch::res_err)) {
	&dismb_msg::err ("Failed to create parse errors log \$1", $disch::res_err);
    } else {
        &dismb_print("Parse errors:\$1", $disch::res_err);
    }

    if (!&dismb_lib::copy ($disch::prs_not, $disch::res_not)) {
	&dismb_msg::err ("Failed to create list of not built files \$1", $disch::res_not);
    } else {
        &dismb_print("Failed files:\$1", $disch::res_not);
    }

    if (!&dismb_lib::copy ("$disch::logfile", "$disch::res_log")) {
	&dismb_msg::err ("Failed to create result log file \$1", $disch::res_log);
    } else {
	local (*RES_LOG,*BLD_LOG);
	if ( open (RES_LOG, ">>$disch::res_log") && open (BLD_LOG, "<$disch::bld_log") ) {
	    my $ll;
	    while ($ll = <BLD_LOG>) {
		print RES_LOG $ll;
	    }
            &dismb_print("Complete log:\$1", $disch::res_log);
	    print RES_LOG " INF: Created log file: $disch::res_log\n";
	} else {
	    &dismb_msg::err ("Failed to add dismb log file to result log file: \$1 \$2", $disch::bld_log, $disch::res_log);
	}
	close RES_LOG; close BLD_LOG;
    }
}

sub disch::main {
    &disch::pre_init();

    $INC[$#INC] = $ENV{'DISMBDIR'};

    require ("dismb_lib.pl");
    require ("dismb_msg.pl");
    require ("dismb_options.pl");
    require ("sch_utils.pl");

    eval '&disch::init()';
    if ( $@ ) {
	&dismb_msg::fixeval;
	&dismb_print (" *** FATAL: \$1", $@); # Review
	&disch::flash_preinit_logfile();
	&dismb_msg::die ("");
    }

    if ( !$disch::is_report ) {
	eval '&disch::init_query_output()';
	&dismb_msg::die ("") if ( $@ );
	
	eval '&disch::run()';
	&dismb_msg::die ("") if $@;

	eval '&disch::check_query_output()';
	&dismb_msg::die ("") if ( $@ );
    } else {
	eval '&disch::record_submission()';
	&dismb_msg::die ("") if ( $@ );
    }
}

eval '&disch::main()';


