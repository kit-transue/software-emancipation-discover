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
# This file includes main ui and callbacks

proc status_line msg {
    global stat
    $stat config -text $msg
}
proc status_line0 msg {
    global stat
    if {[llength $msg] == 1} {
	set msg [lindex $msg 0]
    }
    $stat config -text $msg
}

# dis_sel: 1 -- tree; 2 -- main listbox; 3 -- Clipboard and groups 
proc dis_prepare_selection {} {
    global cur_ind dis_sel Tree result list clipboardlist clipboard_contents
    set res {}
    if {$dis_sel==1} {
	set n [Tree:getselection Tree]
	lappend res $Tree($n:fullname)
    } elseif {$dis_sel==2} {
	set sel [$list curselection]
	set result(sel_$cur_ind) $sel
	foreach el $sel {
	    lappend res [lindex $result(tags_$cur_ind) $el]
	    incr el
	}
    } elseif {$dis_sel==3} {
	set sel [$clipboardlist curselection]
	foreach el $sel {
	    lappend res [lindex $clipboard_contents(tags) $el]
	}
    }
    set retval {}
    foreach el $res {
	lappend retval "$el\n"
    }
    return $retval
}

proc dis_prepare_current_list {} {
    global cur_ind result
    set retval {}
    foreach el $result(tags_$cur_ind) {
	lappend retval "$el\n"
    }
    return $retval
}

proc bw_open_def {} {
    set arglist [dis_prepare_selection]
    open_definition browser $arglist
}

proc open_definition {from arglist} {
    set status [dis_command dis_before_open_def $arglist]
    switch $status {
	dislite_error {
	    if { $from == "editor" } {
		#write [concat "info" [error_msg SYM_NO_DEF]]
		set message [error_msg SYM_NO_DEF]
		editorMessage INFO "$message"
	    } else {
		status_line [error_msg SYM_NO_DEF]
	    }
	}
	single_def {
	    set res [dis_command open_single_def $arglist]
	    if {$res==""} {
		if { $from == "editor" } {
		    #write [concat "info" [error_msg INST_NO_DEF]]
		    set message [error_msg INST_NO_DEF]
		    editorMessage INFO "$message"		
		} else {
		    status_line [error_msg SYM_NO_FNAME]
		}
	    } else {

		set res [string trim $res \{\}]
		set model_fname [change_drivename [lindex $res 0]]
		set line [lindex $res 1]
		open_file $model_fname $line
	    }
	}
	default { 
	    query_command "open_default $arglist"
	    if { $from == "editor" } {
		status_line [join [dis_command describe_symbol $arglist]]
		activate_browser
	    }
	} 
    }
}

proc open_file { model_fname line } {
    global opened_modelflist Transform
    set local_fname [get_localfname $model_fname]
    if {[lsearch $opened_modelflist $model_fname]<0} {
	    if { $Transform<2 && [file exists $local_fname]} {
	        #write [concat {open} [list $local_fname $line]]
	        editorMessage OPEN "$local_fname" "$line"
	    }
    }
    set n [get_index $model_fname 0]
    if {$n<0} { return }
    if {$Transform<2} {
	set local_line [get_local_line $local_fname $line $n]
	#set res [concat open [list $local_fname $local_line]]
	editorMessage OPEN "$local_fname" "$local_line" 0
    } else {
	#set res [concat openreadonly [list $local_fname $line]]
	#write $res
	editorMessage OPEN "$local_fname" "$line" 1
    }
}

proc describe_instance { file line } {
    global dis_fnames modified opened_localflist model_localcopy Psethome Transform
    if [info exists dis_fnames($file)] {
	set model_fname $dis_fnames($file)
    } else {
	set model_fname [string trim [dis_command fname $file] \{\}]
	set dis_fnames($file) $model_fname 
    }
    set n [get_index $model_fname 0]
    if {$n<0} {return ""}	
    set local_fname [lindex $opened_localflist $n]
    set local_line [get_local_line $local_fname $line $n]
    if {$local_line<=0} {
	return "The instance may be deleted."
    } 
    set f [open $local_fname]
    for { set i 1 } { $i <= $local_line } {incr i } {
	set res [gets $f]
    }
    close $f
    return $res
}
proc format_fn fn {
    if {[lindex $fn 0]==""} {
	set no_defs [lindex $fn 1] 
	if { $no_defs !=""} {
	    set fn "$no_defs definitions"
	}
    }
    return $fn
}
proc bw_describe { } {
    global list clipboardlist cur_ind result dis_sel aseticons20
    if {$dis_sel==3} {
	set inds [$clipboardlist curselection]
    } else {
	set inds [$list curselection]
    }
    if {[llength $inds] == 1} {
	set sel [selection get]
	if $aseticons20 {
	    set is_inst [regexp {^. (/[^ ]+)[ ]*:[ ]*([0-9]+)} $sel match file line] 
	} else {
	    set is_inst [regexp {^(/[^ ]+)[ ]*:[ ]*([0-9]+)} $sel match file line] 
	}
	if $is_inst {
	    set res [describe_instance $file $line]
	} else {
	    set arglist [dis_prepare_selection]
	    set res [dis_command dis_describe $arglist]
	    if { $res == "dislite_error" } {
		set res "No description available"
	    } else {
		set r [lindex $res 0]
		set t [lindex $r 0]
		set kind [lindex $t 0]
		set_kind_menu $kind
		if {$dis_sel==2} {
		    set result(kind_$cur_ind) $kind
		}

		if { $kind=="cfun" } {
		    set fn [format_fn [lindex $r 2]]
		    set res "[lindex $r 0] [lindex $r 1] => $fn"
		} elseif { $kind=="module" } {
		    set lname [lindex $r 1]
		    global fnameCache Transform
		    if {$Transform==2} {
			set fnameCache($lname) [dis_command fname $arglist]
		    } else {
			set flocat [dis_command fname $arglist]
			set fnameCache($lname) [get_localfname $flocat]
		    }
		    set res "module $fnameCache($lname)"
		} elseif { $kind=="macro" } {
		    set def [lindex $r 1]
		    set res "macro $def => [lindex $r 2]"
		} elseif {[llength $r] < 3} {
		    set res [lindex $r 0]
		} else {
		    set fn [format_fn [lindex $r 2]]
		    set res "[lindex $r 0] => $fn"
		}
	    }
	}
    } else {
	set res "***No Selection"
	do_clear
    }
    if {[regexp {^Bad set index: [0-9]+$} $res]==1} {
	set res {Please click again}
    }
    status_line $res
}

# builds project/group/clipboard frame 

proc build_leftpanel { t1 } {
    set treeframe [projbrowser_manager $t1]
    set grpframe [group_manager $t1]
    set clpframe [clipboard_manager $t1]

    grid $treeframe $grpframe  $clpframe -row 0 -column 0 -sticky nsew
    grid rowconfigure $t1 0 -weight 1
    grid columnconfigure $t1 0 -weight 1
}

proc tree_selected {t n} {
    upvar #0 $t Tree
    global dis_sel list

    $list selection clear 0 end
    set dis_sel 1 

    set_kind_menu proj
    status_line0 [dis_command "dis_describe \"$Tree($n:fullname)\""] 
}

proc tree_unselected {t n} {
    global dis_sel
    set dis_sel 2
}

proc tree_list {t n} {
    upvar #0 $t Tree
    query_command "filter module \[dis_get_subs \"$Tree($n:fullname)\"\]"
}

proc tree_expand {t n} {
    upvar #0 $t Tree
    global dis_sel
    global list
    set subs [dis_query "filter project||scope \[dis_get_subs \"$Tree($n:fullname)\"\]" name]	
    return $subs
}
proc change_cursor { arg } {
    global tree list grp clipboardlist
    $tree config -cursor $arg
    $list config -cursor $arg
    $grp.list config -cursor $arg
    $clipboardlist config -cursor $arg
}
 
proc init_projbrowser {} {
    global CurrentScope scope_flag
    global treeframe
    if $scope_flag {
	set root_projs $CurrentScope
	dis_command set_client_scope $CurrentScope
    } else {
	set root_projs  [dis_query dis_roots name]
    }
    Tree:init Tree $root_projs
    raise $treeframe
}

proc projbrowser_manager { t1 } {
    global Tree treeparent
    catch {unset Tree}
    set treeparent $t1

    array set Tree {
	icon     idir
	select   tree_selected
	unselect tree_unselected
	expand   tree_expand
	list     tree_list
    }

    global tree treeframe

    set treeframe $t1.treeframe
    catch {destroy $treeframe}
    tree_manager Tree $treeframe
    set tree $Tree(widget)
    return $treeframe
}

proc build_listbox { t2 } {
    global aseticons20 list dis_sel old_bind tree listfont
    if $aseticons20 {
	listbox $t2.list -font aseticons-20 -yscrollcommand "$t2.scroll set" -selectmode extended 
    } else {
	listbox $t2.list -font $listfont -yscrollcommand "$t2.scroll set" -selectmode extended 
    }
    set list $t2.list
    scrollbar $t2.scroll -command "$t2.list yview"
    
    pack $t2.scroll -in $t2 -side right -fill y 
    pack $t2.list -in $t2 -side left -fill both -expand true

    bind $t2.list <Double-Button-1> "set dis_sel 2; bw_open_def; focus %W"
    bind $t2.list <Return> "set dis_sel 2; bw_open_def; focus %W"
    bind $t2.list <Control-Button-1> "Tree:unselect Tree; set dis_sel 2"
    bind $t2.list <Button-1> "Tree:unselect Tree; set dis_sel 2; $old_bind; bw_describe; focus %W"   
    bind $t2.list <KeyRelease-Down> "bw_describe"
    bind $t2.list <KeyRelease-Up> "bw_describe"
}

proc build_statusline { s } {
    global statfont stat
    label $s.tatus -font $statfont -anchor w 
    set stat $s.tatus
    pack  $s.tatus -side top -fill both -expand true
}

proc scope_toggle {w i} {
    global Tree dis_sel treeframe treeparent
    global CurrentScope  scope_flag

    if {$dis_sel != 1} {
	status_line "no scope selected"
	return
    }
    set n [Tree:getselection Tree]
    if {$n < 0} {
	status_line "no scope selected"
	return
    }
    set no_roots $Tree(no_roots)

    if {$n > $no_roots} {
	status_line "a root project must be selected"
	return
    }

    if $scope_flag {
	set scope_flag 0
	set CurrentScope ""
	dis_command unset_client_scope
	projbrowser_manager $treeparent
	grid $treeframe -row 0 -column 0 -sticky nsew
	init_projbrowser
	$w  entryconfigure $i -label "Scope Set"
    } else {
	set scope_flag 1
	set CurrentScope $Tree($n:fullname)
	set root $CurrentScope
	dis_command set_client_scope $root
	projbrowser_manager $treeparent
	grid $treeframe -row 0 -column 0 -sticky nsew
	Tree:init Tree $root
#	raise $treeframe
	$w  entryconfigure $i -label "Scope Unset"
    }
    set dis_sel 2
    global all_symbols_flag
    save_pref_file
    set all_symbols_flag 1
}





