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
require ("dismb_lib.pl");
require ("dismb_prefs.pl");

#Has socket stuff to send message to message/log viewer
require ("viewer_client.pl");

#
# &dismb_msg::join (" *** ERR: ", "line 1", "line 2");
#
sub dismb_msg::join {
    local ($cnt) = 0;
    local ($ans) = "";
    local (@arr) = @_;
    local ($sz) = $#arr;
    
    while ($cnt <= $sz) {
	$ans .= " " x length (@_[0]) if ( $cnt > 1 );
	$ans .= @_[$cnt];
	$ans .= "\n" if ($cnt > 0);
	$cnt ++;
    }
    return $ans;
}


sub dismb_print {
    &dismb_msg::msg;
}

# Eval returns an error message ending with a newline,
# which needs the newline removed before it is suited for use with
# the message API.  But take care not to trivialize a substantial message.

# Perhaps this could be combined with eval to produce &dismb_msg::eval.
sub dismb_msg::fixeval {
    if ($@ && (length $@ > 1)) {
	chomp $@;
    }
}

sub dismb_msg::die {
    &dismb_print("catastrophe", @_);
    # Shut down lpservice connection before it dies, so the other end of the
    # socket won't report an error.  Socket will be reopened if needed by caller.
    &dismb_msg::shutdown;

    # The standard message reporting the problem has been delivered.
    # If the caller catches the failure, the caller may need some indication
    # of how the problem arose, but please let's not duplicate long messages!
    my($msg) = &dismb_msg::substitute_arguments(@_);
    if (length $msg > 60) {
	$msg = substr $msg, 0, 50;
	($msg) = ( $msg =~ /^(.*?)\s*$/ );
	$msg = $msg . " ...";
    }
    # Append \n to suppress Perl verbosity.
    die "$msg\n";
}

sub dismb_msg::inf {
    &dismb_print("normal", @_);
}

sub dismb_msg::vrb {
    if (&dismb_prefs::is_verbose_mb()) {
	&dismb_print("normal", @_) ;
    }
}

sub dismb_msg::err {
    &dismb_print("error", @_);
}


sub dismb_msg::wrn {
    &dismb_print("warning", @_);
}


sub dismb_msg::print_header {
    local($hdr) = "@_";
    local($tm)  = &dismb_lib::time_stamp;
    &dismb_print ("\n                     ===== \$1 \$2 start \$3 =====\n",&dismb_msg::get_host_tag,$hdr,$tm);
}


sub dismb_msg::print_footer {
    local($hdr) = "@_";
    local($tm)  = &dismb_lib::time_stamp;
    &dismb_print ("\n                     ===== \$1 \$2 end \$3 =====\n",&dismb_msg::get_host_tag,$hdr,$tm);
}

sub dismb_msg::write_failed {
    my($file, $err) = @_;
    &dismb_msg::err("Failed to write to file, \$1 : \$2", $file, $err);
}

sub dismb_msg::set_host_cpu {
    local($host, $cpu) = @_;
    $dismb_msg::host = $host;
    $dismb_msg::cpu  = $cpu;
}


sub dismb_msg::get_host_tag {
    if (defined $dismb_msg::host) {
	return "${dismb_msg::host}:${dismb_msg::cpu}";
    } else {
	return "";
    }
}

#Global variable $dismb_msg::lpservice_name is the name of the log presentation
#service that the messages should be sent to (via socket connection).
#If $dismb_msg::lpservice_name is not empty and -stdout option is present then
#messages are sent both on socket and to stdout.
#If $dismb_msg::lpservice_name is empty messages will be sent to stdout.
#$dismb_msg::lpservice_name is first set by getting value of command line
#option -lpservice.
#
#If $dismb_msg::lpservice_flag 
#  = 0 then messages go to just stdout,
#  = 1 then messages go to just socket,
#  = 2 then messages go to both socket and stdout.
#
#Note: Did not use dismb_options::get function since messages start coming
#in before options are initialized.
#
sub dismb_msg::log_viewer_is_running {
  if ( !defined($dismb_msg::lpservice_flag) ) {
    $dismb_msg::lpservice_flag = 0;
    $dismb_msg::lpservice_name = "";
    my($lpser_flag, $stdout_flag) = (0, 0);
    LOOP: foreach $arg (@ARGV) {
      if ( $lpser_flag == 1) {
        $dismb_msg::lpservice_name = $arg if (! ($arg =~ /^-/) );
        $lpser_flag = 0;
      }
      if ( $arg =~ /^-lpservice/ ) {
        $lpser_flag = 1;
      } elsif ( $arg =~ /^-stdout/ ) {
        $stdout_flag = 1;
      }
    }
    if ( $dismb_msg::lpservice_name ne ""  && $stdout_flag) {
      $dismb_msg::lpservice_flag = 2;
    } elsif ( $dismb_msg::lpservice_name ne "") {
      $dismb_msg::lpservice_flag = 1;
    }
  }
  return $dismb_msg::lpservice_flag;
}

sub dismb_msg::lp_flags {
    my($lp_flag) = @_;
    if (not defined($lp_flag)) {
	$lp_flag = "-lpservice";
    }
    $lp_flag =~ m/^(.*)l/;
    my($mg_flag) = $1 . "message_group";  # -message_group or --message_group
    my(@flags) = ();
    my($lp_service) = $dismb_msg::lpservice_name;
    if ($lp_service ne "") {
	push @flags, ($lp_flag, "$lp_service");
	my($message_group) = &dismb_msg::current_auto_message_group;
	if ($message_group ne "") {
	    push @flags, ($mg_flag, $message_group);
	}
    }
    return @flags;
}

#
#Example:
#  &dismb_msg::msg("catastrophe3", "Could not open \$1\$2.", "C:/test/", "a.dat");
#First argument to this function is message severity followed by message_id. 
#The arguments following are arguments for message.
sub dismb_msg::msg {
    my($ret_val) = &dismb_msg::log_viewer_is_running();
    if ( $ret_val == 2 ) {
        &dismb_msg::write_msg_to_socket(@_);
        &dismb_msg::write_msg_to_stdout(@_);
    } elsif ( $ret_val == 1 ) {
        &dismb_msg::write_msg_to_socket(@_);
    } else {
        &dismb_msg::write_msg_to_stdout(@_);
    }
}

sub dismb_msg::write_msg_to_socket {
    my($xml_message) = &dismb_msg::get_xml_fmt_msg(@_);

    #send message on socket to message/log viewer.
    &viewer_client::send_message($dismb_msg::lpservice_name, $xml_message);
}


sub dismb_msg::get_xml_fmt_msg {
    my(@arg_list) = @_;
    my($arg_size) = $#arg_list;
    my($sev, $st_ind) = ("normal", 0);
    my($xml_fmt_msg) = "";

    if ($arg_size >= 0 ) {
        if (@arg_list[0] eq "catastrophe" || @arg_list[0] eq "normal" || @arg_list[0] eq "error" || @arg_list[0] eq "warning" ) {
           $sev = @arg_list[0];
           $st_ind = 1;
        }
        #print message group id, severity and message id
        $xml_fmt_msg .= "<msg parent=\"";
        $xml_fmt_msg .= &dismb_msg::current_auto_message_group . "\"";
        $xml_fmt_msg .= " sev=\"$sev\">";
        $xml_fmt_msg .= dismb_msg::escape_xml_chars(@arg_list[$st_ind]);

        #print all arguments along with xml tags
        $st_ind++;
        my($arg_num, $i) = (1, 0);
        for $i ($st_ind .. $arg_size) {
       	    $xml_fmt_msg = $xml_fmt_msg . "<arg num=\"" . "$arg_num" . "\">";
            $xml_fmt_msg .= dismb_msg::escape_xml_chars(@arg_list[$i]);
            $xml_fmt_msg .= "</arg>";
            $arg_num++;
        }

        #print end message tag
        $xml_fmt_msg .= "</msg>";
    }
    return $xml_fmt_msg;
}

sub dismb_msg::escape_xml_chars {
    my($chars) = @_[0];
    $chars =~ s'&'&amp;'g;
    $chars =~ s'<'&lt;'g;
    $chars =~ s'>'&gt;'g;
    $chars =~ s'"'&quot;'g;
    return $chars;
}

sub dismb_msg::write_msg_to_stdout {

    my($output_msg) = &dismb_msg::substitute_arguments(@_);
    #add a newline character
    $output_msg .= "\n";

    &dismb_msg::write_to_logs_and_stdout($output_msg);
}

# This function writes messages to stdout or to the appropriate logfile.
# It should be passed only messages which have already had their arguments
# substituted and their severities prepended;  for example, messages which
# have just been processed, and messages read from dismb log files.
sub dismb_msg::write_to_logs_and_stdout {
    my($output_msg) = @_;

    # This code decides where to send the message;  if there is a log file,
    # it sends it there, and if not, or sometimes if there is, it writes to
    # stdout as well.
    my($ff) = ($dismb_driver::PREINIT) ? $dismb_driver::PREINIT : $dismb_prefs::logfile;
    if ($ff) {
	if (! (print $ff "$output_msg") ) {
           #Could not write to log file, print the error and exit.
           &dismb_msg::shutdown(); #shutdown lpservice
           die("Failure writing message \"$output_msg\" to logfile : $!\n");
        }
    }
    # print to stdout if there was no log file, or if force_monitor or
    # is_monitoring was set, and suppress_monitor was not set or
    # PSET_TEST_RUNNING was set.  Whew.
    if (! $ff || (($dismb::force_monitor || ($dismb_prefs::initialized && &dismb_prefs::is_monitoring_log())) && (defined $ENV{'PSET_TEST_RUNNING'} || ! defined $dismb::suppress_monitor))) {
	print "$output_msg";
    }
}

# Substitutes arguments in message id and returns the result.
# Checks if the first argument is message severity and adds it,
# unless severity is "normal".
sub dismb_msg::substitute_arguments {
    my(@arg_list) = @_;
    my($ret_msg) = "";
    if ( $#arg_list >= 0 ) {
	my($arg_base, $start, $found) = (0, 0, -1);
	if (@arg_list[0] eq "catastrophe" || @arg_list[0] eq "normal" || @arg_list[0] eq "error" || @arg_list[0] eq "warning" ) {
	    if (@arg_list[0] ne "normal") {
	        $ret_msg .= @arg_list[0] . ": ";
	    }
	    $arg_base = 1;
	}
	my($message_id) = @arg_list[$arg_base];

	my($look_for) = "\$";
	my($preserve, $arg_num, $next);
	while (($preserve, $arg_num, $next) = ($message_id =~ /^(.*?)\$([0-9]+)(.*)$/s)) {
	    $ret_msg .= $preserve;
	    if ( defined(@arg_list[$arg_num]) ) {
		$ret_msg .=  @arg_list[$arg_num + $arg_base];
	    }
	    else {
		$ret_msg .= ("\$" . $arg_num);
	    }
	    $message_id = $next;
	}
	$ret_msg .= $message_id;
    }   
    return $ret_msg;
}


###################################
# Message Group API stuff
#

#Communicates with log_presentation to get a new message group id and calls
#init_auto_message_group which pushes the new id onto the stack.
#
sub dismb_msg::push_new_message_group {
  if (&dismb_msg::log_viewer_is_running()) {
    my($mgroup_name, $parent_mgroup_id) = @_;
   
    my($sending_msg) = &dismb_msg::construct_mgid_xml_msg;
    my($received_msg) = &viewer_client::send_and_receive_message($dismb_msg::lpservice_name, $sending_msg);
    my($new_mgroup_id) = &dismb_msg::extract_mgid_from_xml_msg($received_msg);
    if ($parent_mgroup_id eq "") {
        $parent_mgroup_id = &dismb_msg::current_auto_message_group();
    }

    $sending_msg = &dismb_msg::construct_spawn_xml_msg($mgroup_name, $parent_mgroup_id, $new_mgroup_id);
    &viewer_client::send_message($dismb_msg::lpservice_name, $sending_msg);

    &dismb_msg::init_auto_message_group($new_mgroup_id);
  }
}


#Pops the most recently pushed message group id from the stack.
#message_groupid_stack is the global array that acts as a stack for
#holding message group ids.
sub dismb_msg::pop_new_message_group {
  if (&dismb_msg::log_viewer_is_running()) {
    my($mgroup) = pop @message_groupid_stack;
  }
}


#Pushes a new message group id onto the stack
sub dismb_msg::init_auto_message_group {
  if (&dismb_msg::log_viewer_is_running()) {
    my($mgroup_id) = @_[0];
    push @message_groupid_stack, $mgroup_id;
  }
}


#returns the current message group id
sub dismb_msg::current_auto_message_group {
  my($mgid) = "";
  if (&dismb_msg::log_viewer_is_running()) {
    $mgid = @message_groupid_stack[$#message_groupid_stack];
  }
  return $mgid;
}

%dismb_msg::message_group_id = ();

sub dismb_msg::extract_mgid_from_xml_msg {
  my($xml_msg) = @_[0];
  my($mgid) = "";
  if ( $xml_msg =~ /^<mgid val="([^"]+)"\s*\/>$/ ) {
     $mgid = $1;
  }
  return $mgid;
}

sub dismb_msg::construct_mgid_xml_msg {
  return "<mgid get=\"\"/>";
}

sub dismb_msg::construct_spawn_xml_msg {
  my($mgroup_name, $parent_mgid, $new_mgid) = @_;

  my($smsg) = "<spawn parent=\"" . $parent_mgid . "\" ";
  $smsg .= "child=\"" . $new_mgid . "\">" . $mgroup_name . "</spawn>";

  return $smsg;
}

sub dismb_msg::construct_progress_xml_msg {
  my($prg_msg) = @_[0];
  my($progress_msg) = "<progress done=\"" . $prg_msg . "\"/>";

  return $progress_msg;
}

#send a progress message to log presentation
sub dismb_msg::report_progress {
  if (&dismb_msg::log_viewer_is_running()) {
    my($prg_msg) = @_[0];
    my($progress_msg) = &dismb_msg::construct_progress_xml_msg($prg_msg);
    &viewer_client::send_message($dismb_msg::lpservice_name, $progress_msg);
  }
}

# Define a task name for the sake of reporting progress.
# One arg: name.
sub dismb_msg::define_progress_task {
  if (&dismb_msg::log_viewer_is_running()) {
    my($task_name) = @_[0];
    my($define_task_msg) = "<task name=\"" . $task_name . "\"/>";
    &viewer_client::send_message($dismb_msg::lpservice_name, $define_task_msg);
  }
}

# Specify that a task consists of a number of equal steps.
# Arg 1: name of task that consists of steps
# Arg 2: name of 1 step
# Arg 3: number of steps expected to complete the task
sub dismb_msg::define_progress_steps {
  if (&dismb_msg::log_viewer_is_running()) {
    my($task_name, $step_name, $n_steps) = @_;
    my($define_steps_msg) = "<steps name=\"" . $step_name .
			    "\" count=\"" . $n_steps .
			    "\" for=\"" . $task_name . "\"/>";
    &viewer_client::send_message($dismb_msg::lpservice_name, $define_steps_msg);
  }
}

# Specify that a task's progress is tracked via named subtasks.
# Arg 1: name of task that has subtasks.
# More args: subtasks.
#   Each extra arg is a reference to a 2-element array giving
#   subtask name and size, e.g. ["initialization", 5].
sub dismb_msg::define_progress_subtasks {
  if (&dismb_msg::log_viewer_is_running()) {
    my($task_name, @subtasks) = @_;
    my($msg) = "<breakdown for=\"" . $task_name . "\">";
    foreach $subtask (@subtasks) {
	$subtask_name = $subtask->[0];
	$size = $subtask->[1];
	$msg .= "<task name=\"" . $subtask_name . "\" size=\"" . $size . "\"/>";
    }
    $msg .= "</breakdown>";
    &viewer_client::send_message($dismb_msg::lpservice_name, $msg);
  }
}

#check command line options for -message_group and call 
#init_auto_message_group to initialize 
#This is called before command line options are processed therefore
#dismb_options::get is not used to get value of -message_group.
sub dismb_msg::init_message_group_from_option
{
  my($msg_group_id);
  my($found) = 0;
  LOOP: foreach $arg (@ARGV) {
    if ( $found == 1) {
      $msg_group_id = $arg if (! ($arg =~ /^-/) );
      last LOOP;
    }
    if ( $arg =~ /^-message_group/ ) {
      $found = 1;
    }
  }
  if (not defined($msg_group_id)) {
    $msg_group_id = "0";
  }
  &dismb_msg::init_auto_message_group($msg_group_id);
}

sub dismb_msg::shutdown
{
  &viewer_client::shutdown_lpServiceConnection;
}

return 1;
