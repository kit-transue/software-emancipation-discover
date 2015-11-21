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
%disch::imp2client;		# client files hashed by implicit names (input table)
%disch::imp2all;		# shared files hashed by implicit names
%disch::imp2flags;		# shared files flags hash by implicit names
%disch::imp2server;		# server files hashed by implicit names (local PDF)
%disch::imp2succeeded;		# Generated PDF, Flags and MapTable server files hashed by imp name
%disch::record;                 # submitted files hashed by implicit names

#returns: 1 - success, 0 - failure
#Line format: "Implicit(left) name"	"Temp Physical Name"    "Logical(right) Name"
sub disch::add_imp2files {
    my($line) = @_; my($ret) = 0;
    my($imp,$tfn,$ln) =  grep !/^\s*$/, split /\"/, $line;
    if ( !$imp || !$ln || !$tfn) {
	&dismb_msg::err ("Line is incomplete: \$1", $line);
	return $ret;
    }
    if ( ! -r $tfn ) {
	&dismb_msg::err ("Cannot read source file: \$1", $tfn);
	return $ret;
    }

    $disch::imp2client {$imp} = $line; $ret++ ;
    return $ret;
}

# build_server
# ...
# FileNumber:	<number>
#TableStart
#"Implicit(left) name"	"Temp Physical Name"    "Logical(right) Name"
#...
#TableEnd

#returns: number of loaded files on success, 0 - failure
sub disch::process_fileTable {
    my($ret,$fcnt,$cnt) = (0,0,0);
    my($fn) = &dismb_options::get ( '-SCfileTable' ) ;
    if ( ! $fn ) {
	&dismb_msg::err ("No fileTable filename specified.");
	return $ret;
    }
    local (*TFN);
    if ( ! open TFN, "<$fn" ) {
	&dismb_msg::err ("Failed to read file \$1: \$2",$fn,$!);
	return $ret;
    }

    my($in_build_server,$in_table,$lnumber,$fcnt,$cnt) = (0,0,0,0,0); my($line);
    &dismb_msg::inf ("Processing file table \$1",$fn);
  READ_TFN: while ( $line = <TFN> ) {
      $lnumber++ ;
      if ($in_table) {
	  last READ_TFN if ( $line =~ /^\#TableEnd/ );
	  next READ_TFN if ( $line =~ /^\#/ );
	  $cnt++;
	  if ( ! &disch::add_imp2files ($line) ) {
	      &dismb_msg::err ( "Failed to access a file  table \$1","${fn}:${lnumber}"); 
	      next READ_TFN;
	  }
	  $fcnt++; next READ_TFN;
      }

      if ($in_build_server) {
	  $in_table++ if ( $line =~ /^\#TableStart/);
	  next READ_TFN;
      }

      $in_build_server++ if ( $line =~ /^\s*\# build_server/ );
      next READ_TFN;
  }

    my($s) = ($fcnt == 1) ? "" : "s";
    close TFN;
    if ($fcnt && $fcnt == $cnt) {
	&dismb_msg::inf ("\$1 file\$2 found, table \$3",$fcnt,$s,$fn);
    } else {
	&dismb_msg::inf ("\$1 file\$2 found out of \$3 specified. Aborting ...",$fcnt,$s,$cnt);
	$fcnt = 0;
    }
    return $fcnt;
}

# returns: 0 - can read, -1 file does not exist, -2 cannot read
sub disch::check_read_access {
    my($ret) = 0;
    my($f, $msg) = @_;
    if ( ! -f $f ) {
	&dismb_msg::err ("Does not exist \$1 file \$2",$msg,$f);
	$ret = -1;
    } elsif ( ! -r $f ) {
	&dismb_msg::err ("Could not read \$1 file \$2",$msg,$f);
	$ret = -2;
    }
    return $ret;
}

#returns: 1 - success, 0 - failure
sub disch::check_admindir {
    my($ret) = 0;
    $disch::sh_admin = &dismb_options::get ( '-SCadmindir' ) ;
    if ( ! -d $disch::sh_admin ) {
	&dismb_msg::err ("No shared AdminDir directory \$1", $disch::sh_admin);
	return $ret;
    }

    &dismb_msg::inf ("Checking shared ADMINDIR \$1",$disch::sh_admin);

    $disch::sh_tmpdir = "$disch::sh_admin/tmp/disbuild";
    if ( ! -d $disch::sh_tmpdir ) {
	&dismb_msg::err ("No shared dismb temporary directory \$1",$disch::sh_tmpdir);
	return $ret;
    }

    $disch::sch_prefs    = "$ENV{'DISMBDIR'}/disch.prefs";
    $disch::sh_prefs     = "$disch::sh_tmpdir/mb_envs.prefs";
    $disch::all_lst      = "$disch::sh_tmpdir/all.pmod.lst";
    $disch::sh_flags     = "$disch::sh_tmpdir/new_reuse.flg";

    return $ret if ( &disch::check_read_access ($disch::sch_prefs,    "default disch Preferences") );
    return $ret if ( &disch::check_read_access ($disch::sh_prefs,     "shared dismb Preferences") );
    return $ret if ( &disch::check_read_access ($disch::all_lst,      "shared dismb modules") );
    return $ret if ( &disch::check_read_access ($disch::sh_flags,     "shared dismb flags") );

    $ret = 1;
    return $ret;
}


#returns: number of loaded files - success, 0 - failure
sub disch::load_shared_files {
    my($ret) = 0; local(*ALL);
    if ( ! open ALL, "<$disch::all_lst" ) {
	&dismb::msg_err ("Failed to open $disch::all_lst: $!");
	return $ret;
    }
    my($line,$lcnt,$fcnt) = ("", 0, 0);
    while ($line = <ALL>) {
	$lcnt++ ;
	next if ( $line =~ /^\s*\#/ );
	next if ( $line =~ /^\s*$/ );

	my(@arr) = &dismb_lib::split_prj_line ($line);
	if ( $#arr != 5 ) {
	    &dismb_msg::err ("Wrong PRJ format, file \$1:\$2 line: \$3",${disch::all_lst},$lcnt,$line);
	    close ALL;
	    return $ret;
	}
	my($imp) = $arr[2]; $disch::imp2all{$imp} = $line; $fcnt++;
    }
    close ALL;
    if ( $fcnt) {
	$ret = 1;
	my($s) = ($fcnt == 1) ? "" : "s";
	&dismb_msg::inf ("\$1 shared module\$2 loaded, file \$3",$fcnt,$s,$disch::all_lst);
    } else {
	&dismb_msg::err ("Failed to load shared modules file \$1",$disch::all_lst);
    }

    return $fcnt;
}

#returns: number of loaded flags - success, 0 - failure
sub disch::load_shared_flags {
    my($ret) = 0; local(*FLAGS);
    if ( ! open FLAGS, "<$disch::sh_flags" ) {
	&dismb::msg_err ("Failed to open $disch::sh_flags: $!");
	return $ret;
    }
    my($line,$lcnt,$fcnt) = ("", 0, 0);
    while ($line = <FLAGS>) {
	$lcnt ++ ;
	next if ( $line =~ /^\s*\#/ );
	next if ( $line =~ /^\s*$/ );

	if ( $line =~ /\"([^\"]+)\"\s+(.*)$/ ) {
	    my($imp) = $1;
	    $disch::imp2flags{$imp} = $2;
	    $fcnt++;
	} else {
	    &dismb_msg::err ("Wrong FLAGS format, file \$1:\$2 line: \$3",${disch::sh_flags},$lcnt,$line);
	    close FLAGS;
	    return $ret;
	}
    }
    close FLAGS;
    if ( $fcnt) {
	$ret = 1;
	my($s) = ($fcnt == 1) ? "" : "s";
	&dismb_msg::inf ("\$1 shared set\$2 of compiler flags, loaded file \$3",$fcnt,$s,$disch::sh_flags);
    } else {
	&dismb_msg::err ("Failed to load shared flags file \$1",$disch::sh_flags);
    }
    return $fcnt;
}


#returns: 1 - success, 0 - failure
sub disch::create_localRoot {
    my($ret) = 0;
    $disch::localRoot = &dismb_options::get ( '-SClocalRoot' ) ;
    if ( ! $disch::localRoot ) {
	&dismb_msg::err ("Option -SClocalRoot is missing");
	return $ret;
    }

    if ( -d $disch::localRoot ) {
	&dismb_msg::inf ("Using existing local root directory \$1",$disch::localRoot);
    } else {
	&dismb_msg::inf ("Creating local root \$1",$disch::localRoot);
	eval '&dismb_lib::mkdir_path ($disch::localRoot)';
	if ( ! -d $disch::localRoot ) {
	    &dismb_msg::fixeval;
	    &dismb_msg::err ("Failed to create local root \$1 \$2",$disch::localRoot, $@);
	    return $ret;
	}
    }

    $disch::sharedSrcRoot  = "$disch::localRoot/sharedSrcRoot";
    $disch::tmpdir         = "$disch::localRoot/tmp";

    eval '&dismb_lib::mkdir_path ($disch::sharedSrcRoot)' if ( ! -d $disch::sharedSrcRoot );
    if ( ! -d $disch::sharedSrcRoot ) {
	&dismb_msg::fixeval;
	&dismb_msg::err ("Failed to create local sharedSrcRoot: \$1 \$2",$@,$disch::sharedSrcRoot);
	return $ret;
    }

    eval '&dismb_lib::mkdir_path ($disch::tmpdir)' if ( ! -d $disch::tmpdir );
    if ( ! -d $disch::tmpdir ) {
	&dismb_msg::fixeval;
	&dismb_msg::err ("Failed to create local root temp directory: \$1 \$2", $@, $disch::tmpdir);
	return $ret;
    }
    $ret = 1;
    return $ret;
}


#returns: number of copied files - success, 0 - failure
sub disch::copy_sources_into_localRoot {
    my($shared_submit_dir) =  "$disch::sh_admin/submissions";
    my($submit_dir) = "$disch::tmpdir/submissions";

    if ( -d $submit_dir ) {
	eval '&dismb_lib::unlink($submit_dir)';
	&dismb_msg::fixeval;
	&dismb_msg::die("Failed to unlink \$1 err: \$2",$submit_dir,$@) if ($@); 
    }

    if ( -d $shared_submit_dir) {
	&disch::lock_submission($shared_submit_dir);
	my($copy_result) = eval '&dismb_lib::xcopy($shared_submit_dir,$submit_dir)';
	&dismb_msg::fixeval;
	my($copy_status) = $@;
	&disch::unlock_submission($shared_submit_dir);
	&dismb_msg::die("Failure copying sources: \$1",$copy_status) if (!$copy_result);
    }

    my($el, $fcnt, $cnt) = (0,0,0);
    &dismb_msg::inf("Copying source files into local sharedSrcRoot");
    my($records) = "$submit_dir/records";
    local(*RC);
    my($rec_line);
    my($num_of_records) = 0;
    if (open(RC,"<$records")) {
	while($rec_line = <RC>) {
	    my($key) =  grep !/^\s*$/, split /\"/, $rec_line;
	    if ($key) {
		$num_of_records++;
		my $tail_name = "noname";
		if ( $key =~ /(.*)\/([^\/]+)$/ ) {
		    $tail_name = $2;
		}
		$disch::record{$key} = "$submit_dir/$num_of_records/$tail_name";
	    }
	}
	close (RC);
    }

    foreach $el (sort(keys(%disch::imp2client))) {
	# ADD FILE SIZE for some reason large files are not finished writing.  Have not yet found async process 
        # responible.  Could it just be OS flushing that is delayed?
	my($imp,$tfn,$ln,$fsize) =  grep !/^\s*$/, split /\"/, $disch::imp2client{$el};
	$tfn =~ s%\\%/%g;
	if ( $tfn =~ /(.*)\/([^\/]+)$/ ) {
	    my($dirname,$basename) = ($1, $2);
	    if (!$dirname || !$basename) {
		&dismb_msg::err ("Failed to extract basename for file \$1", $tfn);
		return 0;
	    }
	    $cnt++;
	    $dirname =~ s%[/:]%_%g; $dirname =~ s/^_//;
	    my($phy) = "$disch::sharedSrcRoot/$dirname/$basename";
	    my($m) = ( -f $phy ) ? "replacing" : "creating" ;
	    &dismb_print ("      \$1\t\$2",$m,$phy);
	    &dismb_lib::create_dir_for_file ($phy) if ( ! -f $phy );
	    if ( &dismb_lib::ncopy ($tfn, $phy, $fsize) ) {
		my($line)= "\"$imp\" \"$phy\" \"$ln\" \"fsize\"";
		$disch::imp2server{$imp} = $line;
		$fcnt++;
		if (defined($disch::record{$imp})) {
		    my($stored) = $disch::record{$imp};
		    &disch::restore_record($phy,$tfn,$stored);
		} else {
		    &disch::clear_record($phy,$tfn);
		}
	    } else {
		&dismb_msg::err ("Failed \$1 file \$2", $m, $phy);
	    }
	}
    }
    my($s) = ($fcnt == 1) ? "" : "s";
    if ($fcnt && $fcnt == $cnt) {
	&dismb_msg::inf("\$1 file\$2 copied into local sharedSrcRoot",$fcnt,$s);
	return $fcnt;
    } else {
	&dismb_msg::err("\$1 file\$2 out of \$3 copied into local sharedSrcRoot. Aborting ...",$fcnt,$s,$cnt);
	return 0;
    }
}
#returns: 0 - success, $! - error message
sub disch::open_file {
    local($fn,$tag,*FF) = @_;
    my($act) = ( -f $fn ) ? "replacing" : "creating";

    &dismb_print ("      \$1 \$2 \$3",$act,$tag,$fn);
    if (! open FF, ">$fn") {
	&dismb_msg::err ("Failed \$1 \$2 \$3: \$4",$act,$tag,$fn,$!) ;
	return -1;
    }
    return 0;
}


#returns 0 on failure; 1 on success
sub disch::prefs_expand_ADMINDIR {
    my ($inf,$outf,$adir) = @_;
    local(*INF,*OUTF); my $ret = 0;

    if ( !open (INF, "<$inf") ) {
	&dismb_msg::err ("Failed to read Prefs \$1:  \$2",$inf,$!);
    } elsif ( !open (OUTF,">$outf") ) {
	&dismb_msg::err ("Failed to write Prefs \$1: \$2",$outf,$!);
    } else {
	my $line; $ret = 1;
	while ($line = <INF>) {
	    $line =~ s/\$ADMINDIR/$adir/g;
	    print OUTF $line;
	}
    }
    close INF; close OUTF;
    return $ret;
}


# install AdminDir/prefs/build.prefs and AdminDir/prefs/add.prefs
# returns: amount of installed files on success, 0 on failure
sub disch::install_prefs {
    my($ret) = 0;
    &dismb_msg::inf ("Installing dismb Preferences");
    my($s) = ( -f $disch::build_prefs ) ? "replacing" : "installing";
    &dismb_print ("      \$1 disch build.prefs",$s);
    &dismb_print ("        from: \$1",$disch::sh_prefs);
    &dismb_print ("        to: \$1\n",$disch::build_prefs);

    return $ret if ( ! &disch::prefs_expand_ADMINDIR($disch::sh_prefs,$disch::build_prefs, $disch::sh_admin) );

    $s = ( -f $disch::add_prefs ) ? "replacing" : "installing";
    &dismb_print ("      \$1 disch add.prefs",$s);
    &dismb_print ("        from: \$1",$disch::sch_prefs);
    &dismb_print ("        to: \$1\n",$disch::add_prefs);

    return $ret if ( ! &dismb_lib::copy ($disch::sch_prefs,$disch::add_prefs) );

    $ret = 2;
    return $ret;
}


sub disch::set_envs {
    &dismb_msg::inf ("Setting environment variables:");

    $ENV{'ADMINDIR'}        = $disch::dismb_admindir;
    &dismb_print ("            ADMINDIR \t\$1",${disch::dismb_admindir});

    $ENV{'sharedSrcRoot'}   = $disch::sharedSrcRoot;
    &dismb_print ("       sharedSrcRoot \t\$1",${disch::sharedSrcRoot});

    $ENV{'sharedModelRoot'} = $disch::sharedModelRoot;
    &dismb_print ("     sharedModelRoot \t\$1",$disch::sharedModelRoot);

    $ENV{'privateSrcRoot'}   = $disch::sharedSrcRoot;
    &dismb_print ("      privateSrcRoot \t\$1",${disch::sharedSrcRoot});

    $ENV{'privateModelRoot'} = $disch::sharedModelRoot;
    &dismb_print ("    privateModelRoot \t\$1",$disch::sharedModelRoot);
}


# Processes Rules.pmod.lst and adds traslation from mapping into local name
# substituting the original mapped shared filename with a local filename.
#
# Takes 2 references:
#    \*MAP      - reference to file handle to store file translation
#    \%locals   - reference to hash table of shared<->local translation hashed by shared name
#
# returns amount of added translations
sub disch::add_mapping_rules {
    my($map,$hash) = @_; my $rules = "$disch::sh_tmpdir/RULES.pmod.lst";
    my ($lall,$lcnt, $rcnt, $line) = (0, 0, 0, ""); local(*RULES);
    return $lcnt if ( ! -f $rules || -z $rules );

    if ( ! open RULES, "<$rules" ) {
	&dismb_msg::err ("Failed to open file \$1: \$2",$rules, $!);
	return $lcnt;
    }

    while ($line = <RULES>) {
	$lall++ ;
	my(@tarr) = &dismb_lib::split_prj_line ($line);
	if ($#tarr >= 3) {
	    my ($map_nm, $sh_nm) = ($tarr[0], $tarr[3]);
	    if ( defined $$hash{$sh_nm} ) {
		$sh_nm = $$hash{$sh_nm};
		$rcnt++ ;
	    }
	    print $map $map_nm, "\n";
	    print $map $sh_nm,  "\n";
	    $lcnt++ ;
	}
    }

    close RULES;
    &dismb_print ("      processed \$1 rules, added \$2 (remapped \$3)",$lall,$lcnt,$rcnt);
    return $lcnt;
}


# If the directory to compile a file in is "." or "NULL",
# substitute it with a base name of shared physical file name.
# Also for EDG languages only, adds '-I$sh_dir -I-' not to miss 
# implicit inclusion like #include "a.h"
#
sub disch::correct_flags_dir {
    my ($fl, $sh_phy) = @_;
    if ( $fl =~ /^\s*\"([^\"]+)\"\s+\"([^\"]+)\"\s+(.*)/ ) {
	my ($lng, $dr, $rst) = ($1, $2, $3);
 	if ( $dr eq "NULL" || $dr eq "." ) {
	    (my $sh_dir = $sh_phy) =~ s%[/\\][^/\\]*$%%;
#	    if ( $dismb_lib::edg_languages =~ /\b$lng\b/ ) {
	    if (($lng eq "C") || ($lng eq "CPP")) {
		my $dash_i_dash = "\"-I$sh_dir\" -I-";
		if ( $rst =~ /\b-I-\b/ ) {
		    $rst =~ s%\b-I-\b%$dash_i_dash%;
		} else {
		    $rst = $dash_i_dash . " $rst";
		}
		if ( $lng !~ /java/ ) {
		    my $hpt = "--header_path_translations $disch::map_table";
		    $rst = "$rst " . $hpt;
		}
	    }
	    $fl = "\"$lng\" \"$sh_dir\" $rst";
	}
    }
    return $fl;
}


# install AdminDir/pdf/build.pdf
# returns: amount of pdf lines on success; 0 on failure
sub disch::install_pdf_flags_mapping {
    my($ret) = 0;    local(*PDF,*FLAGS,*MAP);
    return $ret if ( &disch::open_file($disch::dismb_pdf,  "dismb PDF"  ,       \*PDF) );
    return $ret if ( &disch::open_file($disch::dismb_flags,"dismb flags",       \*FLAGS) );
    return $ret if ( &disch::open_file($disch::map_table,  "file mapping table",\*MAP) );

    my $group_dir = "$disch::sharedSrcRoot/subs/ext";
    eval '&dismb_lib::mkdir_path ("$group_dir")';
    if ( ! -d $group_dir ) {
	&dismb_msg::fixeval;
	&dismb_msg::err ("Failed to create subsystem directory \$1 \$2", $group_dir, $@);
	return 0;
    }

    my($lall,$lpdf,$lflags,$lmap) = (0,0,0,0); my(%locals);
    print PDF "private : \$sharedSrcRoot <-> /private \{\n";
    print PDF "\n";

  FEL: foreach $el (sort(keys(%disch::imp2server))) {
      $lall++ ;
      my(@sh_prj) = &dismb_lib::split_prj_line ($disch::imp2all{$el});
      if ($#sh_prj != 5) {
	  &dismb_msg::err ("Wrong PRJ line for imp_name \$1 line: \$2",$el,$disch::imp2all{$el});
	  next FEL;
      }
      (my $sh_phy = $sh_prj[3]) =~ s/_FLDX_\d{4}$//; $sh_phy = &dismb_lib::dis_path($sh_phy);
      my($imp,$phy,$ln) =  grep !/^\s*$/, split /\"/, $disch::imp2server{$el};

      my($flags) = $disch::imp2flags{$el};
      if ( $sh_prj[0] eq "s" ) {
	  if ( ! $flags ) {
	      &dismb_msg::err ("Failed to find compiler flags for source file local name: \$1 shared phy: \$2 shared imp: \$3",$phy,$sh_phy,$imp);
	      next FEL;
	  }

	  # flags: change directory in case of "NULL" or "."
	  $flags = &disch::correct_flags_dir ($flags, $sh_phy);
	  my($fl_txt) = "\"/private$el\" $flags";
	  print FLAGS $fl_txt, "\n";
	  $lflags++ ;
      } else {

	  # mapping
	  print MAP $sh_phy, "\n";
	  print MAP $phy, "\n";
	  $lmap++ ;
	  $locals{$sh_phy} = $phy;
      }

      # pdf
      my($left,$middle); 
      ($left = $imp)   =~ s%^/%%;
      ($middle = $phy) =~ s/^$disch::sharedSrcRoot\/*//;

      #if there are spaces add quotes around
      $left = &dismb_lib::add_quotes($left);
      $ln = &dismb_lib::add_quotes($ln);
      $middle = &dismb_lib::add_quotes($middle);

      print PDF "  $left : $middle <-> $ln\n";
      $lpdf++ ;

      $disch::imp2succeeded{$el} = $disch::imp2server{$el};      
  }

    my $subm = &disch::add_submitted_entries (\*MAP, \%locals);
   
    print PDF "\}\n";
    print PDF "__rules : / <-> /__rules \{\n";
    print PDF "  \"\" : \$sharedSrcRoot/(**)/%/.pset        => \$sharedModelRoot/pset/(1).pset\n";
    print PDF "  \"\" : \$sharedSrcRoot/(*.pmod)/%/.pmoddir => \$sharedModelRoot/pmod/(1)\n";
    print PDF "\}\n";
    
    my $added = &disch::add_mapping_rules (\*MAP, \%locals);

    close PDF; close FLAGS; close MAP;
    &dismb_msg::inf ("Generated PDF, Flags and MapTable needed files \$1, succeeded \$2: \$3 source, \$4 included",$lall,$lpdf,$lflags,$lmap);
    if ($lpdf == 0) {
	&dismb_msg::err ("Failed to collect PDF files. Aborting ...");
	return $ret;
    }
    if ($lflags == 0) {
	&dismb_msg::err ("Failed to collect source files. Aborting ...");
	return $ret;
    }
    if ($added) {
	&dismb_msg::inf ("Added \$1 mapping rules from shared RULES.pmod.lst",$added);
    } else {
	&dismb_msg::inf ("There are no mapping rules added from shared RULES.pmod.lst");
    }

    $ret = 1;
    return $ret;
}


# creates $ADMINDIR, PDF, Prefs, aset_CCcc file table
# returns 1 on success, 0 on failure
sub disch::prepare_admindir {
    my($ret) = 0;
    
    $disch::dismb_admindir = "$disch::localRoot/disch_AdminDir";
    $disch::dismb_pdf      = "$disch::dismb_admindir/pdf/build.pdf";
    $disch::build_prefs    = "$disch::dismb_admindir/prefs/build.prefs";
    $disch::add_prefs      = "$disch::dismb_admindir/prefs/add.prefs";
    $disch::dismb_flags    = "$disch::dismb_admindir/config/reuse.flg";
    $disch::dismb_tmpdir   = "$disch::dismb_admindir/tmp/disbuild";
    $disch::map_table      = "$disch::tmpdir/file_map.tbl";

    $disch::sharedModelRoot = "$disch::dismb_admindir/model";

    &dismb_msg::inf ("Preparing disch AdminDir \$1",$disch::dismb_admindir);

    return $ret if ( &dismb_lib::create_dir_for_file ($disch::dismb_pdf) );
    return $ret if ( &dismb_lib::create_dir_for_file ($disch::build_prefs) );
    return $ret if ( &dismb_lib::create_dir_for_file ($disch::dismb_flags) );
    return $ret if ( &dismb_lib::create_dir_for_file ($disch::dismb_tmpdir) );
    return $ret if ( &dismb_lib::create_dir_for_file ($disch::map_table) );

    return $ret if ( ! &disch::install_prefs() );
    return $ret if ( ! &disch::install_pdf_flags_mapping() );

    &disch::set_envs ();

    $ret = 1;
    return $ret;
}

sub disch::record_submission {
    &dismb_msg::inf("Record submission started ...");

    my($res) = 0;
    my($submit_dir) =  "$disch::sh_admin/submissions";
    if ( ! -d $submit_dir ) {
	&dismb_msg::inf ("Creating directory \$1",$submit_dir);
	eval '&dismb_lib::mkdir_path ($submit_dir)';
	if ( ! -d $submit_dir ) {
	    &dismb_msg::fixeval;
	    &dismb_msg::err ("Failed to create \$1 \$2", $submit_dir, $@);
	    return $res;
	}
    }

    &disch::lock_submission($submit_dir);

    $res = eval '&disch::record_submission_internal($submit_dir)';
    $res = 0 if ($@);

    &disch::unlock_submission($submit_dir);

    &dismb_msg::inf("Record submission succeeded.") if ($res);
    return $res;
}

sub disch::record_submission_internal {
    my($submit_dir) = @_;
    my($res) = 0;
    my($records) = "$submit_dir/records";
    local(*RC);
    my($num_of_records)=0;
    my($rec_line);
    my %inv;

    if (open(RC,"<$records")) {
	while($rec_line = <RC>) {
	    my($key) =  grep !/^\s*$/, split /\"/, $rec_line;
	    if ($key) {
		$num_of_records++;
		my $tail_name = "noname";
		if ( $key =~ /(.*)\/([^\/]+)$/ ) {
		    $tail_name = $2;
		}
		$disch::record{$key} = "$submit_dir/$num_of_records/$tail_name";
		$inv{$num_of_records} = $key;
	    }
	}
	close (RC);
    }

    if (!open(*RC, ">$records")) {
	&dismb_msg::err ("Failed to write to \$1",$records);
	return $res;
    }
	
    my($cnt) = 0;
    foreach $el (sort(keys(%disch::imp2client))) {
	my($imp,$tfn,$ln) =  grep !/^\s*$/, split /\"/, $disch::imp2client{$el};
	$tfn =~ s%\\%/%g;
	if ( $tfn =~ /(.*)\/([^\/]+)$/ ) {
	    my($dirname,$basename) = ($1, $2);
	    if (!$dirname || !$basename) {
		&dismb_msg::err ("Failed to extract basename for file \$1",$tfn);
		return 0;
	    }
	    $cnt++;
	    $dirname =~ s%[/:]%_%g; $dirname =~ s/^_//;
	    my($phy) = "$disch::sharedSrcRoot/$dirname/$basename";
	    my($tail_name) = "noname";
	    if ( $imp =~ /(.*)\/([^\/]+)$/ ) {
		$tail_name = $2;
	    }			

	    if (defined($disch::record{$imp})) {
		my($dest) = $disch::record{$imp};
		&disch::update_record($phy,$dest);
	    } else {
		$num_of_records++;
		$disch::record{$imp} = "$submit_dir/$num_of_records/$tail_name";
		my($dest) = "$submit_dir/$num_of_records/$tail_name";
		$inv{$num_of_records} = $imp;
		&disch::create_record($phy,$dest);
	    }
	}
    }
    my($ii) = 1;
    while ($ii <= $num_of_records) {
	print RC "\"$inv{$ii}\"\n";
	$ii++;
    }
    close RC;

    $res = 1;
    return $res;
}

sub disch::restore_record {
    my($src,$ftp,$dest) = @_;
    &dismb_lib::copy($dest,"$ftp.old");
    &dismb_lib::copy("$dest.ent","$src.ent.old");
    return $@;
}

sub disch::clear_record {
    my($src,$ftp) = @_;
    unlink("$ftp.old") if (-f "$ftp.old"); 
    unlink("$src.ent.old") if (-f "src.ent.old");
    return $@;
}

sub disch::update_record {
    my($src,$dest) = @_;
    &dismb_lib::copy($src,$dest);
    &dismb_lib::copy("$src.ent","$dest.ent");
    return $@;
}

sub disch::create_record {
    my($src,$dest) = @_;
    return 0 if (&dismb_lib::create_dir_for_file($dest));
    &dismb_lib::copy($src,$dest);
    &dismb_lib::copy("$src.ent","$dest.ent");
    return $@;
}

sub disch::add_submitted_entries {
    my($map,$hash) = @_;
    return 0 if &dismb_options::is_defined('-view');
    my $count = 0;
    foreach $el (sort(keys(%disch::record))) {
	my $phy = $disch::record{$el};
	if (defined $disch::imp2server{$el}) {
	    next;
	}
	my(@sh_prj) = &dismb_lib::split_prj_line ($disch::imp2all{$el});
	if ($#sh_prj != 5) {
	    &dismb_msg::err ("Wrong PRJ line for imp_name \$1 line: \$2",$el,$disch::imp2all{$el});
	    next;
	}
	(my $sh_phy = $sh_prj[3]) =~ s/_FLDX_\d{4}$//; $sh_phy = &dismb_lib::dis_path($sh_phy);

	if ( $sh_prj[0] eq "s" ) {
	    next;
	} else {
	  # mapping
	  print $map $sh_phy, "\n";
	  print $map $phy, "\n";
	  $count++ ;
	  $$hash{$sh_phy} = $phy;
      }
    }
    return $count;
}

sub disch::lock_submission {
    my($dir) = @_;
    my($ret) = 0;
    my($lockfile) = "$dir/sch.lck";
    my($waiting) = 0;
    my($timeout) = 120;
    my($msg_count) = 10;
    while (-f $lockfile && $waiting < $timeout) {
	if($msg_count==10) {
	   &dismb_msg::inf("Waiting to lock \$1 ...",$dir);
	   $msg_count = 0;
	}
	$msg_count++;
        $waiting++;
	sleep 1;
    }

    &dismb_msg::die("\$1 is busy ...",$dir) if (-f $lockfile);
    local(*FLCK);
    open (FLCK, ">$lockfile") || &dismb_msg::die ("failed to create lock \$1: \$2",$lock,$!); 
    close FLCK; # created the lock file
    
    return 1;
}

sub disch::unlock_submission {
    my($dir) = @_;
    my($ret) = 0;
    my($lockfile) = "$dir/sch.lck";
    $ret = &dismb_lib::unlink($lockfile);
    return $ret;
}


return 1;
