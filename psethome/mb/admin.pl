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
# setadmin install [-view <view>] [-config <file>]
$disadmin::command = shift;
&dismb_msg::die("  *** FATAL: Wrong command: \$1",$disadmin::command) unless &disadmin::check_command ();

$ENV{'DIS_ADMIN'} = "disadmin";
select STDOUT; $| = 1;

sub disadmin::check_command {
    if ( ! $disadmin::command ) {
	&dismb_msg::die("  *** FATAL: There is no command specified.");
	return 0;
    }
    return 1 if ($disadmin::command eq "install");
    return 1 if ($disadmin::command eq "uninstall");
    return 1 if ($disadmin::command eq "list");
    return 0;
}


sub disadmin::list_services {
    &dismb_lib::init_services("msg");

    my $old_host = "";
    foreach $el (sort(values(%dismb::services))) {
	my ($host,$name) = split ('@',$el);
	if ($host ne $old_host) {
	    &dismb_print("\$1", $host); # Review  "$host\n"
	    $old_host = $host;
	}
	&dismb_print("\t$1",$name); # Review
    }
    exit;
}


sub disadmin::pre_init {
    $dismb::is_NT   = ($ENV{'OS'} eq "Windows_NT") ? 1 : 0;
    $dismb::devnull = ($dismb::is_NT) ? "nul" : "/dev/null";
    my($tmp_dir)    = ($disadmin::is_NT) ? "c:/temp" : "/usr/tmp";
    $ENV{'TMPDIR'}  = "$tmp_dir" if (! defined $ENV{'TMPDIR'});

    &dismb_msg::die("TMPDIR=\$1 is not a directory",$ENV{'TMPDIR'})     if ( ! -d $ENV{'TMPDIR'} );
    &dismb_msg::die("Environment variable PSETHOME is not set")     if ( ! defined $ENV{'PSETHOME'} );
    &dismb_msg::die("PSETHOME=\$1 is not a directory",$ENV{'PSETHOME'}) if ( ! -d $ENV{'PSETHOME'} );

    $ENV{'DISMBDIR'} = "$ENV{'PSETHOME'}/mb"             if ( ! defined $ENV{'DISMBDIR'} );
    &dismb_msg::die("DISMBDIR=\$1 is not a directory",$ENV{'DISMBDIR'}) if ( ! -d $ENV{'DISMBDIR'} );

    if ($disadmin::command ne "list") {
	&dismb_msg::die("Environment variable DIS_DEPLOYMENT is not defined") if ( ! defined $ENV{'DIS_DEPLOYMENT'} );
	&dismb_msg::die("DIS_DEPLOYMENT=\$1 is not a directory",$ENV{'DIS_DEPLOYMENT'}) if ( ! -d $ENV{'DIS_DEPLOYMENT'} );
    }

    ($psethome_bin = "$ENV{'PSETHOME'}/bin") =~ s%\\%/%g;
}


sub disadmin::configure {
    my $cfile;
    if (&dismb_options::is_defined ('-configure')) {
	$cfile = &dismb_options::get('-configure');
	&dismb_msg::die("Missing specified config file: \$1",$cfile) if (! -f $cfile);
	&dismb_msg::inf ("Config file specified by option '-configure' \$1", $cfile);
    } else {
	# The search rule under $DIS_DEPLOYMENT:
	# configure[.<tag>][.<os>] ==> configure[.pmod_server][.nt]
	my $os    = ($dismb::is_NT) ? ".nt" : ".unix";
	my $tag   = (&dismb_options::is_defined ('-tag')) ? "." . &dismb_options::get ('-tag') : "";

	$cfile = "$ENV{'DIS_DEPLOYMENT'}/configure${tag}${os}";
	unless (-f $cfile) {
	    $cfile = "$ENV{'DIS_DEPLOYMENT'}/configure${tag}";
	    unless (-f $cfile) {
		$cfile = "$ENV{'DIS_DEPLOYMENT'}/configure${os}";
		unless (-f $cfile) {
		    $cfile = "$ENV{'DIS_DEPLOYMENT'}/configure";
		    unless (-f $cfile) {
			&dismb_msg::die("Missing constructed config file: \$1",$cfile);
		    }
		}
	    }
	}
	&dismb_msg::inf ("Config file found under \$DIS_DEPLOYMENT \$1", $cfile);
    }

    local (*CF);
    open (CF, "<$cfile") || &dismb_msg::die ("Failed to open config file: \$1 \$2",$!,$cfile);

    my ($line);
    while ($line = <CF>) {
	next unless ( $line =~ /^\s*\"?(\w+)\"?\s*=\s*\"?(\S*)\"?\s*$/ );
	my ($key, $val) = ($1, $2);
	if ( defined $disadmin::config_info {$key} ) {
	    &dismb_msg::wrn ("More than one value for the key \$1 first   value= \$2 ignored value= \$3",$key,$disadmin::config_info{$key},$val);
	    next;
	}
	my $expanded = eval '&dismb_lib::expand_env_vars ( "$val" )';
	$expanded = "" if ( $@ || $val eq $expanded );

	&dismb_print( "\$1 \$2 \$3",$key,$val,$expanded) if ($expanded); # Review
	&dismb_print( "\$1 \$2",$key,$val)           if (! $expanded); # Review
	(my $key_down = $key) =~  tr /A-Z/a-z/;
	if ( &dismb_options::is_defined ("-$key") || &dismb_options::is_defined ("-$key_down") ) {
	    $expanded = ""; 
	    my $opt = (&dismb_options::is_defined ("-$key")) ? "-$key" : "-$key_down";
	    $val = &dismb_options::get ($opt);
	    &dismb_print("\$1 option \$2 \$3","+++ hidden +++",$opt,$val); # Review
	}
	$disadmin::config_info {$key} = ($expanded) ? $expanded : $val;
    }
}


sub disadmin::check_no_view {
    my $ct = ( $disadmin::config_info{'CM'} eq 'clearcase' ) ? 1 : 0;
    $disadmin::cleartool = ( defined $disadmin::config_info{'CLEARTOOL'} ) ? $disadmin::config_info{'CLEARTOOL'} : 'cleartool';
    $disadmin::wv = &dismb_lib::backtick($disadmin::cleartool, "pwv", "-short"); $disadmin::wv =~ s/\s*$//;
    if ( !$disadmin::wv ) {
	&dismb_msg::die ("Cannot execute cleartool command \'\$1\'",$disadmin::cleartool) if $ct;
        undef $disadmin::cleartool;
    } elsif ( $disadmin::wv eq "** NONE **" ) {
	undef $disadmin::wv;
	&dismb_msg::inf ("No working clearcase view") if $ct;
    } else {
	&dismb_msg::wrn ("disadmin started from within Clearcase view \'\$1\', SET recommends to run disadmin outside of Clearcase.",$disadmin::wv);
    }
}


sub disadmin::get_label {
    my $v = $disadmin::config_info{'VERSION'};
    &dismb_msg::die ("No version is specified") if (!$v);

    $disadmin::view = $disadmin::config_info{'VIEWNAME_PREFIX'} . $v;
    $disadmin::adir = $disadmin::config_info{'ADMINDIR_PREFIX'} . $v;
    $disadmin::serv = ($disadmin::config_info{'SERVICE_PREFIX'}) ? $disadmin::config_info{'SERVICE_PREFIX'}  . $v : $disadmin::view;
    
    $disadmin::adir_src = $disadmin::config_info{'ROOTDIR_BUILD'}   . "/$disadmin::adir";
    $disadmin::adir_trg = $disadmin::config_info{'ROOTDIR_INSTALL'} . "/$disadmin::adir";

    if ($disadmin::uninstall) {
	&dismb_msg::inf ("Version: \$1 Service: \$2 AdminDir: \$3",$v,$disadmin::serv,$disadmin::adir_trg);
    } else {
	&dismb_msg::inf ("Model version: \$1 AdminDir  src: \$2 AdminDir  trg: \$3",$v,$disadmin::adir_src,$disadmin::adir_trg);
	&dismb_msg::die ("No AdminDir directory: \$1",$disadmin::adir_src) if ( ! -d $disadmin::adir_src);
    }
}


#dies on error. Takes viewtag and config spec file
sub disadmin::cleartool_catcs {
    my ($tag,$f) = @_;
    local(*SPEC,*OUTF);
    my @cmd = ($disadmin::cleartool, "catcs", "-tag", $tag); 

    &dismb_lib::open_input (\*SPEC, @cmd)
	or &dismb_msg::die ("Failed to execute \'\$1\':  \$2",&dismb_lib::arglist_to_str(@cmd),$!);
    open (OUTF,">$f")     or &dismb_msg::die ("Failed to write clearcase config spec \$1: \$2",$f,$!);
    my $line;
    while ($line = <SPEC>) {
	next if ( $line =~ /^\#/ ); # ignore comments
	print OUTF $line;
    }
    close SPEC; close OUTF;

}


#dies on error. Takes viewtag and config spec file
sub disadmin::cleartool_setcs {
    my ($tag,$f) = @_;
    my @cmd = ("$disadmin::cleartool", "setcs", "-tag", $tag, $f); 

    &dismb_lib::system(@cmd);
    &dismb_msg::fixeval;
    &dismb_msg::die ("Failed to execute \'\$1\': \$2",&dismb_lib::arglist_to_str(@cmd),$@) if $@;
}


#predefined local (*RES, $res);
#dies if the view is not created
#sets config spec from the view VIEWNAME_BUILD (if exists in config);
sub disadmin::create_view {
    local (*SAVEERR,*SAVEOUT);
    open SAVEERR, ">&STDERR"; open STDERR, $dismb::devnull;
    open SAVEOUT, ">&STDOUT"; open STDOUT, $dismb::devnull;

    my $vws = "$disadmin::config_info{'VIEW_VWS_DIR'}/$disadmin::view.vws";
    my @cmd = ($disadmin::cleartool, "mkview", "-tag", $disadmin::view, $vws);	
    &dismb_msg::inf("Creating the view \$1 cmd: \$2",$disadmin::view,&dismb_lib::argstr_to_str(@cmd));

    &dismb_lib::system(@cmd);

    &dismb_lib::open_input(\*RES, $disadmin::cleartool, "lsview", $disadmin::view); $res = <RES>; close RES;

    if (! $res ) {
	open STDERR, ">&SAVEERR"; select STDERR; $| = 1; select STDOUT; close SAVEERR;
	open STDOUT, ">&SAVEOUT"; select STDOUT; $| = 1; close SAVEOUT;
	&dismb_msg::err ("Failed to create view \$1 Create it manually",$disadmin::view);
	&dismb_msg::die ("");
    }
    my $v = "$disadmin::config_info{'VIEWNAME_BUILD'}";
    if ($v) {
	&dismb_lib::open_input(\*RES, $disadmin::cleartool, "lsview", $v); $res = <RES>; close RES;
	if (! $res ) {
	    &dismb_msg::err ("Failed to set config spec for view \$1VIEWNAME_BUILD view \$2 does not exist",$disadmin::view,$v);
	} else {
	    $disadmin::config_file = "$ENV{'TMPDIR'}/disadmin_config_spec.$$";
	    &disadmin::cleartool_catcs($v,$disadmin::config_file);
	    &disadmin::cleartool_setcs($disadmin::view,$disadmin::config_file);
	    &dismb_msg::inf ("Created view \$1 config spec from view \$2",$disadmin::view,$v);
	}
    } else {
	&dismb_msg::inf ("Created view \$1 config spec: default",$disadmin::view);
    }
    open STDERR, ">&SAVEERR"; select STDERR; $| = 1; select STDOUT; close SAVEERR;
    open STDOUT, ">&SAVEOUT"; select STDOUT; $| = 1; close SAVEOUT;
}


sub disadmin::check_clearcase_view {
    return if ($disadmin::config_info{'CM'} ne 'clearcase');
    my $mode = @_[0];
    if (!$disadmin::cleartool) {
	&dismb_msg::err ("Cleartool command does not exist");
	return;
    }
    if ( $disadmin::view eq $disadmin::wv ) {
	if ($mode eq "remove") {
	    &dismb_msg::err ("Failed to remove the current working view  \$1 SET recommends to run disadmin outside of Clearcase.",$disadmin::view);
	} else {
	    &dismb_msg::wrn ("View \$1 is the current working view SET recommends to run disadmin outside of Clearcase.",$disadmin::view);
	    return;
	}
    } 

    open SAVEERR, ">&STDERR"; open STDERR, $dismb::devnull;
    local (*RES, $res);
    if ($mode eq "remove") {
	&dismb_lib::open_input (\*RES, $disadmin::cleartool, "lsview", $disadmin::view);
	$res = <RES>; close RES;
	if (!$res) {
	    &dismb_msg::inf ("The view \$1 does not exist",$disadmin::view);
	} else {
	    &dismb_msg::inf("Removing the view \$1 ...",$disadmin::view);
	    &dismb_lib::open_input(\*RES, $disadmin::cleartool, "rmview", "-tag", $disadmin::view); close RES;
	    &dismb_lib::open_input(\*RES, $disadmin::cleartool, "lsview", $disadmin::view);
	    $res = <RES>; close RES;
	    &dismb_msg::die ("Failed to remove view \$1.  Please remove it manually.", $disadmin::view) if ($res); # Review Why are we removing a view!
	    &dismb_msg::inf ("Removed view \$1", $disadmin::view);
	}
    } else {
	&dismb_lib::open_input(\*RES, $disadmin::cleartool, "lsview", $disadmin::view");
	$res = <RES>; close RES;
    }
    open STDERR, ">&SAVEERR"; select STDERR; $| = 1; select STDOUT; close SAVEERR;

    return if ($mode eq "remove");

    if ( ! $res ) {
	if ($disadmin::config_info{'VIEW_VWS_DIR'}) {
	    if (-d $disadmin::config_info{'VIEW_VWS_DIR'}) {
		&disadmin::create_view();
	    } else {
		&dismb_msg::err ("Directory for view working storage is wrong VIEW_VWS_DIR=\$1",$disadmin::config_info{'VIEW_VWS_DIR'});
		&dismb_msg::die ("");
	    }
	}
    } else {
	&dismb_msg::inf ("Clearcase view \$1 exists \$2",$disadmin::view,$res);
    }
}



#returns found file (basename only)
sub disadmin::complete_file {
    my ($dir, $file) = @_;
    my ($os, $arch, $minor, $prefix, $res);
    $os = ".nt" if ($dismb::is_NT);
    $arch = $disadmin::config_info{'ARCHITECHTURE'}; $arch = "." . $arch if ($arch);
    $minor = &dismb_lib::backtick("uname", "-r") if (!$dismb::is_NT);
    $prefix = $disadmin::config_info{'ADMINDIR_PREFIX'};

    if (      -f "$dir/${prefix}${file}${os}${arch}${minor}") {
	$res = "${prefix}${file}${os}${arch}${minor}";
    } elsif ( -f "$dir/${file}${os}${arch}${minor}") {
	$res = "${file}${os}${arch}${minor}";
    } elsif ( -f "$dir/${prefix}${file}${minor}") {
	$res = "${prefix}${file}${minor}";
    } elsif ( -f "$dir/${file}${minor}") {
	$res = "${file}${minor}";
    } elsif ( -f "$dir/${file}") {
	$res = "${file}";
    }
    return $res;
}


sub disadmin::run_launcher {
    if (-f "$disadmin::adir_trg/tmp/disbuild/all.pmod.lst" &&
	-f "$disadmin::adir_trg/tmp/disbuild/new_reuse.flg"  &&
	-f "$disadmin::adir_trg/dislauncher.log") {
	&dismb_msg::inf ("Model \$1 is installed already dir: \$2 log: dislauncher.log",$disadmin::config_info{'VERSION'},$disadmin::adir_trg);
	return;
    }

    my ($prefs, @opt_prefs,@opt_view);
    # get Preference file name
    $prefs     = &disadmin::complete_file("$disadmin::adir_src/prefs","build.prefs");
    if ($prefs) {
	if ($prefs ne "build.prefs") {
	    &dismb_msg::inf ("Found preferences file \$1",$disadmin::adir_src/prefs/$prefs);
	    @opt_prefs = ($prefs) ? ("-prefs", "$disadmin::adir_src/prefs/$prefs") : ();
	}
    } else {
	&dismb_msg::err ("Failed to find Preference file under source AdminDir \$1", $disadmin::adir_src/prefs);
    }
    @opt_view  = ($disadmin::view) ? ("-view", $disadmin::view) : ();
    
    @cmd = ("$ENV{'PSETHOME'}/bin/disperl", "$ENV{'DISMBDIR'}/launcher.pl", "-masterAdminDir", $disadmin::adir_src, "-workAdminDir", $disadmin::adir_trg, @opt_view, @opt_prefs);

    # run in foreground
    my $res = &dismb_lib::redirect_system("launcher", 0, $dismb::log_dir, @cmd);
    if (! $res) {
	&dismb_msg::err("Failed launcher");
	&dismb_msg::die ("");
    }
    unless ( -f "$disadmin::adir_trg/tmp/disbuild/all.pmod.lst" &&
	     -f "$disadmin::adir_trg/tmp/disbuild/new_reuse.flg" ) {
	&dismb_msg::err ("Launcher failed to emulate dismb. Missing files: \$1 \$2",$disadmin::adir_trg/tmp/disbuild/all.pmod.lst,$disadmin::adir_trg/tmp/disbuild/new_reuse.flg);
	&dismb_msg::die ("");
    }
}


sub disadmin::run_build_server {
    #Run in background build_server
    my $back = 1;
    my @cmd  = ("$ENV{'PSETHOME'}/bin/wish", "$ENV{'PSETHOME'}/lib/dislite/appServer.tcl");

    my $res = &dismb_lib::redirect_system ("build_server", $back, $dismb::log_dir, @cmd);
}


sub disadmin::run_pmod_server {
    $ENV{'ADMINDIR'} = $disadmin::adir_trg;
    undef $ENV{'sharedSrcRoot'}; undef $ENV{'sharedModelRoot'}; 
    undef $ENV{'privateSrcRoot'}; undef $ENV{'privateModelRoot'};
    if ( ($disadmin::config_info{'CM'} eq 'clearcase') && ($disadmin::wv ne $disadmin::view) ) {
	$dis::build_info{'CLEARTOOL'} = $disadmin::cleartool;
	$dis::build_info{'VIEW'}      = $disadmin::view;
    }

    my ($prefs) = "$disadmin::adir_trg/tmp/disbuild/mb_envs.prefs";
    if ( ! -f "$prefs" ) {
	&dismb_msg::err ("Failed to find dismb Preference file under new AdminDir \$1",$prefs);
	&dismb_msg::die ("");
    }
    
    my @cmd = ("$ENV{'PSETHOME'}/bin/pmod_server", "-prefs", $prefs, "-service", $disadmin::serv);
    my $back = 1;
    my $res = &dismb_lib::redirect_system ("pmod_server", $back, $dismb::log_dir, @cmd);
}


#timeout ~ 100 seconds; 2 dots per 10 sec
sub disadmin::check_status {
    &dismb_msg::inf("Checking pmod_server status service \$1 logfile \$2",$disadmin::config_info{'VERSION'},$dismb::log_dir/pmod_server.log);

    my $res = 0;
    foreach $ii (1..15) {
	&dismb_print("."); # Review
	&dismb_lib::init_services();
	if ($dismb::services{$disadmin::serv}) {
	    &dismb_print("."); # Review
	    $res = 1; last;
	}
	&dismb_print(".");
	sleep (5);
    }

    if ($res) {
	&dismb_msg::inf ("Service \$1 is available",$disadmin::config_info{'VERSION'});
    } else {
	&dismb_msg::err ("pmod_server failed to provide service; check the logfile service \$1 logfile \$2",
			 $disadmin::config_info{'VERSION'}, 
			 $dismb::log_dir/pmod_server.log);
	&dismb_msg::die ("");
    }
}

sub disadmin::install {
    local ($disadmin::uninstall) = 0;
    &disadmin::configure();
    &disadmin::check_no_view();
    &disadmin::get_label();

    &dismb_lib::init_services("msg");
    if ( $dismb::services{$disadmin::serv} ) {
	&dismb_msg::wrn ("Service \$1 is active already", $disadmin::serv);
	return;
    } else {
	&dismb_msg::inf ("Service \$1 is not active", $disadmin::serv);
    }

    &disadmin::check_clearcase_view();

    $dismb::log_dir = "$dismb::log_dir/$disadmin::config_info{'VERSION'}";
    if (-d $dismb::log_dir) {
	&dismb_msg::wrn("Log directory exists for version \$1 dir: \$2",$disadmin::config_info{'VERSION'},$dismb::log_dir);
    }

    &disadmin::run_build_server();
    &disadmin::run_launcher();
    &disadmin::run_pmod_server();
    
    if ( defined $disadmin::config_file && -f $disadmin::config_file ) {
	if (&dismb_lib::copy($disadmin::config_file, "$disadmin::adir_trg/config_spec")) {
	    &dismb_msg::inf("Installed config spec: \$1",$disadmin::adir_trg/config_spec);
	} else {
	    &dismb_msg::err("Failed to install config spec copy from: \$1 copy   to: \$2", $disadmin::config_file,$disadmin::adir_trg/config_spec);
	}
    }

    &disadmin::check_status();
}


sub disadmin::uninstall {
    local ($disadmin::uninstall) = 1;
    &disadmin::configure();
    &disadmin::check_no_view();
    &disadmin::get_label();

    &dismb_lib::init_services("msg");
    if ( ! $dismb::services{$disadmin::serv} ) {
	&dismb_msg::inf("Service \$1",$disadmin::serv is not active);
    } else {
	&dismb_msg::inf("Stopping service \$1 ...",$disadmin::serv);
	my ($host,$serv) = split('@',$dismb::services{$disadmin::serv});
	my @cmd = ("$ENV{'PSETHOME'}/bin/dish2", $disadmin::serv, "-connect", $host, "-stop");

	&dismb_lib::system (@cmd);
	&dismb_msg::fixeval;
	&dismb_msg::die("Failed to stop service \$1 cmd: \$2 err: \$3",$disadmin::serv,&dismb_lib::arglist_to_str(@cmd),$@) if $@;

	&dismb_lib::init_services();
	&dismb_msg::die("Failed to stop service \$1",$disadmin::serv) if ($dismb::services{$disadmin::serv});
	&dismb_msg::inf("Service \$1 stopped", $disadmin::serv);
    }
    if ( -d $disadmin::adir_trg ) {
	&dismb_msg::inf("Removing AdminDir \$1",$disadmin::adir_trg);
	eval ('&dismb_lib::unlink ($disadmin::adir_trg)');
	&dismb_msg::fixeval;
	&dismb_msg::die("Failed to unlink AdminDir dir: \$1 err: \$2",$disadmin::adir_trg,$@) if ($@);
	&dismb_msg::inf("AdminDir \$1 unlinked", $disadmin::adir_trg);
    } else {
	&dismb_msg::inf("Non existing AdminDir \$1",$disadmin::adir_trg);
    }
    &disadmin::check_clearcase_view("remove");
}


sub disadmin::main {
    &disadmin::pre_init();

    push @INC, ($ENV{'DISMBDIR'});

    require ("dismb_lib.pl");
    require ("dismb_msg.pl");
    require ("dismb_options.pl");

    &disadmin::list_services() if ($disadmin::command eq "list");

    local($tm) = &dismb_lib::time_stamp;
    &dismb_print ("\n===== disadmin \'\$1\' started   \$2 =====\n",$disadmin::command,$tm);
    &dismb_lib::print_dismb_env ("disadmin", "DIS_DEPLOYMENT PSETHOME DISMBDIR TMPDIR USER");

    &dismb_msg::die ("Command line options are wrong.")          if ( ! &dismb_options::process() );

    my($sec,$min,$hour,$mday,$mon,$year) = &dismb_lib::time2string();
    $dismb::log_dir = "$ENV{'DIS_DEPLOYMENT'}/log";
    $dismb::log_tag = "_$mon" . $mday . "-" . $hour . "_" . $min . "_" . $sec . "_$$";

    if ($disadmin::command eq 'install') {
	&disadmin::install();
    } elsif ( $disadmin::command eq 'uninstall') {
	&disadmin::uninstall();
    }

    $tm = &dismb_lib::time_stamp;
    
    &dismb_print ("\n===== disadmin \'\$1\' succeeded \$2 =====\n",$disadmin::command,$tm);
}

&disadmin::main();
