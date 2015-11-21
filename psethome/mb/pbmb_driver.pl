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
$pbmb_driver::dsp_filename = "";
$pbmb_driver::lpservice = "";
$pbmb_driver::qaTaskFile = "";
$pbmb_driver::file_list = "";
$pbmb_driver::derived_object_filename = "";
select STDOUT; $| = 1;
select STDERR; $| = 1;

sub pbmb_driver::flags_translator
{
  my ($parser_fl, $translator_file) = @_;

  require ("parsercmds.pl");
  &dismb_msg::msg("Running flags translator");
  parsercmds::filter($parser_fl, $translator_file);

  if ( ! -f $translator_file) {
    &dismb_msg::die("Flags translator output file \$1 does not exist.", $translator_file);
  }
  if ( -z $translator_file ) {
    &dismb_msg::die("Flags translator output file: \$1 is empty.", $translator_file);
  }

  #report progress after flags translation
  &dismb_msg::report_progress("flags translation");
}


# Invokes dsp_parser and flags translator. Dsp_parser takes the name of the
# dsp file and output file as arguments. The output file from dsp_parser is
# passed as input to flags translator. Flags translator writes the list of
# files and flags in $translator_file.
sub pbmb_driver::invoke_dsp_parser
{
  my($dsp) = "dsp";
  if ($pbmb_driver::dsp_filename =~ /.dsw$/i) {
    $dsp = "dsw";
  }
  else {
	  if ($pbmb_driver::dsp_filename =~ /.jpr$/i) {
	    $dsp = "jpr";
	  } else {
	    if ($pbmb_driver::dsp_filename =~ /.out$/i) {
	      $dsp = "out";
	    }
    }
  }
  #push a new message group
  &dismb_msg::push_new_message_group("$dsp file");

  my($translator_file) = @_[0];

  my($ret_val);
  my($parser_fl) = "$ENV{'ADMINDIR'}/config/dsp_parser.out";

  if ($dsp eq "out") {
    &dismb_msg::msg("Moving \$1 to \$2", $pbmb_driver::dsp_filename, $parser_fl);
    use File::Copy;
    if ( copy($pbmb_driver::dsp_filename,$parser_fl) == 0 ) {
      &dismb_msg::msg("File copy from \$1 to \$2 failed with error: \$3", "$pbmb_driver::dsp_filename", "$parser_fl", $!);
    }
    unlink $pbmb_driver::dsp_filename;
  }
  else {
    my (@args, $i) = (0,0);

    my $exe = "";
    if ($dsp eq "jpr") {
      $exe = "$ENV{'PSETHOME'}/bin/scanjpr";
    }
    else {
      $exe = "$ENV{'PSETHOME'}/bin/scandsp";
    }
    if ( &pbmb_driver::is_NT ) {
      $exe .= ".exe";
    }
    $args[$i++] = $exe;
    $args[$i++] = $pbmb_driver::dsp_filename;

    #add lpservice and message_group options to scandsp invocation command
    if ($pbmb_driver::lpservice ne "") {
      $args[$i++] = "--lpservice"; $args[$i++] = "$pbmb_driver::lpservice";
      $args[$i++] = "--message_group"; $args[$i++] = &dismb_msg::current_auto_message_group;
    }
    $args[$i++] = "--output";
    $args[$i++] = $parser_fl;

    &dismb_msg::msg("Running scandsp: \$1", &dismb_lib::arglist_to_str(@args));
    $ret_val = &dismb_lib::system(@args);
    if ( $ret_val ) {
      &dismb_msg::die("scandsp failed with a return value of \$1.", $ret_val);
    }
  }

  &pbmb_driver::flags_translator($parser_fl, $translator_file);

  #pop the new message group
  &dismb_msg::pop_new_message_group();
}


# Invokes ClearCase config record parser (scanclrmake) and flags translator.
# The output file from scanclrmake is
# passed as input to flags translator. Flags translator writes the list of
# files and flags in $translator_file.
sub pbmb_driver::invoke_cr_parser
{
  #push a new message group
  &dismb_msg::push_new_message_group("config record");

  my($translator_file) = @_[0];

  my($ret_val);
  my($cr_fl) = "$ENV{'ADMINDIR'}/config/cleartool_catcr.out";
  my(@cmd) = ("cleartool", "catcr", "-recurse", $pbmb_driver::derived_object_filename);
  @cmd = (@cmd, ">", $cr_fl);
  &dismb_msg::msg("Running cleartool: \$1", &dismb_lib::arglist_to_str(@cmd));
  $ret_val = &dismb_lib::system(@cmd);
  if ( $ret_val ) {
    &dismb_msg::die("cleartool failed with a return value of \$1.", $ret_val);
  }

  my($parser_fl) = "$ENV{'ADMINDIR'}/config/cm_parser.out";

  my(@cmd) = ("scanclrmake", $cr_fl,
  #                         "--compilers", "cl:ntcl/gcc:gcc/cc:suncc/CC:sunccpp"
										);
  #add lpservice and message_group options to scanclrmake invocation command
  if ($pbmb_driver::lpservice ne "") {
    @cmd = (@cmd, "--lpservice", $pbmb_driver::lpservice);
    @cmd = (@cmd, "--message_group", &dismb_msg::current_auto_message_group);
  }
  @cmd = (@cmd, ">", $parser_fl);

  &dismb_msg::msg("Running scanclrmake: \$1", &dismb_lib::arglist_to_str(@cmd));
  $ret_val = &dismb_lib::system(@cmd);
  if ( $ret_val ) {
    &dismb_msg::die("scanclrmake failed with a return value of \$1.", $ret_val);
  }

  &pbmb_driver::flags_translator($parser_fl, $translator_file);

  #pop the new message group
  &dismb_msg::pop_new_message_group();
}

sub pbmb_driver::invoke_dismb
{
  &dismb_msg::push_new_message_group("integrated model build (disMB)");

  my($all_headers) = @_[0];

  my $exe = "$ENV{'PSETHOME'}/bin/dismb";
  if ( &pbmb_driver::is_NT ) {
    $exe .= ".exe";
  }
  my(@args,$i) = (0,0);
  #first argument is the executable itself
  $args[$i++] = $exe;

  #disable cache and analysis passes of dismb
  $args[$i++] = "-MBdoPassCache"; $args[$i++] = "no";
  $args[$i++] = "-MBdoPassAnalysis"; $args[$i++] = "no";

  if ($all_headers eq "1") {
    #include all the headers in the model
    $args[$i++] = "-MBincludeAllHdrs"; $args[$i++] = "yes";
  }

  if ($pbmb_driver::file_list ne "") {
    $args[$i++] = "-file_list";
    $args[$i++] = $pbmb_driver::file_list;
  }

  #generate pdf
  $args[$i++] = "-MBgeneratePdf"; $args[$i++] = "yes";

  #Flag ultimately passed on to aset_CCcc to generate multiple IF files.
  $args[$i++] = "-MBmultipleIFFs"; $args[$i++] = "yes";

  #add lpservice and message_group option
  if ($pbmb_driver::lpservice ne "") {
    $args[$i++] = "-lpservice"; $args[$i++] = $pbmb_driver::lpservice;
    $args[$i++] = "-message_group"; $args[$i++] = &dismb_msg::current_auto_message_group;
    $args[$i++] = "-stdout";
  }

  &dismb_msg::msg("Running dismb: \$1", &dismb_lib::arglist_to_str(@args));
  my($status) = &dismb_lib::system(@args);
  if ($status){
    &dismb_msg::die("dismb failed with a return value of \$1.", $status);
  }

  &dismb_msg::pop_new_message_group;
}

#returns name of the project from $ADMINDIR/config/file_attributes.lst file.
sub pbmb_driver::get_project_name
{
  my $attr_file = "$ENV{'ADMINDIR'}/config/file_attributes.lst";
  my($project_name) = "";

  if ( !-f $attr_file) {
    &dismb_msg::die("\"$attr_file\" file does not exist.");
  }
  open( ATTR_FILE, "<$attr_file" ) || &dismb_msg::die("Could not open \"$attr_file\" for reading project name.");
    #project name is on the first line
    my $project_line = <ATTR_FILE>;
    $project_name = ($project_line =~ /^#project_name:(.*)$/);
  }
  close ATTR_FILE;
  return $project_name;
}

sub pbmb_driver::server_is_running
{
  my($server_name) = $_[0];
  my($running) = 0;

  my($timeout) = 120;
LOOP:
  while ($timeout > 0) {
    if (&dish2_client::is_server_running($server_name)) {
      $running = 1;
      last LOOP;
    }
    sleep(1);
    $timeout--;
  }
  return $running;
}

sub pbmb_driver::start_model_server
{
  my($project_name, $model_server_name) = @_;
  my($ret_val) = 1;
  my($extn) = "";
  if (&pbmb_driver::is_NT) {
    $extn = ".exe";
  }
  my (@cmd) = "model_server$extn";
  @cmd = (@cmd, "-pdf", "$ENV{'ADMINDIR'}/pdf/build.pdf");
  @cmd = (@cmd, "$ENV{'ADMINDIR'}/prefs/build.prefs");
  @cmd = (@cmd, "-service", $model_server_name);
  @cmd = (@cmd, "-internal_service");

  #add lpservice and message_group options to scandsp invocation command
  #if ($pbmb_driver::lpservice ne "") {
  #  @cmd = (@cmd, " -lpservice", $pbmb_driver::lpservice);
  #  @cmd = (@cmd, "-message_group", &dismb_msg::current_auto_message_group);
  #}
  &dismb_msg::msg("Running model server: \$1", &dismb_lib::arglist_to_str(@cmd));

  if ( ! &dismb_lib::open_output(\*MODELSERVER, @cmd) ) {
    &dismb_msg::err("Failed to start model server \$1: \$2", &dismb_lib::arglist_to_str(@cmd), $!);
    $ret_val = 0;
  }

  return $ret_val;
}

sub pbmb_driver::start_qa_engine
{
  my($model_server_name, $qa_server_name, $qar_root) = @_;

  my($ret_val) = 1;
  if (&pbmb_driver::server_is_running($model_server_name)) {

    my($extn) = "";
    if (&pbmb_driver::is_NT) {
      $extn = ".exe";
    }

    if (! -f "$ENV{'PSETHOME'}/bin/QAEngine$extn") {
      &dismb_msg::err("No QAEngine executable.");
      return 0;
    }

    my (@cmd) = ("QAEngine$extn");
    @cmd = (@cmd, $qa_server_name);

    @cmd = (@cmd, $qar_root);
    @cmd = (@cmd, "$ENV{'PSETHOME'}/lib/policy");
    
    #add lpservice and message_group options to QAEngine invocation command
    if ($pbmb_driver::lpservice ne "") {
	@cmd = (@cmd, "$pbmb_driver::lpservice", &dismb_msg::current_auto_message_group());
    }

    &dismb_msg::msg("Running QAEngine: \$1", &dismb_lib::arglist_to_str(@cmd));

    if (! &dismb_lib::open_output(\*QAENGINE, @cmd) ) {
      &dismb_msg::err("Failed to start QAEngine \$1: \$2", &dismb_lib::arglist_to_str(@cmd), $!);
      $ret_val = 0;
    }
  }
  else {
    &dismb_msg::err("Model server is not running.");
    $ret_val = 0;
  }
  return $ret_val;
}

sub pbmb_driver::start_QA_build
{
  my($model_server_name, $qa_server_name) = @_;
  if (&pbmb_driver::server_is_running($qa_server_name)) {

    #send "build" command to QAEngine using dish2 protocol
    #build_cmd = build <host> <service> <project>
    my($build_cmd) = "build 127.0.0.1 $model_server_name";

    my $qaproject = "";
    if ($pbmb_driver::qaTaskFile =~ /([^\/\\]+)\.policy\.xml$/ ) {
       $qaproject = $1;
    }

    if ($qaproject ne "") {
      $build_cmd .= " " . &pbmb_driver::add_quotes($qaproject);
    }

    my ($return_code, $recv_msg) = &dish2_client::send_and_receive_command("$qa_server_name", "$build_cmd");
    if ($recv_msg =~ /done/i) {  #"done" from QAEngine implies success.
      &dismb_msg::msg("QA build succeeded.");
    }
    else {
      &dismb_msg::err("QA build failed with error: \$1, return value: \$2", $recv_msg, $return_code);
    }
  }
  else {
    &dismb_msg::err("QAEngine server is not running.");
  }
}

sub pbmb_driver::run_qar
{
  #create new message group for QA phase
  &dismb_msg::push_new_message_group("quality assurance");

  my($project_name) = &pbmb_driver::get_project_name();

  # $qar_root = "$ENV{'ADMINDIR'}/QA"
  my $qar_root = "$ENV{'ADMINDIR'}/QA";

  #create directory $qar_root but do not clean it up if it exists
  &dismb_lib::create_dir("$qar_root", 0);

  #add QAR_Root preference to prefs file
  my $prefs_file = "$ENV{'ADMINDIR'}/prefs/build.prefs";
  my $prefs_line = "*psetPrefs.QAR_Root: $qar_root\n";
  if ( open (PREFS_FILE, ">>$prefs_file") ) {
    print PREFS_FILE $prefs_line;
    close(PREFS_FILE);
  }
  else {
    dismb_msg::err("Could not open preference file: \$1", "$prefs_file");
    #if prefs file could not be opened, set QAR_Root environment variable.
    $ENV{'QAR_Root'} = "$qar_root";
  }

  my($model_server_name) = "$project_name";
  my($qaengine_server_name) = "QA:$project_name";
  my($msg_groupid) = &dismb_msg::current_auto_message_group;

  #push a new message group for model_server
  &dismb_msg::push_new_message_group("model_server", $msg_groupid);

  local(*MODELSERVER);
  if ( &pbmb_driver::start_model_server($project_name, $model_server_name) ) {
    #push a new message group for QAEngine
    &dismb_msg::push_new_message_group("QAEngine", $msg_groupid);

    local(*QAENGINE);
    if ( &pbmb_driver::start_qa_engine($model_server_name, $qaengine_server_name, $qar_root) ) {
      &pbmb_driver::start_QA_build($model_server_name, $qaengine_server_name);

      #shutdown QAEngine and close open file handle
      &dish2_client::send_command($qaengine_server_name, "bye");
      close QAENGINE;
    }
    #pop message group created for QAEngine
    pop_new_message_group;

    #shutdown model server and close open file handle
    &dish2_client::send_command($model_server_name, "stop_server -y");
    close MODELSERVER;
  }
  #pop message group created for model_server
  pop_new_message_group;

  #pop new message group created for QA phase
  &dismb_msg::pop_new_message_group;

  #report progress after QA Caliper is finished
  &dismb_msg::report_progress("qa calliper");
}

#If name has space in it add a pair of quotes around it.
sub pbmb_driver::add_quotes
{
  my($filename) = @_;
  if ( $filename =~ /\s+/ ) {
    $filename = "\"$filename\"";
  }
  return $filename;
}

# Is this path relative, or absolute?
# The call to cwd isn't very efficient.
sub pbmb_driver::is_relative_path
{
  use Cwd;
  my($path) = $_[0];
  my($wd) = cwd();
  my($ret_val) = 1;
  if ( $wd =~ /^[a-zA-z]:/ ) {
    # MS-Windows, should have drive letter
    if ( $path =~ /^[a-zA-z]:/ ) {
      $ret_val = 0;
    }
  }
  elsif ( $wd =~ m|^/| ) {
    # Unix, should have slash
    if ( $path =~ m|^/| ) {
      $ret_val = 0;
    }
  }
  return $ret_val;
}

sub pbmb_driver::is_NT
{
  my $val = 0;
  if ( "$ENV{'OS'}" eq "Windows_NT" ) {
    $val = 1;
  }
  return $val;
}

sub pbmb_driver::create_or_cleanup_admindir
{
  my($admindir) = "$ENV{'ADMINDIR'}";
  if (! -d $admindir) {
    #Create $ADMINDIR directory
    &dismb_lib::create_dir("$admindir", 0);
  }
  else {
    #Cleanup $ADMINDIR selectively, as it may have other interesting data
    #like QA which should NOT be deleted when a new model build is done.
    &dismb_lib::cleanup ("$admindir/model", ".*") if ( -d "$admindir/model" );
    &dismb_lib::cleanup ("$admindir/pdf", ".*") if ( -d "$admindir/pdf" );
    &dismb_lib::cleanup ("$admindir/prefs", ".*") if ( -d "$admindir/prefs" );
    &dismb_lib::cleanup ("$admindir/tmp", ".*") if ( -d "$admindir/tmp" );
    &dismb_lib::cleanup ("$admindir/log", ".*") if ( -d "$admindir/log" );
  }
}

sub pbmb_driver::usage
{
  return "Usage: pbmb_driver.pl (-dsp dsp_file|-derived file) -admindir dir [-qaTaskFile file] [-file_list file -subcheck_dir dir] [-lpservice service_name]";
}

sub pbmb_driver::check_admindir
{
  my($admindir, $option) = @_;
  if ( ! $admindir || $admindir =~ /^-/ ) {
    my($msg) = "Model Directory [$option] value is missing. " . &pbmb_driver::usage();
    &dismb_msg::die($msg);
  }
  if ( &pbmb_driver::is_relative_path($admindir) ) {
    &dismb_msg::die("Model Directory [$option] = $admindir is relative path, please set absolute path.");
  }
  #replace back slashes with forward slashes
  $admindir =~ s%\\%/%g;

  #remove trailing slashes
  $admindir =~ s%/+$%%;

  #replace multiple slashes with single slash
  $admindir =~ s%/+%/%g;

  #set ADMINDIR environment variable
  $ENV{'ADMINDIR'} = $admindir;
}

sub pbmb_driver::check_psethome
{
  if ( ! defined $ENV{'PSETHOME'} ) {
    &dismb_msg::die("Please set PSETHOME environment variable.");
  }
  if ( ! -d $ENV{'PSETHOME'} ) {
    &dismb_msg::die("PSETHOME=$ENV{'PSETHOME'} is not a directory.");
  }

  #replace back slashes with forward slashes
  $ENV{'PSETHOME'} =~ s%\\%/%g;
  #remove any trailing slash from the path
  $ENV{'PSETHOME'} =~ s%/$%%;

  #add $PSETHOME/bin to path
  if ( &pbmb_driver::is_NT ) {
    my($k);
    #find case insensitive "PATH" environment variable in %ENV
  LOOP:
    foreach $k (keys (%ENV)) {
      if ($k =~ /^PATH$/i) {
        $ENV{$k} = "$ENV{'PSETHOME'}/bin" . ";" . "$ENV{$k}";
        last LOOP;
      }
    } 
  } else {
    $ENV{'PATH'} = "$ENV{'PSETHOME'}/bin" . ":" . "$ENV{'PATH'}";
  }
}

sub pbmb_driver::check_qafile
{
  my($task_file) = @_;
  if ( ! $task_file || $task_file =~ /^-/ ) {
    my($die_msg) = "QA Caliper Task File [-qaTaskFile] value is missing. " . &pbmb_driver::usage();
    &dismb_msg::die($die_msg);
  }
  else {
    if ( ! -f $task_file) {
     &dismb_msg::die("Invalid QA Caliper Task File: \$1", $task_file);
    }
  }
  $pbmb_driver::qaTaskFile = $task_file;
}

sub pbmb_driver::check_file_list
{
  my($file) = @_;
  if (! $file || $file =~ /^-/ ) {
    my($msg) = "File list [-file_list] value is missing. " . &pbmb_driver::usage();
    &dismb_msg::die($msg);
  }
  if (! -e $file) {
    my($msg) = "$file file does not exist";
    &dismb_msg::die($msg);
  }
  $pbmb_driver::file_list = $file;
}


sub pbmb_driver::check_dspfile
{
  my($dsp_file) = @_;
  if ( ! $dsp_file || $dsp_file =~ /^-/ ) {
    my($die_msg) = "Project File [-dsp] value is missing. " . &pbmb_driver::usage();
    &dismb_msg::die($die_msg);
  }
  if (! -f $dsp_file) {
    &dismb_msg::die("Invalid Project File: \$1", $dsp_file);
  }
  $pbmb_driver::dsp_filename = $dsp_file;
}

sub pbmb_driver::check_derived_object_file
{
  my($do_file) = @_;
  if ( ! $do_file || $do_file =~ /^-/ ) {
    my($die_msg) = "Derived Object File [-derived] value is missing. " . &pbmb_driver::usage();
    &dismb_msg::die($die_msg);
  }
  if (! -f $do_file) {
    &dismb_msg::die("Invalid Derived Object File: \$1", $do_file);
  }
  $pbmb_driver::derived_object_filename = $do_file;
}

sub pbmb_driver::check_lpservice
{
  my($lp_ser) = @_[0];
  if ( ! $lp_ser || $lp_ser =~ /^-/ ) {
    my($die_msg) = "log presentation service name is missing." . &pbmb_driver::usage();
    &dismb_msg::die($die_msg);
  }
  $pbmb_driver::lpservice = $lp_ser;
}

sub pbmb_driver::process_options
{
  my ($i, $target_flag, $admind_flag, $flist_flag) = (0, 0, 0, 0);
  my $subcheck_dir = "";
  while ($i <= $#ARGV) {
    my($arg) = @ARGV[$i];
    if ($arg =~ /^-dsp$/) {
      $i++;
      $target_flag = 1;
      &pbmb_driver::check_dspfile(@ARGV[$i]);
    }
    elsif ($arg =~ /^-admindir$/) {
      $admind_flag = 1;
      $i++;
      &pbmb_driver::check_admindir("@ARGV[$i]", "-admindir");
    }
    elsif ($arg =~ /^-lpservice$/) {
      $i++;
      &pbmb_driver::check_lpservice(@ARGV[$i]);
    }
    elsif ($arg =~ /^-derived$/) {
      $i++;
      $target_flag = 1;
      &pbmb_driver::check_derived_object_file(@ARGV[$i]);
    }
    elsif ($arg =~ /^-qaTaskFile$/) {
      $i++;
      &pbmb_driver::check_qafile(@ARGV[$i]);
    }
    elsif ($arg =~ /^-file_list$/) {
      $i++;
      $flist_flag = 1;
      &pbmb_driver::check_file_list(@ARGV[$i]);
    }
    elsif ($arg =~ /^-subcheck_dir$/) {
      $i++;
      $subcheck_dir = "@ARGV[$i]";
    }
    $i++;
  }
  if ( $flist_flag) {
    if ( $subcheck_dir eq "" ) {
      my($msg) = "Missing command line option -subcheck_dir. " . &pbmb_driver::usage;
      &dismb_msg::die($msg);
    } else {
      &pbmb_driver::check_admindir("$subcheck_dir", "-subcheck_dir");
    }
  }
  if (! $target_flag || !($flist_flag || $admind_flag) ) {
    my($msg) = "Missing command line option. " . &pbmb_driver::usage;
    &dismb_msg::die($msg);
  }
}


#This function does the base minimum initialization and is the first
#one to be called in main. Any mesaging stuff should be done only after
#this step because dismb_msg.pl etc. are initialized here.
sub pbmb_driver::init
{
  if ( "$ENV{'DISMBDIR'}" eq "" ) {
    $ENV{'DISMBDIR'} = "$ENV{'PSETHOME'}/mb";
  }
  #replace back slashes in DISMBDIR with forward slashes
  $ENV{'DISMBDIR'} =~ s%\\%/%g;

  #
  # Set DISCOVER_VERBOSE so that dismb and other downstream processes
  # will run in verbose mode.
  #
  if (! defined $ENV{'DISCOVER_VERBOSE'} ) {
    $ENV{'DISCOVER_VERBOSE'} = "yes";
  }

  # Don't use Perl scripts from the user's current directory.
  # Last in include path might be ".".
  if ( $INC[$#INC] eq "." ) {
      pop @INC;
  }
  push @INC, ($ENV{'DISMBDIR'});

  require( "dismb_lib.pl" );
  require( "dismb_msg.pl" );
}

sub pbmb_driver::setup
{
  #create new message group for initializing phase
  &dismb_msg::push_new_message_group("initialization");

  &pbmb_driver::check_psethome;

  &pbmb_driver::create_or_cleanup_admindir;

  if ( ! defined $ENV{"DISBUILD_TMPDIR"} ) {
    $ENV{"DISBUILD_TMPDIR"} = "$ENV{'ADMINDIR'}/tmp/disbuild";
  }
  #replace back slashes in DISBUILD_TMPDIR with forward slashes
  $ENV{'DISBUILD_TMPDIR'} =~ s%\\%/%g;

  #create $ADMINDIR/config directory if it doesn't exist
  &dismb_lib::create_dir("$ENV{'ADMINDIR'}/config", 0);

  #set ROVER_TMPDIR to ADMINDIR/tmp if it does not exist
  if ( ! defined $ENV{"ROVER_TMPDIR"} ) {
    $ENV{"ROVER_TMPDIR"} = "$ENV{'ADMINDIR'}/tmp";
  }
  #replace back slashes in ROVER_TMPDIR with forward slashes
  $ENV{'ROVER_TMPDIR'} =~ s%\\%/%g;

  #create directory ROVER_TMPDIR but do not clean it up if it exists
  &dismb_lib::create_dir("$ENV{'ROVER_TMPDIR'}", 0);

  #set TMP and TMPDIR to ROVER_TMPDIR
  $ENV{"TMPDIR"} = $ENV{"ROVER_TMPDIR"};
  $ENV{"TMP"} = $ENV{"ROVER_TMPDIR"};

  #pop new message group created for initializing phase
  &dismb_msg::pop_new_message_group;

  #report progress after initialization
  &dismb_msg::report_progress("initialization");
}

sub pbmb_driver::main
{
  &pbmb_driver::init;

  #initialize root message group
  &dismb_msg::init_auto_message_group("0");

  #process command line options
  &pbmb_driver::process_options(@_);

  #report progress about starting modebuild/modelbuildAndQA depending on whether
  #QA is to be done after modelbuild.
  if ($pbmb_driver::qaTaskFile eq "") {
    &dismb_msg::report_progress("start modelbuild");
  } else {
    &dismb_msg::report_progress("start modelbuildAndQA");
  }

  &pbmb_driver::setup;
  my($translator_file) = "$ENV{'ADMINDIR'}/config/translator.flg";
  if ( $pbmb_driver::dsp_filename ne "" ) {
    &pbmb_driver::invoke_dsp_parser($translator_file);
  }
  if ( $pbmb_driver::derived_object_filename ne "" ) {
    &pbmb_driver::invoke_cr_parser($translator_file);
  }
  if ($pbmb_driver::file_list eq "") {
    #For regular model build all the header files are included in the model.
    &pbmb_driver::invoke_dismb(1);
  } else {
    #when doing model build of list of files do not include the header files
    #encountered during parsing in the model.Thus invoke_dismb is passed 0
    #as its argument.
    &pbmb_driver::invoke_dismb(0);
  }

  #invoke QAR_Driver if $pbmb_driver::qaTaskFile is present and if
  #$ENV{'DISBUILD_TMPDIR'}/finalize.upto_date file exists. The existence
  #of upto_date files in dismb signify the completion of that pass.
  my $finalize_file = "$ENV{'DISBUILD_TMPDIR'}/finalize.upto_date";

  if ($pbmb_driver::qaTaskFile ne "" && -e $finalize_file) {
    &pbmb_driver::run_qar();
  }

  #sleep for a second to make sure all stuff on socket is flushed before
  #closing.
  sleep(1);

  #Shutdown the connection to lpservice. Do this at the end of main, after
  #all messages have been sent
  &dismb_msg::shutdown;
}

&pbmb_driver::main(@ARGV);

