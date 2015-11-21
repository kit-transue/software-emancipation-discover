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
#  %mbPrefs       preference names/values
#  @dismb_keys    preference names, taken from prefs files
#  %key_with_case index lc key, yielding key with canonical case
#  @prefFiles     ids of prefs files, coded as described below
#  %mbPrefs_xxx   preference names/values for 1 file, with tag xxx


require ("dismb_msg.pl");

$dismb_prefs::prefix = "*psetPrefs.";
$dismb_prefs::initialized = 0;
$fdefault    = "$ENV{'DISMBDIR'}/dismb_default.prefs";
$default_tag = "mbPrefs_default";
@dismb_prefs::resumeable_passes =("analysis", "parse", "update", "finalize");

# Adds a key to the @dismb_keys array, if it's not yet present.
# The %dismb_keys hash is used to prevent duplicates in @dismb_keys.
#
sub dismb_prefs::add_key {
    my($key, $file) = @_;
    return if (defined %dismb_keys && defined $dismb_keys{$key});
    if (not defined %key_with_case && defined $key_with_case{lc $key}) {
        $key_with_case{lc $key} = $key;
    }
    elsif (not &dismb_prefs::key_case_is_correct($key)) {
	&dismb_msg::err("Preference key \$1 from file \$3 is incorrect; correct spelling is \$2.",
		$key, $key_with_case{lc $key}, $file);
    }

    push (@dismb_keys, $key);
    $dismb_keys{$key} = $#dismb_keys;
}

sub dismb_prefs::key_case_is_correct {
    my($key) = @_;
    return $key_with_case{lc $key} eq $key;
}

sub dismb_prefs::parse_pref {
    local($f,$tag) = @_;
    local($arr_tag) = ($tag eq "") ? "mbPrefs" : "mbPrefs_$tag";

    if ( ! open (PREF, "<$f") ) {
	&dismb_msg::err ("Cannot open pref file \$1: \$2",$f,$!);
	return 0;
    }
    local($l,$empty,$wrong,$comments,$good) = (0,0,0,0,0);
    my($cur_line,$multi_line,$tmp_buf);
  FILE:
    while ( $tmp_buf = <PREF> ) {
        $tmp_buf =~ s/(.*)\Z(?!\n)/$1\n/;  # in case no \n in the last line in .prefs file (bug 19314) 
	$l ++;			# line number for error message
	$tmp_buf =~ s/^\s+//;	# leading spaces
	$tmp_buf =~ s/\s+$//;	# trailing spaces
	
	$tmp_buf =~ s%(\s*\\)$%%; my($backslash) = ( $1 ) ? 1 : 0;
	$cur_line = "" if ( ! $multi_line );

	$empty++   if ( $tmp_buf eq "" );
	if ( $tmp_buf =~ /^\#/ ) {
	    $comments++ ;
	    next FILE;
	}

	$cur_line   = ($multi_line && $cur_line) ? $cur_line . " " . $tmp_buf : $tmp_buf;
	$multi_line = $backslash;
	next FILE if ($multi_line || ! $cur_line);
    
	if ( ! $cur_line =~ /\:/ ) {
	    $wrong ++ ;
	    &dismb_print ("Pref file \$1, error in line \$2: no \'\:\' character",$f,$l);
	    next FILE;
	} 
	if ( $cur_line =~ /^([^\#\:\s]+)(\s*)\:(\s*)(.*)$/ ) {
	    $good ++ ;
	    local($key) = $1;
	    my($val) = eval '&dismb_lib::expand_env_vars ($4)';;
	    if ($@) {
		&dismb_msg::fixeval;
		&dismb_print ("Pref file \$1, error in line \$2: \$3",$f,$l,$@) if ($@);
		$wrong ++;
		next FILE;
	    }

	    $$arr_tag{$key} = $val;
	    &dismb_prefs::add_key ($key, $f) if ( $tag eq "default" ); # dismb defaults
	    if ( ! defined ($$default_tag{$key}) ) { # handle of generated Preferences, like "key.$pass"
		&dismb_prefs::add_key ($key, $f);
	    }
	} else {
	    &dismb_print ("Pref file \$1, error in line \$2: no name before \'\:\' character",$f,$l);
	    $wrong ++ ;
	}
    }
    close PREF;
    if ($wrong == 0) {
        if ($dismb_prefs::verbose or $mbPrefs{"*psetPrefs.MBverbose"}) {
	    &dismb_msg::inf ("     Pref file \$1: \$2 values, \$2 empty line, \$3 comments",$f,$good, $empty, $comments);
	}
	return 1;
    }
    local($s) = ($wrong > 1) ? "s" : "" ;
    &dismb_msg::err ("Pref file \$1 has \$2 error\$3.",$f,$wrong,${s});
    return 0;
}


sub dismb_prefs::get_mbPrefs_file {
	local($mbPrefs_file) = "$ENV{'DISBUILD_TMPDIR'}/mb.prefs";
	$mbPrefs_file =~ s%\/%\\\\%g if ( $dismb::is_NT ); 
	return $mbPrefs_file;
}

# Load prefs from mb.prefs file, with overriding by command line options.
#
sub dismb_prefs::load_mbPrefs {
    local($mbPrefs_file) = &dismb_prefs::get_mbPrefs_file;
    return 0 if (! -r $mbPrefs_file );
    if ( &dismb_prefs::parse_pref ($mbPrefs_file, "") ) {
	if ( $dismb_options::correct ) {
	    local(@keys) = keys %mbPrefs;
	    local($key);
	    local($over) = 0;
	    foreach $key (@keys) {
		&dismb_prefs::add_key ($key, $mbPrefs_file);
		if (&dismb_options::is_defined_pref ($key)) {
		    local($pref_val) = $mbPrefs{$key}; 
		    local($opt_val)  = &dismb_options::get_pref($key);
		    if ($pref_val ne $opt_val) {
			$mbPrefs{$key}   = $opt_val;
			&dismb_msg::inf ("Overwrite pref \$1 by option; \$2 -> \$3",$key,$pref_val,$opt_val);
			$over ++;
		    }
		}
	    }
	    &dismb_prefs::dump_mbPrefs() if ($over > 0);
	}
	return 1;
    }

    &dismb_msg::die ("Error in existing \$1",$mbPrefs_file);
    return 0;
}


# parameters: ($val ,"ADMINDIR PSETHOME") value and list of ENVs
# returns value with subs for all the ENVs
sub dismb_prefs::convert_to_env {
    my ($val, $lst) = @_;
    return $val unless ($val and $lst);

    my ($el, $new_val); $new_val = $val;
    foreach $el (split(' ',$lst)) {
	my $env = $ENV{$el}; next unless $env;
	$new_val =~ s/$env\b/\$$el/ig;
    }
    return $new_val;
}


# Saves the current preferences to mb.prefs and mb_envs.prefs files.
#
sub dismb_prefs::dump_mbPrefs {
    local($mbPrefs_file) = &dismb_prefs::get_mbPrefs_file;
    local($max_tabs) = 6;
    local($tab_len)  = 8;
    local($key, *PREFS, *PENVS);

    local($mbPrefs_envs) = "$ENV{'DISBUILD_TMPDIR'}/mb_envs.prefs";

    &dismb_msg::die ("Cannot dump non-existing Preferences.") if ( ! defined %mbPrefs );
    open (PREFS, ">$mbPrefs_file") || &dismb_msg::die ("Cannot create dismb Preference file \$1: \$2",$mbPrefs_file,$!);
    open (PENVS, ">$mbPrefs_envs") || &dismb_msg::die ("Cannot create dismb Environment file \$1: \$2",$mbPrefs_envs,$!);

    print PREFS "#\n#  dismb preferences\n#\n";
    print PENVS "#\n#  dismb preferences with ADMINDIR and PSETHOME env vars\n#\n";
    foreach $key (@dismb_keys) {
	if (&dismb_prefs::key_case_is_correct($key)) {
	    local($num)    = $max_tabs - ((length($key) + 2)/8) ;
	    local($whites) = ( $num > 0 ) ? "\t" x $num : " " ;
	    print PREFS $key, ":" , $whites , $mbPrefs{$key}, "\n";
	    print PENVS $key, ":" , $whites , &dismb_prefs::convert_to_env($mbPrefs{$key},"ADMINDIR PSETHOME"), "\n";
	}
    }
    close PREFS; close PENVS;
    &dismb_msg::inf ("Work prefs: \$1",$mbPrefs_file);
}


#
# Fills in the %mbPrefs. If the file $DISBUILD_TMPDIR/mb.prefs exists,
# then just parse it, load it and exit.
# Otherwise, creates creates array @prefFiles[ii]  = "tag=>fname", where tag is
# part of the assoc array, "=>" just acts as a seperator between the two fields.
#
# name: mbPrefs_$tag. Here is the sequence of pref files to search:
#       -prefs - dismb command line option (may be more than one file)
#       fhome  - $HOME/.psetPrefs ( if no MBptestRunning )
#       fadmin - $ADMINDIR/prefs/build.prefs
#       fpset  - $PSETHOME/lib/psetPrefs.default (or /job1/prefs/asetTest.prefs.$ENV{'Arch'} if MBptestRunning=yes)
#
# Dumps $DISBUILD_TMPDIR/mb.prefs
#
sub dismb_prefs::load {
    return 1 if ( &dismb_options::is_defined ('-MBcontinue') && &dismb_prefs::load_mbPrefs );

    &dismb_msg::die("Failed in pref file \$1.",$fdefault) if (! &dismb_prefs::parse_pref ($fdefault,"default"));

    my($ii) = 0;

    if ( &dismb_options::is_defined ('-prefs') ) {
        #if more than one prefs files is mentioned, apth separator should
        # be used. (";" on Unix and ":" on Windows)
        my($separator) = ($dismb::is_NT) ? ";" : ":";
	local (@opt_prefs) = split ($separator, &dismb_options::get ('-prefs'));
	for $ii (0..$#opt_prefs) {
	    @prefFiles[$ii] = "opt_prefs$ii=>@opt_prefs[$ii]";
	}
	$ii = @prefFiles;
    }

    local($tag_line);
    my($ptest) = (&dismb_options::get_pref ('MBptestRunning') eq "yes");
    foreach $tag_line ("fhome=>$ENV{'HOME'}/.psetPrefs", "fadmin=>$ENV{'ADMINDIR'}/prefs/build.prefs", "fpset=>$ENV{'PSETHOME'}/lib/psetPrefs.default") {
	local($tag, $f) = split ('=>', $tag_line);
	next if ($tag eq "fhome" && $ptest); # skip .psetPrefs
	$f = "/job1/prefs/asetTest.prefs.$ENV{'Arch'}" if ( $tag eq "fpset" && $ptest ); # change default
	if ( -f $f ) {
	    @prefFiles[$ii ++] = "$tag=>$f";
	}
    }

    # Process files in reverse order so that names from the installation
    # take precedence in @dismb_keys and %key_with_case.
    foreach $tag_line ( reverse @prefFiles ) {
	local($tag, $f) = split ('=>', $tag_line);
	&dismb_msg::die("Failed in pref file \$1",$f) if (! &dismb_prefs::parse_pref ($f, $tag));
    }
    &dismb_msg::vrb ("Preference file path:");
    foreach $tag_line ( @prefFiles ) {
	local($tag, $f) = split ('=>', $tag_line);
	&dismb_msg::vrb ("     \$1",$f);
    }
    &dismb_msg::vrb ("     \$1\n",$fdefault);


    # fill in assoc array mbPrefs
    local($key);
    foreach $key (@dismb_keys) {
	local($val) = &dismb_prefs::get ($key); # fills in %mbPrefs
    }

    &dismb_msg::die ("Failed to initialize config.") if ( ! &dismb_prefs::init_config );

    &dismb_prefs::dump_mbPrefs();
    return 1;
}


sub dismb_prefs::is_monitoring_log {
    if ( &dismb_prefs::get("logFileMonitor") eq "no" ) {
	return 0;
    } else {
	return 1;
    }
}

sub dismb_prefs::can_parallelize {
    my($phase) = @_[0];
    my($val) = 0;
    if ($dismb::standalone_mode) {
        $val = ( $phase eq 'parse' || $phase eq 'update' || $phase eq 'dfa_init' || $phase eq 'query' ) ? 1 : 0;
    } else {
        #Update phase cannot be parallelized when used from DISIM scripts.
        $val = ( $phase eq 'parse' || $phase eq 'dfa_init' || $phase eq 'query' ) ? 1 : 0;
    }
    return $val;
}

# predefined $pass
sub dismb_prefs::get_MBparallelHosts {
    my($val) = "";
    if (! defined $pass) {
	&dismb_msg::vrb ("Pass is not set. Ignore MBparallelHosts");
    } elsif ( &dismb_prefs::can_parallelize($pass) ) {
	$val = &dismb_prefs::get ("MBparallelHosts");
    } 
    return $val;
}

# dismb_remote.pl just sets the variable without asking Preferences
sub dismb_prefs::is_parallel_mb {
    return (! $dismb_prefs::remote_mb && &dismb_prefs::get_MBparallelHosts eq "") ? 0 : 1;
}


sub dismb_prefs::is_verbose_mb {
    # global $dismb_prefs::verbose (static, initialized once )
    if (! defined $dismb_prefs::verbose) {
	my($verbose) = "no";
	my($got_good_answer) = 0;
	if (defined ($ENV{'DISCOVER_VERBOSE'})) {
	    $verbose = $ENV{'DISCOVER_VERBOSE'};
	    $got_good_answer = 1;
	} elsif (defined($Verbosity)) {
	    $verbose = ($Verbosity=~m@^high$@);
	    $got_good_answer = 1;
	} elsif ($dismb_options::correct && &dismb_options::is_defined ('-MBverbose')) {
	    $verbose = &dismb_options::get('-MBverbose');
	    $got_good_answer = 1;
	} elsif (&dismb_prefs::defined('MBverbose')) {
	    $verbose = &dismb_prefs::get('MBverbose');
	    $got_good_answer = 1;
	}
	if ($got_good_answer) {
	    if ($verbose eq "no" || $verbose eq "0" || $verbose eq "NO") {
		$dismb_prefs::verbose = 0;
	    } else {
		$dismb_prefs::verbose = 1;
	    }
	    return $dismb_prefs::verbose;
	} else {
	    # If asked to render an opinion when command line and environment
	    # are empty, and prefs have not been initialized, say no.
	    return 0;
	}
    }
    return $dismb_prefs::verbose;
}

sub dismb_prefs::init {
    #create new message group for initialize preference
    &dismb_msg::push_new_message_group("initialization of preferences");

    $dismb_prefs::initialized = eval '&dismb_prefs::load()';

    $dismb::standalone_mode = ( &dismb_prefs::get('MBgeneratePdf') ne "yes" ) ? 1 : 0;

    #pop new message group create for initialize preferences
    &dismb_msg::pop_new_message_group;

    return $dismb_prefs::initialized;
}


sub dismb_prefs::defined {
    local($key) = @_;
    $key = ( $key =~ /^\*psetPrefs\./) ? $key : "*psetPrefs." . $key;
    return defined ($mbPrefs{$key});
}

sub dismb_prefs::get {
    local($key) = @_;
    return &dismb_prefs::get_internal ("$key.$pass") if ( defined $pass && &dismb_prefs::defined ("$key.$pass") );
    return &dismb_prefs::get_internal ($key);
}


sub dismb_prefs::get_internal {
    local($key) = @_;
    $key = ( $key =~ /^\*psetPrefs\./) ? $key : "*psetPrefs." . $key;

    return $mbPrefs{$key} if ( defined ($mbPrefs{$key}) );

    local($val) = "";
    local($not_defined) = (! defined $$default_tag{$key}) ? 1 : 0;
    local($found)       = 0;
    local ($tag_line);
    if ( &dismb_options::is_defined_pref ($key) ) {
	$val = &dismb_options::get_pref ($key);
	&dismb_msg::vrb ("\$1: \$2 <== command line option",$key,$val);
    } else {
      LOOP:
	foreach $tag_line (@prefFiles) {
	    local($tag, $f) = split ('=>', $tag_line);
	    local($arr_name) = "mbPrefs_$tag";
	    if (defined ($$arr_name{$key})) {
		$val = $$arr_name{$key};
		&dismb_msg::vrb ("\$1: \$2 <== \$3",$key,$val,$f);
		$found = 1;
		last LOOP;
	    }
	}

	if (! $found ) { 
	    if ( $not_defined ) {
		&dismb_msg::err ("Unknown Preference \$1",$key);
	    } else {
		$val = $$default_tag{$key};
		&dismb_msg::vrb ("\$1: \$2 <== \$3",$key,$val,$fdefault);
	    }
	}
    }
    $mbPrefs{$key} = $val;

    return $val;
}

	
sub dismb_prefs::set {
    my ($key, $val) = @_;
    $key =  $dismb_prefs::prefix . $key if (! ($key =~ /^\*psetPrefs\./) );

    if ( $mbPrefs{$key} eq $val ) {
	return;
    }

    if ( &dismb_prefs::is_verbose_mb() ) {
	if ( defined ($mbPrefs{$key} ) ) {
	    &dismb_msg::vrb ("RESET pref \$1, new: \$2, old: \$3",$key, $val, $mbPrefs{$key});
	} else {
	    my $old_val = &dismb_prefs::get ($key);
	    &dismb_msg::vrb ("SET pref \$1, new: \$2, old: \$3",$key, $val, $old_val);
	}
    }
    $mbPrefs{$key} = $val;
}


sub dismb_prefs::init_hostResources {
    my ($phase) = @_[0];
    return 0 if ! $dismb_prefs::initialized ;

    local($lhost) = &dismb_lib::get_local_host();
    local($val)   = &dismb_prefs::get_MBparallelHosts();

    $val =~ s/^\s+//;		# leading whites
    $val =~ s/\n\r//;		# newlines
    $val =~ s/\s+$//;		# tail whites

    if ( $val eq "" ) {
	@hostResources[0] = $lhost . ":" . "1";
        if ( &dismb_prefs::can_parallelize($phase) ) {
	    &dismb_msg::vrb ("MBparallelHosts is not specified in the Preferences. Parallel modelbuild is disabled. Local host: \$1",$lhost);
        }
	return 1;
    }

    local(@items, $item, $host, $cpus, $cnt);
    my($ii);
    if ( $val =~ /\s/ ) {
	@items = split (' ', $val);
    } else {
	@items[0] = $val;
    }
    $ii = 0;
    foreach $item (@items) {
	if ( $item =~ /\:/ ) {
	    ($host, $cpus) = split (':', $item);
	    if ( $cpus eq "" ) {
		$cpus = 1;
	    }
	} else {
	    $host = $item;
	    $cpus = 1;
	}
	$cnt = 1;
	while ($cnt <= $cpus) {
	    @hostResources[$ii] = $host . ":" . $cnt;
	    $ii ++ ; $cnt ++ ;
	}
    }
    if ($#hostResources == 0 && ($hostResources[0] =~ /^$lhost:1$/) ) {
        &dismb_msg::vrb ("MBparallelHosts specified in the Preferences set to single cpu on local host");
        &dismb_prefs::set("MBparallelHosts.$phase", "");
    } else {
        &dismb_msg::inf ("parallel MB: MBparallelHosts= \$1",$val);
        &dismb_msg::vrb ("    hosts/cpus:");
        my($hres);
        foreach $hres (@hostResources) {
	    &dismb_msg::vrb ("         \$1", $hres);
        }
    }
    return 1;
}


sub dismb_prefs::init_query {
    my $res = 0;
    my $script = &dismb_prefs::get('MBqueryTclScript');
    if ( $script ) {
	&dismb_msg::die ("Preference MBqueryTclScript points to non-existing script \$1", $script) if ( ! -f $script );
	$res = 1;
	&dismb_msg::vrb ("Query script in preference MBqueryTclScript: \$1", $script);
    }
    return $res;
}

sub dismb_prefs::init_qar {
    my $res = 0;
    my $script = &dismb_prefs::get('MBQarTclScript');
    if ( $script ) {
	&dismb_msg::die ("Preference MBQarTclScript points to non-existing script \$1", $script) if ( ! -f $script );
	
	&dismb_msg::vrb ("Qar batch script in preference MBQarTclScript: \$1", $script);
        my $project = &dismb_prefs::get('MBQarHomeProject');
        if ( $project ) {
            $res = 1;
            &dismb_msg::vrb ("Qar home project in preference MBQarHomeProject: \$1", $project);
        } else {
            &dismb_msg::vrb ("No Qar home project in preference MBQarHomeProject");
        }
    } 

    return $res;
}


sub dismb_prefs::init_passes {
    #create new message group for init passes
    &dismb_msg::push_new_message_group("list of disMB passes");

    &dismb_prefs::verify_continue_value();

    # Analysis is implied by parse, for compatibility with the 7.4 release of DISIM.
    my($DISIM_old) = "$ENV{DISIM}" ne "" and (-e "$ENV{DISIM}/bin/admin/IMUtil.pl");
    my($do_analysis) = &dismb_prefs::get("MBdoPassAnalysis") ne "no"
		       && !&dismb_prefs::skip_pass("analysis");
    my($do_parse) = &dismb_prefs::get("MBdoPassParse") ne "no"
		    && !&dismb_prefs::skip_pass("parse");
    if (&dismb_prefs::get('MBgeneratePdf') eq "yes"
		and $do_parse and ! $do_analysis and $DISIM_old) {
	&dismb_msg::inf("Performing analysis in spite of options to the contrary, because \$DISIM=\$1 indicates an old version that relies on the parse pass providing analysis.", $ENV{DISIM});
	$do_analysis = 1;
    }

    my($ii) = 0;
    @dismb_prefs::passes[$ii ++ ] = "cache"    if ( &dismb_prefs::get("MBdoPassCache")    ne "no" && !&dismb_prefs::skip_pass("cache"));
    @dismb_prefs::passes[$ii ++ ] = "analysis" if ( $do_analysis );
    @dismb_prefs::passes[$ii ++ ] = "parse"    if ( $do_parse );
    @dismb_prefs::passes[$ii ++ ] = "update"   if ( &dismb_prefs::get("MBdoPassUpdate")   ne "no" && !&dismb_prefs::skip_pass("update"));
    @dismb_prefs::passes[$ii ++ ] = "finalize" if ( &dismb_prefs::get("MBdoPassFinalize") ne "no" && !&dismb_prefs::skip_pass("finalize"));
    @dismb_prefs::passes[$ii ++ ] = "dfa_init" ,
    @dismb_prefs::passes[$ii ++ ] = "dfa"      if ( &dismb_prefs::get("doBuildDFA")       eq "yes" );
    @dismb_prefs::passes[$ii ++ ] = "query"    if ( &dismb_prefs::init_query()                     );
    @dismb_prefs::passes[$ii ++ ] = "qar"      if ( &dismb_prefs::init_qar()                     );

    &dismb_msg::die ("All disMB passes skipped from Preferences.") if ( $ii == 0 ) ;
    &dismb_msg::inf ("disMB passes: \$1","@dismb_prefs::passes");

    #pop new message group created for init passes
    &dismb_msg::pop_new_message_group;

    return 1;
}

@PDF_ENVS = ("sharedSrcRoot", "sharedModelRoot", "privateSrcRoot", "privateModelRoot");
use File::Path;
sub dismb_prefs::init_env_vars {
    #create new message group for pdf environment variables
    &dismb_msg::push_new_message_group("PDF environment variables");

    local($key);
    &dismb_msg::inf ("PDF environment variables:");
    foreach $key (@PDF_ENVS) {
	local($pref_def) = &dismb_prefs::defined($key);
	local($pref_val) = &dismb_prefs::get($key);
	local($env_def)  = defined ($ENV{$key});
	local($env_val)  = $ENV{$key};

	if ( (! $env_def  && ! $pref_def) || ($env_val eq "" && $pref_val eq "") ) {
	    &dismb_msg::err ("\$1 is not defined neither in prefs nor with env variable",$key);
	    return 0;
	}
	local($val, $msg, $frm);
	$val = ""; $msg = "";
	if ( ! $pref_def || $pref_val eq "" ) {
	    $frm = "ENV : ";
	    $val  = $env_val;
	}
	if ( ! $env_def || $env_val eq "" ) {
	    $frm = "Pref: ";
	    $val = $pref_val;
	}
	if ($val eq "") {
	    $frm = "ENV : ";
	    $val = $env_val;
	    $msg = ( $env_val eq $pref_val ) ? "(same Prefs and ENV)" : "(hides pref $pref_val)";
	}
    
	&dismb_prefs::set ($key, $val) if ($val ne $pref_val);
	$ENV{$key} = $val if ($val ne $env_val);
	&dismb_print ("      \$1\$2 \t\$3 \t\$4",${frm},$key,$val,$msg);
        if(! -d $ENV{$key}) {
	    if((uc($key) eq "PRIVATEMODELROOT") || (uc($key) eq "PRIVATESRCROOT")) {
		&dismb_msg::msg ("Creating \$1 directory.",$ENV{$key});
		mkpath ("$ENV{$key}", 0, 0777);
		if (! -d $ENV{$key} ) {
		    &dismb_msg::err ("Failed to create \$1 directory.", $key);
		}
	    } else {
		&dismb_msg::wrn ("\$1 directory does not exist.",$key); 
	    }
	}
    }

    #pop new message group created for pdf environment variables
    &dismb_msg::pop_new_message_group;

    return 1;
}


sub dismb_prefs::is_continue_mb {
    if ( &dismb_prefs::get ('MBcontinue') eq "no" ) {
	return 0;
    } else {
	return 1;
    }
}

sub dismb_prefs::verify_continue_value()
{
    if ( &dismb_prefs::is_continue_mb() && !$dismb::standalone_mode ) {
	my($resume_pass) = &dismb_prefs::get('MBcontinue');
        if ( $resume_pass ne "yes" ) {
            my($found) = 0;
            my($p);
	    foreach $p (@dismb_prefs::resumeable_passes) {
		if ($p eq $resume_pass) {
		    $found = 1;
                    last;
                }
            }
            if ( $found ) {
                &dismb_msg::inf("Resume dismb at pass \$1.", $resume_pass);
            } else {
                &dismb_msg::die("Invalid pass \$1 specified for MBcontinue. Valid passes for continuing are: \$2", $resume_pass, "@dismb_prefs::resumeable_passes");
            }
        } else {
            &dismb_msg::inf("Resume dismb automatically.");
        }
    }
}

#Returns true if pass1 occurs before pass2 in the list of all passes
#(@dismb_prefs::resumeable_passes), returns false otherwise.
sub dismb_prefs::is_earlier_pass {
    my($pass1, $pass2) = @_;
    my($ret) = 0;
    if ($pass1 ne $pass2) {
        my($p);
	foreach $p (@dismb_prefs::resumeable_passes) {
            last if ($p eq $pass2); #pass2 occurs before pass1, return false
            if ($p eq $pass1) { #pass1 occurs before pass2, return true
                $ret = 1;
                last;
            }
        }
    }
    return $ret;
}

sub dismb_prefs::skip_pass {
    my($cur_pass) = $_[0];
    my($skip) = 0;
    if (&dismb_prefs::is_continue_mb()) {
        my($upto_date_file) = "$ENV{'DISBUILD_TMPDIR'}/$cur_pass.upto_date";
        my($resume_pass) = &dismb_prefs::get('MBcontinue');

        #If MBcontinue has "yes" as its value, then dismb will try to continue
        #where it left off. But if MBcontinue value is the name of one of the
        #passes, dismb will try to start with the specified pass.
        if ( $resume_pass eq "yes" ) { 
            if ( -f $upto_date_file ) {
	        &dismb_msg::inf("continue - pass \$1 is up to date.",$cur_pass);
	        $skip = 1;
            }
        } elsif ( !$dismb::standalone_mode) {
	    if (&dismb_prefs::is_earlier_pass($cur_pass, $resume_pass)) {
		#MBcontinue has the name of the pass to continue from
		if ( -f $upto_date_file ) {
		    &dismb_msg::vrb("continue - pass \$1 is up to date.", $cur_pass);
		    $skip = 1;
		} else {
		    $skip = 0;
		    &dismb_msg::die("Cannot continue from pass \$1, \$2 pass is incomplete.", $resume_pass, $cur_pass);
		}
	    } else {
		if ( -f $upto_date_file ) {
		    &dismb_msg::vrb("Up to date file for pass \$1 exists, removing it.", $cur_pass);
		    unlink($upto_date_file);
		}
	    }
        }
    } 
    return $skip;
}


sub dismb_prefs::init_logfile {
    local($txt, $logfile, $dir, $nm);


    if ( &dismb_prefs::get ('logFileCreate') eq "no" ) {
	$logfile = "$ENV{'DISBUILD_TMPDIR'}/build.log";
	$txt = "Work log: $logfile";
        &dismb_msg::inf("Work log: \$1 \$2",$logfile, (&dismb_prefs::is_continue_mb) ? "(Adding to existing log)" : "" );
	$txt = "Adding to " . $txt if (&dismb_prefs::is_continue_mb);
    } else {
	local($dir) = dismb_prefs::get ('logFileDir');
	$dir = "$ENV{'ADMINDIR'}/log" if ($dir eq "");
	eval '&dismb_lib::mkdir_path ($dir)';
	&dismb_msg::fixeval;
	&dismb_msg::die ("Failed to create directory \$1 \$2",$dir,$@) if ( $@ || ! -d $dir );
	if ( &dismb_prefs::get ('logFileNameFixed') eq "no" ) {
	    local($sec,$min,$hour,$mday,$mon,$year) = &dismb_lib::time2string ;
	    local($nm) = "$mon" . $mday . "-" . $hour . "_" . $min . "_" . $sec . ".log";
	    $logfile = "$dir/$nm";
	    $txt = "Permanent log: $logfile";
	    &dismb_msg::inf("Permanent log: \$1",$logfile);
	} else {
	    $logfile="$dir/$ENV{'USER'}_dismb.log";
	    $txt = "Fixed log: $logfile";
	    &dismb_msg::inf("Fixed log: \$1 \$2",$logfile, (&dismb_prefs::is_continue_mb) ? "(Adding to existing log)" : "");
	    $txt = "Adding to " . $txt if (&dismb_prefs::is_continue_mb);
	}
    }
    
    $dismb_prefs::logfile_msg = $txt;

    if ( &dismb_prefs::is_continue_mb ) {
	if ( ! open (LOGFILE,">>$logfile") ) {
	    &dismb_msg::err ("Could not create logfile \$1: \$2",$logfile,$!);
	    return 0;
	}
    } else {
	if ( ! open (LOGFILE,">$logfile") ) {
	    &dismb_msg::err ("Could not create logfile \$1: \$2",$logfile,$!);
	    return 0;
	}
    }

    select LOGFILE ; $| = 1; select STDOUT;

    $dismb_prefs::logfile = LOGFILE;
    return 1;
}


sub dismb_prefs::remote_env {
    return "NULL" if ( &dismb_prefs::get ('MBremoteEnvVarList') eq "" );

    local ($fenv) = "$ENV{'DISBUILD_TMPDIR'}/list_$$.env";
    local (*FENV, $v);
    &dismb_msg::vrb ("Saving environment in the file \$1", $fenv);
    open (FENV, ">$fenv") || &dismb_msg::die ("Failed to write file \$1: \$2",$fenv,$!);
    foreach $v ( split(/\s+/, &dismb_prefs::get ('MBremoteEnvVarList')) ) {
	if ( ! defined $ENV{$v} ) {
	    &dismb_msg::err ("Env variable \$1 is undefined",$v);
	} else {
	    local($val); ($val = $ENV{$v}) =~ s/([^\/A-Za-z0-9._-\s])/\\$1/g;
	    print FENV "\$ENV\{\'$v\'\} = \"", $val, "\";\n";
	}
    }
    close FENV;
    return $fenv;
}


sub dismb_prefs::remote_eval {
    local($rem_ev) = &dismb_prefs::get ('MBremoteEval');
    return "NULL" if ( $rem_ev eq "" );
    return $rem_ev;
}


@dismb_prefs::pmod_exts = ("sym", "ind", "lin", "met", "dfa", "prj");

#returns: 1 - succeeded; 0 - failed
sub dismb_prefs::init_config {
    return $dismb::config_initialized if ( defined $dismb::config_initialized );

    local($config) = &dismb_prefs::get("configuratorSystem");
    local($do_get) = &dismb_prefs::get("getModel");
    local($do_put) = &dismb_prefs::get("putModel");

    $config = "" if ($config eq "no_cm");
    if ( $config && $dismb::is_NT ) {
	&dismb_msg::wrn ("Ignore configuratorSystem= \'\$1\'  for Windows NT",$config);
	$config = "";
    } 

    if ( ! $config) {
	&dismb_msg::wrn ("Unset preference getModel: \$1", $do_get) if ($do_get && $do_get ne 'no');
	&dismb_msg::wrn ("Unset preference putModel: \$1", $do_put) if ($do_put && $do_put ne 'no');

	&dismb_prefs::set("configuratorSystem", "");
	&dismb_prefs::set("getModel", "no");
	&dismb_prefs::set("putModel", "no");
	&dismb_lib::write_build_info() if (! $dismb_prefs::remote_mb );
	$dismb::config_initialized = 1;
	return $dismb::config_initialized;
    }

    &dismb_msg::vrb ("configuratorSystem preference is set to: \$1", $config);
    $dismb::config_initialized = 0;
    my $pref_cfg_dir = &dismb_prefs::get("configuratorCustomDirectory");
    if ($pref_cfg_dir) {
	if (! defined $ENV{'PSETCFG'}) {
	    $ENV{'PSETCFG'} = $pref_cfg_dir;
	    &dismb_msg::vrb ("Set env PSETCFG to pref configuratorCustomDirectory \$1", $pref_cfg_dir);
	} else {
	    if ($ENV{'PSETCFG'} eq $pref_cfg_dir) {
		&dismb_msg::vrb ("Same env PSETCFG and pref configuratorCustomDirectory \$1", $pref_cfg_dir);
	    } else {
		&dismb_msg::vrb ("Env PSETCFG hides pref configuratorCustomDirectory PSETCFG= \$1 pref= \$2",$ENV{'PSETCFG'},$pref_cfg_dir);
	    }
	}
    }

    $ENV{'PSETCFG'} = ( defined $ENV{'PSETCFG'} ) ? $ENV{'PSETCFG'} : "$ENV{'PSETHOME'}/config/$config";
    &dismb_msg::die ("Wrong preference configuratorSystem: \$1 directory \$2 does not exist.",$config,$ENV{'PSETCFG'}) if ( ! -d $ENV{'PSETCFG'} );

    #globals:
    $dismb_prefs::change2write = "$ENV{'PSETCFG'}/cm_change_to_write";
    &dismb_msg::die ("Wrong preference configuratorSystem: \$1 can not execute script \$2",$config,$dismb_prefs::change2write) if (! -x "$dismb_prefs::change2write");

    &dismb_prefs::set("getModel", 'no') if ( $do_get ne "yes" && $do_get ne "no" ); 
    &dismb_prefs::set("putModel", 'no') if ( $do_put ne "yes" && $do_put ne "no" ); 

    &dismb_msg::inf ("Configurator preferences: CMsystem= \$1 getModel= \$2 putModel= \$3",$config,$do_get,$do_put);
    &dismb_lib::write_build_info() if (! $dismb_prefs::remote_mb );
    $dismb::config_initialized = 1;
    return $dismb::config_initialized;
}

sub dismb_prefs::is_fork_supported {
    return 1 if ( ! $dismb::is_NT && &dismb_prefs::get('MBavoidFork') eq "no" );
    return 0;
}

return 1;
