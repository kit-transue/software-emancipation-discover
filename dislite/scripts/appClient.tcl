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
# for talking to the build/submission check server

if ![info exists unix_platform] {
    set unix_platform 0
    if [info exists tcl_platform(platform)] {
	if {$tcl_platform(platform)=="unix"} {
	    set unix_platform 1
	}
    }
    proc isunix {} {global unix_platform; return $unix_platform}
}

if {[info commands dbg_msg] == "" } {
    proc dbg_msg args  { puts $args }
    proc dbg_msg args  {}
}

array set serverCommands {
    login 1
    logout 1
    data   1
    exec 1 
    get 1  
    put 1
    status 1
    info 1
    error 1
    eval 0
    submit 1
    report 1
    list 1
}

proc client_connect { host {port 61796} } {
    global serverInfo
    client_end
	global closed
    set closed 0
    set err [catch {
               set f [socket $host $port]
               fconfigure $f -buffering line
	        }]
	if {$err != 0} {
	    return "0";
	}

    set serverInfo(stream) $f
    set serverInfo(port) $port
    set serverInfo(host) $host
    set serverInfo(status) uninit
    set serverInfo(myname) {}
    
    global env
    set user $env(USER)
    set host [info hostname]
    puts $f "login $user $host"
    fileevent $f readable client_read_command
    
    vwait serverInfo(subdir)
    return $f
}

set closed 0
proc client_end {} {
    global closed
	set closed 1
    global serverInfo
    if [info exists  serverInfo(stream)] {
	    catch {close $serverInfo(stream)}
	    unset serverInfo(stream)
    }
    if [info exists  serverInfo(exec)] {
	    catch {close $serverInfo(exec)}
	    unset serverInfo(exec)
    }
    if [info exists  serverInfo(submit)] {
	    catch {close $serverInfo(submit)}
	    unset serverInfo(submit)
    }
    if [info exists  serverInfo(put)] {
	    catch {close $serverInfo(put)}
	    unset serverInfo(put)
    }
    if [info exists  serverInfo(get)] {
	    catch {close $serverInfo(get)}
	    unset serverInfo(get)
    }

}

proc client_read_command {} {
global closed

    if { $closed != 0 } {
	  return
	}
    global serverInfo
    if {[catch {eof $serverInfo(stream)} closed] || $closed } { 
	   dbg_msg "server closed"
	   catch {close $serverInfo(stream)}
	   unset serverInfo
	   return
    }
    set cmd [gets $serverInfo(stream)]
    if {$cmd == ""} {return}
    dbg_msg "received from server: $cmd"
    set name [lindex $cmd 0]
    global serverCommands
    if [info exists serverCommands($name)] {
	set args [lrange $cmd 1 end]
	eval msg_$name $args
    } else {
	dbg_msg "*** unknown server command"
    }
}

proc msg_login {clid subdir} {
    global serverInfo
    set serverInfo(myname) $clid
    set serverInfo(status) login
    set serverInfo(subdir) $subdir
}

proc client_send_command {args} {
    global serverInfo
    puts $serverInfo(stream) $args
}

proc client_eval args {
    global eval_result
    global serverInfo
    puts $serverInfo(stream) "eval $args"
    vwait eval_result
    set res $eval_result
    unset eval_result
    return $res
}
proc client_list {{patt {*}}} {
    global serverInfo
    puts $serverInfo(stream) [list list $patt]
    set res [gets $serverInfo(stream)]
    return $res
}

proc msg_eval args {
    global eval_result
    set eval_result $args
}

proc client_open_data_channel {tag {line 1 } {translation 1}} {
    global serverInfo
    set f [socket $serverInfo(host) $serverInfo(port)]
    if $line {
	    fconfigure $f -buffering line
    }
    fconfigure $f -blocking 0
    if !$translation {
	   fconfigure $f -translation lf
    }

    set serverInfo($tag) $f
    puts $f "data $serverInfo(myname)"
    return $f
}

proc client_start args {
    set f [client_open_data_channel exec 1 1]
    puts $f "exec $args"
    fileevent $f readable "client_exec_echo $f"
    return $f
}
proc client_submit {admindir workdir} {
    global CM
    set f [client_open_data_channel submit 1 1]
    set view ""
    if [isunix] {
	if $CM {
	    set view [cm_get_view]
	}
    }
    puts $f "submit $admindir $view"
    fileevent $f readable [list client_submit_echo $admindir $workdir $f]
    return $f
}

proc client_report { admindir } {
    global serverInfo
    set f $serverInfo(stream)
    puts $f "report $admindir"
}

proc client_submit_echo {admindir workdir f} {
    if {[catch {eof $f} closed] || $closed } { 
	    dbg_msg "submit channel closed"
	    catch { close $f }
        client_submit_end $admindir $workdir
	    return
    }
    set msg [gets $f]
    dbg_msg $msg
}

proc client_exec_echo {f} {
    if {[catch {eof $f} closed] || $closed } { 
	   dbg_msg "exec channel closed"
	   catch {close $f}
	   return
    }
    set msg [gets $f]
    puts $msg
}

proc msg_report args {
    dbg_msg "report finished: $args"
}

proc msg_submit args {
    dbg_msg "submit finished: $args"
}

proc msg_exec args {
    dbg_msg "exec finished: $args"
}
proc msg_get args {
    dbg_msg "get finished: $args"
}
proc msg_put args {
    dbg_msg "put finished: $args"
}
proc msg_error args {
    dbg_msg "error: $args"
}
proc fcopy_my {in out} {
    puts -nonewline $out [read $in]
}
proc client_put_file {fname tag} {
    set in [open $fname r]
    set f [client_open_data_channel put 0 0]
    puts $f "put $tag"
    flush $f
    fcopy_my $in $f
    close $in
    close $f
    set file_size [file size $fname]
    dbg_msg "finished put $tag $file_size"
}

proc client_get_file {fname tag} {
    set f [client_open_data_channel get 0 0]
    puts $f "get $tag"
    flush $f

    set out [open $fname w]
    fcopy $f $out
    close $out
    close $f
    dbg_msg "finished get $tag"
}

proc client_get_file_list {workdir} {
    set ftable $workdir/submission_check.out
    client_get_file submission_check.out $ftable

    set f [open $ftable r]
    set total_list [read $f]
    close $f

    foreach {tag pname lname} $total_list {
	client_get_file $workdir/$tag $tag
    }
}

proc client_put_file_list {workdir fn} {
    global serverInfo

    set ftable $workdir/submission_check.in
    set ft [open $ftable w]
    puts $ft "# build_server"
    puts $ft "#TableStart"

    set f [open $fn]
    set total_list [read $f]
    close $f

    set w $serverInfo(subdir)
    foreach {tag pname fname lname} $total_list {
	client_put_file $fname $tag
	puts $ft "\"$pname\" \"$w/$tag\" \"$lname\" \"[file size $fname]\""
    }
    puts $ft "#TableEnd"
    close $ft

    client_put_file $ftable submission_check.in
}
