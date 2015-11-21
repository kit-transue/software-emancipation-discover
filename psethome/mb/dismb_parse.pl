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
$dismb_parse::prs_count     = 0;
$dismb_parse::forgive_count = 0;
$dismb_parse::fail_count    = 0;
$dismb_parse::error_node_count = 0;

$prs_forgiv = "$ENV{'DISBUILD_TMPDIR'}/prs_forgiv.lst" ;
$prs_failed = "$ENV{'DISBUILD_TMPDIR'}/prs_failed.lst" ;
$prs_missed = "$ENV{'DISBUILD_TMPDIR'}/prs_missed.lst" ;
$prs_errors = "$ENV{'DISBUILD_TMPDIR'}/prs_errors.log" ;
$frg_errors = "$ENV{'DISBUILD_TMPDIR'}/frg_errors.log" ;
$prs_tmperr = "$ENV{'DISBUILD_TMPDIR'}/prs_tmperr.lst" ;

$lst_file = "$ENV{'DISBUILD_TMPDIR'}/all.pmod.lst" ; # list of all files

$prs_file = "$ENV{'DISBUILD_TMPDIR'}/all.pmod.prs" ; # => @prs_arr
$prs_line = "$ENV{'DISBUILD_TMPDIR'}/prs_line";      # => dis_lock index in prs_arr

$nif_file = "$ENV{'DISBUILD_TMPDIR'}/all.pmod.nif" ; # => %nif_arr, indexed by if, list of all outdated source/header files
$prs_todo = "$ENV{'DISBUILD_TMPDIR'}/prs_todo.lst" ; # phys_name per line for parser

$nif_parsed      = "$ENV{'DISBUILD_TMPDIR'}/all.parse.nif" ;  # nif after parse (IFFs)
$parsed_projects = "$ENV{'DISBUILD_TMPDIR'}/parsed_projects.lst" ;  # list of succeeded projects

$lines_file       = "$ENV{'DISBUILD_TMPDIR'}/all.lines.cnt" ;  # Total number of lines built

$nif_updated      = "$ENV{'DISBUILD_TMPDIR'}/all.update.nif" ; # nif after update (psets)
$updated_projects = "$ENV{'DISBUILD_TMPDIR'}/updated_projects.lst" ;  # list of succeeded projects

$incremental_modelbuild = ( &dismb_prefs::get('MBscratch') ne "yes" ) ? 1 : 0;

$els_languages = &dismb_prefs::get('*psetPrefs.ELS.List');
if ( $els_languages ) {
    my($user_file) = &dismb_prefs::get('*psetPrefs.ELS.MBuserEvalFile');
    if ( -f $user_file ) {
	&dismb_msg::inf ("Evaluate pref ELS.MBuserEvalFile= \$1",$user_file);
	eval 'require $user_file';
	&dismb_msg::fixeval;
	&dismb_msg::err ("Error evaluating \$1 \$2",$user_file, $@) if ($@); 
    }
}

$exe_ext = ($dismb::is_NT) ? ".exe" : "";
$ifext = (defined $ENV{'DIS_IFEXT'}) ? "$ENV{'DIS_IFEXT'}" : "$psethome_bin/ifext$exe_ext";

$debug_sizes   = (&dismb_prefs::get('MBdebugSizes') eq yes) ? 1 : 0;
$compress_exe  = &dismb_prefs::get('MBiffCompressExe');
$compress_ext  = &dismb_prefs::get('MBiffCompressExt');

sub dismb_print_times {
    local($tag, $arr) = @_;
    local ($user, $system, $cuser, $csystem) = @$arr;
    printf ("TIMES %s: %.2f : u %.2f s %.2f cu %.2f cs %.2f\n", $tag, 
	    $user + $system + $cuser + $csystem, $user, $system, $cuser, $csystem);
}


sub dismb_diff_times {
    local($tag, $en, $st) = @_;
    local(@arr) = (@$en[0] - @$st[0], @$en[1] - @$st[1], @$en[2] - @$st[2], @$en[3] - @$st[3]);
    &dismb_print_times ($tag, "arr");
}


sub dismb_add_times {
    local($tot, $en, $st) = @_;
    @$tot[0] += @$en[0] - @$st[0];
    @$tot[1] += @$en[1] - @$st[1];
    @$tot[2] += @$en[2] - @$st[2];
    @$tot[3] += @$en[3] - @$st[3];
}


sub dismb_parse::is_good_language {
    local($lan) = @_;
    return 1 if ( $dismb_lib::edg_languages =~ /\b$lan\b/ );
    return 1 if ( $els_languages =~ /\b$lan\b/ );
    return 0;
}

sub dismb_parse::is_java
{
    my($str) = @_;
    return ($str =~ /^java$/i ) ? 1 : 0;
}

sub dismb_parse::compilation_dir
{
    my($phy, $dir) = @_;

    my($comp_dir) = $dir;
    if ( $dir eq "." || $dir eq "NULL" ) {
        $phy = &dismb_lib::dis_path($phy);
        ($comp_dir = $phy ) =~ s/[\/\\][^\/\\]*$//;
    }
    if ($dismb::is_NT) {
        if ($comp_dir =~ /^[a-zA-Z]:$/) { #if dir is just drive letter add '/'
          $comp_dir .= "/";
        }
    }
    return $comp_dir;
}

#All the Java files with the same package name will be compiled 
#together (instead of compiling each file separately) to increase the visiblity
#of non-public package classes. To do this we will create groups of files, based
#on their package names, compilation directory and flags in 
#dismb_parse::init_prs_arr routine.  

sub dismb_parse::init_prs_arr {
    local($cnt) = 0;
    local(*PRS);
    open (PRS,"<$prs_file")  || &dismb_msg::die ("Failed to read \$1: \$2",$prs_file,$!);
    while (<PRS>) {
        s/\s+$//;
        my($iff, $phy, $mgroup_id, $lang, $dir, $flags);
	if ( $_ =~ /^\"([^\"]*)\"\s+\"([^\"]*)\"\s+\"([a-zA-Z][^\"]*)\"\s+\"([^\"]*)\"(.*)$/ ) {
	    # old format: Third element is language.
            ($iff, $phy, $lang, $dir, $flags) = ($1, $2, $3, $4, $5);
	}
	elsif ( $_ =~ /^\"([^\"]*)\"\s+\"([^\"]*)\"\s+\"([^\"]*)\"\s+\"([^\"]*)\"\s+\"([^\"]*)\"(.*)$/ ) {
	    # new format: Third element is message group id.
            ($iff, $phy, $mgroup_id, $lang, $dir, $flags) = ($1, $2, $3, $4, $5, $6);
	}
	if (defined($phy)) {
            if ( !&dismb_parse::is_java($lang) ) { #for non Java file
	        @prs_arr[$cnt++] = "\"$iff\" \"$phy\" \"$mgroup_id\" \"$lang\" \"$dir\"$flags";
            }
	    else {                                 #for Java file
                #For Java files format for prs_arr values is different from that of 
                #non Java files. Package_name, comp_dir and flags separated by "," 
                #form the values which are also the keys to %java_prs_group hash.
                #Each key in %java_prs_groups hash can have an array of files as value.
                #Each file is tab separated with its corresponding message group id.
                my($package_name) = "";
		if ( $flags =~ /--package_name \"([^\"]*)\"/ ) {
                    $package_name = $1;
                    $flags =~ s/--package_name \"([^\"]*)\"//g; 
                }
                $flags =~ s/^\s+//; $flags =~ s/\s+$//;       #trim spaces

                my($comp_dir) = &dismb_parse::compilation_dir($phy, $dir);
                my($key) = "$package_name,$comp_dir,$flags"; #comma separated key
                if ( exists($java_prs_groups{$key}) ) {
                    #If the key already exists, append the file(and message group id) 
                    #to the array
                    push ( @{ $java_prs_groups{$key} }, "$phy\t$mgroup_id" );
                } 
                else {
                    #if key does not exist, create it and also add to prs_arr
                    $java_prs_groups{$key} = [ "$phy\t$mgroup_id" ];
                    @prs_arr[$cnt++] = "$key";  

                    &dismb_msg::vrb("Created new java parse group with package name: \$1, compilation dir: \$2, flags: \$3", $package_name, $comp_dir, $flags);
                }
            } 
        }
    }
    &dismb_msg::vrb ("Created prs array, size = \$1", $cnt) ;
    close PRS;
    &dismb_lib::status ("$pass $cnt 0 start");
}


sub dismb_parse::init_nif_arr {
    local($lines, $nifs) = (0, 0);
    local(*NIF);
    open (NIF, "<$nif_file");
    while (<NIF>) {
	$lines ++ ;
        if (/^\"([^\"]+)\"\s+\"([^\"]+)\"/) {
	    local($iff) = $1;

	    local($phy) = &dismb_lib::dis_path ($2); # always canonicalize names in %nif_arr
	    if ( ! $phy ) {
		&dismb_msg::err ("Failed to canonicalize file name \'\$1\'.",$2);
	    } else {
		if ( ! defined ($nif_arr{$phy}) ) {
		    $nif_arr{$phy} = $iff;
		    $nifs ++ ;
		} else {
		    &dismb_msg::wrn ("Duplicate name \$1 in file \$2. Ignore",$phy,$nif_file);
		}
	    }
	} else {
	  dismb_msg::err ("Wrong line in \$1: \'\$2\' .Ignore",$nif_file,$_);
        }
    }    
    &dismb_msg::vrb ("Created nif array, size = \$1",$nifs);

    close NIF;
}


sub dismb_parse::init_host_tag {
    local($host,$cpu) = @_;
    $host_tag = "${host}:${cpu}";
}


sub dismb_parse::init {
    local($host,$cpu) = @_;
    &dismb_msg::set_host_cpu   ($host, $cpu);
    &dismb_parse::init_prs_arr;
    &dismb_parse::init_nif_arr;

    if ( $compress_exe ) {
	if ( ! -x $compress_exe ) {
	    $compress_ext = "";
	    $compress_exe = "";
	    &dismb_msg::wrn ("Wrong IFF compress executable \$1. Ignore",$compress_exe);
	} else {
	    if ( ! $compress_ext ) {
		$compress_ext = "";
		$compress_exe = "";
		&dismb_msg::wrn ("Missing Preferences \'MBiffCompressExt\' for executable \$1. Ignore",$compress_exe);
	    }
	}
    } else {
	$compress_ext = "";
    }
    return 1;
}

# %hdr_arr(one for each instance of parse) stores new header files, 
#that are encountered during parsing source files in %prs_arr, 
#which are not listed in %nif_arr. %hdr_arr is created if dismb command 
#line option -MBincludeAllHdrs is yes. 
sub dismb_parse::add_header {
    if ( ! defined $include_all_headers ) {
        return "";
    }
    my($hdr) = $_[0];
    my($iff_hdr);
    if ( ! defined $hdr_arr{$hdr} ) {
	$iff_hdr = &dismb_lib::get_iff_location($hdr);
        $hdr_arr{$hdr} = $iff_hdr;
        dismb_msg::vrb("Adding Header file: $hdr, iff file: $iff_hdr");
    } else {
        $iff_hdr = $hdr_arr{$hdr};
    }
    return $iff_hdr;
}

sub dismb_parse::in_multiple_IFF_mode
{
   if (defined($multiple_IF_files)) {
       return $multiple_IF_files;
   }
   $multiple_IF_files = 0;
   if (&dismb_prefs::get('MBmultipleIFFs') eq "yes" && 
       &dismb_prefs::get('MBgeneratePdf') eq "yes") {
       $multiple_IF_files = 1;
   }
   return $multiple_IF_files;
}

# returns: 0 - succeeded, 1 - failed
# $phy, $fla, $tmpif, $iff, $dir preset
# either dies or returns no zero status on failure
sub dismb_parse::compile {
    my($mult_IFF) = $_[0];
    my($status)          = 0;
    my $geniffs_file = "$ENV{'TMPDIR'}/geniffs_file";

    my($exe) = "aset_CCcc$exe_ext";
    &dismb_msg::die ("No parser executable \$1",$exe) if ( ! -f "$psethome_bin/$exe" );

    my (@IFF_flags) = ();
    if ($mult_IFF) {
      &dismb_lib::unlink($geniffs_file);     

      @IFF_flags = ("--multiple_IF_files");
      @IFF_flags = (@IFF_flags, "--IF_file_base_dir", "$ENV{'ADMINDIR'}/model/pset");
      @IFF_flags = (@IFF_flags, "--generated_IF_files_file", $geniffs_file);
      if (&dismb_lib::preserve_case()) {
          @IFF_flags = (@IFF_flags, "--preserve_case");
      }
    }
    else {
      @IFF_flags = ("--IF_file", $tmpif);
    }

    my @brief = (&dismb_prefs::get ('parserBriefDiagnostics') eq "yes") ? ("--display_error_number", "--brief_diagnostics") : ();

    my(@cmd) = ($exe, @IFF_flags, @brief, "--checksum", "--GNU_compatible_AST", &dismb_msg::lp_flags("--lp_service"), &dismb_lib::argstr_to_list($fla), $phy);

    &dismb_print ("\$1\n",&dismb_lib::arglist_to_str(@cmd));

    my($status) = &dismb_lib::system (@cmd);
    my $return_value = &dismb_lib::get_return_value($status);
  
    if ($return_value == 4 && $mult_IFF) {
        #Parser had catastrophic error, remove the IF files that were created
        #as they could be incomplete.
        if (open(GIF, "<$geniffs_file")) {
            my ($f);
            &dismb_msg::msg("Cleaning up after catastrophic error occured in the parser.");
            while($f = <GIF>) {
                chomp($f);
                &dismb_lib::unlink($f);
            }
            close GIF;
        } 
    }
    elsif (($return_value == 2) && ($mult_IFF || -f $tmpif)) {
	if ( &dismb_prefs::get ('acceptSyntaxErrors') eq "yes") {
	    &dismb_print ("Accepting parse errors.");
	    $status = 0;
	}
    }

    return $status;
}


#returns 0 on success
# $dir, $tmpif, $iff preset
#either dies or returns no zero status on failure
sub dismb_parse::process_iff {

    my($second_iff) = @_;
    if ( ! open(IFF_TMP,"<$tmpif") ) {
	&dismb_msg::err ("Failed to read tmp if file \$1: \$2",$tmpif,$!);
	return 1;
    }

    my($fext) = "$iff.ext";
    &dismb_lib::create_dir_for_file ($fext);
    if ( ! open(FEXT,">$fext") ) {
	&dismb_msg::err ("Failed to write extractor file \$1: \$2",$fext,$!);
        #close open file handle before returning
        close IFF_TMP;
	return 1;
    }

    #create new message group for scan iff
    &dismb_msg::push_new_message_group("scan of parser output");

    my($i, $all, $skipped, $foreign, $cnt) = (0, 0, 0, 0, 0); my(%found, %set);

# debug
#    local(@scan_st) = times;

  IFF_LOOP:
    while (<IFF_TMP>) {

# debug	
#       $iff_lines ++ ;

	last IFF_LOOP if ( /^SMT file \"\"/ ); # aharlap said that this is the end of the story
	if ( /^SYM \[[0-9]+\] file \"([^\"]+)\"/ ) {

# debug
#	    local(@in_found_st) = times;

	    my($f) = $1; my($newf) = $f;

	    next IFF_LOOP if (defined $set{$f});
	    $set{$f} = $all ++ ;
	    if ( defined $foreign_set{$f} ) {
		$foreign ++ ;
		next IFF_LOOP;
	    }
	    if ( defined $skipped_set{$f} ) {
		&dismb_msg::vrb ("File \$1 is up to date, iff exists.",$f);
		$skipped ++ ;
		next IFF_LOOP;
	    }

	    if ( $f =~ /^[^\/\\]/ && ! ($f =~ /^[A-Za-z]\:/) ) { # adding directory to a relative filename
		if ( $dir =~ /[\/\\]$/ ) {
		    $newf = $dir . $f;
		} else {
		    $newf = $dir . "/" . $f;
		}
	    } 
#debug
#	    local(@path_times_st) = times;

	    $newf = &dismb_lib::dis_path ($newf);

#	    local(@path_times_en) = times;
#	    &dismb_add_times ("all_path_time", "path_times_en", "path_times_st");
#	    $iff_path ++ ;
#debug

	    if ( ! $newf ) {
		&dismb_msg::err ("Failed to canonicalize file name \'\$1\'",$f);
                #close open file handles before returning
                close IFF_TMP;
                close FEXT;
		return 1;
	    }

            my($iff) = "";
            if ( ! defined ($nif_arr{$newf}) ) {
              my($is_new) = 1;
              if ( !$dismb::standalone_mode && ! &dismb_parse::is_java($lan)) {
                #If doing incremental model build and the header file is not in the
                #outdated file list (%nif_arr), then see if a pset file exists,
                #otherwise this is a new header file.
                if ( $incremental_modelbuild ) {                  
                  my($pset_name) = &dismb_lib::pset_filename_of($newf);
                  if ( -f $pset_name ) {
                    $is_new = 0;  #if pset exists, file is not new
                  }
                }
                if ( $is_new ) {
                  $iff = &dismb_parse::add_header($newf);
                }
	      }
              if ($is_new && $iff eq "") {
	        &dismb_msg::msg ("Foreign file: \$1",$f);
	        $foreign_set{$f} = $foreign ++ ; # global set
	      }
            } else {
              $iff = $nif_arr{$newf};
            }

	    if ( $iff ne "" ) {
		my($zip) = ($compress_ext) ? $iff . $compress_ext : "";

		if ( -f $zip || -f $iff ) {
		    $skipped_set{$f} = $skipped ++ ; # global set
		    if ( $zip ) {
			&dismb_msg::vrb ("File \$1 is up to date, compressed iff \$2",$f,$zip);
		    } else {
			&dismb_msg::vrb ("File \$1 is up to date, iff \$2",$f,$iff);
		    }
		} else {
#debug
#		    local(@lock_times_st) = times;

		    my($ln) = &dismb_lib::get_locked_linenum ("$iff.lck");

#		    local(@lock_times_en) = times;
#		    &dismb_add_times ("all_lock_time", "lock_times_en", "lock_times_st");
#		    $iff_locked ++ ;		
#debug
		    if ( $ln > 1 ) {
			&dismb_msg::vrb ("File \$1 is up to date, lock exists \$2",$f,"$iff.lck");
			$skipped ++ ;
		    } else {
			if ($ln == 1) {
			    $cnt ++ ;
			    $found{$iff} = $f;        # %found - original names
			    (print FEXT $f, "\n", $iff, "\n") or &dismb_msg::write_failed($fext, $!); 
			} else {
			    &dismb_msg::err("Could not lock iff \$1","$iff.lck");
			}
		    }
		}
	    }
#debug
#	    local(@in_found_en) = times;
#	    &dismb_add_times( "all_found_time", "in_found_en", "in_found_st" );

	}
    }

#debug
#   local(@scan_en) = times;
#   &dismb_add_times ( "all_scan_time", "scan_en", "scan_st" );

    close IFF_TMP;
    close FEXT;

    #pop new message group created for scan iff
    &dismb_msg::pop_new_message_group;

    if ( ! $all ) {
	&dismb_msg::err ("Did not find a single file in iff \$1.",$tmpif);
	return 1;
    }

    if ( ! $cnt ) {
	&dismb_msg::wrn ("All files are skipped in iff \$1.",$tmpif);
	return 0;
    }
    &dismb_msg::inf ("Scanned \$1 \$2 all files, \$3 up to date, \$4 to extract, \$5 foreign",$tmpif,$all,$skipped,$cnt,$foreign);

    my($message_group) = &dismb_msg::current_auto_message_group;
    my($lp_service) = $dismb_msg::lpservice_name;

    my (@args, $i) = (0,0);
    $args[$i++] = $ifext;
    $args[$i++] = $tmpif;
    if ($second_iff ne "") {
	$args[$i++] = $second_iff;
    }
    $args[$i++] = "-f";
    $args[$i++] = $fext;
    push @args, &dismb_msg::lp_flags("-lp_service");
    &dismb_print ("\$1 \$2 \$3 -f \$4 -lp_service \$5 -message_group \$6",$ifext,$tmpif,"$second_iff",$fext,$lp_service,$message_group);
    &dismb_lib::system(@args);
    my($status) = $?;

    my($cleanup) = (&dismb_prefs::get("MBforceCleanup") eq "yes") ? 1 : 0;
    unlink ($fext) if ($cleanup);

    my($f);
    foreach $f ( keys (%found) ) {
	unlink ("$f.lck");
    }

    if ($status != 0) {
	&dismb_msg::err ("Failed to extract if files. Cleaning up ...");

	# Avoid leaving IF files which might be corrupt.
	foreach $f ( keys (%found) ) {
	    unlink ($f);
	}
	return 1;
    } else {
        #create new message group for all the extracted iff files
        &dismb_msg::push_new_message_group("extracted intermediate files");

	foreach $f ( keys (%found) ) {
	    if ( -f $f ) {
		my($sz_tag) = "   SIZES src/iff ";
		my($sz_txt) = ($debug_sizes) ?  (-s $found{$f}) . "/" . (-s $f) : "" ;
		if ($compress_exe && $compress_ext) {
		    my(@cmd) = ($compress_exe, $f);
		    &dismb_print ("compress : \$1 \$2 ... ",$compress_exe,$f);
		    &dismb_lib::system (@cmd); my($status) = $?;

		    if ( ! -f $f && ! -f $f . $compress_ext ) {
			&dismb_print (" *** FAILED"); # Review fatal?
			&dismb_msg::err ("Compress executable deleted IFF \$1 *** Disable compression",$f);
			$compress_exe = ""; $compress_ext = "";
		    } else {
			if ($status != 0 ) {
			    &dismb_print (" *** FAILED"); # Review fatal?
			} else {
			    if ( -f $f . $compress_ext ) {
				&dismb_print ("succeeded");
				$sz_tag = "   SIZES src/iff/zip ";
				$sz_txt = $sz_txt . "/" . (-s $f . $compress_ext) if ($sz_txt);
			    } else {
				&dismb_print (" *** FAILED"); # Review fatal?
			    }
			}
		    }
		}
		$sz_txt = $sz_tag . $sz_txt if ($sz_txt);
		&dismb_print ("extracted: \$1 \$2 \$3", $found{$f}, $sz_txt, $f);
	    } else {
		&dismb_msg::err ("Failed to extract iff \$1",$f);
	    }
	}
        #pop group created for extraction of iff files
        &dismb_msg::pop_new_message_group;
    }

    return 0;
}

sub guard_against_curlies {
    my($p_cmd) = @_[0];
    foreach $cmd_elem (@{$p_cmd}) {
	if ($cmd_elem =~ m/[{}]/) {
	    &dismb_msg::die ("Curly braces leaked into parser command: \$1", &dismb_lib::arglist_to_str(@{$p_cmd}));
        }
    }
}

#returns 0 on success
#$tmpif, $iff, $lan, $dir, $fla, %comp_list are preset
sub dis_parse_java {
    my($status) = 0;
    my(@cmd, $file, $msg_grp_id);

    my($comp_file) = "$ENV{'TMPDIR'}/jfe_comp$$.lst";
    &dismb_lib::create_dir_for_file($comp_file);
    open(COMP, ">$comp_file") || &dismb_msg::die("Failed to open \$1: \$2", $comp_file, $!);
    while ( ($file, $msg_grp_id) = each(%comp_list) ) {
        if ($msg_grp_id) {
            print(COMP "$file\t$msg_grp_id\n") or &dismb_msg::die("Failed to write to file, \$1 : \$2", $comp_file, $!);
        } else {
            print(COMP "$file\n") or &dismb_msg::die("Failed to write to file, \$1 : \$2", $comp_file, $!);
        }
    } 
    close(COMP);

    my($exe) = "$psethome_bin/aset_jfe$exe_ext";
    &dismb_msg::die ("No Java parser executable \$1",$exe) if ( ! -x "$exe" );
    $exe = "aset_jfe$exe_ext";

    @cmd = ($exe, &dismb_msg::lp_flags("--lp_service"), &dismb_lib::argstr_to_list($fla),
		"--checksum", "--IF_file", $tmpif, "--file_list", $comp_file);

    &dismb_print ("\n\$1\n",&dismb_lib::arglist_to_str(@cmd));

    guard_against_curlies(\@cmd);
    $status = &dismb_lib::system(@cmd);
    my $return_value = &dismb_lib::get_return_value($status);

    if ( $return_value == 2 && -f $tmpif ) {
        if ( &dismb_prefs::get ('acceptSyntaxErrors') eq "yes") {
            #The "Accepting" message is intentionally different from other 
            #parser messages so as not to trigger forgiven file count while 
            #reading log file. Parser reports the number of IF files with
            #errors which gets used.
	    &dismb_print ("Accepting java parse errors.");
	    $status = 0;
	}
    }

    if ( $status == 0 ) {
	$status = eval '&dismb_parse::process_iff()';
	&dismb_msg::fixeval;
	&dismb_msg::die ("Failure while processing iff: \$1",$@) if $@;
	&dismb_msg::die ("Failed to process iff.") if ( $status );
    } else {
        #Although putting out error message about each java file clutters 
        #up the log file, it is important for getting the parse summary 
        #numbers right. 
        while ( ($file, $msg_grp_id) = each(%comp_list) ) {
	    &dismb_msg::err ("Failed compiling \$1", $file);
            $status++;
	}
    }
    return $status;
}


#returns 0 on success
# $tmpif, $iff, $phy, $lan, $dir, $fla are preset
sub dis_parse_sql {
    my($status);

    my($exe) = "$psethome_bin/dis_sql$exe_ext";
    &dismb_msg::die ("No sql parser executable \$1",$exe) if ( ! -x "$exe" );

    my(@cmd)    = ($exe, &dismb_lib::argstr_to_list($fla), "-o", $tmpif, $phy);
    &dismb_print ("\n\$1 \$2 -o \$3 \$4\n",$exe,$fla,$tmpif,$phy);

    guard_against_curlies(\@cmd);
    $status = &dismb_lib::system(@cmd);

    if ($status) {
	if ( &dismb_prefs::get ('acceptSyntaxErrors') eq "yes") {
	    &dismb_print ("Accepting parse errors.");
	    $status = 0;
	}
    }

    $status = eval '&dismb_parse::process_iff()';
    &dismb_msg::fixeval;
    &dismb_msg::die ("Failure while processing iff: \$1", $@) if $@;
    &dismb_msg::die ("Failed to process iff for $phy") if ( $status );

    return 0;
}

#returns 0 on success
# $tmpif, $iff, $phy, $lan, $dir, $fla are preset
sub dis_parse_tlb {
    my($status);

    my($exe) = "$psethome_bin/tlb_parser$exe_ext";
    &dismb_msg::die ("No tlb parser executable \$1",$exe) if ( ! -x "$exe" );

    my(@cmd)    = ($exe, $phy, $tmpif);
    &dismb_print ("\n\$1 \$2 \$3\n",$exe,$phy,$tmpif);

    guard_against_curlies(\@cmd);
    $status = &dismb_lib::system(@cmd);

    if ($status) {
	if ( &dismb_prefs::get ('acceptSyntaxErrors') eq "yes") {
	    &dismb_print ("Accepting parse errors.");
	    $status = 0;
	}
    }

    $status = eval '&dismb_parse::process_iff()';
    &dismb_msg::fixeval;
    &dismb_msg::die ("Failure while processing iff: \$1", $@) if $@;
    &dismb_msg::die ("Failed to process iff for \$1",$phy) if ( $status );

    return 0;
}

#returns 0 on success
# $tmpif, $iff, $phy, $lan, $dir, $fla are preset
sub dis_parse_rc {
    my($status);

    my($exe) = "$psethome_bin/rcparser$exe_ext";
    &dismb_msg::die ("No rc parser executable \$1",$exe) if ( ! -x "$exe" );

    my(@cmd)    = ($exe, $phy, $tmpif);
    &dismb_print ("\n\$1 \$2 \$3\n",$exe,$phy,$tmpif);

    guard_against_curlies(\@cmd);
    $status = &dismb_lib::system($cmd);

    if ($status) {
	if ( &dismb_prefs::get ('acceptSyntaxErrors') eq "yes") {
	    &dismb_print ("Accepting parse errors.");
	    $status = 0;
	}
    }

    $status = eval '&dismb_parse::process_iff()';
    &dismb_msg::fixeval;
    &dismb_msg::die ("Failure while processing iff: \$1", $@) if $@;
    &dismb_msg::die ("Failed to process iff for \$1",$phy) if ( $status );

    return 0;
}

#returns 0 on success
# $tmpif, $iff, $phy, $lan, $dir, $fla are preset
sub dis_parse_esql {
    my($sql_fla,$c_fla) = split (/__FLAGSflagsFLAGS__/, $fla);
    my($sql_lan)        = $lan;

    my($suff, $sql_if, $mod);

    if ( $sql_lan eq "ESQL_C" ) {
	$lan = "C";
	$suff   = &dismb_prefs::get('cDfltSrcSuffix');
    } else {
	$lan = "CPP";
	$suff = &dismb_prefs::get('cPlusDfltSrcSuffix');
    }
    &dismb_msg::die ("Unknown \$1 c/c++ default suffix",$lan) if (!$suff);

    my($mod)      = "$tmpif$suff";
    my($sql_if)   = "$tmpif.sql.if";
    my($sql_c_if) = "$tmpif$suff.if";
    my($exe)      = "$psethome_bin/dis_sql$exe_ext";

    &dismb_msg::die ("No dis_sql executable \$1",$exe) if ( ! -f "$exe" );

    my(@cmd) = ($exe, &dismb_lib::argstr_to_list($sql_fla), "-o", $sql_if, "-c", $mod, $phy);
    &dismb_print ("\n\$1 \$2 -o \$3 -c \$4 \$5\n",$exe,$sql_fla,$sql_if,$mod,$phy);

    guard_against_curlies(\@cmd);
    my($status) = &dismb_lib::system(@cmd);
    &dismb_msg::die ("dis_sql failed: \$1",$!) if ( $status > 1 );

    my($my_fla) = $fla; $fla = $c_fla;
    my($my_phy) = $phy; $phy = $mod;

    $status = eval '&dismb_parse::compile(0)';
    &dismb_msg::fixeval;
    &dismb_msg::die ("Failure parsing: \$1", $@) if $@;
    &dismb_msg::die ("Failed compiling \$1",$phy) if ( $status );

    $status = eval '&dismb_parse::process_iff()';  # preset $dir, $tmpif, $iff
    &dismb_msg::fixeval;
    &dismb_msg::die ("Failure processing iff: \$1", $@) if $@;
    &dismb_msg::die ("Failed to process iff for \$1",$phy) if ( $status );
    
    my(@args, $i) = (0,0);
    $args[$i++] = $ifext;
    $args[$i++] = $tmpif;
    $args[$i++] = "-o"; $args[$i++] = $sql_c_if;
    &dismb_print ("\n\$1 \$2 -o \"\$3\"\n",$ifext,$tmpif,$sql_c_if);

    $status = &dismb_lib::system(@args);
    &dismb_msg::die ("Failed extractor, status= \$1: \$2",$status,$!) if ( $status );
   
    my($my_tmpif) = $tmpif; $tmpif = $sql_if;
    $status = eval '&dismb_parse::process_iff ($sql_c_if)';	# preset $dir, $tmpif, $iff
    &dismb_msg::fixeval;
    &dismb_msg::die ("Failure processing iff: \$1", $@) if $@;
    &dismb_msg::die ("Failed to process iff for \$1",$phy) if ( $status );

    $tmpif = $my_tmpif; $phy = $my_phy; $fla = $my_fla;
    unlink ($sql_if); unlink($sql_c_if); unlink ($mod);
    return 0;
}


#returns 0 on success
# $tmpif, $iff, $phy, $lan, $dir, $fla are preset
sub dis_parse_ESQL_C {
    return &dis_parse_esql();
}


#returns 0 on success
# $tmpif, $iff, $phy, $lan, $dir, $fla are preset
sub dis_parse_ESQL_CPP {
    return &dis_parse_esql();
}


#returns 0 on success
# $tmpif, $iff, $phy, $lan, $dir, $fla are preset
sub dis_parse_C {

    my($status);

#debug
#   local (@comp_st) = times;    

    my($mult_IFF) = &dismb_parse::in_multiple_IFF_mode();

    $status = eval '&dismb_parse::compile($mult_IFF)';
    &dismb_msg::fixeval;
    &dismb_msg::die ("Failure parsing: \$1", $@) if $@;
    &dismb_msg::die ("Failed compiling \$1",$phy) if ( $status );

#debug
#    local (@comp_en) = times;
#    &dismb_diff_times ("Comp CPU time:", "comp_en", "comp_st");
#    &dismb_add_times ("all_comp_time", "comp_en", "comp_st");
#
#    local($iff_lines,$iff_path,$iff_locked);

    if (! $mult_IFF) {
      $status = eval '&dismb_parse::process_iff()';
      &dismb_msg::fixeval;
      &dismb_msg::die ("Failure processing iff: \$1", $@) if $@;
      &dismb_msg::die ("Failed to process iff for \$1",$phy) if ( $status );
    }

    return 0;
}


#returns 0 on success
# $tmpif, $iff, $phy, $lan, $dir, $fla) are preset
sub dis_parse_CPP {
    my($status);
    
    my($mult_IFF) = &dismb_parse::in_multiple_IFF_mode();
    $status = eval '&dismb_parse::compile($mult_IFF)';
    &dismb_msg::fixeval;
    &dismb_msg::die ("Failure parsing: \$1", $@) if $@;
    &dismb_msg::die ("Failed compiling \$1",$phy) if ( $status );

    if (! $mult_IFF) {
      $status = eval '&dismb_parse::process_iff()';
      &dismb_msg::fixeval;
      &dismb_msg::die ("Failure processing iff: \$1", $@) if $@;
      &dismb_msg::die ("Failed to process iff for \$1",$phy) if ( $status );
    }

    return 0;
}

# returns 0 on success, $host, $cpu are preset
sub dismb_parse::one_file {
    local ($iff,$phy,$lan,$dir,$fla) = @_; # iff, file, lang, dir, flags

    &dismb_msg::die ("Not supported language \$1",$lan) if ( ! &dismb_parse::is_good_language ($lan) );

    my($fun) = "dis_parse_" . "$lan";
    &dismb_msg::die ("Parser \$1 is not defined for language \$2",$fun,$lan) if (! defined $fun);
    &dismb_msg::die ("Could not read file \$1",$phy) if ( ! -r $phy );

    if ( -f $iff ) {
	&dismb_msg::inf ("File \$1 is up to date. iff exists \$2",$phy,$iff);
	return 0;
    }
    if ( -f $iff.lck ) {
	&dismb_msg::inf ("File \$1 is up to date. lock exists \$2",$phy,"$iff.lck");
	return 0;
    }

    local($tmpif) = "";
    my($mult_IFF) = &dismb_parse::in_multiple_IFF_mode();

    if ($mult_IFF && $lang =~ /^C$/i || $lang =~ /^CPP$/i ) {
      $mult_IFF = 1;
    } 
    else {
      $mult_IFF = 0;
    }
    
    if (! $mult_IFF ) {
        $tmpif = "$ENV{'TMPDIR'}/tmpif.$$";
        &dismb_lib::create_dir_for_file ($tmpif);
        unlink $tmpif;
        &dismb_msg::die ("Failed to cleanup iff \$1",$tmpif) if (-f $tmpif);
    }

    my($fff) = eval '&dismb_lib::expand_env_vars ($fla)';
    if ($@) {
	&dismb_msg::fixeval;
	&dismb_msg::err ("Expanding env variables in flags \$1, \$2", $@, $fla);
    } else {
	$fla = $fff;
    }
	    
    &dismb_msg::vrb ("\$1: file = \$2 dir = \$3 flags = \$4 iff = \$5 tmpif= \$6",$fun,$phy,$dir,$fla,$iff,$tmpif);

    ($dir = $phy ) =~ s/[\/\\][^\/\\]*$// if ($dir eq "." || $dir eq "NULL");
    if ($dismb::is_NT) {
        if ($dir =~ /^[a-zA-Z]:$/) { #if dir is just drive letter add '/'
          $dir .= "/";
        }
    }
    &dismb_msg::die ("Non existing directory \$1",$dir) if ( ! -d $dir );
    &dismb_print ("Parsing \$1 file: \'\$2\'",$lan,$phy);

    my $old_dir = $ENV{'PWD'};
 
   # Change working directory:
    &dismb_print ("cd \"$dir\"");
    &dismb_msg::die ("Can not change to directory \'\$1\': \$2",$dir,$!) if (! &chdir($dir));

    # $tmpif, $iff, $phy, $lan, $dir, $fla are preset
    my($res) = eval '&$fun';

    if (! $mult_IFF) {
        unlink ($tmpif);
    }

    # restore PWD
    &chdir ($old_dir);
 
    &dismb_msg::fixeval;
    &dismb_msg::err ("Failure in \$2 parsing: \$1", $@, $lan) if ($@);

    return $res;
}

# returns 0 on success, $host, $cpu are preset
sub dismb_parse::java_file_group {
    my ($key) = $_[0];
    my($res) = 0;
    $maximum_java_group_size = &dismb_prefs::get("MBmaximumJavaGroupSize");
    if ( exists( $java_prs_groups{$key} ) ) {
        my($package_name);
        local($tmpif, $iff, $lan, $dir, $fla);
        $lan = "java";
	($package_name, $dir, $fla) = split(",", $key); #comma separated key
	
	$tmpif = "$ENV{'TMPDIR'}/tmpif.$$";
	&dismb_lib::create_dir_for_file ($tmpif);

	my($fff) = eval '&dismb_lib::expand_env_vars ($fla)';
	if ($@) {
	    &dismb_msg::fixeval;
	    &dismb_msg::err ("Expanding env variables in flags \$1, \$2", $@, $fla);
	} else {
	    $fla = $fff;
	}

	&dismb_msg::die ("Non existing directory \$1",$dir) if ( ! -d $dir );
	my $old_dir = $ENV{'PWD'};
 
	# Change working directory:
	&dismb_print ("\ncd \"$dir\"");
	&dismb_msg::die ("Can not change to directory \'\$1\': \$2",$dir,$!) if (! &chdir($dir));
        my(@files_inf) = @{ $java_prs_groups{$key} }; #get the array of files
	local(%comp_list);
	my($prs_num, $failed_num, $cur_group_size) = (0, 0, 0);
        my($i) = 0;
      LOOP:
        for $i (0..$#files_inf ) {
            my($file, $msg_grp_id) = split("\t", $files_inf[$i]);
            if ( ! -r $file ) {
                &dismb_msg::err ("Could not read file \$1",$file);
                next LOOP;
            }

            my($iffile) = $nif_arr{$file};
            if ( -f $iffile ) {
	        &dismb_msg::inf ("File \$1 is up to date. iff exists \$2",$file,$iffile);
                next LOOP;
            }
            if ( -f $iffile.lck ) {
	        &dismb_msg::inf ("File \$1 is up to date. lock exists \$2",$file,$iffile.lck);
                next LOOP;
            }
            
            &dismb_print ("Parsing java file: \'\$1\'",$file);
            $comp_list{$file} = $msg_grp_id;
            $cur_group_size++;
	    $prs_num++;
            if ( $cur_group_size >= $maximum_java_group_size || $i == $#files_inf ) {
		unlink $tmpif;
		&dismb_msg::die ("Failed to cleanup iff \$1",$tmpif) if (-f $tmpif);
		&dismb_msg::inf ("package name = \$1, dir = \$2, flags = \$3, tmpif= \$4",$package_name,$dir,$fla,$tmpif);
		
                &dismb_msg::inf("Number of files in the current invocation of aset_jfe: \$1",$cur_group_size
);
                $iff = $iffile;

		# $tmpif, $iff, $lan, $dir, $fla are preset
		$res = eval '&dis_parse_java()';
	      
		&dismb_msg::fixeval;
		&dismb_msg::err ("Failure in Java parsing for package \$1 : \$2",$package_name, $@) if ($@);
                $failed_num += $res;
                $cur_group_size = 0;  #reset current group size
                %comp_list = ();
	    }
        }
	# restore PWD
	&chdir ($old_dir);
	if ($prs_num <= 0) {
	    &dismb_msg::inf("All files in the java package \$1 are upto date", $package_name);
	    return 0;
	}
	if ($failed_num) {
	    &dismb_msg::err ("Parse failed for \$1 files within package: \$2", $failed_num, $package_name);
	}
    }
    else {
        &dismb_msg::die("Failed to find java parse file group for \$1.", $key);
    }
    return $res;
}

sub dismb_parse::run {
    #create new message group for init parse pass data structures
    &dismb_msg::push_new_message_group("initialization within parse pass");

#debug
#   local(@cpu_run) = times;
    local($host, $cpu) = @_;
    if ( -z $prs_file ) {
	&dismb_msg::wrn("Parse file \$1 is empty. Skip parse pass",$prs_file); 
	return 0;
    }
    &dismb_msg::die ("Failed to initialize parse; host= \$1 cpu= \$2",$host,$cpu) if (! &dismb_parse::init($host, $cpu) );
    if (&dismb_prefs::get ('MBlogStatus') eq "yes" ) {
	select SAVESTDOUT; $| = 1; select STDOUT;
    }
    if (&dismb_prefs::get('MBincludeAllHdrs') eq "yes") {
        $include_all_headers = 1;
    }
    #pop new message group created for init parse pass data structures
    &dismb_msg::pop_new_message_group;

    my($ln,$line,$res);
  LOOP:
    while ( 1 ) {
	my $ind = &dismb_lib::get_locked_linenum ($prs_line);
	last LOOP if ( ! $ind || $ind <= 0 );
	$ln = $ind - 1;

	last LOOP if ( $ln > $#prs_arr || ! defined @prs_arr[$ln] );

	$line = @prs_arr[$ln];
	$line =~ s/\s+$//;
	last LOOP if ( $line eq "" );

	$res = 1;
        #following is prs_arr line format for non Java files
	if ( $line =~ /^\"([^\"]*)\"\s+\"([^\"]*)\"\s+\"([^\"]*)\"\s+\"([^\"]*)\"\s+\"([^\"]*)\"(.*)$/ ) { 
	    #create new message group for each file attempted to parse
	    my($mgroup_id) = $3;
	    if ($mgroup_id ne "") {
		&dismb_msg::init_auto_message_group($mgroup_id);
	    }
	    else {
	        &dismb_msg::push_new_message_group("$2");
	    }

	    $res = eval '&dismb_parse::one_file ( $1, $2, $4, $5, $6 )'; # iff, file, lang, dir, flags
	    &dismb_msg::fixeval;
	    &dismb_msg::err ("Failure for one file: \$1", $@) if $@;
	    
	    #pop new message group created for each file attempted to parse
	    if ($mgroup_id eq "") {
		&dismb_msg::pop_new_message_group;
	    }
	    #report progress about done parsing one source file
	    &dismb_msg::report_progress("parse_file"); 
	    
	    if ($res) {
		&dismb_msg::err ("Parse failed  \$1",$2);
	    }
	    if ($mgroup_id ne "") {
		&dismb_msg::pop_new_message_group;
		&dismb_msg::inf("Parse of \$1 complete. See \"compilation units\" section for details.", $2);
	    }
	}
	elsif ( $line =~ /^(.*),(.*),(.*)$/ ) {   #prs_arr "," separated line format # @#$
	    #create new message group for all files in a directory 
	    my($pkg,$cdir,$flgs) = ($1,$2,$3);
	    &dismb_msg::push_new_message_group("files with a common package name, compilation dir, and flags");
	    &dismb_msg::inf("package name: $pkg, compilation dir: $cdir, flags: $flgs");
	    
	    $res = eval '&dismb_parse::java_file_group ($line)'; 
	    &dismb_msg::fixeval;
	    &dismb_msg::err ("Failure for grouped files: \$1", $@) if $@;

	    #pop new message group created for all files in a directory 
	    &dismb_msg::pop_new_message_group;
	}
        else {
            &dismb_msg::err("Wrong format of line: \$1", $line);
        }
	my $cnt = $#prs_arr + 1;
	if ($res) {
	    &dismb_lib::status ("$pass $cnt $ind failed");
	} else {
	    &dismb_lib::status ("$pass $cnt $ind succeeded");
	}
    }

    &dismb_lib::close2pipe('dis_path');    
}


sub dismb_parse::prepare {
    if ( ! &dismb_prefs::is_continue_mb ) {
	local(*NIF,*TODO);
	open (NIF, "<$nif_file")  || &dismb_msg::die ("Failed to read \$1: \$2",$nif_file,$!);
	open (TODO,">$prs_todo") || &dismb_msg::die ("Failed to create \$1: \$2",$prs_todo,$!);
	while (<NIF>) {
	    if (/^\"([^\"]+)\"\s+\"([^\"]+)\"/) {
		local($iff,$phy) = ($1,$2);
		(print TODO $phy, "\n") or &dismb_msg::write_failed($prs_todo, $!);
		unlink ( "$iff" ); unlink ( "$iff.lck" ); unlink ( "$iff.ext" );
		unlink ($iff . $compress_ext) if ( $compress_ext );
	    }
	}
	close NIF; close TODO;
	unlink ($prs_line);
    } else {
        my($from_beginning) = (&dismb_prefs::is_earlier_pass(&dismb_prefs::get('MBcontinue'), "parse")) ? 1 : 0;
        &dismb_msg::inf("Continue pass parse from the beginning.") if ($from_beginning);
        &dismb_msg::inf("Cleaning up IF/pset/lock files.");

        #If continuing parse from the beginning delete all IF/pset files.
        #If continuing parse from the middle, delete only those IF files 
        #from previous session that could be incomplete.
        my($filearr_ref) = &dismb_lib::find_files_with_pattern("$ENV{'ADMINDIR'}/model/pset", ".pset.iff");
        my($lckf, $iff, $psetf);
        foreach $iff (@$filearr_ref) {
	    $lckf = $iff  .  ".lck";
            if ($from_beginning) {    #doing parse from the beginning
                &dismb_msg::vrb("Deleting IF file: $iff");
		unlink($iff);
                ($psetf = $iff) =~ s%.iff$%%; 
                if ( -f $psetf ) {
                  &dismb_msg::vrb("Deleting pset file: $psetf");
		  unlink($psetf);
                }
            } else {                 #continuing parse from the middle 
                #If lock file corresponding to an IF file exists, then delete 
                #the IF so that it will be handled in the current session. 
                #Existence of lock implies that the IF file could be 
                #incomplete leading to errors later on.
                if ( -f $lckf ) {
                    &dismb_msg::vrb("Deleting incomplete IF file: $iff");
		    unlink($iff);
                }
            }
	}

        #remove lock files in all cases
        $filearr_ref = &dismb_lib::find_files_with_pattern("$ENV{'ADMINDIR'}/model/pset", ".pset.iff.lck");
	foreach $lckf (@$filearr_ref) {      
	    &dismb_msg::vrb("Deleting lock file: $lckf");
	    unlink($lckf);
	}
	unlink ($prs_line);
    }
    &dismb_parse::add_package_names();
    &dismb_msg::report_progress("parse preparations");
}


sub dismb_parse::add_package_names {
    #Add package name of the java file to flags which will used only to group 
    #the files together for parsing. The package name is calculated here so 
    #that each instance of parallel parse does not have to open java files 
    #to get the name of the package. 
    my($java_cnt) = 0;
    if ( open (PRS, "<$prs_file") ) { 
	# Count Java files.
	my($line);
	while ( $line = <PRS> ) {
	    $line =~ s/\s+$//;
	    if ( $line =~ /^\"[^\"]*\"\s+\"[^\"]*\"\s+(|\"[^\"]*\"\s+)\"([a-zA-Z][^\"]*)\"\s+\"[^\"]*\"/ ) {
		my($lang) = $2;
		if ( &dismb_parse::is_java($lang) ) {
		    $java_cnt += 1;
		}
	    }
	}
	close PRS;
    } else {
	&dismb_msg::err ("Failed to open \$1:\$2",$prs_file,$!);
    }
    if ($java_cnt == 0) {
	# Nothing to do.
    }
    elsif ( open (PRS, "<$prs_file") && open (NEWPRS, ">$prs_file.new") ) { 
	my($line);
	my($changed) = 0;
	&dismb_msg::inf("Scanning \$1 Java files for package names.", $java_cnt);
	&dismb_msg::define_progress_steps("parse preparations","scan Java file",$java_cnt);
	while ( $line = <PRS> ) {
	    $line =~ s/\s+$//;
	    if ( $line =~ /^\"([^\"]*)\"\s+\"([^\"]*)\"\s+\"([a-zA-Z][^\"]*)\"\s+\"([^\"]*)\"(.*)$/ ) {
		# old format: Third element is language.
		my($iff, $phy, $lang, $dir, $flags) = ($1, $2, $3, $4, $5);
		if ( &dismb_parse::is_java($lang) ) {
		    my($package_name) = &dismb_lib::package_name_of_java_file($phy);
		    $flags = "--package_name \"$package_name\" " . $flags;
		    $changed = 1;
		}
		(print NEWPRS "\"$iff\" \"$phy\" \"$lang\" \"$dir\" $flags\n") or &dismb_msg::write_failed("$prs_file.new", $!) ;
	    }
	    elsif ( $line =~ /^\"([^\"]*)\"\s+\"([^\"]*)\"\s+\"([^\"]*)\"\s+\"([^\"]*)\"\s+\"([^\"]*)\"(.*)$/ ) {
		# new format: Third element is message group id.
		my($iff, $phy, $mgroup_id, $lang, $dir, $flags) = ($1, $2, $3, $4, $5, $6);
		if ( &dismb_parse::is_java($lang) ) {
		    my($package_name) = &dismb_lib::package_name_of_java_file($phy);
		    $flags = "--package_name \"$package_name\" " . $flags;
		    $changed = 1;
		}
		(print NEWPRS "\"$iff\" \"$phy\" \"$mgroup_id\" \"$lang\" \"$dir\" $flags\n") or &dismb_msg::write_failed("$prs_file.new", $!);
	    } else {
		(print NEWPRS "$line\n") or &dismb_msg::write_failed("$prs_file.new", $!);
	    }
	}
	close PRS;
	close NEWPRS;
	if ($changed) {
	    &dismb_lib::rename("$prs_file", "$prs_file.old");
	    if (&dismb_lib::rename("$prs_file.new", "$prs_file")) {
		&dismb_lib::unlink("$prs_file.old");
	    } else {
		&dismb_msg::wrn("Failed to rename \$1 to \$2, restoring previous file.", "$prs_file.new", $prs_file);
		&dismb_lib::rename("$prs_file.old", "$prs_file");
	    }
	} else {
	    #no difference between the old and new files
	    &dismb_lib::unlink("$prs_file.new");
	}
    } else {
	&dismb_msg::err ("Failed to open \$1:\$2",$prs_file,$!);
    }
}

#This subroutine performs 2 things:
#  Stores all IF/PSET files in the specified file, to be used by pdfgen
#  Saves new IF files in all.pmod.nif file
sub dismb_parse::find_IF_files {
  my($ifflist_file) = $_[0];

  #find files with ".pset" or ".pset.iff" extension in $ADMINDIR/model/pset
  my($root) = "$ENV{'ADMINDIR'}/model/pset";
  my($extension) = ".pset(.iff)?";
  my($psetIF_files_arr_ref) = &dismb_lib::find_files_with_pattern($root, $extension);

  &dismb_msg::die("No pset/IF files found.") if ($#$psetIF_files_arr_ref < 0);
  open(IFFS_FILE, ">$ifflist_file" ) or &dismb_msg::die("Could not open \"$ifflist_file\".");
  my($iff, $path, $physical_name, $is_iff);
  my(%new_headers_arr) = ();
  my($new_hdr_cnt) = 0;
  &dismb_parse::init_nif_arr() if ( ! defined(%nif_arr) );
  foreach $iff (@$psetIF_files_arr_ref) {
      ($path = $iff ) =~ s%^$root%%g; $path =~ s%${extension}$%%g;
      (print IFFS_FILE "$path\n") or  &dismb_msg::err("Failed to write to IF list file, : \$2", $ifflist_file, $!);
      $physical_name = &dismb_lib::get_physical_name("$path", "");
      $is_iff = ($iff =~ /.iff$/) ? 1 : 0;
      if ($is_iff && defined(%nif_arr) && !defined($nif_arr{$physical_name})) {
          $iff =~ tr [A-Z] [a-z] if (! &dismb_lib::preserve_case()); 
          $new_headers_arr{$physical_name} = $iff;
          $new_hdr_cnt++;
      }
  }
  close(IFFS_FILE);

  #if some new header files have been found add them to all.pmod.nif($nif_file)
  if ( $new_hdr_cnt > 0 ) {
      my($hdr_file, $nif_prs_line, $logical_nm);
      open (NIF, "+<$nif_file")  || &dismb_msg::err ("Failed to open \$1: \$2",$nif_file,$!);
      if (&dismb_prefs::is_continue_mb() && $dismb_analysis::project_name eq "") {
	  my($line) = <NIF>;
          my(@arr) = split("\" \"", $line);
          $dismb_analysis::project_name = @arr[2];
      }
      seek(NIF, 0, 2) || dismb_msg::err("Seek failed on file, \$1: \$2", $nif_file, $!);
      foreach $hdr_file ( keys (%new_headers_arr) ) {
	  $logical_nm = &dismb_lib::get_logical_name($hdr_file, $dismb_analysis::project_name);
          &dismb_msg::vrb("Adding new header file to all.pmod.nif: \$1", $hdr_file);
	  $nif_prs_line = "\"$new_headers_arr{$hdr_file}\" \"$hdr_file\" \"$dismb_analysis::project_name\" \"$logical_nm\" \"$dismb_analysis::project_name\"\n";
	  (print NIF $nif_prs_line) or &dismb_msg::write_failed($nif_file, $!);
      }
      close(NIF);
  }
}

#creates build.prefs file in $ADMINDIR/prefs directory, if the file doesn't exist.
sub dismb_parse::generate_prefs
{
  my($proj_name, $pdf_name, $server_pdfname) = @_;
  my($prefs_file) = "$ENV{'ADMINDIR'}/prefs/build.prefs";
  &dismb_msg::die("Could not create prefs directory for $prefs_file") if ( &dismb_lib::create_dir_for_file($prefs_file) );
  open(PREFS_FILE, ">>$prefs_file") or &dismb_msg::die("Could not open \"$prefs_file\".");

  #Add some preferences
  print PREFS_FILE "\n";
  print PREFS_FILE "*psetPrefs.ADMINDIR:	$ENV{'ADMINDIR'}\n";
  print PREFS_FILE "*psetPrefs.defaultPDF:	$server_pdfname\n";
  print PREFS_FILE "*psetPrefs.pdfFileBuild:	$pdf_name\n";
  print PREFS_FILE "*psetPrefs.pdfFileUser:	$pdf_name\n";
  print PREFS_FILE "*psetPrefs.projectHome:	$proj_name\n";

  print PREFS_FILE "*psetPrefs.cPlusFlags: \n";
  print PREFS_FILE "*psetPrefs.cPlusDefines: \n";
  print PREFS_FILE "*psetPrefs.cPlusIncludes: \n";
  print PREFS_FILE "*psetPrefs.cFlags: \n";
  print PREFS_FILE "*psetPrefs.cDefines: \n";
  print PREFS_FILE "*psetPrefs.cIncludes: \n";

  close(PREFS_FILE);
}


# Invoke pdfgen executable
sub dismb_parse::generate_pdf
{
  my($src_root, $project_name) = @_;
  $project_name =~ s%^/%%;
  my($pdf_name) = "$ENV{'ADMINDIR'}/pdf/build.pdf";
  my($server_pdf) = "$ENV{'ADMINDIR'}/pdf/server.pdf";
  &dismb_msg::die("Could not create pdf directory for $pdf_name") if ( &dismb_lib::create_dir_for_file($pdf_name) );

  my($ifflist_file) = "$ENV{'DISBUILD_TMPDIR'}/iff_files.lst";
  if ( !-f $ifflist_file) {
    &dismb_parse::find_IF_files($ifflist_file);
  }

  if ( -z  $ifflist_file ) {
    &dismb_msg::die("\"$ifflist_file\" is empty.");
  }
  my $pdfgen_exe = "$psethome_bin/pdfgen";
  if ($dismb::is_NT) { 
    $pdfgen_exe .= ".exe";
  }
  my(@args, $i) = (0,0);
  $args[$i++] = $pdfgen_exe;
  $args[$i++] = "-project"; $args[$i++] = $project_name;
  $args[$i++] = "-src_root"; $args[$i++] = $src_root; 
  $args[$i++] = "-ifflist"; $args[$i++] = $ifflist_file;
  $args[$i++]= "-pdf"; $args[$i++] = $pdf_name;
  $args[$i++]= "-server_pdf"; $args[$i++] = $server_pdf;
 
  #add lpservice and message_group options
  push @args, &dismb_msg::lp_flags();

  dismb_msg::inf("Running pdf generator: \$1", "@args");
  my($status) = &dismb_lib::system(@args);
  if ($status) {
    &dismb_msg::die("pdf generator failed with a return value of \$1.", &dismb_lib::get_return_value($status));
  }
  if ( ! -e $pdf_name ) {
    &dismb_msg::die("Pdf file $1 does not exist.", $pdf_name);
  }

  #generate build.prefs file
  &dismb_parse::generate_prefs($project_name, $pdf_name, $server_pdf);
}

#returns: 1 on success
#         0 on failure
sub dismb_parse::open_summary_file {
    local (*FF) = @_;
    my $ret = 1;
    my $upto_date = "$ENV{'DISBUILD_TMPDIR'}/$pass.upto_date";

    if (! open (FF, ">$upto_date") ) {
	&dismb_msg::err ("Failed to open summary file \$1: \$2",$upto_date,$!);
	$ret = 0;
    } 
    return $ret;
}


sub dismb_parse::print_and_log {
    my($do_log) =  shift ;
    local(*FF) =  shift ;
    &dismb_print(@_);
    if ($do_log) {
        (print FF &dismb_msg::substitute_arguments(@_)) or &dismb_msg::die("Failure writing to log file : \$1", $!);
    }
}


#Stores all IF/pset files in $ADMINDIR/model/pset in iff_files.lst file
sub dismb_parse::gather_IF_files {
    my($ifflist_file) = "$ENV{'DISBUILD_TMPDIR'}/iff_files.lst"; #file used by pdf generator
    &dismb_parse::find_IF_files($ifflist_file); 
}

sub dismb_parse::summary {
    require ("dismb_parse.pl");
    &dismb_parse::gather_IF_files() if (! $dismb::standalone_mode );
    unlink ($nif_parsed); unlink ($parsed_projects);
    local (*UPTO); my $do_log = &dismb_parse::open_summary_file (\*UPTO);
    my $prs_file_size = 0;

    local(*SRC, *NIF, *PRS, *LST, *MIS);
    my($line, $pmod, $root, %iffs);
    my($total, $succeeded, $all, $good, $src_succ) = (0,0,0,0,0); 
    &dismb_parse::print_and_log ($do_log, \*UPTO, "  ===================== Parse Summary ======================");

    my $fmt2 = "    @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< @<<<<<<<@<<<<<<<@<<<<<<<<";
    #               /top/proj2                         4       4       4

    if      ( !open (NIF, "<$nif_file") ) {
	&dismb_msg::err ("Failed to open \$1", $nif_file);
    } elsif ( !open (PRS, ">$nif_parsed") ) {
	&dismb_msg::err ("Failed to open \$1", $nif_parsed);
    } elsif ( !open (LST, ">$parsed_projects") ) {
	&dismb_msg::err ("Failed to open \$1", $parsed_projects);
    } elsif ( !open (SRC, "<$prs_file") ) {
	&dismb_msg::err ("Failed to open \$1", $prs_file);
    } elsif ( !open (MIS, ">$prs_missed") ) {
	&dismb_msg::err ("Failed to open \$1", $prs_missed);
    } else {
        my($ln);
	while ( $ln = <SRC>) {
	    $prs_file_size++ ;
            my(@prsline) = split ("\" \"", $ln); @prsline[0] =~ s/^\"//;
	    if ( -f @prsline[0] ) {
		$src_succ++;
            }
	}

	&dismb_parse::print_and_log ($do_log, \*UPTO, &dismb_lib::formline($fmt2, "Projects", "Needed", "Parsed", "Missing"));
	
	IFFS: while ( $line = <NIF> ) {
	    my(@line) = split ("\" \"", $line);
	    @line[0] =~ s/^\"//; @line[$#line] =~ s/\"\s+$//;
	    next IFFS if (defined $iffs{@line[0]});
	    $iffs{@line[0]} = 'yes';
	    if ( $pmod && $pmod ne @line[2] ) {
		&dismb_parse::print_and_log ($do_log, \*UPTO, &dismb_lib::formline($fmt2, $pmod, $all, $good, $all - $good));
		$total += $all; $succeeded += $good;
                if ($good) {
		    (print LST "\"$pmod\" \"$root\"\n") or &dismb_msg::write_failed($parsed_projects, $!); 
                }
		$all = 0; $good = 0;
	    }

	    $root = @line[$#line]; $pmod = @line[2]; $all++;
	    if ( -f @line[0] ) {
		$good++;
                (print PRS $line) or &dismb_msg::write_failed($nif_parsed, $!);
	    } else {
		print MIS @line[1], "\n"; # msg OK missed file
	    }
	}
    }

    if ($pmod) {
	&dismb_parse::print_and_log ($do_log, \*UPTO, &dismb_lib::formline($fmt2, $pmod, $all, $good, $all - $good));
	$total += $all; $succeeded += $good;
        if ($good) { 
	    (print LST "\"$pmod\" \"$root\"\n") or &dismb_msg::write_failed($parsed_projects, $!);
        } 
    }

    close NIF if (defined NIF);
    close PRS if (defined PRS);
    close LST if (defined LST);
    close MIS if (defined MIS);

    if ($dismb_parse::prs_count != $prs_file_size) {
	&dismb_parse::print_and_log ($do_log, \*UPTO, "warning", "Attempted to parse only $dismb_parse::prs_count files out of all $prs_file_size");
    }

    if ( $succeeded ) {
	my $missing  = $total - $succeeded;
	my $oth_iffs = $total - $prs_file_size;
	my $oth_succ = $succeeded - $src_succ;
	my $oth_miss = $oth_iffs - $oth_succ;
	my $src_miss = $prs_file_size - $src_succ;
	my $forg     = (&dismb_prefs::get ('acceptSyntaxErrors') eq "yes") ? " (forgiven $dismb_parse::forgive_count)" : "";

	my $frmt1 = "    @<<<<<<<<<<<@<<<<<<<<<<<<<<<@<<<<<<<<<<<<<<<@<<<<<<<<<<<<<<<<<<<<<<<<";
	#                needed      8               2               6

	&dismb_parse::print_and_log ($do_log, \*UPTO, "    Total IFFs: needed $total, parsed $succeeded, missing $missing");
	&dismb_parse::print_and_log ($do_log, \*UPTO, &dismb_lib::formline ($frmt1,    ""   , "Total", "include", "src"));
	&dismb_parse::print_and_log ($do_log, \*UPTO, &dismb_lib::formline ($frmt1, "needed", $total    , $oth_iffs, $prs_file_size));
	&dismb_parse::print_and_log ($do_log, \*UPTO, &dismb_lib::formline ($frmt1, "parsed", $succeeded, $oth_succ, $src_succ . $forg));
	&dismb_parse::print_and_log ($do_log, \*UPTO, &dismb_lib::formline ($frmt1, "missing",$missing  , $oth_miss, $src_miss));
    } else {
	&dismb_parse::print_and_log ($do_log, \*UPTO, "    Total IFFs: needed $total, parsed 0, missing $total");
    }
    &dismb_parse::print_and_log ($do_log, \*UPTO, "  ==========================================================");
    if ($dismb_parse::error_node_count > 0 ) {
        &dismb_parse::print_and_log($do_log, \*UPTO, "Total number of error nodes in all IF files: $dismb_parse::error_node_count");
    }
    close UPTO;
    &dismb_msg::report_progress("parse core");
    &dismb_msg::report_progress("parse pass");
}


sub dismb_update::summary {
    require ("dismb_parse.pl");
    unlink ($nif_updated); unlink ($updated_projects);
    local (*UPTO); my $do_log = &dismb_parse::open_summary_file (\*UPTO);
    my($total, $succeeded, $removed, $all, $good, $lines, $sum_lines) = (0,0,0,0,0,0,0); 
    my($count_lines) = 0;
    my($cleanup) = (&dismb_prefs::get("MBforceCleanup") eq "yes") ? 1 : 0;
    my($iff);

    &dismb_parse::print_and_log ($do_log, \*UPTO, "  ===================== Update Summary =====================");

    my($rem_file) = "$ENV{'DISBUILD_TMPDIR'}/all.update.rem";
    $removed = &dismb_lib::count_lines($rem_file);
    $total += $removed;

    local (*LST);
    if ( !open (LST, ">$updated_projects") ) {
	&dismb_msg::err ("Failed to open \$1", $updated_projects);
    }
    if ( !$dismb::standalone_mode && ! -f $nif_parsed && $removed > 0) {
	(print LST "\"$dismb_analysis::project_name\" \"$dismb_analysis::project_name\"") or &dismb_msg::write_failed($updated_projects, $!);
    } else {
	my $fmt;
	$count_lines = (&dismb_prefs::get('MBcountLines') &&  &dismb_prefs::get('MBcountLines') ne "no") ? 1 : 0;
	local(*NIF, *UPD);
	my($line, $pmod, $root, $pset, %iffs);
	
	if ( !open (NIF, "<$nif_parsed") ) {
	    &dismb_msg::err ("Failed to open \$1", $nif_parsed);
	} elsif ( !open (UPD, ">$nif_updated") ) {
	    &dismb_msg::err ("Failed to open \$1", $nif_updated);
	} else {
	    if ($count_lines) {
		$fmt = "    @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< @<<<<<<<@<<<<<<<@<<<<<<<<<<<";
		#           /top/proj2                         4       4       4
		&dismb_parse::print_and_log ($do_log, \*UPTO, &dismb_lib::formline ($fmt, "Projects", "Needed", "Updated", "Lines"));
	    } else {
		$fmt = "    @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< @<<<<<<<@<<<<<<<";
		#           /top/proj2                         4       4
		&dismb_parse::print_and_log ($do_log, \*UPTO, &dismb_lib::formline ($fmt, "Projects", "Needed", "Updated"));
	    }
	  IFFS:
	    while ( $line = <NIF> ) {
		my(@line) = split ("\" \"", $line);
		@line[0] =~ s/^\"//; @line[$#line] =~ s/\"\s+$//;
		next IFFS if (defined $iffs{@line[0]});
		$iffs{@line[0]} = 'yes';
		if ( $pmod && $pmod ne @line[2] ) {
		    my($s) = ($count_lines) ? &dismb_lib::formline ($fmt, $pmod, $all, $good, $lines) : &dismb_lib::formline ($fmt, $pmod, $all, $good);
		    &dismb_parse::print_and_log ($do_log, \*UPTO, "$s");
		    $total += $all; $succeeded += $good; $sum_lines += $lines;
		    if ($good) { 
			(print LST "\"$pmod\" \"$root\"\n") or &dismb_msg::write_failed($updated_projects, $!);
		    }
		    $all = 0; $good = 0; $lines = 0;
		}
	      
		$root = @line[$#line]; $pmod = @line[2]; $all++;
                $iff = @line[0];
		($pset = @line[0]) =~ s/\.iff$//;
		if ( -f $pset && &dismb_lib::is_created_after_start($pset) ) { # compare with startup time ( set in ::init_one_pass )
		    $good++;	              # file exist and younger than the script startup time
		    $lines += &dismb_lib::count_lines (@line[1]) if ($count_lines);
		    (print UPD $line) or &dismb_msg::write_failed($nif_updated, $!);
                    unlink(@line[0]) if (-f @line[0] && $cleanup);
		}
	    }
	}

	if ($pmod) {
	    my($s) = ($count_lines) ? &dismb_lib::formline ($fmt, $pmod, $all, $good, $lines)
		: &dismb_lib::formline ($fmt, $pmod, $all, $good);
	    &dismb_parse::print_and_log ($do_log, \*UPTO, "$s");
	    $total += $all; $succeeded += $good; $sum_lines += $lines;
	    if ($good) {
		(print LST "\"$pmod\" \"$root\"\n") or &dismb_msg::write_failed($updated_projects, $!);
	    }
	}
	
	close NIF if (defined NIF);
	close UPD if (defined UPD);
    }
    close LST if (defined LST);

    if ($count_lines) {
	&dismb_parse::print_and_log ($do_log, \*UPTO, "    Total: Needed $total, Updated $succeeded, Processed lines $sum_lines, Removed $removed");
	local (*LINES);
	if (open LINES, ">$lines_file") {
	    print LINES "Processed lines: ", $sum_lines, "\n"; 
	} else {
	    &dismb_msg::err ("Failed to create file \$1.",$lines_file);
	}
    } else {
	&dismb_parse::print_and_log ($do_log, \*UPTO, "    Total: needed $total, Updated $succeeded, Removed $removed");
    }
    &dismb_parse::print_and_log ($do_log, \*UPTO, "  ==========================================================");
    close UPTO;
}


sub dismb_finalize::summary {
    require ("dismb_parse.pl");
    local (*UPTO); my $do_log = &dismb_parse::open_summary_file (\*UPTO);

    local(*LST);
    my($line);
    my($lst) = "$ENV{'DISBUILD_TMPDIR'}/ROOTS.pmod.lst";
    my($all, $good) = (0,0,0,0); 
    &dismb_parse::print_and_log ($do_log, \*UPTO, "  ===================== Finalize Summary ===================");

    if ( !open (LST, "<$lst") ) {
	&dismb_msg::err ("Failed to open ROOTS \$1",$lst);
    } else {
	while ( $line = <LST> ) {
	    my(@proj) = split (' ', $line);
	    my($f,$prj,$bpc);
            $f = @proj[$#proj];
            if ($f =~ /\"$/ && $f !=~ /^\"/) {
              my($ind) = $#proj;
              WHLOOP: while (--$ind > 0) {
                 $f = "@proj[$ind] " . $f;
              last WHLOOP if (@proj[$ind] =~ /^\"/);
              }
              $f =~ s/\"//g;
            }

            $f =~ s/sym_FLDX_[0-9]+$//; $prj = $f . "prj";
	    # $bpc = $f . "bpc"; unlink ($bpc);

	    $pmod = @proj[2]; $all++;
	    if ( -f $prj && &dismb_lib::is_created_after_start($prj) ) {   # compare with startup time ( set in ::init_one_pass )
		$good++;                      # file exist and younger than the script startup time
	    } else {
		&dismb_msg::err ("no new .prj file for project \$1",$pmod);
	    }
	}
	close LST if (defined LST);
	&dismb_parse::print_and_log ($do_log, \*UPTO, "    Total PRJs: needed $all, finalized $good");
    }
    &dismb_parse::print_and_log ($do_log, \*UPTO, "  ==========================================================");
    close UPTO;
}

$dismb_parse::summary_state = 0;
$dismb_parse::summary_fname;
$dismb_parse::file_to_check;

# Predefined filehandlers: PRS_ERRORS, PRS_FAILED, PRS_FORGIV
sub dismb_parse::add_summary {
    my($line) = @_;

    if ( ! $line || ( $line =~ /^\s*$/ ) ) {
	print PRS_ERRORS ("\n" ) if ($dismb_parse::summary_state);
	$dismb_parse::summary_state = 0;
    } elsif ( $line =~ /Parsing [^ ]+ file: / ) {
	($dismb_parse::summary_fname = $') =~ s/^\s*[\'\`\"]?// ; 
	$dismb_parse::summary_fname =~ s/[\'\`\"]?\s*$// ;
	$dismb_parse::prs_count++ ;
	if ( $line =~ /^Parsing java file: '(.*)'[^\']*$/ ) {
	    $dismb_parse::file_to_check = $1;
	}
    } elsif ( $line =~ /Accepting parse errors/ ) {
	$dismb_parse::forgive_count++ ;              #For non Java files
	print PRS_FORGIV $dismb_parse::summary_fname, "\n";
    } elsif ( $line =~ /Accepting java parse errors/ ) {
	if (defined $dismb_parse::file_to_check) {
	    # No errors reported for specific files in the package.
	    $dismb_parse::forgive_count++ ;              #For Java files
	    print PRS_FORGIV "${dismb_parse::file_to_check}\n";
	}
    } elsif ( $line =~ /aset_jfe: Number of error nodes in IF for file \"([^\"]*)\" : (.*)$/ ) {
        my($filename, $num_errs) = ($1, $2);         #For Java file
	$dismb_parse::error_node_count += $num_errs ;
	$dismb_parse::forgive_count++ ;
	print PRS_FORGIV $filename, "\n";
	$dismb_parse::file_to_check = undef;

    } elsif ( $line =~ /^(catastrophe|error): Failed compiling / ) {
	$dismb_parse::fail_count++ ;
	print PRS_FAILED $dismb_parse::summary_fname, "\n";
    } elsif ( $line =~ /Foreign file: (.*)/ ) {
        my($dep_file) = $1;
	if ( &dismb_lib::is_java_dependency_file($dep_file) ) {
	  $dismb_analysis::other_dependencies{$dep_file} = "";
        }
    } elsif ( $line =~ /error:.*\"[^\"]+\", line \d+:/ ) {
	if (&dismb_prefs::get ('parserBriefDiagnostics') eq "yes") {
	    print PRS_ERRORS ("\"$dismb_parse::summary_fname\" " , $line );
	} else {
	    $dismb_parse::summary_state = 1;
	}
    }

    print PRS_ERRORS ("$dismb_parse::summary_fname " , $line) if ($dismb_parse::summary_state);
}

return 1;
