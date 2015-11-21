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
$dismb_lib::edg_languages = "C CPP ESQL_C ESQL_CPP";
$dismb::is_NT = ($ENV{'OS'} eq "Windows_NT") ? 1 : 0;

#
# local($sec,$min,$hour,$mday,$mon,$year) = &dismb_lib::time2string ;
#
sub dismb_lib::time2string {
    local($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
    local (@res);

    @res[0] = sprintf ("%2d",$sec);  @res[0] =~ s/\s/0/;
    @res[1] = sprintf ("%2d",$min);  @res[1] =~ s/\s/0/;
    @res[2] = sprintf ("%2d",$hour); @res[2] =~ s/\s/0/;
    @res[3] = sprintf ("%2d",$mday); @res[3] =~ s/\s/0/;
    @res[4] = (Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec)[$mon];
    @res[5] = sprintf ("%4d",1900 + $year);
    return @res;
}


sub dismb_lib::time_stamp {
    local (@ttt) = &dismb_lib::time2string;
    local ($str) = @ttt[4].@ttt[3]."-".@ttt[5]." ".@ttt[2].":".@ttt[1].":".@ttt[0];
    return $str;
}

#
# delete files from a directory
#
# usage:     &dismb_lib::cleanup <dir> <ext>       , exs:     &cleanup "/usr/tmp" "*.C"
sub dismb_lib::cleanup {
    my($dir,$pattern) = @_;
    &dismb_msg::wrn ("cleanup: \$1 is not a directory",$dir) if ( ! -d $dir );
    local(*DIR);

    if ( ! opendir (DIR,$dir) ) {
	&dismb_msg::err ("cleanup: failed to open directory \$1: \$2",$dir,$!);
	return;
    }

    my(@all) = readdir (DIR);
    closedir(DIR);
    foreach $item (@all) {
	next if ( $item =~ /^[\.\s]*$/ );
	if ( ! $pattern || $item =~ /$pattern/ ) {
	    &dismb_msg::vrb ("unlink \$1", "$dir/$item");
	    eval '&dismb_lib::unlink ("$dir/$item")';
	    &dismb_msg::fixeval;
	    &dismb_msg::wrn ("Failed to delete \$1: \$2",$item,$@) if ($@) ;
	}
    }
}


sub dismb_lib::cleanup_if_force {
    &dismb_lib::cleanup ("@_") if ( $mbPrefs{'forceCleanup'} eq "yes" );
}


# $cmd $host $cpu $pass $fenv $feval are preset
sub dismb_lib::exec_rsh {
    local(@actionRsh,@rshSuffix);
    local($arch) = `"$psethome_bin/DISarch"`;
    @actionRsh   = ( $arch eq "hp700" ) ? ("remsh", $host, "-n") : ("rsh", "-n", $host);
    local(@cmd)  = ("$ENV{'PSETHOME'}/bin/disperl", "$ENV{'DISMBDIR'}/dismb_remote.pl", $host, $cpu, $pass, $fenv, $feval);
    push @cmd, (&dismb_msg::lp_flags(), "-stdout");

    local($config) = &dismb_prefs::get ('configuratorSystem');
    if ($config eq "clearcase") {
	@actionRsh=(@actionRsh, "cleartool", "setview", "-exec");
	@rshSuffix = ("`", "cleartool", "pwv", "-short", "`");
    }

    @cmd = (@actionRsh, "\'@{cmd}\'", @rshSuffix);
    &dismb_msg::vrb ("child pid \$1: exec \$2",$$,&dismb_lib::arglist_to_str(@cmd));
    my($cmd_name) = @cmd[0];
    exec { $cmd_name } @cmd;
}



#usage: &do_fork "oxen" 3 "parse_runner.pl"
# uses: rsh   command, that may not be available on NT
#
# $host, $cpu, $pass, $hostlog, $fenv, $feval       are preset
sub dismb_lib::do_fork {
    local ($pid);
  FORK: {
      if ($pid = fork) {
          &dismb_msg::vrb ("Started \$1:\$2, pid \$3",${host},$cpu,$pid);
          return $pid;
      } elsif (defined $pid) {
	  if ($host eq &dismb_lib::get_local_host()) {
	      &dismb_msg::vrb ("child pid \$1: exec local \$2 \$3 \$4",$$,$pass,$host,$cpu);
	      eval '&dismb_local::run_pass ($host, $cpu)';
	      exit;
	  } else {
	      &dismb_lib::exec_rsh;	# $host $cpu $pass, $hostlog, $fenv, $feval are preset
	  }
      } elsif ($! =~ /No more process/) {
	  &dismb_msg::vrb ("EAGAIN; try after 5 seconds"); # Review
	  sleep 5;
	  redo FORK;
      } else {
	  &dismb_msg::die ("Can't fork: \$1",$!);
      }
  }
}

#
# Optional parameter - mode (default: 0777).
# Dies of failure, must be called with evaluate:
#
#      eval '&dismb_lib::mkdir_path ($dir)';
#
sub dismb_lib::mkdir_path {
    my($d,$mode) = @_; $mode = 0777 if ( ! $mode );
    $d =~ s/^\s*//; $d =~ s/\s*$//; $d =~ s%[/\\]+$%%;

    return if ( ! $d || -d $d );

    my(@comps) = split( /[\\\/]/, $d);
    return if ( $#comps == 0 );

    my($st, $cur) = (0, "");
    if ($comps[0] =~ /^[a-zA-Z]:$/) {
	return if ( $#comps == 1 );
	$st = 1; $cur = $comps[0];
    }

    for $ii ($st..$#comps) {
	next if ( $comps[$ii] =~ /^[\s\\\/\.]*$/ );
	$cur = $cur . '/' . $comps[$ii];
	if ( ! -d $cur ) {
	    if ( !mkdir ($cur, $mode) ) {
		if ( ! -d $cur ) {
		    mkdir ($cur, $mode) || &dismb_msg::die("Failed to create dir \$1: \$2",$cur,$!);
		}
	    }	
	}
    }
}

sub dismb_lib::create_dir {
    local ($dir, $cleanup) = @_;
    if ( ! -d $dir ) {
	eval '&dismb_lib::mkdir_path ($dir)';
	&dismb_msg::fixeval;
	&dismb_msg::die("Failed to create dir \$1 \$2",$dir,$@) if ( $@ );
    } else {
	&dismb_lib::cleanup ($dir, ".*") if ($cleanup);
    }
}

# returns 0 on success
sub dismb_lib::create_dir_for_file {
    local($ff) = @_;
    return 0 if ( -f $ff );

    local($pset_dir);
    ($pset_dir = $ff) =~ s/[\/\\][^\/\\]*$//;

    eval '&dismb_lib::mkdir_path ($pset_dir)';
    return 0 if ( -d $pset_dir );
    &dismb_msg::fixeval;
    &dismb_msg::err ("Failed to create dir \$1 \$2",$pset_dir, $@);
    return 1;
}


sub dismb_lib::get_locked_linenum {
    local($fln) = @_; my($ln);
    return -1 if (&dismb_lib::create_dir_for_file ($fln));

    if ($dismb::is_NT) {
	$ln = eval '&dismb_lib::dis_lock ($fln)';
	if ($@) {
	    &dismb_msg::fixeval;
	    &dismb_msg::err ("Failure accessing lock file \$2: \$1",$@, $fln);
	    $ln = 0;
	}
    } else {
	my($exe) = "$psethome_bin/dis_lock";
	$ln = &dismb_lib::backtick($exe, $fln);
        my($ret) = $?;
	if ( $ln <= 0 ) {
	    &dismb_msg::err ("Wrong line number from lock file \$1: \$2 : \$3",$fln, $ln, $ret);
	    $ln = 0;
	}
    }
    return $ln;
}

sub dismb_lib::get_upper_byte {
    my($value)=@_[0];
    my($retvalue)=0;

    if( $value & 0xff00 ) {
	$retvalue = ($value >> 8) & 0x00ff ;
    }
    return $retvalue;
}

sub dismb_lib::get_lower_byte {
    my($value)=@_[0];
    my($retvalue)=0;

    if( $value & 0x00ff ) {
	$retvalue = $value & 0x00ff ;
    }
    return $retvalue;
}

sub dismb_lib::get_return_value {
    my($ecode)=@_[0];
    if( $dismb::is_NT ) {
	return &dismb_lib::get_lower_byte($ecode);
    } else {
	return &dismb_lib::get_upper_byte($ecode);
    }
}

sub dismb_lib::get_signal_value {
    my($ecode)=@_[0];
    if( $dismb::is_NT ) {
	return &dismb_lib::get_upper_byte($ecode);
    } else {
	return &dismb_lib::get_lower_byte($ecode);
    }
}

sub dismb_lib::system {
    my($ecode);

    # Check for special syntax that requires a shell rather than simple exec.
    my($i);
    my($execcable) = 1;
    foreach $i (@_) {
        if ($i eq ">" or $i eq "&" or $i eq ">&" or $i eq "|" or $i eq "|&") {
	    $execcable = 0;
        }
    }
    if ($execcable and (not $dismb::is_NT)) {
	my($cmd) = @_[0];
        $ecode = system { $cmd } @_;
    } 
	else {
		my($cmd) = &dismb_lib::arglist_to_fixed_command(@_);
		$ecode = system $cmd;
    }
    return $ecode;
}

sub dismb_lib::munch_backslashes {
    my($argstr) = @_[0];
    my($res) = "";
    while ($argstr =~ m/\\/) {
	if ($argstr =~ m/^([^\\]*)\\(.)(.*)$/) {
	    $res .= $1 . $2;
	    $argstr = $3;
	}
	elsif ($argstr =~ m/^([^\\]*)\\(.*)$/) {
	    $res .= $1;
	    $argstr = $2;
	}
	else {
	    $res .= $argstr;
	    $argstr = "";
	}
    }
    return ($res . $argstr);
}

# Make a command list from a command string.
sub dismb_lib::argstr_to_list {
    my($argstr) = @_[0];
    my(@res) = (); # for building up the result list
    my($str); # initially undefined, holds partial argument
    while ($argstr ne "") {
	# Handle what's at the left end of $argstr,
	# removing the recognized portion.
	if ($dismb::is_NT and ($argstr =~ m/^\\\"(.*)$/)) {
	    # Windows: Recognize backslash-quote as an escape sequence.
	    if (!(defined $str)) { $str = ""; }
	    $str .= '"';
	    $argstr = $1;
    	}
	elsif ($dismb::is_NT and ($argstr =~ m/^\\(.*)$/)) {
	    # Windows: Recognize backslash as an ordinary character.
	    if (!(defined $str)) { $str = ""; }
	    $str .= '\\';
	    $argstr = $1;
    	}
	elsif ($argstr =~ m/^([^\\"\s]+)(|[\\"\s].*)$/) {
	    # Recognize some regular characters.
	    if (!(defined $str)) { $str = ""; }
	    $str .= $1;
	    $argstr = $2;
    	}
	elsif ($argstr =~ m/^\s+(|[^\s].*)$/) {
	    # Whitespace: recognize an argument if present.
	    if (defined $str) {
	        push @res, $str;
	    }
	    undef $str;
	    $argstr = $1;
	}
	elsif ($argstr =~ m/^\\(.)(.*)$/ and (not $dismb::is_NT)) {
	    # Backslash-escape. Convert to single character.
	    if (!(defined $str)) { $str = ""; }
	    $str .= $1;
	    $argstr = $2;
	}
	elsif ($argstr =~ m/^"((\\.|[^\\"]+)*)"(.*)$/) {
	    # Quoted string: Convert to string.
	    if (!(defined $str)) { $str = ""; }
	    if (not $dismb::is_NT) {
		$str .= &dismb_lib::munch_backslashes($1);
	    }
	    else {
		$str .= $1;
	    }
	    $argstr = $3;
	}
	elsif ($argstr =~ m/^["\\](.*)$/) {
	    # Dangling quote or backslash: ignore.
	    if (!(defined $str)) { $str = ""; }
	    $argstr = $1;
    	}
	else {
	    die "error in argstr_to_list";
	}
    }
    if (defined $str) {
	push @res, $str;
    }
    return @res;

}

# Make a command string from a command list.
sub dismb_lib::arglist_to_str {
    my(@cmds);
    my($i);
    foreach $i (@_) {
	if ($i =~ /\s/ or $dismb::is_NT ? ($i =~ /\+/) : ($i =~ /[\(\)]/)) {
	    push @cmds, ("\"" . $i . "\"");
    	}
	else {
	    push @cmds, $i;
	}
    }
    return (join " ", @cmds);
}

sub dismb_lib::arglist_to_fork_str {
    my(@cmds);
    my($i);
    foreach $i (@_) {
	if ($i =~ /\s/) {
	    push @cmds, ("\\\"" . $i . "\\\"");
    	}
	else {
	    push @cmds, $i;
	}
    }
    return (join " ", @cmds);
}

# Make a command string from a command list.
# On NT, commands are interpreted strangely if they begin with a quote.
sub dismb_lib::arglist_to_fixed_command {
    my($cmd) = &dismb_lib::arglist_to_str(@_);
    if ($dismb::is_NT) {
	if ($cmd =~ /^\s*"/) {
	    $cmd = "\"" . $cmd . "\"";
	}
    }
    return $cmd;
}

sub dismb_lib::backtick {
    my($cmd) = &dismb_lib::arglist_to_fixed_command(@_);
    `$cmd`;
}

sub dismb_lib::open_input {
    my($p_in_handle, @cmds) = @_;
    my($cmd) = &dismb_lib::arglist_to_fixed_command(@cmds);
    return (open($p_in_handle, "${cmd}|"));
}

sub dismb_lib::open_output {
    my($p_out_handle, @cmds) = @_;
    my($cmd) = &dismb_lib::arglist_to_fixed_command(@cmds);
    return (open($p_out_handle, "|${cmd}"));
}

# dies on error exit. Should be called with eval '&dismb_lib::system(...)'
# chars '{' and  '}' get substituted with '"'
sub dismb_lib::system_bkgd {
    my($back, @ccc) = @_;
    # my $ccc = @_[0]; $ccc =~ s/[{}]/\"/g; 
    @ccc = (@ccc, "\&") if $back;

    local($tag) = (defined($host) && $host ) ? "${host}:$cpu " : "";

    &dismb_print ("\n\$1Running: \$2", ${tag}, &dismb_lib::arglist_to_str(@ccc));
    my($ecode) = 0xffff & (&dismb_lib::system (@ccc));

    if ($ecode == 0) {
        &dismb_print ("\$1SUCCEEDED \$2", ${tag}, &dismb_lib::arglist_to_str(@ccc));
    } else {
        my $return_value = &dismb_lib::get_return_value($ecode);
        my $signal_value = &dismb_lib::get_signal_value($ecode);
        if ($return_value != 0) {
            &dismb_msg::die("\$1FAILED \$2 exit status: \$3 ",${tag},&dismb_lib::arglist_to_str(@ccc),$return_value);
        } elsif ($signal_value != 0) {
            my($sig_msg) = "died with ";
            if ($signal_value & 0x80) {
                $signal_value &= ~0x80;
	        $sig_msg = $sig_msg . "coredump from ";
	    }
	    $sig_msg = $sig_msg . "signal " . "$signal_value";
	    &dismb_msg::die ("\$1FAILED \$2 \$3",${tag},&dismb_lib::arglist_to_str(@ccc),$sig_msg);
        }
    }
    return $ecode;
}

sub dismb_lib::get_release_id {
    return $dismb_lib::ReleaseID if ( defined $dismb_lib::ReleaseID );
    $dismb_lib::ReleaseID = "Not found";

    my($f) = "$ENV{'PSETHOME'}/.version";
    local(*FFF);
    if ( open (FFF, "<$f") ) {
	my($id) = <FFF>; $id =~ s/^\s+//; $id =~ s/\s+$//;
	$dismb_lib::ReleaseID = $id if $id;
	close FFF;
    }
    return $dismb_lib::ReleaseID;
}


sub dismb_lib::print_dismb_env {
    #create new message group for printing dismb environment
    &dismb_msg::push_new_message_group("disMB environment");

    my($tag, $envs) = @_; 
    $envs   = "PSETHOME ADMINDIR DISMBDIR TMPDIR DISBUILD_TMPDIR USER" if ( ! $envs );
    my @arr = split (' ', $envs);

    my($host)  = &dismb_lib::get_local_host();
    my($rel_id)= &dismb_lib::get_release_id();

    my($uname) = ($dismb::is_NT) ? `cmd /c ver` : `uname -a`;
    $uname =~ s/^\s+//; $uname =~ s/\s+$//;

    my $fmt1 = 'env:  @>>>>>>>>>>>>>>>= @*';
    #           env:  DISBUILD_TMPDIR= $ENV{'DISBUILD_TMPDIR'}\n");

    foreach $var (@arr) {
	&dismb_print("env: \$1= \$2",$var,$ENV{$var}) if (defined $ENV{$var});
    }
    &dismb_print("env: \$1= \$2","PLATFORM",$uname);
    &dismb_print("env: \$1= \$2","DISCOVER releaseID",$rel_id);

    #pop new message group created for printing dismb environment
    &dismb_msg::pop_new_message_group;
}


sub dismb_lib::finish_one_pass {
    return if ($pass eq "");
    eval ('require "dismb_parse.pl"');
    &dismb_msg::fixeval;
    if ($@) { &dismb_msg::die("can't require dismb_parse.pl, because $@"); }
    local($sum_fun) = "dismb_" . $pass . "::summary";
    eval '&$sum_fun' if (defined &$sum_fun);
    &dismb_msg::fixeval;
    &dismb_msg::die ("Summarizing pass \$1: \$2",$pass,$@) if ( $@ );

    require ("dismb_local.pl");
    local($after) = "dismb_local::" . $pass . "_after";
    eval '&$after' if (defined &$after);
    if ( ! $@ ) {
	eval '&dismb_local::finish' if (defined &dismb_local::finish);
    }
    if ( $@ ) {
	&dismb_msg::fixeval;
	&dismb_lib::status ("$pass 1 1 failed") if ( $pass ne "parse" );
	&dismb_msg::die ("Finishing pass \$1: \$2",$pass,$@);
    } else {
	&dismb_lib::status ("$pass 1 1 succeeded") if ( $pass ne "parse" );
    }
}


# $pass is predefined
sub dismb_lib::init_one_pass {
    return if ($pass eq "");

    if ( &dismb_prefs::is_continue_mb ) {
	&dismb_msg::inf ("Continue pass \$1", $pass);
    }

    $^T = time if ($pass eq 'update');	# set the script startup time for update::summary

    if ( ! defined $dismb_prefs::change2write && ! &dismb_prefs::init_config ) {
	&dismb_msg::die ("Failed to initialize config.");
    }

    if ( $pass eq 'parse' ) {
	require ("dismb_parse.pl");
	eval '&dismb_parse::prepare()';
    } else {
	require ("dismb_local.pl");
	eval '&dismb_local::prepare()';
	if ( ! $@ ) {
	    local($before) = "dismb_local::" . $pass . "_before";
	    eval '&$before' if (defined &$before);
	}
    }
    &dismb_msg::fixeval;
    &dismb_msg::die ("Initializing \$1: \$2",$pass,$@) if ( $@ );
    &dismb_lib::status ("$pass 1 0 start") if ( $pass ne "parse" );
}

sub dismb_lib::status {
    return unless &dismb_prefs::get ('MBlogStatus') eq "yes";
    my $msg = @_[0];

    #submission check relies on this message to update its 
    #progress bar, make sure it is flushed to stdout
    my $fd = ( $pass eq "parse" ) ? SAVESTDOUT : STDOUT;
    print $fd "@ " . $msg . "\n";
}

sub dismb_lib::fork2pipe {
    local($tag, @cmd) = @_;
    local($cmd_name) = @cmd[0];
    local($pid); local($init) = $tag.PID;

    return $$init if ( defined $$init );

    if ($dismb::is_NT) {
	$$init = 0;
	return $$init;
    }

    if ( ! &dismb_prefs::is_fork_supported ) {
	$$init = 0;
	return $$init;
    }

    local($read1,$write1,$read2,$write2) = ($tag.READ1,$tag.WRITE1,$tag.READ2,$tag.WRITE2);

    pipe($read1, $write1);
    pipe($read2, $write2);

FORK:
    if ($pid = fork) {
	close $write2; close $read1;
	select ($write1) ; $| = 1;
	select (STDOUT)  ; $| = 1;
	$$init = $pid;
    } elsif ( defined $pid ) {
	close $write1; close $read2;
	open(STDIN,"<&$read1");
	open(STDOUT,">&$write2");
	open(STDERR,">/dev/null");
	select (STDERR); $| = 1;
	select (STDOUT); $| = 1;

	exec { $cmd_name } @cmd;
    } elsif ($! =~ /No more process/) {
	&dismb_msg::vrb ("EAGAIN; try after 5 seconds");
	sleep 5;
	redo FORK;
    } else {
	$$init = 0;
    }

    if ( $$init ) {
	&dismb_msg::inf ("Initialized 2 way pipe: \$1, pid= \$2 \$3",$tag,$$init,&dismb_lib::arglist_to_str(@cmd));
    } else {
	&dismb_msg::err("Failed to initialize 2 way pipe: \$1 \$2", $tag, &dismb_lib::arglist_to_str($cmd));
    }
    return $$init;
}

sub dismb_lib::preserve_case {
    return $dismb_lib::keepFileCase if defined $dismb_lib::keepFileCase;
    $dismb_lib::keepFileCase = 1;
    if ($dismb::is_NT) {
	my($val) = &dismb_prefs::get ('PreserveFilenameCase');
	$dismb_lib::keepFileCase = 0 if ($val == 0 || $val eq "no");
	if (&dismb_prefs::is_verbose_mb) {
	    if ($dismb_lib::keepFileCase) {
		&dismb_msg::vrb ("Preserving file name case, pref PreserveFilenameCase=\$1", $val);
	    } else {
		&dismb_msg::vrb ("Not preserving file name case, pref PreserveFilenameCase=\$1",$val);
	    }
	}
    }
    return $dismb_lib::keepFileCase;
}

sub dismb_lib::dis_path {
    local($nm)  = @_;
    $nm =~ s/\s+$//;
    return $dis_pathSET{$nm} if (defined $dis_pathSET{$nm});

    my($cmd) = ($dismb::is_NT) ? "dis_path.exe" 
	                       : "dis_path";
    
    &dismb_msg::die ("dis_path: \$1 is not a executable",$cmd) if ( ! -x "$psethome_bin/$cmd" );

    if ( ! defined $dis_pathPID ) {
	&dismb_lib::fork2pipe ('dis_path', $cmd);
	if (! $dis_pathPID) {
	    &dismb_msg::vrb ("Using direct \$1", $cmd);
	}
    }
    local($res) = "";
    if ($dis_pathPID) {
	print dis_pathWRITE1 $nm, "\n";
	$res = <dis_pathREAD2>;
	$res = "" if ($res =~ /^ERROR/);
    } else {
	$res = &dismb_lib::backtick($cmd, $nm);
	$res = "" if ( $? != 0 );
    }
    if ( $res ) {
	$res =~ s/\s+$//; $res =~ tr [A-Z] [a-z] unless &dismb_lib::preserve_case();
	$dis_pathSET{$nm} = $res;
	&dismb_msg::inf ("dis_path: \$1 => \$2",$nm,$res) if ("$nm" ne "$res");
    }
    return $res;
}

sub dismb_lib::close2pipe {
    my($tag) = @_;
    my($nm_pid)  = $tag . "PID";
    my($nm_set)  = $tag . "SET";

    close $tag.WRITE1; close $tag.READ2;
    undef %$nm_set;
    kill $$nm_pid;
}


sub dismb_lib::project_list {
    local($lst) = (@_[0] ne 'root') ? "$ENV{'DISBUILD_TMPDIR'}/PMODS.pmod.lst" : "$ENV{'DISBUILD_TMPDIR'}/ROOTS.pmod.lst";
    local(@projs, *LIST, $line); local($ind) = 0;
    open (LIST, "<$lst") || &dismb_msg::die ("Failed to read \$1 list \$2",@_[0],$lst);
    while ($line = <LIST>) {
	local(@arr) = split (' ', $line);
	@projs[$ind++] = @arr[2];
    }
    return @projs;
}

# returns 1 on success
sub dismb_lib::copy {
    local($from,$to) = @_;
    local(*FROM, *TO, $line);
    local($ret) = 0;
    if (open (FROM, "<$from")) { 
	if ( open (TO,   ">$to") ) {
	    $ret = 1;
	    while ( $line = <FROM> ) {
		print TO $line;
	    }
	    close TO;
	} else {
	    &dismb_msg::err ("copy: failed to write \$1", $to);
	}
	close FROM;
    } else {
	&dismb_msg::err ("copy: failed to read \$1", $from);
    }
    return $ret;
}

sub dismb_lib::ncopy {
    local($from,$to,$trgsize) = @_;

    local($dev, $ino, $mode, $nlink, $uid, $gid, $rdev, $size, $atime, $mtime, $ctime, $blksize, $blocks) = stat $from ;

    if ( $size < $trgsize ) {
	&dismb_msg::wrn ("ncopy: \$1 is smaller than requested size.  Waiting for it to flush...", $from);

        my ($times, $max_times) = (0, 60);
        while ( $size < $trgsize  && $times < $max_times) {
	    sleep 1;
	    ($dev, $ino, $mode, $nlink, $uid, $gid, $rdev, $size, $atime, $mtime, $ctime, $blksize, $blocks) = stat $from ;
            $times++;
        }
        &dismb_msg::msg("Waited for \$1 seconds for \$2 to flush.", $times, $from);
    }

    &dismb_msg::wrn ("ncopy: \$1 is still smaller than requested size.  Going ahead with copy.", $from) if ( $size < $trgsize );
    &dismb_msg::wrn ("ncopy: \$1 is larger than requested size.  Will trucate after copy.", $from)      if ( $size > $trgsize );
    
    local(*FROM, *TO, $line);
    local($ret) = 0;
    if (open (FROM, "<$from")) { 
	if ( open (TO,   ">$to") ) {
	    $ret = 1;
	    while ( $line = <FROM> ) {
		print TO $line;
	    }
	    truncate (TO, $trgsize) if ( $size > $trgsize ) ;
	    close TO;
	} else {
	    &dismb_msg::err ("ncopy: failed to write \$1", $to);
	}
	close FROM;
    } else {
	&dismb_msg::err ("ncopy: failed to read \$1", $from);
    }
    return $ret;
}

# returns: 0 - identical; 1 differ; -1 failed
sub dismb_lib::cmp {
    local($from,$to) = @_;
    return -1 if (! (-f $from && -f $to) );
    return  0 if ($from eq $to);

    local($sz_from) = ( -s $from ); local($sz_to)   = ( -s $to );
    return 1 if ( $sz_from != $sz_to );
    return 0 if ( ! $sz_from );

    local(*FROM, *TO, $ln_from, $ln_to, $ret);
    $ret = -1;
    if (open (FROM, "<$from")) { 
	if ( open (TO,   "<$to") ) {
	    $ret = 1;

	  cmpLOOP: while ( 1 ) {
	      if ( eof(FROM) && eof(TO) ) {
		  $ret = 0; last cmpLOOP;
	      }
	      last cmpLOOP if ( eof(FROM) || eof(TO) );
	      $ln_from = <FROM>; $ln_to = <TO>;
	      last cmpLOOP if ($ln_from ne $ln_to);
	  }
	    close TO;
	} else {
	    &dismb_msg::err ("cmp: failed to read \$1",$to);
	}
	close FROM;
    } else {
	&dismb_msg::err ("cmp: failed to read \$1",$from);
    }
    return $ret;
}

# returns 1 om success; dies on failure
# run with  eval ('&dismb_lib::unlink ($dir)'); check $@
sub dismb_lib::unlink {
    my($dir,$level) = @_;
    my($ret) = 0;

    if (-f $dir) {
	$ret = unlink ($dir);
	&dismb_msg::die ("Failed to unlink \$1: \$2",$dir,$!) if (! $ret);
	return $ret;
    }

    $dir =~ s%[\/\\]+$%%;
    return 1 if ( ! -d $dir );

    $level = int($level);
    local(*DIR);

    opendir (DIR,$dir) || &dismb_msg::die ("Failed to open directory \$1: \$2",$dir,$!);
    my(@all) = readdir (DIR);
    closedir(DIR);

    if ($level) {
	my $name = $dir;
	$name =~ s%^.+[\/\\]([^\/\\]+)%$1%;
	&dismb_msg::vrb ( "unlink \$1", $name );
    } else {
	&dismb_msg::vrb ( "unlink \$1", $dir );
    }

    $level++;

    my($item);
    
    foreach $item (@all) {
	if (-f "$dir/$item") {
	    &dismb_msg::vrb ( "unlink \$1", $item );
	    $ret = unlink ("$dir/$item");
	    &dismb_msg::die ("Failed to unlink \$1/\$2",$dir,$item) if (! $ret || -f "$dir/$item" );
	}		       
    }
    foreach $item (@all) {
	next if ( $item eq "." || $item eq ".." );
	if (-d "$dir/$item") {
	    $ret = &dismb_lib::unlink ("$dir/$item", $level);
	    &dismb_msg::die ("Failed to rmdir \$1/\$2: \$3",$dir,$item,$!) if (! $ret);
	}		       
    }
    $ret = rmdir ($dir);
    &dismb_msg::die ("Failed to rmdir \$1: \$2",$dir,$!) if (! $ret);
    return $ret;
}

$dismb::LOCK_SH = 1;
$dismb::LOCK_EX = 2;
$dismb::LOCK_NB = 4;
$dismb::LOCK_UN = 8;

$dismb_lib::MAX_LOCK_LENGTH = 16;

# returns: next int from the specified file
#          dies on failure
#          use with eval '&dismb_lib::dis_lock ($f)' ;
sub dismb_lib::dis_lock {
    my($f, $script) = @_;
    local(*LOCK, $buf, $len, $ret_num, $new_buf, $new_num);

    if (! -f $f) {
	 open  (LOCK, ">>$f") || &dismb_msg::die ("dis_lock: can't create \$1: \$2",$f,$!);
	 close LOCK;
    }
    open  (LOCK, "+<$f") || &dismb_msg::die ("dis_lock: can't open \$1: \$2",$f,$!);
    flock (LOCK,$dismb::LOCK_EX);

    $len     = read (LOCK,$buf,$dismb_lib::MAX_LOCK_LENGTH);
    $ret_num = ($len) ? int ($buf) : 1;
    $new_num = $ret_num + 1;
    $new_buf = sprintf ("%${dismb_lib::MAX_LOCK_LENGTH}s",$new_num);
    
    seek (LOCK, 0, 0);
    print LOCK $new_buf;

    flock (LOCK,$dismb::LOCK_UN);
    close LOCK;
    return $ret_num;
}

sub dismb_lib::quote {
    my($str) = @_;
    if ($str) {
	$str =~ s%\\%/%g;      #back slash to forward slash
	$str =~ s% %\\\\ %g; #quote spaces
    }
    return $str;
}

sub dismb_lib::expand_env_unix {
    my($cur_line) = @_;
    my($val)      = "";
    my($chard)    = '(?:(?:\\\\.)|[^\\$])';
    my($head,$tail,$evar);

    while ( $cur_line =~ /^($chard*)(.*)/o ) {
	($head, $tail) = ( $1, $2 );
	last if ( ! $tail || ! ( $tail =~ /^\$(\w+|\{\w+\})(.*)/ ) );

	($evar, $tail) = ( $1, $2 ); $evar =~ s/[\{\}]//g;
	&dismb_msg::die("env var \$1 is not defined.",$evar) if ( ! defined ($ENV{$evar}) );

	$cur_line = $tail; $val = $val . $head . &dismb_lib::quote($ENV{$evar});
    }

    $val = $val . $cur_line;
    return $val;
}


sub dismb_lib::expand_env_nt {
    my($cur_line) = @_;
    my($charp)    = '(?:(?:\\\\.)|[^\\%])';
    my($val)      = "";
    my($head,$tail,$evar);

    while ( $cur_line =~ /^($charp*)(.*)/o ) {
	($head, $tail) = ( $1, $2 );
	last if ( ! $tail || ! ( $tail =~ /^\%(\w+)\%(.*)/ ) );

	($evar, $tail) = ( $1, $2 );
	&dismb_msg::die("env var \$1 is not defined.",$evar) if ( ! defined ($ENV{$evar}) );
	
	$cur_line = $tail; $val = $val . $head . $ENV{$evar};
    }

    $val = $val . $cur_line;
    return $val;
}

sub dismb_lib::get_not_quoted_char_index2 {
    my($str,$chr,$qchr) = @_;

    my($ind) = -1; my($pos) = 0; my($len) = length($str);
  LOOP: while ($ind == -1 && $pos < $len) {
      my($found) = index ($str, $chr, $pos);
      last LOOP if ($found == -1);

      $pos = $found + 1;
      my($prev) = ($found == 0) ? 0 : substr($str,$found-1, 1);
      $ind = $found if ($prev ne $qchr);
    }
    return $ind;
}

sub dismb_lib::expand_env_unix_ntbug {
    my($cur_line) = @_;
    my($val)      = "";
    my($head,$tail,$evar);

  LOOP: while (1) {
      my($ind) = &dismb_lib::get_not_quoted_char_index2 ($cur_line, "\$", "\\");
      last LOOP if ($ind == -1);

      ($head, $tail) = ( substr($cur_line,0,$ind), substr($cur_line,$ind) );
      last LOOP if ( ! $tail || ! ( $tail =~ /^\$(\w+|\{\w+\})(.*)/ ) );
      
      ($evar, $tail) = ( $1, $2 ); $evar =~ s/[\{\}]//g;
      &dismb_msg::die("env var \$1 is not defined.",$evar) if ( ! defined ($ENV{$evar}) );
      
      $cur_line = $tail; $val = $val . $head . $ENV{$evar};
  }

    $val = $val . $cur_line;
    return $val;
}


sub dismb_lib::expand_env_nt_ntbug {
    my($cur_line) = @_;
    my($val)      = "";
    my($head,$tail,$evar);

  LOOP: while (1) {
      my($ind) = &dismb_lib::get_not_quoted_char_index2 ($cur_line, "\%", "\\");
      last LOOP if ($ind == -1);
      
      ($head, $tail) = ( substr($cur_line,0,$ind), substr($cur_line,$ind) );
      last if ( ! $tail || ! ( $tail =~ /^\%(\w+)\%(.*)/ ) );
      
      ($evar, $tail) = ( $1, $2 );
      &dismb_msg::die("env var \$1 is not defined.",$evar) if ( ! defined ($ENV{$evar}) );

      $cur_line = $tail; $val = $val . $head . $ENV{$evar};
  }

    $val = $val . $cur_line;
    return $val;
}

#
# returns string with expanded env variables
# handles: $... ${...} %...%
# dies on undefined variable (must be called with :
#         eval '&dismb_lib::expand_env_vars($line)'
#
#
sub dismb_lib::expand_env_vars {
    my($cur_line) = @_;
    $cur_line     = ($dismb::is_NT) ? &dismb_lib::expand_env_unix_ntbug ($cur_line)
                                    : &dismb_lib::expand_env_unix ($cur_line);
    $cur_line     = &dismb_lib::expand_env_nt_ntbug ($cur_line) if $dismb::is_NT;

    return $cur_line;
}


$dismb_lib::tm_msg = "";

sub dismb_lib::do_timecheck {
    if ( ! $dismb_lib::tm_msg ) {
	$dismb_lib::tm_msg = &dismb_prefs::get ('MBsyncTimeCheck');
	if ( $dismb_lib::tm_msg eq "no" ) {
	    &dismb_msg::wrn ("MBsyncTimeCheck disabled. Skip file time checking.");
	} else {
	    $dismb_lib::tm_msg = "yes";
	    &dismb_msg::vrb ("MBsyncTimeCheck enabled.");
	}
    }
    return $dismb_lib::tm_msg;
}

# returns
#     1    if file is created after the sript started 
#          or MBsyncFileCheck is disabled
#
#     0    if file does not exist or file created before the script started
#
sub dismb_lib::is_created_after_start {
    my($f) = @_;

    return 0 if ( ! -f $f );
    return 1 if ( &dismb_lib::do_timecheck eq "no" );
    return 1 if ( -M $f < 0 );

    &dismb_msg::wrn ("File \$1 created before the script started.",$f);

    return 1;
}

sub dismb_lib::count_lines {
    my    ($f)   = @_;
    my    ($cnt) = 0 ;
    local (*FFF);

    open FFF, "<$f" or return 0;

    while (<FFF>) {
	$cnt++;
    }

    close FFF;
    &dismb_msg::vrb ("Count lines: file= \$1 lines= \$2",$f,$cnt);
    return $cnt;
}


sub dismb_lib::formline {
    my $frmt = shift;
    $^A = "";
    formline ($frmt, @_);
    return $^A;
}

# copies source under target; both must exist
# returns: 1 on success or dies
# call with eval 'dismb_lib::xcopy($src $trg)'
sub dismb_lib::xcopy {
    my($src, $trg) = @_;

    &dismb_msg::die("xcopy: no source file or directory \$1",$src) if ( ! (-d $src || -f $src) );

    my @comm;
    if ( $dismb::is_NT ) {
	$src  =~ s%\/%\\%g;
	$trg  =~ s%\/%\\%g;
	if ((-f $src) and not (-d $trg)) {
	    # Xcopy would hang.
	}
	else {
	    @comm = ("xcopy", $src, $trg, "/s", "/e", "/i", "/q");
	}
    } else {
	@comm = ("cp", "-r", $src, $trg);
    }
    my $ret    = &dismb_lib::system (@comm);
    &dismb_msg::die ("Failed \$1",&dismb_lib::arglist_to_str(@comm)) if ($ret != 0);
    1;
}


#returns: array of fields in PRJ format
sub dismb_lib::split_prj_line {
    my($line) = @_[0];
    my(@arr,$el);

    my(@tarr) = split (' ', $line); my($add) = "";
    foreach $el (@tarr) {
	if (!$add && ! ($el =~ /^\"/) ) {
	    # Does not begin with quote. Append to result.
	    push ( @arr, $el );
	    next;
	}
	$el =~ s/^\"// ;        # Remove leading quote, if present.
	if ($add) {
	    $add = $add . ' ';  # Restore the delimiter (space).
	}
	$add = $add . $el;      # Append to current element.
	next if ( ! ($add =~ /\"$/) );

	# This chunk ends with a quote.
        # Strip it and append current element to result.
	$add =~ s/\"$// ;
	push (@arr, $add );
        $add = "";
	next;
    }
    return @arr;
}

sub dismb_lib::get_local_host {
    my $val = ($dismb::is_NT) ? `hostname` : `uname -n`;
    $val =~ s/\s+$//;
    return $val;
}


#returns 1 on success
#        0 on failure
sub dismb_lib::cleartool_system_bkgd {
    my($back, @cmds) = @_;
    my(@r_cmds) = @cmds;

    if ( $dis::build_info{'VIEW'} ) {
	my $v = $dis::build_info{'VIEW'}; my $ct = $dis::build_info{'CLEARTOOL'};
	@r_cmds = ($ct, "setview", "-exec", "\'" . &dismb_lib::arglist_to_str(@cmds) . "\'", $v);
	if ( ! $dismb::is_NT && -x "$ENV{'PSETHOME'}/bin/disshell" ) {
	    $ENV{'DISSHELL'} = $ENV{'SHELL'};
	    $ENV{'SHELL'}    = "$ENV{'PSETHOME'}/bin/disshell";
	}
    }

    my $ecode = eval '&dismb_lib::system_bkgd ($back, @r_cmds)';
    if ( $ecode || $@) {
	&dismb_msg::err ("system error: \$1; error code= \$2",$!,$ecode) if $ecode;
	&dismb_msg::fixeval;
	&dismb_msg::err ("system error: \$1", $@)                     if $@;
	return 0;
    }
    return 1;
}


#returns 1 on success
#        0 on failure
sub dismb_lib::redirect_system {
    my ($tag, $back, $log_dir_param, @cmds) = @_;
    my ($log) = ($dismb::log_dir) ? "$dismb::log_dir/$tag.log" : "$ENV{'TMPDIR'}/${tag}${dismb::log_tag}.log";
    @cmds = (@cmds, ">", $log, "2", ">&", "1");
    my $bbb = ($back) ? "in background " : "in foreground ";
    &dismb_msg::inf ("Running \$1\$2",${bbb},$tag);
    &dismb_msg::die("Cannot create log directory for \$1",${log}) if ( &dismb_lib::create_dir_for_file($log) );

    my $res = &dismb_lib::cleartool_system_bkgd($back, @cmd); # background if $back > 0
}


#dies on error
#returns associative array
sub dismb_lib::read_build_info {
    my (%arr, $lll);
    local (*FFF);
    open (FFF, "<$dismb_lib::build_info") || &dismb_msg::die("Failed to read \$1: \$2",$dismb_lib::build_info,$!);
    while ($lll = <FFF>) {
	next if ( $lll =~ /^[^\#]/ );
	if ( $lll =~ /^\#(\w+)=(.*)/ ) {
	    $arr{$1} = $2;
	}
    }
    close FFF;
    return %arr;
}


sub dismb_lib::write_build_info {
    my $binfo = "$ENV{'DISBUILD_TMPDIR'}/build_info";
    if (! &dismb_prefs::is_continue_mb() || ! -f $binfo) {
	&dismb_msg::vrb ("Writing build info");
	local (*BINFO);
	if (open (BINFO, ">$binfo")) {
	    my ($h, $a, $v, $p, $c) = (&dismb_lib::get_local_host(), $ENV{'ADMINDIR'}, "", "", "");
	    $h = join '.', unpack 'C4', gethostbyname $h; # internet host address instead of a name
	    $p = &dismb_options::get ('-prefs') if  &dismb_options::is_defined ('-prefs');
	    if (&dismb_prefs::get ('configuratorSystem') eq "clearcase") {
		$v = `$c pwv -short`; $v =~ s/\s*$//;
		if ( !$v || $v eq "** NONE **" ) {
		    &dismb_msg::err ("Wrong working view for configuratorSystem=\'clearcase\'. view= \$1",$v);
		    $v = "";
		} else {
		    $c = &dismb_prefs::get ('MBcleartool'); $c = "cleartool" if ( ! $c );
		}
	    }
	    print BINFO "#HOST=$h\n"; print BINFO "#ADMINDIR=$a\n";
	    print BINFO "#VIEW=$v\n"      if $v;
	    print BINFO "#CLEARTOOL=$c\n" if $v;
	    print BINFO "#PREFS=$p\n"     if $p;
	    close BINFO;
	    &dismb_msg::vrb ("\$1: #HOST=\$2 #ADMINDIR=\$3 #VIEW=\$4 #CLEARTOOL=\$5 #PREFS=\$6",$binfo,$h,$a,$v,$c,$p);
	} else {
	    &dismb_msg::err ("Failed to open \$1: \$2", $binfo,$!);
	}
    } else {
	&dismb_msg::inf ("Using existing build info \$1", $binfo);
    }
}


# returns 1 on success, 0 on failure
# uses %dis::build_info
sub dismb_lib::option_build_info {
    my ($opt, $key, $nm, $default) = @_ ;
    my $v;
    
    if ( &dismb_options::is_defined($opt) ) {
	$v = &dismb_options::get($opt);
	if (! $v) {
	    &dismb_msg::err ("There is no \$1 specified after the option \'\$2\'",$nm,$opt);
	    return 0;
	}
	if ( $dis::build_info{$key} ) {
	    if ( $dis::build_info{$key} eq $v ) {
		&dismb_msg::inf ("Same \$1=\$2 in build info and command option",$nm,$v);
	    } else {
		&dismb_msg::inf ("Overwrite build info \$1 with command option \$2: old value: \$3 new value: \$4",$nm,$opt,$dis::build_info{$key},$v);
		$dis::build_info{$key} = $v;
	    }
	} else {
	    &dismb_msg::inf ("Set build info \$1 with command option \$2: \$3",$nm,$opt,$v);
	    $dis::build_info{$key} = $v;
	}
    } else {
	if ( $dis::build_info{$key} ) {
	    $v = $dis::build_info{$key};
	    &dismb_msg::inf ("      \#\$1=\$2",$key,$v);
	} else {
	    if ($default) {
		$v = $default;
		&dismb_msg::inf("Default build info \$1: \$2",${nm},$v);
		$dis::build_info{$key} = $v;
	    }
	}
    } 
    return 1;
}

# returns 1 on success; 0 on failure
# %dis::build_info
sub dismb_lib::init_build_info {
    $dismb_lib::build_info = @_[0];
    undef %dis::build_info;
    
    if ( -f $dismb_lib::build_info ) {
	&dismb_msg::inf ("Read build info from \$1",$dismb_lib::build_info);
	%dis::build_info = &dismb_lib::read_build_info();
    } else {
	&dismb_msg::inf ("No build info file: \$1",$dismb_lib::build_info);
	$dis::old_build_info{'new'} = 1;
    }
 
    return 0 if (!&dismb_lib::option_build_info ("-view", "VIEW", "view"));
    if ( $dis::build_info{'VIEW'} ) {
	return 0 if (!&dismb_lib::option_build_info ("-cleartool", "CLEARTOOL", "cleartool command", cleartool));
    }
    return 0 if (!&dismb_lib::option_build_info ("-prefs", "PREFS", "preferences"));
    return 1;
}


#initializes %dismb::services
#dies on error; use eval '&dismb_lib::init_services()'
sub dismb_lib::init_services {
    &dismb_msg::inf ("Checking active services ... ") if (@_[0]);
    undef %dismb::services;
    my $exe = "$ENV{'PSETHOME'}/bin/dish2";

    local(*HOSTS,*NAMES,$host,$name);
    &dismb_lib::open_input(\*HOSTS, $exe, "-hosts") or &dismb_msg::die("Failed to run \$1 -hosts",$exe);

    while ($host = <HOSTS>) {
	chop($host);
	&dismb_lib::open_input(\*NAMES, $exe, "-test", $host) or &dismb_msg::die("Failed to run \$1 -list \$2",$exe,$host);
	while ($name = <NAMES>) {
	    chop ($name);
	    $dismb::services{$name} = "${host}\@${name}";
	}
	close NAMES;
    }
    close HOSTS;
}

#return 1 if the path is absolute else return 0.
#On NT the path is absolute if there is a drive letter followed by ':'.
#On Unix the path is absolute if the first character is "/". 
sub dismb_lib::is_absolute_path
{
  my($filename) = $_[0];
  my($ret_val) = 0;
  if ($dismb::is_NT) {
    if ( $filename =~ /^[a-zA-Z]:/ ) {
      $ret_val = 1;
    }
  } else {
    if ($filename =~ /^\//) {
      $ret_val = 1;
    }
  } 
  return $ret_val;
}


#calculate physical name changing "*_drive" to "drive_letter" . ":" and
#removing project name from the beginning
sub dismb_lib::get_physical_name
{  
  my($file, $proj_name) = @_;

  if ($proj_name ne "") {
    $file =~ s/^($proj_name)//;
  }

  if ($file =~ /^[\/\\]?[a-zA-Z]_drive/ ) {
    $file =~ s/^[\/\\]?([a-zA-Z])_drive//;
    $file = "$1" . ":" . "$file";
  }
  return $file;
}

#calculate logical name changing drive names to "*_drive" and
#adding project name if present
sub dismb_lib::get_logical_name
{
  my($src_file, $projname) = @_;
  my($logical_name);

  if ($src_file =~ /^[a-zA-Z]:/ ) {
    my($drive_letter, $path) = split(':', $src_file);
    $logical_name = $drive_letter . "_drive" . $path;
  } else {
    $logical_name = $src_file;
  }

  if ($projname ne "") {
    #remove any leading slashes from $logical_name and $projname
    $logical_name =~ s%^/%%; $projname =~ s%^/%%;
    $logical_name = "/$projname/$logical_name";
  }
  return $logical_name;
}

#calculates pset file location/name for a given source. 
#Also, on NT if PreserveFilenameCase preference is not set all filenames are
#converted to lower case. 
sub dismb_lib::pset_filename_of
{
  my($src_file) = $_[0];
  $src_file =~ s%^/%%;
  my($logical_fname) = &dismb_lib::get_logical_name($src_file);

  my($pset_filename) = "$ENV{'ADMINDIR'}/model/pset";
  $pset_filename = "$pset_filename/${logical_fname}.pset";
  if ( ! &dismb_lib::preserve_case() ) {
    $pset_filename =~ tr [A-Z] [a-z];
  }
  return $pset_filename;
}

sub dismb_lib::get_iff_location
{
  my($src_file) = $_[0];
  my($iff_filename) = &dismb_lib::pset_filename_of($src_file);

  #add ".iff" extension to the pset location
  $iff_filename .= ".iff";

  return $iff_filename;
}

#If name has space in it add a pair of quotes around it.
sub dismb_lib::add_quotes
{
  my($filename) = @_;
  if ( $filename =~ /\s+/ ) {
    $filename = "\"$filename\"";
  }
  return $filename;
}

sub dismb_lib::is_header
{
  my($lang) = $_[0];
  return 1 if ( $lang =~ /[hH]/ ); 
  return 0;
}

sub dismb_lib::last_modified_time
{
  my($file) = $_[0];
  my($dev, $inode, $mode, $nlink, $uid, 
   $gid, $rdev, $size, $atime, $mtime,
   $ctime, $blksize, $blocks) = stat($file);
  return $mtime;
}

sub dismb_lib::rename
{
  my($val) = 1;
  my($oldname, $newname) = @_;
  if ( !rename($oldname, $newname) ) {
    dismb_msg::err("Failed to rename \$1 to \$2 : \$3", $oldname, $newname, $!);
    $val = 0;
  }
  return $val;
}

sub dismb_lib::parent_dir
{
  my($parent) = $_[0];
  my($is_root) = 0;

  if ( $dismb::is_NT ) {
    if ( $parent =~ /^[a-zA-Z]:[\/\\]?$/ ) {
      $is_root = 1;
      $parent = "";
    }
  } else {
    if ( $parent eq "/" ) {
      $is_root = 1;
      $parent = "";
    }
  }  
  if ( !$is_root ) {
    #remove trailing slash
    $parent =~ s%[/\\]$%%;
    $parent =~ s%[/\\][^/\\]*$%%;
  }

  return $parent;
}

sub dismb_lib::suffix_of_file
{
  my($file) = $_[0];
  my($suffix) = ( $file =~ /\.([^\.]*)$/ ) ? $1 : ""; 
  return $suffix;
}

sub dismb_lib::checksum_of_file
{
  my($filename) = $_[0];
  my($md5_exe) = "$psethome_bin/md5";
  if ( $dismb::is_NT ) {
    $md5_exe .= ".exe";
  }
 
  #run md5 command
  my($chksum) = &dismb_lib::backtick($md5_exe, $filename);
  my($ret) = $?;
  if ( $ret != 0 ) {
    &dismb_msg::err("Failed to calculate checksum of file \$1, return value: \$2, signal value: \$3", $filename, &dismb_lib::get_return_value($ret), &dismb_lib::get_signal_value($ret));
    $chksum = 0;
  } else {
    chomp($chksum);
  }
  return $chksum;
}

sub dismb_lib::size_of_hash
{
  my($hash_reference) = $_[0];
  my($size) = 0;
  my($key, $value);
  while ( ($key, $value) = each(%$hash_reference) ) {
    $size++;
  }
  return $size;
}

#returns 1 if the suffix of the specified file is "class" or "jar" or "zip", 
#returns 0 otherwise
sub dismb_lib::is_java_dependency_file
{
  my($file) = $_[0];
  my($val) = 0;
  my($suffix) = &dismb_lib::suffix_of_file($file);
  if ( ($suffix =~ /^class$/i) || ($suffix =~ /^jar$/i) || ($suffix =~ /^zip$/i) )
  {
    $val = 1;
  }
  return $val
}

sub dismb_lib::package_name_of_java_file
{
  my($file) = $_[0];
  my($package_name) = "";
  my($line);
  my($comment_start_str) = "(\/\\\*)";  #matches '/*' 
  my($comment_end_str) = "(\\\*\/)";    #matches '*/'
  my($package_str) = "(package \s*([^${comment_start_str}${comment_end_str}]*);)"; #matches 'package SOMETHING;'  
  if ( open(JF, "<$file") ) {
    my($comment_start_found, $comment_end_found) = (0,0);
    while ( $line = <JF> ) {
      chomp($line);
      
      #skip empty lines or line starting with '//'
      next if ( $line =~ /^\s*$/ || $line =~ /^\s*\/\// );
      
      if ( $line =~ /^\s*${comment_start_str}[^${comment_end_str}]*${comment_end_str}/ ) {
        $comment_start_found = 1; $comment_end_found = 1; #found '/*' and '*/'
      }
      elsif ( $line =~ /^\s*${comment_start_str}[^${comment_end_str}]*/ ) {
        $comment_start_found = 1; $comment_end_found = 0; #found '/*' but not '*/'
      }
      elsif ( $line =~ /[^${comment_start_str}]*${comment_end_str}/ ) {
	$comment_end_found = 1; #found '*/' but not '/*'
      }
      #if there were no comments or all the comments are ended and there is a 
      #"package" statement not enclosed in comments then we found the right string
      if ( (!$comment_start && !$commnet_end) || ($comment_start && $comment_end)) {
        if ( $line =~ /${package_str}/ && $line !~ /$1[^${comment_start_str}]*${comment_end_str}/ ) {
	  $package_str_match = $1;
          ($package_name = $package_str_match) =~ s/^package\s*//;
	  $package_name =~ s/\s*;$//;
	  last;   #found package string, stop reading file now
	}
      }
    }
    close(JF);
  }
  &dismb_msg::report_progress("scan Java file");
  return $package_name; 
}

#Recursively goes through all the directories starting at specified 
#root directory and finds files with the given extension.
sub dismb_lib::find_files_with_pattern
{
    my($rootdir, $pattern) = @_;
    my(@matchfiles) = ();
    
    if ($rootdir eq "" || $pattern eq "") {
	return \@matchfiles;
    }
    my($num_dir) = 0;
    my(@dirlist) = $rootdir;
    
    while ($num_dir <= $#dirlist) {
        my($dir) = $dirlist[$num_dir++];
	
	opendir DIRHANDLE, $dir;
	my(@list) = grep { $_ ne '.' and $_ ne '..' } readdir DIRHANDLE;
	closedir DIRHANDLE;
	
        my($filename);
	foreach $filename (@list) {
	    my($fullname) = $dir . "/" . $filename;
	    if ( ! -d $fullname ) {
		if ( $dismb::is_NT ? $filename =~ /${pattern}$/i : $filename =~ /${pattern}$/ ) { 
		    push(@matchfiles, $fullname);  #filename matches specified pattern
		}
	    } else {  
		push(@dirlist, $fullname);         #If file is a directory, add to the list
            }
        }
    }
    return \@matchfiles;
}
   
return 1;
