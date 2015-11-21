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
if {[info commands dbg_msg] == "" } {
    proc dbg_msg args  { puts $args }
    proc dbg_msg args  {}
}
set file_counter 0
proc open_file { file {mode r}} {
# throw error if cannot open
    set f [open $file $mode]
    global file_counter

    incr file_counter
    dbg_msg "$file_counter ***open $f"

    return $f
} 

proc close_file { f } {
    global file_counter
    set st [catch {close $f} msg]
    if $st {
	dbg_msg $msg
    } else {
	dbg_msg "$file_counter ***close $f"
	incr file_counter -1
    }
    return $st
}

proc check_dir_list args {
    foreach var $args {
	set dir [check_writable_dir $var]
	if {$dir != "" } { return $dir }
    }
    return ""
}

proc check_writable_dir var {
    global env
    set dir ""
    if [info exists env($var)] {
	if [file writable $env($var)] {
	    set dir $env($var)
	    puts "$var: $env($var)"
	} else {
	    puts "$var: $env($var): is not writable"
	}
    } 
    return $dir
}

if ![info exists unix_platform] {
    set unix_platform 0
    if [info exists tcl_platform(platform)] {
	if {$tcl_platform(platform)=="unix"} {
	    set unix_platform 1
	}
    }
    proc isunix {} {global unix_platform; return $unix_platform}
    
    proc make_tempdir {} {
	global env Tempdir
	set Tempdir [check_writable_dir DIS_SHARED_TMP]
	if {$Tempdir != "" } {
	    return $Tempdir 
	}

	if [isunix] {
	    set Tempdir [check_dir_list TMPDIR TMP]
	    if {$Tempdir == "" } {
		set Tempdir "/usr/tmp"
	    }
	} else {
	    set Tempdir [check_dir_list TEMP TMP]
	    if {$Tempdir == "" } {
		set Tempdir "C:/Temp"
	    }
	    regsub -all {\\} $Tempdir / Tempdir
	}

	if [file writable  $Tempdir] {
	    return $Tempdir
	} else {
	    puts stderr "$Tempdir is not writable"
	    puts stderr "check   DIS_SHARED_TMP TMPDIR TEMP TMP and restart"
	    exit 1
	}
    }
}

proc server_create_subdir { user host } {
    global Tempdir
    if ![info exists Tempdir] {
	set Tempdir [make_tempdir]
    }
    set subdir $Tempdir/disServer_$user@$host
    if [file exists $subdir] {
	file delete -force $subdir
    } 
    file mkdir $subdir
    if [isunix] {
	file attributes $subdir -permissions 0777
    }

    return $subdir
}

array set clientCommands {
    login 1
    logout 1
    data   1
    exec 1 
    submit 1 
    list 1 
    get 1  
    put 1
    report 1
    eval 0
}

# 1 means async

proc server_init {} {
    global serverInfo
    set serverInfo(no_clients) 0
    set serverInfo(port) 0
    set serverInfo(stream) 0
    set serverInfo(clients)  {}
}
if ![info exists serverInfo] {
    server_init
}

proc server_start { {port 61796}} {
    make_tempdir
    global serverInfo
    if [info exists serverInfo] {
	server_end
    }
    set err [catch { set f [socket -server server_accept_client $port] } ]

    if {$err != 0} {
	puts "Build server failed to connect to port $port."
	puts "Perhaps a build server is already running?"
	exit $err
    }

    server_init
    set serverInfo(stream) $f
    set serverInfo(port)   [lindex [fconfigure $f -sockname] 2]

    puts "[info hostname]:$serverInfo(port) : Build server started."
}

proc server_end {} {
    global serverInfo
    foreach id $serverInfo(clients) {
	server_end_client $id
    }
    if {$serverInfo(stream) != 0} {
	close_file $serverInfo(stream)
	set serverInfo(stream) 0
    }
} 

proc server_end_client {id {status exit}} {
    dbg_msg end_client $id
    upvar #0 $id client
    if { $client(stream) != 0 } {
	close_file $client(stream)
	set client(stream) 0
    }
    if { $client(status) == "data" } {
	if [info exists client(file)] {
	    close_file $client(file)
	}
	set pid client(parent)
	upvar #0  pid m
#	server_send_cmd $pid end $id $status
	set m(data) {} 
    }
    set client(status) $status
}

proc server_accept_client {f h p } {
    global serverInfo
    global file_counter
    incr file_counter
    dbg_msg "accept $f $h $p"
    incr serverInfo(no_clients)
    set id c_$serverInfo(no_clients)
    lappend serverInfo(clients) $id

    upvar #0 $id client
    set client(stream) $f
    set client(host) $h
    set client(port) $p
    set client(status) uninit

    fconfigure $f -buffering line
    fileevent $f readable "server_read_command $f $id"
}

proc server_read_command {f id } {
    global clientCommands
    if {[eof $f]} {
	server_end_client $id
	dbg_msg "close client $id"
	return
    }
    if {[catch {gets $f} cmd]} { return }
    if {$cmd == ""} {return}

    dbg_msg "received from $id: $cmd"
    set name [lindex $cmd 0]
    if [info exists clientCommands($name)] {
	set args [lrange $cmd 1 end]
	eval cmd_$name $id $args
    } else {
	server_error_cmd $id $cmd
    }
}
proc server_send_cmd {id cmd info {status {}}} {
    upvar #0 $id client
    if { $client(status) == "data" } {
	set id $client(parent)
	upvar #0 $id m
	set o $m(stream)
    } else {
	set o $client(stream)
    }
    puts $o [list $cmd $info $status]
    dbg_msg [list send $cmd $info $status]
}

proc server_error_cmd {id cmd} {
    dbg_msg "error: `$cmd`"
    server_send_cmd $id error $cmd
}

proc cmd_login {id user host } {
    upvar #0 $id client
    if { $client(status) != "uninit" } {
	server_error_cmd $id "already login-ed"
    } else {
	set client(user) $user
	set client(hostname) $host
	set client(status) user
	if [ catch {server_create_subdir $user $host} subdir] {
	    puts stderr $subdir
	    server_send_cmd $id login $id ""
	    server_error_cmd $id $subdir
	    server_end_client $id
	} else {
	    set client(subdir) $subdir
	    server_send_cmd $id login $id $subdir
	}
    }
}

proc cmd_data {id parent } {
    upvar #0 $id client
    if { $client(status) != "uninit" } {

    } else {
	set client(parent)   $parent
	set client(status) data
	upvar #0 $parent m
	set m(data) $id
    }
}

proc cmd_exec {id args } {
    cmd_exec_cmd $id exec $args
}

proc cmd_exec_cmd {id tag cmd} {
    dbg_msg "cmd_exec $id $tag $cmd"
    upvar #0 $id client
    set parent $client(parent)
    upvar #0 $parent m
    if [catch {open_file "| $cmd 2>$m(subdir)/$tag.stderr"} f] {
	server_error_cmd $id $f
    } else {
	set client(tag) $cmd
	set client(cmd) $tag

	fcopy $f $client(stream) -command "server_cmd_end $f $id"
    }

    return $f
}
proc cmd_list { id patt} {
    dbg_msg "cmd_list $id $patt"
    upvar #0 $id client
    set files [glob  -nocomplain $client(subdir)/$patt]
    set res {}
    foreach file $files {
	lappend res [file tail $file]
    }
    puts $client(stream) $res
}

proc cmd_submit {id admindir {view ""}} {
    dbg_msg "cmd_submit $id $admindir"
    upvar #0 $id client
    set parent $client(parent)
    upvar #0 $parent m
    set dir $m(subdir)
    global env
    set psh $env(PSETHOME)
    set cmd [list $psh/bin/disperl $psh/mb/disch.pl -SClocalRoot $dir -SCfileTable $dir/submission_check.in -SCadmindir $admindir]

    if { $view != "" } {
	lappend cmd -view $view
    }
#    set cmd [list /users/mg/bin/sbmcheck return $dir $dir/submission_check.out]
    cmd_exec_cmd $id submit $cmd
}

proc cmd_report {id admindir } {
    dbg_msg "cmd_submit $id $admindir"
    upvar #0 $id m
    set dir $m(subdir)
    global env
    set psh $env(PSETHOME)
    set cmd [list $psh/bin/disperl $psh/mb/disch.pl -SCreport -SClocalRoot $dir -SCfileTable $dir/submission_check.in -SCadmindir $admindir]
   
    eval exec $cmd &
}

proc server_cmd_end {f id bytes {error {}}} {
    upvar #0 $id client
    close_file $f
    catch {unset client(file)}
    server_send_cmd $id $client(cmd) $client(tag) [list $bytes $error]
    dbg_msg "end $client(cmd) $client(tag)"
    server_end_client $id
}

proc cmd_eval { id args } {
    upvar #0 $id client
    dbg_msg eval: '$args'
    set err [catch {eval eval $args} res]
    puts  $client(stream) [list eval $err $res]
    return $err
}

# put from client
proc cmd_put { id tag } {
    upvar #0 $id client
    set client(tag) $tag
    set client(cmd) put
    fconfigure $client(stream) -translation lf
    set fname [server_tag_fname $id $tag]
    if [catch {open_file $fname w} f] {
	server_error_cmd $id $f
	server_end_client $id
	return 0
    }
    set client(file) $f
    fcopy $client(stream) $f -command "server_cmd_end $f $id"
    return $f
}

#get to client
proc cmd_get { id tag } {
    upvar #0 $id client
    set client(tag) $tag
    set client(cmd) get
    fconfigure $client(stream) -translation lf
    set fname [server_tag_fname $id $tag]
    if ![file exists $fname] {
	server_error_cmd $id "$fname: does not exist"
	server_end_client $id
	return 0
    }
    if [catch {open_file $fname} f] {
	server_error_cmd $id "$fname: cannot read"
	server_end_client $id
	return 0
    }
    set client(file) $f
    fcopy $f $client(stream) -command "server_cmd_end $f $id"
    return $f
}

proc server_tag_fname {id tag} {
    upvar #0 $id client
    if {$client(status) == "data" } {
	set id $client(parent)
	upvar #0 $id m
	set subdir $m(subdir)
    } else {
	set user $client(subdir)
    }
    set fname $subdir/$tag
    return $fname
}

proc server_fname_tag {id fname} {
    file tail $fname
}

#
# To use with wish rather than tclsh, uncomment this line:
#
#wm withdraw .

if {[file tail $argv0]=="appServer.tcl"} {
    server_start 
}

vwait NeverSetVariableName

