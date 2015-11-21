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
if $CM {
    set fn [lindex $CMInfo $CM]
    source [string tolower $fn.tcl]
}

proc cm_get {com} {
    set arglist [dis_prepare_selection]

    # if no selection in listbox or selection is tree node, don't do anything
    if {[llength $arglist]<1} { return }

    set flist [dis_command apply fname $arglist]
    if { [llength $flist]==1 } {
	set flist [lindex $flist 0]
    }
    foreach ff $flist {
	set cm_sysfile [get_sharedfname $ff]
	set cm_workfile [get_privatefname $ff]
	if {$com} {
	    set comment [get_comment] 
	} else {
	    set comment {}
	}
	    set res [cm_get_src $cm_sysfile $cm_workfile $comment]
	if !$res {
	    myputs "cm_get $cm_workfile"
	}
	display_status Get $ff $res
    }
}

proc cm_unget {} {
    set arglist [dis_prepare_selection]

    # if no selection in listbox or selection is tree node, don't do anything
    if {[llength $arglist]<1} return
    
    set msg "Are you sure you want to discard all the modifications to the selected files\nand unget them?"
    if {[tk_messageBox -title "CM Message" -message $msg -type yesno -icon question] == "no"} return

    set flist [dis_command apply fname $arglist]
    if { [llength $flist]==1 } {
	set flist [lindex $flist 0]
    }
    foreach ff $flist {
	set cm_sysfile [get_sharedfname $ff]
	set cm_workfile [get_privatefname $ff]
	set res [cm_unget_src $cm_sysfile $cm_workfile]
	if !$res {
	    myputs "cm_unget $cm_workfile"
	    ed_kill_hook $cm_workfile
	}
	display_status Unget $ff $res
    }
} 

proc text_ok { w } {
    global comment_txt comment_status
    set comment_txt [$w.text get 1.0 end]
    set comment_status 1
    destroy $w
}

proc text_cancel { w } {
    global comment_txt comment_status
    set comment_txt Cancelled
    set comment_status 0
    destroy $w
}

proc get_comment {{ title "Enter comment" } } { 
    
    set w .text
    catch {destroy $w}
    toplevel $w
     wm title $w $title
    wm iconname $w "text"
#    positionWindow $w
    
    frame $w.buttons    
    pack $w.buttons -side bottom -fill x -pady 2m
    button $w.buttons.dismiss -text "OK" -command "text_ok $w"
    button $w.buttons.code -text "Cancel" -command "text_cancel $w"
    pack $w.buttons.dismiss $w.buttons.code -side left -expand 1
                 
    text $w.text -relief sunken -bd 2 -yscrollcommand "$w.scroll set" -setgrid 1 -height 5
    scrollbar $w.scroll -command "$w.text yview"
    pack $w.scroll -side right -fill y
    pack $w.text -expand yes -fill both
    $w.text insert 0.0 ""
    $w.text mark set insert 0.0
    set oldfocus [focus]
    focus $w.text
    catch {grab $w}
    tkwait window .text
    focus $oldfocus
    global comment_txt
    return [string trim $comment_txt]
}

proc cm_put {{fnames {}} {descr ""}} {
    if { $fnames == "" } {
       set arglist [dis_prepare_selection]
	} else {
	   set arglist $fnames
	}
    set flist [dis_command apply fname $arglist]
    if { [llength $flist]==1 } {
	set flist [lindex $flist 0]
    }
    global stat
    if { [llength $flist] == 0 } {
	$stat config -text "No files for put"
	return
    }
	if { $descr == "" } {
        set comment [get_comment] 
        global comment_status
        if { $comment_status==0 } {
	        $stat config -text "Put cancelled"
	         return
         }
	} else {
        set comment $descr
	}
	set retval ""
    foreach ff $flist {
	    set cm_sysfile [get_sharedfname $ff]
	    set cm_workfile [get_privatefname $ff]
	    if [isunix] {
	        global bufmodified
	        set bufmodified -1
	        myputs "cm_before_put $cm_workfile"
	        tkwait variable bufmodified
	        # 1 -- file not modified in editor buffer ; 2 -- file modified
	        if $bufmodified {
		    set msg "\"[file rootname $cm_workfile]\" is not saved in editor buffer.\nDo you still want to check in the file?"  
		        if {[tk_messageBox -title "CM Message" -message $msg -type yesno -icon question] == "no"} continue
	        }
	    }  
        set res [cm_put_src $cm_sysfile $cm_workfile $comment]
		lappend retval $res 
	    if !$res {
	        myputs "cm_put $cm_workfile"
	        ed_kill_hook $cm_workfile
	    }
	    display_status Put $ff $res  
    }
	return $retval
}

proc display_status { op fname res } {
    if !$res {
	set msg "$op $fname succeeded"
    } else {
	set msg "$op $fname failed"
    }
    status_line $msg
}

proc cm_exec_command { args } {
    global Tempdirectory
    set cm_err $Tempdirectory/cm_err.out
    set cm_log $Tempdirectory/cm_log
    set ff [open $cm_log a+]
    puts $ff "cm_command: $args"
    set err [ catch { eval exec $args 2>$cm_err } msg ]
    if { $err } {
	set f [open $cm_err]
	set msg [read $f]
	close $f
	puts $ff "cm_error:   $msg"
	tk_messageBox -title "CM error" -message $msg -type ok -icon error
    } else {
	global stat cm_exec_msg
	set cm_exec_msg $msg
	$stat config -text $msg
	if { [file size $cm_err] != 0 } {
	    set f [open $cm_err]
	    set warn [read $f]
	    close $f
	    puts $ff "info cm_info:    $warn"
	    tk_messageBox -title "CM warning" -message $warn -type ok -icon warning
	}
	puts $ff "info cm_info:    $msg"
    }
    puts $ff "==================================="
    close $ff
    return $err
}

proc cm_local {{is_query 1}} {
    global list
    global Transform driver errorInfo

    set serverlist {}
    set clientlist [cm_local_list]
	if {$clientlist == -1} {
		return
	}
    global all_modules
    if ![info exists all_modules] {
	   set mods [dis_cmd dis_modules]
	   set ii 0
	   foreach mod $mods {
	       incr ii
		   if {![isunix]} {
		      set mod [string toupper $mod]
		   }
	       set all_modules($mod) $ii
	   }
    }

    set inds ""
    foreach nm $clientlist {
	    regsub -all {\\} $nm / nm
	    if { ![isunix] && !$Transform } {
	        set nm ${driver}:$nm
	    }
	    if { !$Transform } {				
	        set nm [get_modelfname $nm]
	    }

	    if [info exists all_modules($nm)] {
	        lappend serverlist $nm
	        lappend inds $all_modules($nm)
	    }
    }
    if $is_query {
	   if { $inds != {} } {
	       query_command "set_copy \$all_modules $inds"
	   } else {
	       query_command "new_set"
	   }
	   status_line "[$list size] file(s) checked out"
    }
    return  $serverlist
}

proc cm_list_command { filter filelist } {
    set linelist {}
    foreach line $filelist {
	if [$filter $line] {
	    lappend linelist $line
	}
    }
    return $linelist
}

proc cm_file_filter { line } {
    if  { [file pathtype $line]=="relative" } {
	    return 0
    }
    if { [string first "<" $line]>=0 } {
	    return 0
    }
    
    return 1
}

array set integers { 1 1 2 2 3 3 4 4 5 5 6 6 7 7 8 8 9 9 }
proc cm_map { f1 f2 map } {
    global env integers
    upvar #0 $map this
    set diff $env(PSETHOME)/bin/diff
    regsub -all {\\} $diff {/} diff
    set f [open "| $diff -w $f1 $f2"]
    while {![eof $f]} {
	set line [gets $f]
	set c [string index $line 0]
	if {[info exists integers($c)]} {
	    if [regexp {(.*)([ac])(.*)} $line m x y z ] {
		set zz [split $z ,]
		if {[llength $zz] == 1} {
		    set start $zz
		    set end $zz
		} else {
		    set start [lindex $zz 0]
		    set end [lindex $zz 1]
		}
		for {set i $start} {$i <= $end} {incr i } { 
		    set this($f2:$i) $y
		}
	    } 
	}
    }
    catch {close $f}
}
