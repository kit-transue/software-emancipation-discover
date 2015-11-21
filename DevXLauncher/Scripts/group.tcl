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
set groupDir $env(HOME)/dislite/$Model_name
if {![file exists $groupDir]} {
    file mkdir $groupDir
}

proc group_list {} {
    global groupDir group
    if ![file isdirectory $groupDir] { return }
    set list [glob -nocomplain $groupDir/*.grp]
    set res {Clipboard}
    set group(Clipboard.index) 0
    set group(Clipboard.modified) 0
    
    set ind 1
    foreach el $list {
	regsub {.*/(.*)\.grp$} $el {\1} x
	if {$x != "Clipboard"} {
	    lappend res $x
	    set group($x.index) $ind
	    set group($x.modified) 0
	    incr ind
	}
    }
    return $res
}
proc group_restore { name } {
    global group groupDir
    set fn $groupDir/$name.grp
    if [file exists $fn] {
	change_cursor watch
	update idletasks
	set f [open $fn]
	set list [read $f]
	close $f
	dis_command "set group($name) \[parse_selection {$list}\]; return 0"
	set group($name.loaded) 1
	change_cursor ""
    }
}

proc group_get { isclipboard } {
    global group active_grp
    if ![info exists group($active_grp.loaded)] {
	group_restore $active_grp
    }
    if $isclipboard {
	clipboard_command
    } else {
	query_command "set_copy \$group($active_grp)"
    }
}

proc group_save { name } {
    global groupDir group
    if {$group($name.modified)==1} {
	set file $groupDir/$name.grp
	copy_result_to_file "print_attribute etag \$group($name)" $file
	set group($name.modified) 0
	grouplist_cb $group($name.index) 0
    }
}

proc grp_new {} {
    global group grp list stat active_grp opt_menu choice dis_sel
    while {1} {
	set name [Dialog_Action new_group Name]
	if {$name=={}} {return} 
	if {[info exists group($name.index)]} {
	    tk_messageBox -title Message -message "Group $name already exists" -type ok -icon error
	} else {
	    break
	}
    }
    set group($name.index) [$grp.list size]
    set group($name.modified) 1
    set group($name.loaded) 1
    if {$dis_sel==2 && [$list curselection]==""} {
	dis_command "set group($name) \[ new_set \]"
    } else {
	set arglist [dis_prepare_selection]
	dis_command "group_op set_assign $name $arglist"
    }

    set active_grp $name
    grouplist_cb $group($name.index) 1
    $opt_menu add radiobutton  -label $name -variable choice -command "dis_select $name"
    dis_select $name
    set choice $name
}

proc grp_save {} {
    global active_grp
    if [info exists active_grp] {
	group_save $active_grp
    }
}

proc grp_saveas {} {
    global grp group stat groupDir active_grp opt_menu choice
    set res [string trim [Dialog_Action save_as Name]]
    if {$res!={}} { 
	if {[info exists group($res.index)]} {
	    set answer [tk_messageBox -title Message -message "Group $res already exists.\nReplace existing file?"  -type okcancel -icon question]
	    if {$answer=="ok"} {
		combobox_add  .save_as.entry
		set arglist [dis_prepare_selection]
		copy_result_to_file "print_attribute etag $arglist" $groupDir/$res.grp
		dis_command "set group($res) \[ set_copy $arglist \] \; return 0"
	    } else {
		return
	    }
	} else {
	    combobox_add  .save_as.entry
	    set arglist [dis_prepare_selection]
	    copy_result_to_file "print_attribute etag $arglist"  $groupDir/$res.grp
	    set group($res.index) [$grp.list size]
	    $opt_menu add radiobutton  -label $res -variable choice -command "dis_select $res"
	}
	set active_grp $res
	grouplist_cb $group($res.index) 0
	set group($res.modified) 0
	dis_select $res
	set choice $res
    }
}

proc grp_saveall {} {
    global group grp active_grp
    set grp_size [$grp.list size]
    for {set i 0} {$i < $grp_size} {incr i} {
	set name [string range [$grp.list get $i] 1 end]
	if {$group($name.modified)==1} {
	    set active_grp $name
	    group_save $name
	}
    }
    $grp.list selection clear 0 end
}

proc init_group { grouplist } {
    global opt_menu choice groupDir group
    set grplist [group_list]
    foreach el $grplist {
	$grouplist insert end " $el"
	$opt_menu add radiobutton  -label $el -variable choice -command "dis_select $el"
    }
}    

proc group_op { op } {
    global active_grp list group dis_sel
    if ![info exists group($active_grp.loaded)] {
	group_restore $active_grp
	set group($active_grp.modified) 0
    }
    set old_size [dis_command "size \$group($active_grp)"]
    if {$dis_sel==1} {do_clear}
    set arglist [dis_prepare_selection]
    query_command [list group_op $op $active_grp $arglist]
    set new_size [$list size]

    if {$group($active_grp.modified)==0 && $old_size != $new_size} {
	grouplist_cb $group($active_grp.index) 1
	set group($active_grp.modified) 1
    }
}

proc grouplist_cb {index op} {
    global grp active_grp
    if {$index < [$grp.list size]} {
	$grp.list delete $index
    }
    if {$op==0} {
	$grp.list insert $index " $active_grp"
    } else {
	$grp.list insert $index "*$active_grp"
    }
    $grp.list selection clear 0 end
    $grp.list selection set $index
}
   
proc group_cleanup {} {
    global group grp env
    set modified 0
    set grp_size [$grp.list size]
    for {set i 1} {$i < $grp_size} {incr i} {
	set name [string range [$grp.list get $i] 1 end]
	if {$group($name.modified)==1} { 
	    set modified 1
	    break
	}
    }
    if $modified {
	if {[info exists env(DIS_VIM_DEVXPRESS)] && $env(DIS_VIM_DEVXPRESS)==1} {
	    set ans [tk_messageBox -title Message -message "There are unsaved groups.\nSave them before exit?"  -type yesno -icon question]
	} else {
	    set ans [tk_messageBox -title Message -message "There are unsaved groups.\nSave them before exit?"  -type yesnocancel -icon question]
	}
	if {$ans=="cancel"} {return 0}
	if {$ans=="yes"} {grp_saveall}
    }
    return 1
}

proc select_activegroup {w} {
    global active_grp stat
     set sels [$w curselection]
    if {$sels!=""} {
	set cursel [lindex $sels 0]
	set active_grp [string range [$w get $cursel] 1 end]
	$stat config -text "Group: $active_grp"
    }
}

proc group_manager { t1 } {
    global old_bind listfont grp grpframe active_grp
    set grpframe $t1.grpframe
    frame $grpframe
    set grp $grpframe.left
    frame $grp
    scrollbar $grpframe.sb -orient vertical -command "$grp.list yview"    
    pack $grpframe.sb -in $grpframe -side right -fill y
    pack $grp -in $grpframe -side left -fill both -expand 1

    listbox $grp.list -font $listfont -exportselection no -height 400 -yscrollcommand "$grpframe.sb set"
    frame $grp.b -height 1
    pack  $grp.b -side bottom -fill x -expand 1
    pack $grp.list -side top -fill both -expand 1

    button $grp.b.add -text "+" -relief groove -command "group_op set_union"
    button $grp.b.extract -text "-" -relief groove -command "group_op set_subtract"
    button $grp.b.new -text "*" -relief groove -command "group_op set_intersect"
    button $grp.b.assign -text "=" -relief groove -command "group_op set_assign"
    pack $grp.b.add $grp.b.extract $grp.b.new $grp.b.assign -side left -fill x -expand 1

    bind $grp.list <Double-Button-1> { 
	global list stat grp active_grp
	set sels [%W curselection]
	if {$sels!=""} {
	    set cursel [lindex $sels 0] 
	    set active_grp [string range [%W get $cursel] 1 end]
	    group_get 0
	}
	focus %W
    }

    bind $grp.list <Return> { 
	global grp active_grp
	set sels [%W curselection]
	if {$sels!=""} {
	    set cursel [lindex $sels 0] 
	    set active_grp [string range [%W get $cursel] 1 end]
	    group_get 0
	}
    }
    bind $grp.list <Button-1> "$old_bind; select_activegroup %W; focus %W"
    init_group $grp.list
    set active_grp Clipboard
    return $grpframe
}

proc clipboard_op { op } {
    global clipboardlist dis_sel active_grp group
    
    if {$dis_sel==1} {do_clear}
    set old_size [$clipboardlist size]
    set arglist [dis_prepare_selection]
    $clipboardlist delete 0 end
    eval $clipboardlist insert end [dis_query "group_op $op $active_grp $arglist"]
    set new_size [$clipboardlist size]
    status_line $new_size

    if {$group($active_grp.modified)==0 && $old_size != $new_size} {
	grouplist_cb $group($active_grp.index) 1
	set group($active_grp.modified) 1
    }
}

proc clipboard_command { } {
    global clipboardlist stat dis_sel active_grp
    $clipboardlist delete 0 end
    eval $clipboardlist insert end [dis_print \$group($active_grp) ]
    $stat config -text [$clipboardlist size]
}

proc clipboard_manager { t1 } {
    global listfont clipboardframe clipboardlist dis_sel
    global aseticons20 old_bind
    set clipboardframe $t1.clipboard
    frame $clipboardframe
    
    set clipboard $clipboardframe.left
    frame $clipboard
    scrollbar $clipboardframe.sb -orient vertical -command "$clipboard.list yview"    
    pack $clipboardframe.sb -in $clipboardframe -side right -fill y
    pack $clipboard -in $clipboardframe -side left -fill both -expand 1

    set clipboardlist $clipboard.list
    if $aseticons20 {
	listbox $clipboardlist -font aseticons-20 -height 400 -selectmode extended  -yscrollcommand "$clipboardframe.sb set"
    } else {
	listbox $clipboardlist -font $listfont -height 400 -selectmode extended -yscrollcommand "$clipboardframe.sb set"
    }
    frame $clipboard.b -height 1
    pack  $clipboard.b -side bottom -fill x -expand 1
    pack $clipboard.list -side top -fill both -expand 1

    button $clipboard.b.add -text "+" -relief groove -command "clipboard_op set_union"
    button $clipboard.b.extract -text "-" -relief groove -command "clipboard_op set_subtract"
    button $clipboard.b.new -text "*" -relief groove -command "clipboard_op set_intersect"
    button $clipboard.b.assign -text "=" -relief groove -command "clipboard_op set_assign"
    pack $clipboard.b.add $clipboard.b.extract $clipboard.b.new $clipboard.b.assign -side left -fill x -expand 1
    
    bind $clipboardlist <Double-Button-1> "set dis_sel 3; bw_open_def; focus %W"
    bind $clipboardlist <Return> "set dis_sel 3; bw_open_def; focus %W"
    bind $clipboardlist <Control-Button-1> "Tree:unselect Tree; set dis_sel 3"
    bind $clipboardlist <Button-1> "Tree:unselect Tree; set dis_sel 3; $old_bind; bw_describe; focus %W"

    return $clipboardframe

}

proc do_clipboard { label } {
    global active_grp clipboardframe
    set active_grp $label
    group_get 1
    raise $clipboardframe
}

proc dis_select { label } {
    global treeframe grpframe dis_sel
    switch -exact $label {
	"Projects" {raise $treeframe}
	"Groups" {raise $grpframe}
	default { do_clipboard $label }
    }
    if {$dis_sel!=2} { do_clear }
}

# copy from tk_optionMenu in optMenu.tcl, slightly changed
proc optionMenu {w relief varName firstValue args} {
    upvar #0 $varName var
    if ![info exists var] {
	set var $firstValue
    }
    if [isunix] {
	menubutton $w -textvariable $varName -indicatoron 1 -menu $w.menu \
	    -relief $relief -bd 2 -highlightthickness 2 -anchor c -width 7
    } else {
	menubutton $w -textvariable $varName -indicatoron 1 -menu $w.menu \
		-relief $relief -anchor c -width 7
    }
    # -direction flush
    menu $w.menu -tearoff 0
    $w.menu add radiobutton -label $firstValue -variable $varName -command "dis_select $firstValue"
    foreach i $args {
    	$w.menu add radiobutton  -label $i -variable $varName -command "dis_select $i"
    }
    return $w.menu
}

