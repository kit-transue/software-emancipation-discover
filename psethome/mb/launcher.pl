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
$ENV{'DIS_LAUNCHER'} = yes;

use File::Copy;
use File::Path;

$dismb_driver::PREINIT = "";
select STDOUT; $| = 1;

sub dislauncher::pre_init {
    $dismb::is_NT = ($ENV{'OS'} eq "Windows_NT") ? 1 : 0;
    my($tmp_dir)         = ($dislauncher::is_NT) ? "c:/temp" : "/usr/tmp";
    $ENV{'TMPDIR'}       = "$tmp_dir" if (! defined $ENV{'TMPDIR'});

    &dismb_msg::die ("TMPDIR=\$1 is not a directory",$ENV{'TMPDIR'})     if ( ! -d $ENV{'TMPDIR'} );
    &dismb_msg::die ("Environment variable PSETHOME is not set")     if ( ! defined $ENV{'PSETHOME'} );
    &dismb_msg::die ("PSETHOME=\$1 is not a directory",$ENV{'PSETHOME'}) if ( ! -d $ENV{'PSETHOME'} );

    $ENV{'DISMBDIR'} = "$ENV{'PSETHOME'}/mb"             if ( ! defined $ENV{'DISMBDIR'} );
    &dismb_msg::die ("DISMBDIR=\$1 is not a directory",$ENV{'DISMBDIR'}) if ( ! -d $ENV{'DISMBDIR'} );

    $dislauncher::preinit_file = "$ENV{'TMPDIR'}/dislauncher_init_$$.log";
    open (PREINIT_LOGFILE, ">$dislauncher::preinit_file") || &dismb_msg::die ("Failed to create temporary init file  \$1",$dislauncher::preinit_file);
    $dismb_driver::PREINIT = PREINIT_LOGFILE;

    ($psethome_bin = "$ENV{'PSETHOME'}/bin") =~ s%\\%/%g;
}


#returns 1 on success
sub dislauncher::init_logfile {
    my($txt, $nm);
    $dislauncher::logfile = "$ENV{'TMPDIR'}/dislauncher_$$.log";
    return 0 if ( &dismb_lib::create_dir_for_file ($dislauncher::logfile) );


    $txt = "Temporary dislauncher log: $dislauncher::logfile";
    &dismb_msg::inf ("Temporary dislauncher log: \$1",$dislauncher::logfile);
    $dismb_prefs::logfile_msg = $txt;

    if ( ! open (LOGFILE,">$dislauncher::logfile") ) {
	&dismb_msg::err ("Could not create logfile \$1: \$2",$dislauncher::logfile,$!);
	return 0;
    }

    select LOGFILE ; $| = 1; select STDOUT;

    $dismb_prefs::logfile = LOGFILE;
    if ( ! &dislauncher::flash_preinit_logfile() ) {
	&dismb_msg::err ("Cannot flash preinit logfile.");
	return 0;
    }
    return 1;
}


sub dislauncher::flash_preinit_logfile {
    close $dismb_driver::PREINIT;
    $dismb_driver::PREINIT = "";
    local(*PREINIT);
    open (PREINIT,"<$dislauncher::preinit_file")  || &dismb_msg::die ("Failed to read preinit log \$1 ",$dislauncher::preinit_file);
    
    my ($ln); $dismb::force_monitor = 1;
    while ($ln = <PREINIT>) {
	&dismb_msg::write_to_logs_and_stdout($ln);
    }
    close PREINIT;
}


sub dislauncher::model_subdir {
    my $subdir = "model";
    $subdir = &dismb_options::get('-modelSubDir') if (&dismb_options::is_defined('-modelSubDir'));
    return $subdir;
}


#returns: 0 on failure 
sub dislauncher::prepare_model {
    my $work   = $ENV{'ADMINDIR'};
    my $subdir = &dislauncher::model_subdir();
    return 0 if !$subdir;

    if ( ! -d "$work/$subdir" ) {
	&dismb_msg::inf("There is no existing model directory \ $1", $work/$subdir);
	return 1;
    }

    if ( &dismb_options::is_defined('-incremental') ) {
	&dismb_msg::inf ("Incremental build, keep existing model directory \$1", $work/$subdir);
	return 1;
    }

    &dismb_msg::inf ("Clean build, delete existing model directory \$1",  $work/$subdir);
    my $ret = eval ('&dismb_lib::unlink ("$work/$subdir")');
    &dismb_msg::fixeval;
    if ($@ || !$ret) {
	&dismb_msg::err ("Failed to unlink existing model \$1 \$2", $work/$subdir, $@);
	return 0;
    }
    
    return 1;
}


#returns: 0 on failure
#copies:   AdminDir: master into work
sub dislauncher::copy_admindir {
    my ($mdir, $wdir) = ($dislauncher::master, $ENV{'ADMINDIR'});
    my $subdir = &dislauncher::model_subdir();
    return 0 if !$subdir;
    if (-d $wdir) {
	&dismb_msg::inf ("Remove existing work area: \$1", $wdir);
	rmtree($wdir, 0, 0); rmdir $wdir;
#	eval ('&dismb_lib::unlink ($dir)');
	if ( -d $wdir || $@ ) {
	    &dismb_msg::err ("Failed to remove existing work AdminDir \$1 \$2", $wdir, $@);
	    return 0;
	}
    }
    
    mkpath ("$wdir", 0, 0777);
    if (! -d $wdir ) {
	&dismb_msg::err ("Failed to create work AdminDir \$1", $wdir);
	return 0;
    }

    &dismb_msg::inf ("Install master AdminDir into work area: master: \$1 work:   \$2",$mdir,$wdir);
    local(*MDIR);
    if ( ! opendir (MDIR,$mdir) ) {
	&dismb_msg::err ("Failed to open AdminDir directory \$1: \$2", $mdir, $!);
	return 0;
    }
    my(@all) = readdir (MDIR);
    closedir(DIR); my $copied = 0;
    foreach $item (@all) {
	next if ($item eq "." || $item eq ".." || (-f "$mdir/$item"));
	my $do_copy = 1;
	if ( &dismb_options::is_defined('-build') && $item eq $subdir ) {
	    if ( &dismb_options::is_defined('-incremental') ) {
		&dismb_msg::inf("Incremental build, keep existing model");
	    } else {
		&dismb_msg::inf("Clean build, ignore existing model");
		$do_copy = 0;
	    }
	}
	if ( $do_copy ) {
	    &dismb_print ("      \$1 ...",$item);
	    my $ret = eval '&dismb_lib::xcopy ("$mdir/$item", "$wdir/$item")';
	    &dismb_msg::fixeval;
	    if ($@ || !$ret) {
		&dismb_msg::err ("Failed to install work AdminDir \$1 \$2", $wdir/$item, $@);
		return 0;
	    }
	    $copied++;
	}
    }
    if ($copied == 0) {
	&dismb_msg::err ("Created empty work AdminDir \$1", $wdir);
	return 0;
    }
    1;
}


#returns: 0 on failure 
#    sets $ENV{'ADMINDIR'}
sub dislauncher::init_admindir {
    my ($master, $work); my $adir = $ENV{'ADMINDIR'};
    $master = &dismb_options::get('-masterAdminDir') if (&dismb_options::is_defined('-masterAdminDir'));
    $work   = &dismb_options::get('-workAdminDir')   if (&dismb_options::is_defined('-workAdminDir')); 

    if (!$adir && !$work) {
	&dismb_msg::err("Neither ADMINDIR nor -workAdminDir are specified");
	return 0;
    }
    &dismb_print ("  env: ADMINDIR=\$1",$ENV{'ADMINDIR'}) if (!$master && !$work);
    if ($work) {
	if ($adir) {
	    if ($work eq $adir) {
		&dismb_msg::inf ("Same workAdminDir and ADMINDIR \$1", $work);
	    } else {
		&dismb_msg::inf ("ADMINDIR=\$1 (reset from -workAdminDir)",$work);
		$ENV{'ADMINDIR'} = $work;
	    }
	} else {
	    &dismb_msg::inf ("ADMINDIR=\$1 (from -workAdminDir)",$work);
	    $ENV{'ADMINDIR'} = $work;
	}
    } else {
	&dismb_msg::inf ("Set workAdminDir to ADMINDIR \$1", $adir);
	$work = $adir;
    }
	
    &dismb_msg::inf ("Same workAdminDir and masterAdminDir \$1", $work) if ($master && $master eq $work);
    $dislauncher::master = $master if ($master && $master ne $work);
    return 1;
}

#returns 1 on success; 0 on failure
sub dislauncher::set_lock {
    my $lck = "$ENV{'ADMINDIR'}/dislauncher.lck";
    local (*LCK);
    if ( -f $lck ) {
	if (open (LCK, "<$lck")) {
	    my $txt = <LCK>; close LCK;
	    &dismb_msg::err ("Found existing dislauncher lock \$1 \$2", $lck, $txt);
	} else {
	    &dismb_msg::err ("Failed to open existing dislauncher lock file \$1 \$2", $lck, $!);
	}
	return 0;
    }
    my($sec,$min,$hour,$mday,$mon,$year) = &dismb_lib::time2string() ;
    $dislauncher::time_stamp = "${mon}-${mday}-${year}." . $hour . "_" . $min . "_" . $sec;
    if ( open (LCK, ">$lck") ) {
	print LCK $dislauncher::time_stamp; close LCK;
	&dismb_msg::inf ("Locked work AdminDir \$1 \$2", $lck, $dislauncher::time_stamp);
    } else {
	&dismb_msg::err ("Failed to lock work AdminDir \$1 \$2", $lck, $dislauncher::time_stamp);
	return 0;
    }
    1;
}


sub dislauncher::init {
    local($tm) = &dismb_lib::time_stamp;
    &dismb_print ("\n===== Copy Model init started   \$1 =====\n",$tm);
    &dismb_lib::print_dismb_env ("driver");

    &dismb_msg::die ("Command line options are wrong.")          if ( ! &dismb_options::process() );
    &dismb_msg::die ("ADMINDIR=\$1 is wrong.",$ENV{'ADMINDIR'})      if ( ! &dislauncher::init_admindir() );

    &dismb_msg::die ("Failed to create logfile.")                  if ( ! &dislauncher::init_logfile );

    unlink $dislauncher::preinit_file if $dislauncher::preinit_file;
    $tm = &dismb_lib::time_stamp;
    &dismb_print ("\n===== Copy Model init succeeded \$1 =====\n",$tm);
}


sub dislauncher::run () {
    my @cmd = ("$ENV{'PSETHOME'}/bin/disperl", "$ENV{'DISMBDIR'}/dismb_driver.pl", "-prefs", $dislauncher::add_prefs);
    &dismb_print("\nRunning dismb: \$1 \$2 -prefs \$3",$ENV{'PSETHOME'}/bin/disperl,$ENV{'DISMBDIR'}/dismb_driver.pl,$dislauncher::add_prefs);
    my $res = &dismb_lib::system(@cmd);
    &dismb_msg::die ("Failed Submission Check") if $res;
    return $res;
}


# setup tmp/disbuild directory if Model is not built with dismb
# uses -prefs <file.prefs> option f the Pref file name is not build.prefs
# returns 1 on success;
sub dislauncher::emulate_dismb {
    if ( -f "$ENV{'ADMINDIR'}/tmp/disbuild/all.pmod.lst" &&
	 -f "$ENV{'ADMINDIR'}/tmp/disbuild/new_reuse.flg" ) {
	&dismb_msg::inf ("Model is built with dismb.");
	return 1;
    }
    &dismb_msg::inf ("Model is not built with dismb. Emulating dismb ...");

    my @pref  = ($dis::build_info {'PREFS'})     ? ("-prefs", $dis::build_info {'PREFS'}) : ();
    my @ctool = ($dis::build_info {'CLEARTOOL'}) ? ("-MBcleartool", $dis::build_info {'CLEARTOOL'}) : ();
    my @cmd   = ("$ENV{'PSETHOME'}/bin/dismb", @pref, @ctool, "-MBlogFileCreate", "no", "-MBstopAfter", "analysis", "-MBscratch", "yes");

    undef $ENV{'sharedSrcRoot'}; 
    undef $ENV{'sharedModelRoot'}; 
    undef $ENV{'privateSrcRoot'}; 
    undef $ENV{'privateModelRoot'};
    my $ret = eval '&dismb_lib::cleartool_system_bkgd (0, @cmd)';
    return $ret;
}



#changes ADMINDIR in $ADMINDIR/tmp/disbuild/build_info
#returns 1 on success
sub dislauncher::change_build_info {
    $dismb_lib::build_info = "$ENV{'ADMINDIR'}/tmp/disbuild/build_info";
    my %old_info;
    if ( -f $dismb_lib::build_info ) {
	%old_info = &dismb_lib::read_build_info();
	&dismb_msg::inf ("Changing build info \$1",$dismb_lib::build_info);
	rename $dismb::build_info, "$dismb_lib::build_info.old";
    } else {
	&dismb_msg::inf ("Writing build info \$1", $dismb_lib::build_info);
    }
    local (*BINFO);
    open (BINFO, ">$dismb_lib::build_info") || &dismb_msg::die ("Failed to write \$1: \$2",$dismb_lib::build_info,$!);
    my $host  = ( defined $old_info{'HOST'} ) ? $old_info{'HOST'} : &dismb_lib::get_local_host();
    my $adir  = $ENV{'ADMINDIR'};
    my $view  = $dis::build_info{'VIEW'};
    my $prefs = $dis::build_info{'PREFS'};
    my $ctool = $dis::build_info{'CLEARTOOL'};

    print BINFO "#HOST=$host\n"       if ($host);
    print BINFO "#ADMINDIR=$adir\n"   if ($adir);
    print BINFO "#VIEW=$view\n"       if ($view);
    print BINFO "#CLEARTOOL=$ctool\n" if ($view);
    print BINFO "#PREFS=$prefs\n"     if ($prefs);
    close BINFO;

    return 1;
}


# returns 1 on success; 0 on failure
# init %dis::old_build_info; %dis::build_info
sub dislauncher::init_build_info {
    my $f = ( $dislauncher::master ) ? "$dislauncher::master/tmp/disbuild/build_info" : "$ENV{'ADMINDIR'}/tmp/disbuild/build_info";
    return &dismb_lib::init_build_info ( $f );
}


sub dislauncher::main {
    &dislauncher::pre_init();

    push @INC, ($ENV{'DISMBDIR'});

    require ("dismb_lib.pl");
    require ("dismb_msg.pl");
    require ("dismb_options.pl");

    #initialize message group from -message_group option
    &dismb_msg::init_message_group_from_option;

    eval '&dislauncher::init()';
    if ( $@ ) {
	&dismb_msg::fixeval;
	local($msgid) = &dismb_msg::join ($@);
	local($msg) = &dismb_msg::join (" *** FATAL: ", $@);
	&dismb_print ("\$1",$msg); # Review
	&dislauncher::flash_preinit_logfile();
	&dismb_msg::die ("\$1",$msg); # Review
    }

    &dismb_msg::die ("") if ( ! eval '&dislauncher::init_build_info()' );

    if (!$dislauncher::master || $dislauncher::master eq $ENV{'ADMINDIR'}) {
	if ( &dismb_options::is_defined('-build') ) {
	    &dismb_msg::die ("") if ( ! &dislauncher::prepare_model() );
	}
    } elsif ($dislauncher::master && $dislauncher::master ne $ENV{'ADMINDIR'}) {
	&dismb_msg::die ("") if ( ! &dislauncher::copy_admindir() );
    }
    &dismb_msg::die ("") if ( ! eval '&dislauncher::emulate_dismb()' );
    &dismb_msg::die ("") if ( ! eval '&dislauncher::change_build_info()' );
    
    $tm = &dismb_lib::time_stamp;
    
    &dismb_print ("\n===== Succeeded launch \$1 =====\n",$tm);

    # For compatibility with DISIM 7.4:
    my($DISIM_old) = 0;
    if ("$ENV{DISIM}" ne "") {
	$DISIM_old = (-e "$ENV{DISIM}/bin/admin/IMUtil.pl");
    }
    my($perm_log) = $DISIM_old ? "$ENV{'ADMINDIR'}/dislauncher.log" : "$ENV{'ADMINDIR'}/log/dislauncher.log";
    &dismb_msg::inf ("Permanent dislauncher log: \$1", $perm_log);
    close(LOGFILE);
    unlink("$dislauncher::logfile") if (&dismb_lib::copy ($dislauncher::logfile, $perm_log));

}

eval '&dislauncher::main()';
if ( $@ ) {
    my $log = ( defined $dislauncher::logfile) ? $dislauncher::logfile : $dislauncher::preinit_file;
    dismb_msg::err ("Failed launch dismb\nlog file: \$1", $log); # Review
}

