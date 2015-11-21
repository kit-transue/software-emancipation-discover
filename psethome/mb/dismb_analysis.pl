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
###############################################################################
#Procedures to perform impact/dependency analysis using saved file attribute 
#info.
#

$dismb_analysis::project_name = "";
$dismb_analysis::method = "TS";

#File to save previous model build information, with tab separated fields:
#^FULL_FILENAME\tTIMESTAMP\tLANGUAGE\tPARSE_STATE\tCOMPILATION_DIR\tPARSER_FLAGS$
$dismb_analysis::prev_attributes_file = "$ENV{ADMINDIR}/config/file_attributes.lst";
$dismb_analysis::other_dependencies_file = "$ENV{ADMINDIR}/config/other_dependencies.lst";

#Associative array to hold dependencies (like .class/.jar/.zip) of java 
#files that are not included in the pdf.
%dismb_analysis::other_dependencies = ();

#Associative array to hold previous (last model build) files and attributes
%dismb_analysis::prev_fileattr_arr = ();

#Associative array to hold current source files and flags.
%dismb_analysis::cur_parserflags_arr = ();

#Associative array to store checksums retrieved from the model
%dismb_analysis::prev_checksum_arr = ();
$dismb_analysis::checksum_arr_initialized = 0;

#Some variables to write summary of modelbuild process
$dismb_analysis::cur_attempted = 0;
$dismb_analysis::cur_succeeded = 0;
$dismb_analysis::cur_clean = 0;
$dismb_analysis::total_attempted = 0; #same as cur_attempted incase of full build
#In case of incremental model build the following two variables hold all
#files in the model and all files cleanly built in to the model respectively.
$dismb_analysis::total_succeeded = 0;
$dismb_analysis::total_clean = 0; 

sub dismb_analysis::run_discover_script
{
  my($script) = $_[0];

  if (&dismb_prefs::is_verbose_mb()) {
      $ENV{'DISCOVER_VERBOSE'} = "yes";
  }

  my($exe) = "$ENV{'PSETHOME'}/bin/model_server";
  if ( $dismb::is_NT ) {
    $exe .= ".exe";
  }
  my(@args) = ();
  push( @args, &dismb_lib::add_quotes($exe) );
  push( @args, "-batch");
  push( @args, "-pdf"); 
  push( @args, &dismb_lib::add_quotes("$ENV{'ADMINDIR'}/pdf/build.pdf") );
  push( @args, "-prefs"); 
  push( @args, &dismb_lib::add_quotes("$ENV{'DISBUILD_TMPDIR'}/mb.prefs") );
  push( @args, "-source"); 
  push( @args, &dismb_lib::add_quotes($script) );
  
  #add lpservice and message_group options
  push (@args, &dismb_msg::lp_flags());
  
  &dismb_msg::inf("Running model_server: \$1", "@args");
  my($status) = system { $exe } @args;
  if ( $status != 0) {
    &dismb_msg::err("model_server failed with return value: \$1, signal value: \$2", &dismb_lib::get_return_value($status), &dismb_lib::get_signal_value($status));
  }
}

#The following procedure uses model_server to get list of all files
#that are affected by the given list of files. For files in the model,
#logical file names are used as logical names work better even
#after a file has been physically deleted from the disk.
sub dismb_analysis::impact_analysis
{
  my($initial_outdatedarr_ref) = $_[0];
  my(%new_outdatedarr);

  my($input_file) = "$ENV{'DISBUILD_TMPDIR'}/analysis_initial.lst";
  my($output_file) = "$ENV{'DISBUILD_TMPDIR'}/analysis_final.lst";
  if ( open( IMPINF, ">$input_file" ) ) {
    my($file, $logical_filename, $val);
    my($old_files_num) = 0;
    while( ($file, $val) = each(%$initial_outdatedarr_ref) ) {
      if ( $val == 10 ) { 
        (print IMPINF "$file\n") or &dismb_msg::write_failed($input_file, $!);
        $old_files_num++;
      }
      elsif ( $val != 1 ) {      #skip new files for doing impact analysis
        $logical_filename = &dismb_lib::get_logical_name($file, $dismb_analysis::project_name);
        (print IMPINF "$logical_filename\n") or &dismb_msg::write_failed($input_file, $!);
        $old_files_num++;
      }
      #make the new outdated list, start off with all the initial files
      #except the dependency files 
      $new_outdatedarr{$file} = $val if ( $val != 10 ); 
    }
    close IMPINF;

    #if atleast one file is old, then do impact analysis 
    if ( $old_files_num > 0 ) {
      &dismb_msg::inf("Doing impact analysis on \$1 files.", $old_files_num)
      &dismb_analysis::run_discover_script("$ENV{'PSETHOME'}/lib/dismb_file_impact.dis");

      #now read the output file create by dismb_file_impact.dis
      if ( open( IMPOUTF, "<$output_file" ) ) {
        while ( $logical_filename = <IMPOUTF> ) {
          chomp($logical_filename);
          $file = &dismb_lib::get_physical_name($logical_filename, $dismb_analysis::project_name);
          if ( ! &dismb_lib::preserve_case() ) {
            $file =~ tr [A-Z] [a-z]; #convert files name to lower case
          }
          if ( !exists($initial_outdatedarr_ref->{$file}) ) {
             if ( -f $file ) {
               $new_outdatedarr{$file} = 2; #outdated, due to impact
             } else {
               $new_outdatedarr{$file} = -1; #outdated, file deleted from disk 
             }
          }
        }
        close IMPOUTF
      } else {
        &dismb_msg::err("Failed to open \$1 : \$2", $output_file, $!);
      }
    }
  } else {
    &dismb_msg::err("Failed to open \$1 : \$2", $input_file, $!);
  }
  return \%new_outdatedarr;
}

sub dismb_analysis::initialize_cur_files_and_flags
{
  my($trans_file) = "$ENV{'ADMINDIR'}/config/translator.flg" ;
  my($line);
  my($field) = "([^\t]*)"; 
  open( TRNSLTF, "<$trans_file" ) || &dismb_msg::die("Could not open \$1 : \$2", "$trans_file", $!);
  while ( $line = <TRNSLTF> ) {
    my($proj, $dir, $filename, $lang, $flags, $mgroup_id);
    chomp($line);
    #read tab separated fields from $trans_file
    #line format is: PROJECT_NAME\tCOMPILATION_DIR\tFILENAME\tLANGUAGE\tPARSER_FLAGS
    # or: PROJECT_NAME\tCOMPILATION_DIR\tFILENAME\tMSG_GRP_ID\tLANGUAGE\tPARSER_FLAGS
    my(@fields) = split("\t", $line);
    if (scalar(@fields) <= 5 and @fields[3] !~ /^\d/) {
        if ($line =~ /^${field}\t\s*${field}\t\s*${field}\t\s*${field}\t(.*)$/ ) { 
            $proj = $1; $dir = $2; $filename = $3; $lang = $4; $flags = $5;
	}
    }
    else {
        if ($line =~ /^${field}\t\s*${field}\t\s*${field}\t\s*${field}\t\s*${field}\t(.*)$/ ) { 
            $proj = $1; $dir = $2; $filename = $3; $mgroup_id = $4; $lang = $5; $flags = $6;
	}
    }
    $dismb_analysis::total_attempted++ if ( !&dismb_lib::is_header($lang));
    if (defined($dir)) {
      #set $dismb_analysis::project_name once 
      if ($dismb_analysis::project_name eq "") {
        $dismb_analysis::project_name = "/" . $proj;      #first field of each line
        #replace spaces in project name with underscore
        $dismb_analysis::project_name =~ s%\s+%_%g;
      }

      #replace back slashes with forward slashes and remove trailing slash from $dir
      $dir =~ s%\\%/%g; $dir =~ s%/$%%;
      #replace back slashes with forward slashes from $filename
      $filename =~ s%\\%/%g;
      $flags =~ s/^\s+//; $flags =~ s/\s+$//;       #trim spaces

      my($full_path) = ${filename};
      if ( !&dismb_lib::is_absolute_path($filename) ) {
        $filename =~ s%^./%%;
        $full_path = ${dir} . "/" . ${filename};
      }
      $full_path = &dismb_lib::dis_path($full_path);

      if ( !exists($dismb_analysis::cur_parserflags_arr{$full_path})) {
        $dismb_analysis::cur_parserflags_arr{$full_path} = "$lang\t$dir\t$flags\t$mgroup_id";
      } else {
	  &dismb_msg::err("Duplicate file name: \$1", $full_path);
      }
    } else {
      &dismb_msg::err("Wrong format of line in \$1, ignoring this line: \$2", $trans_file, $line);
    }
  }
  close TRNSLTF; 
}

#
#restore previous file attributes
#
sub dismb_analysis::restore_prev_file_attributes
{
  my($line, $filename, $timestamp, $compdir, $flags);
  if ( open(PREVATTRF, "<$dismb_analysis::prev_attributes_file") ) {
    $line = <PREVATTRF>;
    if ( $dismb_analysis::project_name eq "" ){
      #set project name from the first line if it is not already set
      ($dismb_analysis::project_name) = ($line =~ /^#project_name:(.*)$/);
    } 
    while ( $line = <PREVATTRF> ) {
      chomp($line);
      #Read tab separated fields
      if ( $line =~ /^([^\t]+)\t+([^\t]+)\t+([^\t]+)\t+([^\t]*)\t+([^\t]*)\t+(.*)$/ ) { 
        ($filename, $timestamp, $lang, $parse_status, $compdir, $flags) = ($1, $2, $3, $4, $5, $6);
        if ( !exists($dismb_analysis::prev_fileattr_arr{$filename}) ) {
          $dismb_analysis::prev_fileattr_arr{$filename} = "$timestamp\t$lang\t$compdir\t$flags";
          if ( !&dismb_lib::is_header($lang) ) {
            $dismb_analysis::total_succeeded++;
	    $dismb_analysis::total_clean++ if ( $parse_status eq "clean" );
          }
        }
      }
    }
    close PREVATTRF;
  } else {
    &dismb_msg::die("Could not open file \$1 : \$2", $dismb_analysis::prev_attributes_file, $!);
  }
  &dismb_analysis::restore_other_dependencies();
}

sub dismb_analysis::restore_other_dependencies
{
  if ( -f $dismb_analysis::other_dependencies_file && open(DEP_F, "<$dismb_analysis::other_dependencies_file") ) {
    my($line, $filename, $ts);
    while ( $line = <DEP_F> ) {
      chomp($line);
      ($filename, $ts) = split("\t", $line);
      $dismb_analysis::other_dependencies{$filename} = $ts;
    }
    close DEP_F;
  } elsif ( -f $dismb_analysis::other_dependencies_file ) {
    &dismb_msg::err("Failed to open dependencies file, \$1 : \$2", $dismb_analysis::other_dependencies_file, $!);
  }
}

sub dismb_analysis::initialize_prev_checksum_arr
{
  &dismb_msg::inf("Getting stored checksums from the model.");

  $dismb_analysis::checksum_arr_initialized = 1;
  my($checksums_file) = "$ENV{'DISBUILD_TMPDIR'}/checksums.lst";
  unlink($checksums_file);

  &dismb_analysis::run_discover_script("$ENV{'PSETHOME'}/lib/checksums.dis");

  if ( open(CHKSUMF, "<$checksums_file") ) {
    while ( $line = <CHKSUMF> ) {
      chomp($line);
      ($file, $chksum) = split(", ", $line);
      if ( !&dismb_lib::preserve_case() ) {
        $file =~ tr [A-Z] [a-z]; #convert files name to lower case
      }
      if ( !exists($dismb_analysis::prev_checksum_arr{$file}) ) {
        $dismb_analysis::prev_checksum_arr{$file} = $chksum;
      }
    }
    close CHKSUMF;
  } else {
    &dismb_msg::err("Failed to open checksums file \$1 : \$2.", $checksums_file, $!);
  }
}

#
#Listed below are the codes of an outdated file and a brief description.
# CODE    DESCRIPTION
#  -2     file removed from the current list(exists in baseline model)
#  -1     file physically deleted from the disk
#   0     file is not outdated
#   1     new file(file not in the baseline model)
#   2     impacted file, file marked outdated after impact analysis
#   3     time stamp is different
#   4     checksum is different
#   5     user specified file
#   6     flags are different
#   7     compilation directory is different
#   8     lang is different
#   9     pset file is missing
#   10    dependency file(for java, .class/.jar/.zip files that are not
#                         included in the list of files that go into model)
#

sub dismb_analysis::are_flags_different
{
  my($filename) = $_[0];
  my($outdated) = 0;
  my ($prev_ts, $prev_lang, $prev_dir, $prev_flags) = split("\t", $dismb_analysis::prev_fileattr_arr{$filename});
  if ( ! &dismb_lib::is_header($prev_lang) ) {
    my($cur_lang, $cur_dir, $cur_flags, $mgroup_id);
    if ( exists($dismb_analysis::cur_parserflags_arr{$filename}) ) {
      ($cur_lang, $cur_dir, $cur_flags, $mgroup_id) = split("\t", $dismb_analysis::cur_parserflags_arr{$filename});

      #compare previous and current values
      $outdated = ($cur_lang ne $prev_lang) ? 8 : 0; #outdated, lang different
      if ( !$outdated ) {
        $outdated = ($cur_dir ne $prev_dir) ? 7 : 0; #outdated, compdir different
        if ( !$outdated ) {
          $outdated = ($cur_flags ne $prev_flags) ? 6 : 0; #outdated, flags different
        }
      }
    } else {
      $outdated = -2; #outdated, file deleted from list
    }
  }
  return $outdated;
}

#
#returns 0 if the timestamp (last modified time) is same as the specified
#timestamp, returns non zero value otherwise
#
sub dismb_analysis::is_timestamp_different
{
  my($filename, $prev_timestamp) = @_;
  my($outdated, $cur_timestamp) = (0, 0);
  $cur_timestamp = dismb_lib::last_modified_time($filename);
  $outdated = (!$cur_timestamp || !$prev_timestamp || $cur_timestamp != $prev_timestamp) ? 3 : 0; #outdated, timestamp different
  return $outdated;
}

#
#returns 0 if the checksum of file is same as the previously saved 
#checksum(in the model), returns non zero value otherwise
#
sub dismb_analysis::is_checksum_different
{
  my($filename) = $_[0];
  my($outdated, $cur_checksum, $prev_checksum) = (0, 0, 0);
  if ( ! $dismb_analysis::checksum_arr_initialized ) {
    &dismb_analysis::initialize_prev_checksum_arr();
  }
  if ( exists($dismb_analysis::prev_checksum_arr{$filename}) ) {
    $prev_checksum = $dismb_analysis::prev_checksum_arr{$filename};
  }
  $cur_checksum = &dismb_lib::checksum_of_file($filename);
  $outdated = (!$cur_checksum || !$prev_checksum || $cur_checksum ne $prev_checksum) ? 4 : 0; #outdated, checksum different
  return $outdated;
}

#return 0 if file has not been modified
#non zero otherwise
sub dismb_analysis::is_file_modified
{
  my($filename, $ts) = @_;
  my($outdated) = 0;
  if ( $dismb_analysis::method eq "TS" ) {
    $outdated = &dismb_analysis::is_timestamp_different($filename, $ts);
  }
  elsif ( $dismb_analysis::method eq "CS" ) {
    $outdated = &dismb_analysis::is_checksum_different($filename);
  }
  elsif ( $dismb_analysis::method eq "TS_CS" ) {
    if ( &dismb_analysis::is_timestamp_different($filename, $ts) != 0 ) {
      $outdated = &dismb_analysis::is_checksum_different($filename);
    }
  }
  return $outdated;
}

#
#Automatic analysis of whether a file in the model is out-of-date
#with the existing file either because of changes to parser flags
#or because of modifications to the file itself.
#
sub dismb_analysis::is_file_outdated
{
  my($filename) = $_[0];
  my($outdated) = 0;
  if ( !-f $filename ) {
    $outdated = -1; #outdated, file deleted from disk
  } else {
    #first check if parser flags have been changed
    $outdated = &dismb_analysis::are_flags_different($filename);
    if ( !$outdated ) {
      #parser flags for the file are same, now check if the file changed
      ($pts, $pl, $pd, $pflags) = split("\t", $dismb_analysis::prev_fileattr_arr{$filename});
      $outdated = &dismb_analysis::is_file_modified($filename, $pts);
    }
  }
  if ( !$outdated && !-f &dismb_lib::pset_filename_of($filename) ) {
    $outdated = 9; #outdated, pset does not exist
  }
  return $outdated;
}

#
#This procedure tries a guess parser flags for the given file by searching
#initially for a file in a same directory as the given file and with the 
#same suffix. If there is no file then search will proceed with the parent
#directory and so on untill either a file is found or all the directories 
#are exhausted. If there are no files within the same directory structure,
#then use the first file that has the smae suffix.
#
sub dismb_analysis::guess_parser_flags
{
  my($filename) = $_[0];
  my($targdir, $targsuffix, $f, $val, $found, $suff);
  my($ts, $lang, $dir, $flags, $mimic_file) = ("", "", "", "", "");
  
  $targsuffix = &dismb_lib::suffix_of_file($filename);
  $targdir = &dismb_lib::parent_dir($filename);

  #try to find file by going through directories
WHLLOOP:
  while ( $targdir ne "" ) {
    while( ($f, $val) =  each(%dismb_analysis::prev_fileattr_arr) ) {
      #found a file with the same target directory
      if ( $f =~ /^$targdir/ ) {
        $suff = &dismb_lib::suffix_of_file($f);
        if ( $suff eq $targsuffix ) {
          #found a file with the same suffix, will use these parser flags
          $mimic_file = $f;
          ($ts, $lang, $dir, $flags) = split("\t", $val);
          last WHLLOOP;
        }
      }
    }
    #go one directory up
    $targdir = &dismb_lib::parent_dir($targdir);
  }

  #if did not find file with the same dirs, use the first file with the
  #same suffix
  if ($mimic_file eq "") {
FORLOOP:
    while( ($f, $val) = each(%dismb_analysis::prev_fileattr_arr) ) {
      $suff = &dismb_lib::suffix_of_file($f);
      if ( $suff eq $targsuffix ) {
        $mimic_file = $f;
        ($ts, $lang, $dir, $flags) = split("\t", $val);
        last FORLOOP;
      }
    } 
  }
  return ($mimic_file, $lang, $dir, $flags);
}

sub dismb_analysis::user_specified_files
{
  &dismb_analysis::restore_prev_file_attributes();
  my(%outdated_file_arr);
  my($line, $filename, $rem_flag, $lang, $dir, $flags, $ts, $val);
  if ( open(USRSPF, "<$dismb::user_specified_list") ) {
    while ( $line = <USRSPF> ) {
      chomp($line); 
      $filename = $line;
      if ( !&dismb_lib::is_absolute_path("$filename") ) {
        &dismb_msg::err("\$1 is not an valid filename, please specify absolute filenames in \$2.", $filename, $dismb::user_specified_list);
        next; #skip this line
      }
      $filename = &dismb_lib::dis_path("$filename");
      if ( !-f $filename ) {
        &dismb_msg::err("File \$1 specified in \$2 does not exist.", $filename, $dismb::user_specified_list);
        if ( exists($dismb_analysis::prev_fileattr_arr{$filename}) ) {
          #remove file from the model
          $outdated_file_arr{$filename} = -2; #file deleted from list
        }
      }
      elsif ( exists($dismb_analysis::prev_fileattr_arr{$filename}) ) {
        #gather flags from previously stored information for old file
        ($ts, $lang, $dir, $flags) = split("\t", $dismb_analysis::prev_fileattr_arr{$filename});
        $dismb_analysis::cur_parserflags_arr{$filename} = "$lang\t$dir\t$flags\t";
        $outdated_file_arr{$filename} = 5; #user specified old file
      } else {
        #new file, get parser flags
        my $mimic_file;
        ($mimic_file, $lang, $dir, $flags) = &dismb_analysis::guess_parser_flags($filename);
        if ( $mimic_file ne "" ) {
          &dismb_msg::inf("Guessed parser flags for new file \$1, mimicking file \$2", $filename, $mimic_file);
          $dismb_analysis::cur_parserflags_arr{$filename} = "$lang\t$dir\t$flags\t";
          $outdated_file_arr{$filename} = 1; #user specified new file
        } else {
          &dismb_msg::err("Failed to guess parser flags for new file \$1", $filename);
        }
      }
    }
    close USRSPF;
  } else {
    &dismb_msg::err("Could not open file \$1 : \$2", $dismb::user_specified_list, $!); 
  }

  #fill up %dismb_analysis::cur_parserflags_arr with the source files that 
  #are not listed
  while( ($filename, $val) =  each(%dismb_analysis::prev_fileattr_arr) ) {
    ($ts, $lang, $dir, $flags) = split("\t", $val);
    if ( !&dismb_lib::is_header($lang) && !exists($dismb_analysis::cur_parserflags_arr{$filename}) ) {
      $dismb_analysis::cur_parserflags_arr{$filename} = "$lang\t$dir\t$flags\t";
    }
  }
  return (\%outdated_file_arr);
}

sub dismb_analysis::dump_cur_parser_flags
{
  my($all_list) = "$ENV{'DISBUILD_TMPDIR'}/all.pmod.lst";
  my($flags_file) = "$ENV{'DISBUILD_TMPDIR'}/new_reuse.flg";

  open( ALL_F, ">$all_list" ) || &dismb_msg::die("Could not open \$1 : \$2", $all_list, $!);
  open( FLAGS_F, ">$flags_file" ) || &dismb_msg::die("Could not open \$1 : \$2", $flags_file, $!);

  my($filename, $val, $lang, $dir, $flags, $logical_name, $mgroup_id);
  while ( ($filename, $val) = each(%dismb_analysis::cur_parserflags_arr) ) {
    $logical_name = &dismb_lib::get_logical_name($filename, $dismb_analysis::project_name);
    (print ALL_F "s 0 \"$logical_name\" \"$filename\" \"$logical_name\" \"\"\n") or &dismb_msg::write_failed($all_list, $!);

    ($lang, $dir, $flags, $mgroup_id) = split("\t", $val);
    # NULL triggers special include-file handling that's critical to 
    # submission check, but are there cases for which non-NULL would 
    # be needed?
    (print FLAGS_F "\"$logical_name\" \"$lang\" \"NULL\" $flags\n") or &dismb_msg::write_failed($flags_file, $!);
  }
  close ALL_F;
  close FLAGS_F;
}

# "parse" pass of dismb needs two files: all.pmod.nif and all.pmod.prs
# While all.pmod.nif has both source and header files all.pmod.prs file 
# has list of only source files and the associated flags. all.analysis.rem
# file has list of files to be removed.
sub dismb_analysis::create_nif_prs_and_rem_files
{
  my($outdated_files_ref) = $_[0];
  my($update_count, $rem_count, $src_count) = (0, 0, 0); 
  my($nif_file) = "$ENV{'DISBUILD_TMPDIR'}/all.pmod.nif";
  my($prs_file) = "$ENV{'DISBUILD_TMPDIR'}/all.pmod.prs";
  my($rem_file) = "$ENV{'DISBUILD_TMPDIR'}/all.analysis.rem";

  # Report on files that are not outdated, to explain why they will not be parsed.
  if ($dismb::incremental_modelbuild and defined $dismb_msg::message_group_id{"compilation units"}) {
      my($file, $val);
      while( ($file, $val) = each(%dismb_analysis::cur_parserflags_arr) ) {
	  my($outdated) = ${$outdated_files_ref}{$file};
	  if (not (defined $outdated) or $outdated <= 0) {
	      my($cur_lang, $cur_dir, $cur_flags, $mgroup_id) = split("\t", $val);
	      if ("$mgroup_id" ne "") {
		  &dismb_msg::init_auto_message_group($mgroup_id);
		  &dismb_msg::inf("Analysis pass: This compilation unit is not affected by changes."
				  . " No need to parse it for incremental model build.");
		  &dismb_msg::pop_new_message_group;
	      }
	  }
      }
  }

  my($filename, $val);
  my($logical_name, $iff_filename);
  while ( ($filename, $val) = each(%$outdated_files_ref) ) {
    if ( $val > 0 ) { #file needs to be updated in the model
      my($lang, $dir, $flags, $mgroup_id);
      if ( $update_count == 0 ) {
        open( NIFFILE, ">$nif_file" ) || &dismb_msg::die("Could not open \$1 : \$2", $nif_file, $!);
      }
      if ( exists($dismb_analysis::cur_parserflags_arr{$filename}) ) {
        ($lang, $dir, $flags, $mgroup_id) = split("\t", $dismb_analysis::cur_parserflags_arr{$filename});
      } else {
        #if a file is not in cur_fileattr_arr then it must be header file
        ($lang, $dir, $flags) = ("h", "", "");
      }
      $iff_filename = &dismb_lib::get_iff_location($filename);
      $logical_name = &dismb_lib::get_logical_name($filename, $dismb_analysis::project_name);     
      #write a line to all.pmod.nif file
      (print NIFFILE "\"$iff_filename\" \"$filename\" \"$dismb_analysis::project_name\" \"$logical_name\" \"$dismb_analysis::project_name\"\n") or &dismb_msg::write_failed($nif_file, $!);
      $update_count++;

      if ( !&dismb_lib::is_header($lang)) {
        #write source file line to all.pmod.prs file
        if ( $src_count == 0 ) {
          open( PRSFILE, ">$prs_file") || &dismb_msg::die("Could not open \$1 : \$2", $prs_file, $! );
        }
	if ("$mgroup_id" eq "") {
	    my($cumg);
	    if (defined $dismb_msg::message_group_id{"compilation units"}) {
		$cumg = $dismb_msg::message_group_id{"compilation units"};
		&dismb_msg::push_new_message_group("parse file", $cumg);
		$mgroup_id = &dismb_msg::current_auto_message_group;
		&dismb_msg::inf("Parsing \$1 for: Analysis indicates this file needed.", $filename);
		&dismb_msg::pop_new_message_group();
	    }
	}
        (print PRSFILE "\"$iff_filename\" \"$filename\" \"$mgroup_id\" \"$lang\" \"$dir\" $flags\n") or &dismb_msg::write_failed($prs_file, $!);
        $src_count++;
      }
      else {
	if ("$mgroup_id" ne "") {
	    &dismb_msg::init_auto_message_group($mgroup_id);
	    &dismb_msg::inf("Analysis pass: Dropping compilation unit consisting of just a header file."
			    . " Header files are parsed when #included by another compilation unit.");
	    &dismb_msg::pop_new_message_group;
	}
      }
    } elsif ( $val < 0 ) { #file needs to be removed from the model
      if ( $rem_count == 0 ) {
        open( REMFILE, ">$rem_file" ) || &dismb_msg::die("Could not open \$1 : \$2", $rem_file, $!); 
      }
      $logical_name = &dismb_lib::get_logical_name("$filename", $dismb_analysis::project_name);
      (print REMFILE "\"$dismb_analysis::project_name\" \"$logical_name\" \"$filename\"\n") or &dismb_msg::write_failed($rem_file, $!);
      $rem_count++;
    }
  }
  close NIFFILE if (defined NIFFILE);
  close PRSFILE if (defined PRSFILE);
  close REMFILE if (defined REMFILE);
  $dismb_analysis::cur_attempted = $src_count;

  return ($update_count, $rem_count, $src_count);
}

sub dismb_analysis::get_prjlike_file_perms
{
  my($filename) = $_[0];
  my($ret_val) = "_FLDX_";
  
  $ret_val = $ret_val . ( ( -f $filename ) ? 1 : 0 );
  $ret_val = $ret_val . ( ( -l $filename ) ? 1 : 0 );
  $ret_val = $ret_val . ( ( -d $filename ) ? 1 : 0 );
  $ret_val = $ret_val . ( ( -X $filename ) ? 1 : 0 );
  return $ret_val;
}

sub dismb_analysis::write_to_file
{
  my($filename, $text) = @_;
  open( FILEHANDLE, ">$filename") or &dismb_msg::die("Could not open \"$filename\" for writing");
  (print FILEHANDLE $text) or &dismb_msg::write_failed($filename, $!);
  close FILEHANDLE;
}

#
#Creates ROOTS.pmod.lst and PMODS.pmod.lst files with format similar to 
#prj files. These files are needed for update pass.
#NOTE:pdf generator, as it is now, can create pdf with one root project
# and pmods buildable at the root level only. Therefore ROOTS.pmod.lst 
#and PMODS.pmod.lst have only one line and are exactly same.
#
sub dismb_analysis::create_project_list_files
{
  my($src_root) = ($dismb::is_NT) ? "C:/" : "/";
  $src_root_perm = &dismb_analysis::get_prjlike_file_perms($src_root);

  my($sym_pmod_file) = "${dismb_analysis::project_name}.pmod.sym";
  $sym_pmod_file = "$ENV{'ADMINDIR'}/model/pmod" . $sym_pmod_file;
  $sym_pmod_perm = &dismb_analysis::get_prjlike_file_perms($sym_pmod_file);

  my($sym_size) = (-e $sym_pmod_file) ? (-s $sym_pmod_file) : 0;
  my($pmod_type) = "X";

  my($pmod_line) = "$pmod_type $sym_size $dismb_analysis::project_name ${src_root}${src_root_perm} $dismb_analysis::project_name ";
  $pmod_line .= &dismb_lib::add_quotes("${sym_pmod_file}${sym_pmod_perm}");
  $pmod_line .= "\n";

  $pmodlist_file = "$ENV{'DISBUILD_TMPDIR'}/PMODS.pmod.lst";
  &dismb_analysis::write_to_file($pmodlist_file, $pmod_line);

  $rootlist_file = "$ENV{'DISBUILD_TMPDIR'}/ROOTS.pmod.lst";
  &dismb_analysis::write_to_file($rootlist_file, $pmod_line);
}

sub dismb_analysis::delete_psets_of_files
{
  my($outdated_files_ref) = $_[0];
  my($filename, $pset_filename, $val);
  &dismb_msg::inf("All outdated files:");
  while( ($filename, $val) = each(%$outdated_files_ref) ) {
    &dismb_msg::inf("$filename : $val");
    $pset_filename = &dismb_lib::pset_filename_of($filename);
    eval '&dismb_lib::unlink("$pset_filename")';
    eval '&dismb_lib::unlink("$pset_filename.iff")';
    eval '&dismb_lib::unlink("$pset_filename.iff.lck")';
  }
}

sub dismb_analysis::cleanup_global_arrays
{
  undef %dismb_analysis::prev_fileattr_arr;
  undef %dismb_analysis::cur_parserflags_arr;
  undef %dismb_analysis::prev_checksum_arr;
}

#perform automatic analysis to get list of outdated files, includes
#all existing files that need to be updated/imported into the model and 
#non-existing files that need to be removed from the model
sub dismb_analysis::automatic_analysis
{
  my(%outdated_arr);
  &dismb_analysis::restore_prev_file_attributes();
  &dismb_analysis::initialize_cur_files_and_flags();
  my($file, $val, $outdated);
  while( ($file, $val) = each(%dismb_analysis::prev_fileattr_arr) ) {
    $outdated = &dismb_analysis::is_file_outdated($file);
    if ( $outdated != 0 ) {
      if ( exists($outdated_arr{$file}) ) {
        &dismb_msg::wrn("\$1 was already marked outdated", $file);
      } else {
        $outdated_arr{$file} = "$outdated";
      }
    }
  }
  #add new files to update list
  while( ($file, $val) = each(%dismb_analysis::cur_parserflags_arr) ) {
    if ( !exists($dismb_analysis::prev_fileattr_arr{$file}) ) {
      $outdated_arr{$file} = 1; #new file
    }
  }

  #check dependencies for outdated files and add them to the outdated list
  while ( ($file, $val) = each(%dismb_analysis::other_dependencies) ) {
    if ( &dismb_analysis::is_file_modified($file, $val) != 0 ) {
      $outdated_arr{$file} = 10; #outdated dependency file
      $dismb_analysis::other_dependencies{$file} = ""; #mark as been modified
    }
  }

  return (\%outdated_arr);
}

sub dismb_analysis::compile_outdated_filelist
{
  $dismb_analysis::remove_file = "$ENV{'DISBUILD_TMPDIR'}/all.analysis.rem";  
  my(%outdated_files) = ();
  my($outdated_size) = 0;
  if ( $dismb::incremental_modelbuild ) {
    my($outdated_files_ref);

    if ( $dismb::user_specified_list eq "" ) {
      #automatic analysis
      $outdated_files_ref = &dismb_analysis::automatic_analysis();
    } else {
      #initial set of outdated files provided by user
      $outdated_files_ref = &dismb_analysis::user_specified_files();
    }
    %outdated_files = %$outdated_files_ref;

    #Got an initial set of update/remove files, now do impact analysis 
    #to get all other files that need to be reimported into model.
    $outdated_size = &dismb_lib::size_of_hash(\%outdated_files);
    if ( $outdated_size > 0 ) {
      $outdated_files_ref = &dismb_analysis::impact_analysis(\%outdated_files);
      #get new set of update files after impact analysis
      %outdated_files = %$outdated_files_ref;

      &dismb_analysis::delete_psets_of_files(\%outdated_files);
    }
  } else {
    #full model build, add all files to outdated list
    my($file, $val);
    &dismb_analysis::initialize_cur_files_and_flags();
    while( ($file, $val) = each(%dismb_analysis::cur_parserflags_arr) ) {
      $outdated_files{$file} = 1; #new file
    }
  }
  &dismb_analysis::dump_cur_parser_flags();

  $outdated_size = &dismb_lib::size_of_hash(\%outdated_files);
  my($update_cnt, $rem_cnt, $src_cnt) = (0, 0, 0);
  if ( $outdated_size > 0 ) {
    ($update_cnt, $rem_cnt, $src_cnt) = &dismb_analysis::create_nif_prs_and_rem_files(\%outdated_files);
    &dismb_analysis::create_project_list_files();
  }
  if ( $dismb::incremental_modelbuild ) {
    &dismb_msg::inf ("Number of outdated files: \$1, update files: \$2, remove files: \$3", $outdated_size, $update_cnt, $rem_cnt) ;
  }

  #report message about number of files to be parsed
  &dismb_msg::define_progress_steps("parse core","parse_file",$src_cnt);
  &dismb_msg::inf ("Total number of source files to be parsed: \$1", $src_cnt) ;
  &dismb_analysis::cleanup_global_arrays();
  &dismb_msg::vrb ("Cleaned up global arrays.") ;
  return ($src_cnt) ? 1 : 0; 
}


###############################################################################
#Procedures to save file attributes after "update" pass. This information will
#be used in the following incremental model builds. 
#
sub dismb_analysis::initialize_prs_arr
{
  %dismb_analysis::prs_arr = ();
  my($prs_file) = "$ENV{'DISBUILD_TMPDIR'}/all.pmod.prs";
  if ( open(PRSF, "<$prs_file") ) {
    my($field) = "([^\"]*)"; 
    my($filename, $lang, $compdir, $flags);
    while ( $line = <PRSF> ) {
      #each line has 5 fields (first 4 fields have quotes around)
      if ( $line =~ /^\"${field}\"\s+\"${field}\"\s+\"([a-zA-Z]\w*)\"\s+\"${field}\"\s+(.*)$/ ) { 
        $filename = $2; $lang = $3; $compdir = $4; $flags = $5;
      }
      elsif ( $line =~ /^\"${field}\"\s+\"${field}\"\s+\"\d*\"\s+\"${field}\"\s+\"${field}\"\s+(.*)$/ ) { 
        $filename = $2; $lang = $3; $compdir = $4; $flags = $5;
      }
      if ( $flags =~ /--package_name \"([^\"]*)\"/ ) {
	  $package_name = $1;
	  $flags =~ s/--package_name \"([^\"]*)\"//g; 
      }
      $flags =~ s/^\s+//; $flags =~ s/\s+$//;       #trim spaces

      if (defined($filename)) {
        if ( !exists($dismb_analysis::prs_arr{$filename}) ) {
          $dismb_analysis::prs_arr{$filename} = "$lang\t$compdir\t$flags";
        } else {
          dismb_msg::wrn("Duplicate entry for file \$1 in \$2.", $filename, $prs_file);
        }
      }
    }
    close PRSF;
  } 
}

#Create an associative array of files that were imported into the
#model from all.update.nif file.
sub dismb_analysis::initialize_update_nif_arr
{
  %dismb_analysis::update_nif_arr = ();
  my($update_nif_file) = "$ENV{'DISBUILD_TMPDIR'}/all.update.nif";
  if ( open(UPDF, "<$update_nif_file") ) {
    my($line);
    while ( $line = <UPDF> ) {
      chomp($line);
      my(@fields) = split ("\" \"", $line);
      #filename is second field in each line
      $dismb_analysis::update_nif_arr{$fields[1]} = "0";
    }
    close UPDF;
  } 
}

#Create an associative array of files that were removed from the
#model from all.update.rem file.
sub dismb_analysis::initialize_removed_arr
{
  %dismb_analysis::removed_arr = ();
  my($rem_file) = "$ENV{'DISBUILD_TMPDIR'}/all.update.rem";
  if ( -f $rem_file ) {
    if ( open(REMF, "<$rem_file") ) {
      my($line);
      while ( $line = <REMF> ) {
        chomp($line);
        my(@fields) = split ("\" \"", $line);
        $fields[0] =~ s%^"%%; # remove quotes at the beginning
        $fields[1] =~ s%"$%%; # remove quotes at the end

        #logical filename is second field on each line
        my($physical_name) = &dismb_lib::get_physical_name("$fields[1]", $fields[0]);
        $dismb_analysis::removed_arr{$physical_name} = "";
      }
      close REMF;
    } else {
      &dismb_msg::err("Failed to open file \$1 : \$2", $rem_file, $!); 
    }
  }
}


#Returns the parse status of the given file. If the file is in
#prs_forgiv.lst file then it is forgiven otherwise it is
#cleanly parsed.
#
sub dismb_analysis::get_parse_status
{
  my($filename) = $_[0]; 
  if ( !defined(%dismb_analysis::forgiven_arr) ) {
    %dismb_analysis::forgiven_arr = ();
    my($forgiven_file) = "$ENV{'DISBUILD_TMPDIR'}/prs_forgiv.lst";
    if ( -f $forgiven_file ) {
      if ( open(FRGVF, "<$forgiven_file") ) {
        my($line);
        while ( $line = <FRGVF> ) {
          chomp($line);
          if ( !exists($dismb_analysis::forgiven_arr{$line}) ) {
            $dismb_analysis::forgiven_arr{$line} = "";
          }
        }
        close FRGVF;
      }
    }
  }
  my($status) = (exists($dismb_analysis::forgiven_arr{$filename})) ? "forgiven" : "clean";
  return $status;
}

sub dismb_analysis::create_new_attributes_file
{
  my ($new_attr_file) = $_[0];
  #associative array of files updated/imported in the current model build session
  &dismb_analysis::initialize_update_nif_arr();
  &dismb_analysis::initialize_removed_arr();

  #associative array of all files attempted to be parsed in the current model 
  #build session
  &dismb_analysis::initialize_prs_arr();

  $dismb_analysis::total_succeeded = 0;
  $dismb_analysis::total_clean = 0;

  unlink($new_attr_file);
  open(NEWATTRF, ">$new_attr_file") || &dismb_msg::die("Failed to open \$1: \$2", $new_attr_file, $!);

  #save name of the project on the first line
  (print NEWATTRF "#project_name:$dismb_analysis::project_name\n") or &dismb_msg::write_failed($new_attr_file, $!);

  my($line, $handled, $filename, $timestamp, $lang, $parse_status, $compdir, $flags);
  if ( $dismb::incremental_modelbuild && open(OLDATTRF, "<$dismb_analysis::prev_attributes_file") ) {
    while ( $line = <OLDATTRF> ) {
      chomp($line);
      #Read tab separated fields
      if ( $line =~ /^([^\t]+)\t+([^\t]+)\t+([^\t]+)\t+([^\t]*)\t+([^\t]*)\t+(.*)$/ ) {
	($filename, $timestamp, $lang, $parse_status, $compdir, $flags) = ($1, $2, $3, $4, $5, $6);
        next if ( exists($dismb_analysis::removed_arr{$filename}) );
        if ( exists($dismb_analysis::update_nif_arr{$filename}) ) {
          #file has been updated in the current session, get new attributes
          $timestamp = &dismb_lib::last_modified_time($filename);
          if ( exists($dismb_analysis::prs_arr{$filename}) ) {
            ($lang, $compdir, $flags) = split("\t", $dismb_analysis::prs_arr{$filename});
            $parse_status = &dismb_analysis::get_parse_status($filename);
            $dismb_analysis::cur_clean++ if ($parse_status eq "clean");
            $dismb_analysis::cur_succeeded++;
          }
          #mark the entry as handled
          $dismb_analysis::update_nif_arr{$filename} = "1";          
        }
        if ( !dismb_lib::is_header($lang) ) {
          $dismb_analysis::total_clean++ if ($parse_status eq "clean");
	  $dismb_analysis::total_succeeded++;
        }

        (print NEWATTRF "$filename\t$timestamp\t$lang\t$parse_status\t$compdir\t$flags\n") or &dismb_msg::write_failed($new_attr_file, $!);
      }
    }
    close OLDATTRF;
  }
  elsif ( $dismb::incremental_modelbuild ) {
    &dismb_msg::err("Failed to open file \$1 : \$2", "$dismb_analysis::prev_attributes_file", $!)
  }

  #Go through %dismb_analysis::update_nif_arr to find files that are not
  #handled above and add to the new attributes file. (All files in case of full 
  #modelbuild and new files in case of incremental model build.)
  while( ($filename, $handled) = each(%dismb_analysis::update_nif_arr) ) {
    if ( $handled == 0 ) {
      $timestamp = &dismb_lib::last_modified_time($filename);
      if ( exists($dismb_analysis::prs_arr{$filename}) ) {
        ($lang, $compdir, $flags) = split("\t", $dismb_analysis::prs_arr{$filename});
        $parse_status = &dismb_analysis::get_parse_status($filename);
        $dismb_analysis::cur_clean++ if ($parse_status eq "clean");
        $dismb_analysis::cur_succeeded++;
        if ( $dismb::incremental_modelbuild ) {
          $dismb_analysis::total_clean++ if ($parse_status eq "clean");
	  $dismb_analysis::total_succeeded++;
        }
      } else {
        #if a file is in %dismb_analysis::update_nif_arr but is not in 
        #%dismb_analysis::prs_arr, then it is a header file
        $lang = "h"; $parse_status = ""; $compdir = ""; $flags = "";
      }
      (print NEWATTRF "$filename\t$timestamp\t$lang\t$parse_status\t$compdir\t$flags\n") or &dismb_msg::write_failed($new_attr_file, $!);
    }
  } 
  close NEWATTRF;
  return 1;
}


sub dismb_analysis::save_other_dependencies
{
  if ( &dismb_lib::size_of_hash(\%dismb_analysis::other_dependencies) > 0 ) {
    eval '&dismb_lib::unlink("$dismb_analysis::other_dependencies_file")';
    if ( open(DEPF, ">$dismb_analysis::other_dependencies_file") ) {
      my($f, $ts);
      while( ($f, $ts) = each(%dismb_analysis::other_dependencies) ) {
        if ($ts eq "") {
          $ts = &dismb_lib::last_modified_time($f);
        }
        (print DEPF "$f\t$ts\n") or &dismb_msg::write_failed($dismb_analysis::other_dependencies_file, $!);
      }
      close DEPF;
    } else {
      &dismb_msg::err("Failed to open other dependencies file, \$1 : \$2", $dismb_analysis::other_dependencies_file, $!);
    }
  }
}

#Write a brief summary of number of source files that were attempted 
#to parse and succeeded etc. Note that this does NOT include
#information about header files.
sub dismb_analysis::write_summary_info
{
  my($summary_file) = "$ENV{'DISBUILD_TMPDIR'}/summary.txt";
    if ( open(SUMMF, ">$summary_file") ) {
      if ($dismb_analysis::total_attempted > 0) {
        print SUMMF "attempted: $dismb_analysis::total_attempted\n";
      }

      my($succeed) = ($dismb::incremental_modelbuild) ? $dismb_analysis::total_succeeded : $dismb_analysis::cur_succeeded;
      print SUMMF "succeeded: $succeed\n";
       
      my($clean) = ($dismb::incremental_modelbuild) ? $dismb_analysis::total_clean : $dismb_analysis::cur_clean;
      print SUMMF "clean: $clean\n";

      if ($dismb::incremental_modelbuild) {
        print SUMMF "current attempted: $dismb_analysis::cur_attempted\n";
        print SUMMF "current succeeded: $dismb_analysis::cur_succeeded\n";
        print SUMMF "current clean: $dismb_analysis::cur_clean\n";
      }      
      close SUMMF;
    } else {
      &dismb_msg::err("Failed to open summary file \$1 : \$2.", $summary_file, $!);
    }
}

#Saves attributes of files in the model. This procedure is called after UPDATE 
#pass.
sub dismb_analysis::save_file_attributes
{
  my ($new_attr_file) = "$ENV{'ADMINDIR'}/config/file_attributes.lst.new";
  if (&dismb_analysis::create_new_attributes_file($new_attr_file)) {
    eval 'dismb_lib::unlink("$dismb_analysis::prev_attributes_file")';
    &dismb_lib::rename( $new_attr_file, $dismb_analysis::prev_attributes_file);
    &dismb_analysis::save_other_dependencies();
    &dismb_analysis::write_summary_info();
  }
}

###############################################################################

return 1;
