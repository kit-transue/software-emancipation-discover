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
sub dismb_local::prepare {
    local($flock) = "$ENV{'DISBUILD_TMPDIR'}/" . $pass . "_lock";
    unlink ($flock);
    &dismb_msg::vrb ("Pass \$1, lockfile \$2",$pass,$flock);
}


sub dismb_local::finalize_before {
    return if ( ! &dismb_prefs::get("configuratorSystem") );

    if (&dismb_prefs::get("getModel") ne "yes") {
	return;
    }

    &dismb_msg::vrb ("Preference getModel set to \'yes\'. Checking out PRJ files");
    local($list) = "$ENV{'DISBUILD_TMPDIR'}/ROOTS.pmod.lst";
    local(*ROOTS);
    open (ROOTS, "<$list") || &dismb_msg::die ("Finalize: missing ROOTS file \$1",$list);
    while (<ROOTS>) {
	local(@proj) = split (' ', $_); local ($f);
	($f = @proj[5]) =~ s/sym_FLDX_[0-9]+$/prj/;
	if (! -f $f ) {
	    &dismb_print ("      PRJ \$1 does not exist; skip checkout",$f);
	} else {
	    if ( -w $f ) {
		&dismb_print ("      PRJ \$1 is writable; skip checkout",$f);
	    } else {	    
	      &dismb_local::change2write();
	    }
	}
    }    
}


sub dismb_local::update_before {
    if ( !$dismb::standalone_mode ) {
       require ("dismb_parse.pl");
       &dismb_msg::vrb ("Preparing for update pass");
       if (&dismb_prefs::is_continue_mb()) {
	   #In continue mode, set root project name
           if ($dismb_analysis::project_name eq "") {
	       my(@projects) = &dismb_lib::project_list('root');
	       $dismb_analysis::project_name = $projects[0];
           }
           #Delete psets that were created in the previous aborted "update"
           #phase forcing recreation of psets so that pmods have complete
           #information.
	   my($filearr_ref) = &dismb_lib::find_files_with_pattern("$ENV{'ADMINDIR'}/model/pset", ".pset.iff");
	   my($fl, $psetname);
	   &dismb_msg::inf("Cleaning up previous pset files.") if ($#$filearr_ref >= 0);
	   foreach $fl (@$filearr_ref) {
               ($psetname = $fl) =~ s%.iff$%%;
               if (-f $psetname) {     #pset exists
		   &dismb_msg::vrb("Deleting pset file: $psetname");
		   unlink($psetname);
               }
	   }
       }
       #generate PDF
       my($src_root) = ($dismb::is_NT) ? "C:/" : "/";
       &dismb_parse::generate_pdf($src_root, $dismb_analysis::project_name);
    } 

    local($list) = "$ENV{'DISBUILD_TMPDIR'}/PMODS.pmod.lst";
    local(*PMODS);
    open (PMODS, "<$list") || &dismb_msg::die ("Missing PMODS file \$1",$list);
    while (<PMODS>) {
	local(@proj) = split (' ', $_); local ($f);
	$f = @proj[$#proj];
        if ($f =~ /\"$/ && $f !=~ /^\"/) {
          my($ind) = $#proj;
          WHLOOP: while (--$ind > 0) {
            $f = "@proj[$ind] " . $f;
            last WHLOOP if (@proj[$ind] =~ /^\"/);
          }
          $f =~ s/\"//g;
        }
	$f =~ s/sym_FLDX_[0-9]+$/dfa/;

	if ( -f $f ) {
	    &dismb_msg::vrb ("Cleanup DFA files ...");
	    if ( -w $f ) {
		&dismb_msg::vrb ("\$1 is writable;", $f);
	    } else {	    
		&dismb_local::change2write();
	    }
	    &dismb_msg::vrb ("unlinking \$1", $f);
	    unlink ($f);
	}
    }    
}

sub dismb_local::update_after {
  &dismb_analysis::save_file_attributes();
}

#initialize global @pass_arr and %pass_projects
sub dismb_local::init_parallel {
    local($level) = @_; $level = "pmod" if ($level ne "root");
    local($f)  = ($level eq "pmod") ? "$ENV{'DISBUILD_TMPDIR'}/PMODS.pmod.lst" : "$ENV{'DISBUILD_TMPDIR'}/ROOTS.pmod.lst"; 
    local(*LLL);
    open (LLL, "<$f") || &dismb_msg::die ("Could not read list \$1: \$2",$f,$!);
    local($idx) = 0;
    while ( <LLL> ) {
	@pass_arr[$idx ++] = $_;	                 # not local
    }
    close LLL;
    if ($#pass_arr > 0) {
	&dismb_msg::vrb ("Pass \$1, level \$2, parallelized by \$3",$pass,$level,$f);
    }

    if ($pass eq 'update') {
	require ("dismb_parse.pl");
	local($pfile);		# init @pass_projects
	$pfile = $parsed_projects if ($pass eq 'update');
	if (! -f $pfile) {
            if ( !$dismb::standalone_mode ) {
              $pass_projects{$dismb_analysis::project_name} = $dismb_analysis::project_name;
            } else {
	      &dismb_msg::err ("Missing summary file \$1", $pfile);
            }
	} elsif ( -z $pfile ) {
	    &dismb_msg::err ("Zero length summary file \$1", $pfile);
	} elsif ( ! -z $pfile ) {
	    if ( ! open (LLL, "<$pfile") ) {
		&dismb_msg::err ("Cannot open summary file \$1", $pfile);
	    } else {
		local($pn);
		&dismb_msg::vrb ("projects to process:");
		while ($pn = <LLL>) {
		    chop $pn;
		    local($pmod, $root) = split ("\" \"", $pn);
		    $pmod =~ s/^\"//; $root =~ s/\"$//;
		    if ($level eq "pmod") {
			$pass_projects{$pmod} = $root;
			&dismb_msg::vrb ("pmod: \$1; root: \$2",$pmod,$root);
		    } else {
			if (!defined $pass_projects{$root}) {
			    &dismb_msg::vrb ("root: \$1",$root);
			    $pass_projects{$root} = $pass_projects{$root};
			} else {
			    $pass_projects{$root} = $pass_projects{$root} . " " . $pmod;
			}
		    }
		}
	    }
	}
    }

    return 1;
}


sub dismb_local::get_next_project_line {
    local($flock) = "$ENV{'DISBUILD_TMPDIR'}/" . $pass . "_lock";
    local($ln)    = &dismb_lib::get_locked_linenum ("$flock");
    local($ind)   = $ln - 1;
    local($txt)   = @pass_arr[$ind];
    return $txt;
}


# predefined $pn, $pn_line
sub dismb_local::writable_pmods {
    local($cm)      = &dismb_prefs::get("configuratorSystem");
    local($scratch) = (&dismb_prefs::get("MBscratch") eq "yes");
    local($co)      = (&dismb_prefs::get("getModel")  eq "yes");

    local($file) = @_; local ($ext);
    $file =~ s/sym_FLDX_[0-9]+$//;
    local($foutd) = $file . "outdated_pset";
    if ( -f $foutd ) {
	&dismb_msg::wrn ("Deleting existing file \$1", $foutd);
	unlink ( "$foutd" );
    }

    if ($cm && ! $co) {
	&dismb_msg::vrb ("Preference getModel set to \'no\'. Skip checking out pmod files project \$1 model \$2",$pn,$file);
    }

  EXTS: foreach $ext (@dismb_prefs::pmod_exts) {
      next EXTS if ($ext eq "prj"); # finalize pass ci/co the PRJs
      next EXTS if ($ext eq "dfa"); # dfa_init pass ci/co the DFAs
      local($f) = $file . $ext;
      if ( -f $f ) {
	  &dismb_msg::vrb ("\$1 file exists, check whether it is writable",$ext);
	  if ( -w $f ) {
	      if ($cm && $co) {
		  &dismb_msg::vrb ("\$1 is writable; skip checkout",$ext);
	      } else {
		  &dismb_msg::vrb ("\$1 is already writable;",$ext);
	      }
	  } else {	    
	      &dismb_local::change2write();
	  }

	  if ( $scratch ) {
	      unlink ( "$f" );
	      &dismb_msg::vrb ("Scratch build; delete \$1", $f);
	  }
      }
  }
}

#predefined $f
sub dismb_local::change2write {
    &dismb_msg::die ("Cannot change readonly mode without configuratorSystem $f") if ( !defined $dismb_prefs::change2write );
    eval '&dismb_lib::system_bkgd (0, &dismb_lib::argstr_to_list($dismb_prefs::change2write), $f)';
    &dismb_msg::fixeval;
    &dismb_msg::die ( "Failed to checkout $f: $@") if $@;
}

sub dismb_local::checkin {
    if (&dismb_prefs::get("putModel") ne "yes") {
	&dismb_msg::inf ("putModel set to \'no\'. Skip checking in Model files root project \$1", $pn);
	return;
    }

    (my($exe) = "$ENV{'PSETCFG'}/cm_PutModel") =~ s%\\%/%g;

    if ( ! -x $exe ) {
	&dismb_msg::err ("No executable \$1. Skip checking in Model files.",$exe);
	return;
    }

    $ENV{'PATH'} = "$ENV{'PATH'}" . ":" . "$psethome_bin";
    &dismb_msg::vrb ("\$1", $ENV{'PATH'});

    &dismb_lib::system_bkgd (0, $exe);
}


sub dismb_local::env_dump_csh {
    my $fff = @_[0];

    open FOUT, ">$fff"   or &dismb_msg::die ("env_dump_csh: Failed to write $fff: $!");
    open ENV, "env |"    or &dismb_msg::die ( "Can not execute env: $!");
    my $line;

    while ($line = <ENV>) {
	my @arr = split ("=", $line);
	if ( ($arr[0] =~ /Root/) || ($arr[0] =~ /[Dd][Ii][Ss]/) || ($arr[0] =~ /PSET/) or ($arr[0] =~ /PTEST/) ) {
	    print FOUT "setenv $arr[0] $arr[1]";
	}
    }
    close FOUT;
}


# predefined $pn_line @discover_cmd 
sub dismb_local::update_one_proj {
    local(@proj) = split (' ', $pn_line);

    local($pn) = @proj[2];
    $pn =~ s/^\s+//;
    $pn =~ s/\s+$//;

    if ($pass eq "dfa_init") {
	eval '&dismb_lib::system_bkgd (0, @discover_cmd, "-dfa_init", $pn)';
	&dismb_msg::fixeval;
	&dismb_msg::die ( "$@" ) if $@;
    } elsif ( ! defined $pass_projects{$pn} ) {
	&dismb_msg::inf ("The project \$1 is not needed for pass \$2. Skipping",$pn,$pass);
    } else {
	if ( $pass eq "update" ) {
	    &dismb_local::writable_pmods (@proj[5]);
#	    &dismb_local::env_dump_csh ( "/usr/tmp/env_csh.out" );
            my @remove_option = ( $dismb::incremental_modelbuild ) ? ("-remove") : () ;
	    eval '&dismb_lib::system_bkgd (0, @discover_cmd, "-update", @remove_option, $pn)';
	    &dismb_msg::fixeval;
	    &dismb_msg::die ( "$@" ) if $@;
	}
    }
}


sub dismb_local::run_discover {   
#   $ENV{'PSET_HALT'}    = 1800; #debug
    local($host,$cpu)    = @_;
    local($pass)         = $ENV{'DISBUILD'} ;
    local($discover_exe) = ( $dismb::is_NT ) ? "model_server.exe" : "model_server";
    my($pdfvalue)             = &dismb_prefs::get('pdfFileBuild');
    #"pdfFileBuild" preference can have more than one pdf file listed
    my(@pdfs) = split("\\.pdf", $pdfvalue);

    local(@discover_cmd) = ($discover_exe, "-batch");

    #Add each pdf file to the invocation line separately
    foreach $p (@pdfs) {
        $p .= "\.pdf"; $p =~ s/^\s+//;
        @discover_cmd = (@discover_cmd, "-pdf", $p);
    }
    @discover_cmd = (@discover_cmd, "-prefs", &dismb_prefs::get_mbPrefs_file);

    push @discover_cmd, &dismb_msg::lp_flags();

    if ( &dismb_prefs::get('MBptestRunning') eq "yes" ) {
	$ENV{'t_sysPrefs'} = "-sysPrefs /job1/prefs/asetTest.prefs.$ENV{'Arch'} -noUserPrefs" if ( ! defined $ENV{'t_sysPrefs'} );
	$ENV{'PSET_TEST_RUNNING'} = "yes";
    }

    if (&dismb_prefs::is_verbose_mb()) {
        $ENV{'DISCOVER_VERBOSE'} = "yes";
    }

    my (@opt);
  RUN: {
      @opt=("-save_proj")  , last RUN   if ( $pass =~ /^cache/    ); 
      @opt=("-analysis")   , last RUN   if ( $pass =~ /^analysis/ );
      @opt=("-update")     , last RUN   if ( $pass =~ /^update/   );
      @opt=("-save_proj")  , last RUN   if ( $pass =~ /^finalize/ );
      @opt=("-dfa_init")   , last RUN   if ( $pass =~ /^dfa_init/ );

      @opt=("-dfa", &dismb_lib::project_list('pmod'))      , last RUN   if ( $pass =~ /^dfa/ );
      @opt=("-home", &dismb_prefs::get('MBQarHomeProject'), "-source", &dismb_prefs::get('MBQarTclScript')) , last RUN   if ( $pass =~ /^qar/);
      @opt=("-source", &dismb_prefs::get('MBqueryTclScript')) , last RUN   if ( $pass =~ /^query/);
      
      @opt=()              , last RUN   if ( $pass =~ /^checkin/);
      &dismb_msg::die ("Wrong pass value DISBUILD=\$1",$pass);
  }

    if ( ! defined $dismb_prefs::change2write && ! &dismb_prefs::init_config ) {
	&dismb_msg::die ("Failed to initialize config.");
    }

    if ( $pass eq "update" || $pass eq "dfa_init" ) {
	local($level) = ($pass eq "update" || $pass eq "dfa_init") ? "pmod" : "root";
	&dismb_local::init_parallel ($level);
	if ($pass eq "update" && ! defined %pass_projects) {
	    &dismb_msg::wrn ("There are no projects to process");
	} else {
	    local($pn_line)  = &dismb_local::get_next_project_line;
	    while ( $pn_line ) {
		&dismb_local::update_one_proj; # predefine  $pn_line
		$pn_line = &dismb_local::get_next_project_line;
	    }
	    undef @pass_arr;
	    undef %pass_projects;	# global array
	}
    } elsif ($pass eq "checkin") {
	&dismb_local::checkin();
    } else {
	eval '&dismb_lib::system_bkgd (0, @discover_cmd, @opt)';
	&dismb_msg::fixeval;
	&dismb_msg::die ("Running model server failed: \$1", $@) if $@;
    }
}


sub dismb_local::run_pass {
    local($host,$cpu)  = @_;
    local($cleanup) = (&dismb_prefs::get("MBforceCleanup") eq "yes") ? 1 : 0;
    &dismb_msg::set_host_cpu ($host, $cpu);    

    local($pass)       = $ENV{'DISBUILD'};
    local($tmp_logdir) = "$ENV{'DISBUILD_TMPDIR'}/mblog_$pass";
    local($passlog)    = "$tmp_logdir/${host}_$cpu.log";
    local($tmp_tmpdir) = "$ENV{'TMPDIR'}/mbtmp_${pass}__${host}_${cpu}.$$";
    &dismb_msg::wrn ("Temp directory \$1 already exists",$tmp_tmpdir) if ( -d $tmp_tmpdir );
    eval '&dismb_lib::mkdir_path ($tmp_tmpdir)';
    &dismb_msg::fixeval;
    &dismb_msg::err ("Failed to create temp dir \$1 $\2", $tmp_tmpdir ,$@) if ( ! -d $tmp_tmpdir || $@ );

    open (SAVESTDOUT, ">&STDOUT");
    open (SAVESTDERR, ">&STDERR");
    local($savelog)     = $dismb_prefs::logfile;
    local($save_tmpdir) = $ENV{'TMPDIR'};

    open (STDOUT,">$passlog") || &dismb_msg::die ("Failed to write pass log \$1: \$2",$passlog,$!);
    open (STDERR,">&STDOUT")  || &dismb_msg::die ("Cannot duplicate STDOUT: \$1",$!);
    $dismb_prefs::logfile = "";	        # disable regular log file descriptor. Use only STDOUT
    $ENV{'TMPDIR'}        = $tmp_tmpdir;
    
    select (STDERR); $| = 1;
    select (STDOUT); $| = 1;
    
    &dismb_msg::print_header ("local $pass");
    &dismb_remote::start_log if ($dismb_prefs::remote_mb); # initializes Preferences

    if ($pass eq "parse") {
	require ("dismb_parse.pl");
	eval '&dismb_parse::run ($host, $cpu)';
    } elsif ($pass eq "analysis" && !$dismb::standalone_mode) {
        eval '&dismb_analysis::compile_outdated_filelist()';
    } else {
	eval '&dismb_local::run_discover ($host, $cpu)';
    }
    &dismb_msg::fixeval;
    my($die_msg) = $@;

    &dismb_msg::print_footer ("local $pass");

    close (STDOUT);
    close (STDERR);
    
    open (STDOUT, ">&SAVESTDOUT");
    open (STDERR, ">&SAVESTDERR");
    $dismb_prefs::logfile = $savelog;
    $ENV{'TMPDIR'}        = $save_tmpdir;

    if ( $cleanup ) {
	eval '&dismb_lib::unlink ("$tmp_tmpdir")';
	&dismb_msg::fixeval;
	&dismb_msg::wrn ("Failed deleting \$1: \$2",$_, $@) if ($@) ;
    }

    &dismb_msg::die("\$2 pass failed with: \$1",$die_msg, ucfirst $pass) if $die_msg;
}


sub dismb_local::analysis_after {
    &dismb_msg::die ("\$1 was not created during pass analysis.",$prs_file) if ( -f $prs_file && ! &dismb_lib::is_created_after_start($prs_file) );
}

return 1;
