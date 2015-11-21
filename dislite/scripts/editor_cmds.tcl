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
# This file deals with commands sent from editors (emacs, xemacs, 
#  vim amd Dev. Studio)
# the commands include:
# pid_reply -- for emacs foucs, emacs in unix only
# save_hook -- file buffer was saved
# kill_hook -- file buffer was killed
# exit_hook -- editor want to exit 
# activate  -- deiconify Dev. Express browser and bring it into top 
# inst_def  -- ask for description of the selected symbol
# query     -- bring selected symbol in editor to Dev. Express browser
# open_def  -- ask for the definition of the selected symbol
# instances -- list all instances of the selected symbol in both Dev. Xpress 
#                browser and editor buffer 
# editor_get -- checkout file in editor, emacs only

proc ed_describe { fn pos } {
    set res [dis_command ed_inst_def \"$fn\" $pos]
    if { $res == "dislite_error" } {
	set res [error_msg INST_NO_SYM]
    } else {
	set r [lindex $res 0]
	set fn [format_fn [lindex $r 2]]
	set res "{[lindex $r 0] [lindex $r 1] => $fn}"
    }
    editorMessage INFO $res 
    #write "info $res"
}

proc ed_query { fn pos } {
    set res [dis_command check_inst_sym \"$fn\" $pos]
    if { $res == "dislite_error" } {
	set message [concat "info" [error_msg INST_NO_SYM]]
	editorMessage INFO $message
	#write [concat "info" [error_msg INST_NO_SYM]]
	
    } else {
	query_command "get_inst_sym \"$fn\" $pos"
	activate_browser
    }
}

proc ed_open_def { fn pos } {
    set res [dis_command check_inst_sym \"$fn\" $pos]   
    if { $res == "dislite_error" } {
	set message [concat "info" [error_msg INST_NO_SYM]]
	editorMessage INFO $message
	#write [concat "info" [error_msg INST_NO_SYM]]
    } else {
	set sym [dis_command get_inst_sym \"$fn\" $pos]
	open_definition editor $sym
    }
}

proc ed_instances {fn pos} {
    global inst
    if !$inst(running) {
	set res [dis_command check_inst_sym \"$fn\" $pos]
	if { $res == "dislite_error" } {
	    set message [concat "info" [error_msg INST_NO_SYM]]
	    editorMessage INFO $message
	    #write [concat "info" [error_msg INST_NO_SYM]]
	} else {
	    set instances [dis_command "dis_instances \[ get_inst_sym \"$fn\" $pos \] "]
	    do_instances $instances
	}
    }
}
    
proc ed_save_hook local_fname {
    global opened_localflist Transform 
    set len [llength $opened_localflist]
    set n [get_index $local_fname 1]
    if {$n<0} {return}
    if {$n>=0 && $n<$len && $Transform<2} {
	update_filecache $local_fname $n
    }
}

proc ed_kill_hook local_fname {
    global opened_localflist opened_modelflist
    set n [lsearch $opened_localflist $local_fname]
    if {$n>=0} {
	set opened_modelflist [lreplace $opened_modelflist $n $n {}]
	set opened_localflist [lreplace $opened_localflist $n $n {}]
    }
}

proc ed_get local_fname {
    global CM
    if $CM {
	set cm_sysfile [get_sharedfname $local_fname]
	set cm_workfile [get_privatefname $local_fname]
	if ![cm_get "Checked out in 'ed_get'." $cm_workfile] {
	    #set res "editor_get_reply $cm_workfile"
	    editorMessage EDITOR_GET_REPLY $cm_workfile
	} else {
	    set message "Error: Cannot check out $local_fname"
	    editorMessage INFO $message
	}
    } else {
	#set res "info [error_msg NO_CM]"
	set message "info [error_msg NO_CM]"
	editorMessage INFO $message
    }
}
	
proc ed_pid_reply {} {
    global win_tag Service
    set editor_window [wm_extern name "DISCOVER $win_tag"]
    wm_extern
    myputs "Dislite_Service $Service 0"
}

