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
# this file includes all procedures that builds menu and toolbar

# toolbar button
proc my_button { nm txt rel cmd } {
    if [isunix] {
	set pady 3
    } else {
	set pady 0
    }
    button  $nm -width 8 -padx 0 -pady $pady -bd 2 -relief $rel -anchor c -text $txt -command $cmd
}

proc build_toolbar {n relief} {
    global opt_menu choice
    set opt_menu [optionMenu $n.choice $relief choice Projects Groups ]
    pack $n.choice -side left -anchor w 
    my_button $n.back "Back" $relief do_back
    my_button $n.forward "Forward" $relief do_forward
    my_button $n.find "Find " $relief "Dialog_Find 0"
    my_button $n.search "Search " $relief do_search
    pack $n.back $n.forward $n.find $n.search -side left -anchor w
}

# menu item with accelerator
proc my_menu_acc { nm lbl acc und cmd } {
    if [catch {$nm index $lbl}] {
	if {$und<0} {
	    $nm add command -label $lbl -accelerator $acc -command $cmd
	} else {
	    $nm add command -label $lbl -accelerator $acc -underline $und -command $cmd
	}
	bind .d1 <$acc> $cmd
    }
}

# menu item without accelerator
proc my_menu { nm lbl und cmd {ind end}} {
    if [catch {$nm index $lbl}] {
	if {$und<0} {
	    $nm insert $ind command  -label $lbl -command $cmd
	} else {
	    $nm insert $ind command  -label $lbl -underline $und  -command $cmd
	}
    }
}

proc build_menu {m} {
    global env CM
    menu  $m -tearoff 0 -relief raised -bd 1

    set top [winfo parent $m]
    $top config -menu $m

    set mm $m.file
    $m add cascade  -label "File " -menu $mm -underline 0 
    menu $mm -tearoff 0

    set mode [dis_command set_mode]
    if {[lindex $mode 0] != "off" } {
	global CurrentScope scope_flag
	if $scope_flag {
	    set label "Scope Unset"
	} else {
	    set label "Scope Set"
	}
	my_menu $mm $label 2 "scope_toggle $mm 0"
	$mm add separator
    }
    my_menu $mm "New Group" 0 grp_new
    my_menu $mm "Save" 0 grp_save
    my_menu $mm "SaveAs ..." 4 grp_saveas
    my_menu $mm "SaveAll" 5 grp_saveall
    $mm add separator
    my_menu $mm "Source ..." 0 "do_source"
    $mm add separator
    my_menu $mm "Grep ..." 0 "do_grep"
    my_menu $mm "Instances" 0 "query_instances"
    if $CM {
	my_menu $mm "Sub. Check" 5 "do_submitcheck"
    }
    $mm add separator
    my_menu $mm "Print to File ..." 0 "do_print"
    my_menu $mm "Service ..." 3 "do_prefs"
    my_menu $mm Exit 1 "do_exit"

    if $CM {
	    set mm $m.manage
	    $m add cascade  -label "Manage " -menu $mm -underline 0 
	    menu $mm -tearoff 0
	    my_menu $mm "Get with comments" 0 "cm_get 1"
	    my_menu $mm "Get without comments" 0 "cm_get 0"
	    my_menu $mm "Unget " 0 "cm_unget"
	    my_menu $mm "Put " 0 "cm_put"
	    my_menu $mm "Local " 0 "cm_local"
	    if { $CM == 3 } {
            $mm add separator
	        my_menu $mm "CCM Select Default Task..." 0 "ccmSelectTask"
	        my_menu $mm "CCM Check In Default Task..." 0 "ccmTaskCheckIn"
	        my_menu $mm "CCM Create Task..." 0 "ccmTaskCreate"
	        my_menu $mm "CCM Reconfigure..." 0 "ccmTaskReconfigure"
	        my_menu $mm "CCM Reconcile..." 0 "ccmTaskReconcile"
        }
	}
    build_kind_menu $m
    set_kind_menu proj

    build_customized_menu [find_script menu.dat] $m
    set langs [dis_command languages]
    foreach lan [join $langs] {
	set ln [string tolower $lan ].dat
	set fn [find_script $ln]
	puts stdout "$ln $fn"
	if { $fn != "" } {
	    build_customized_menu $fn $m
	}
    }

    set fn [find_script user.dat]
    if { $fn != "" } {
	build_customized_menu $fn $m
    }
    set mm $m.access
    $m add cascade  -label "Access" -menu $mm -underline 0
    menu $mm  -tearoff 0
    my_menu_acc $mm "History" Control-h 0 "query_command  {set_reverse \$selections }"
    my_menu_acc $mm "Exec..."  Control-e 0 "do_exec"
    $mm add separator
    my_menu_acc $mm "Query..." Control-q 0 "do_query"
    $mm add separator
    my_menu_acc $mm "Filter..." Control-f 0 "do_filter"

    set mm $m.help
    $m add cascade  -label "Help" -menu $mm -underline 0
    menu $mm -tearoff 0
    my_menu $mm "About..." 0 {tk_messageBox -title "About ... " -message "Developer Xpress v1.0" -type ok}
 
}

proc exec_unsafe_query { query } {
    global list stat
    set res [dis_command "unsafe_query $query"]
    if [ string match _DI_* $res ] {
	query_command "get_cur_query_result"
	return 0
    } else {
	status_line [join $res]
	return 1
    }
}

proc find_script { file } {
    global env Service Model_name
    set fullname $env(HOME)/dislite/$Model_name/$file
    if [file exists $fullname] {
	return $fullname
    }
    set fullname $env(HOME)/dislite/$file
    if [file exists $fullname] {
	return $fullname
    }
    if [info exists env(DISCOVER_SCRIPTS)] {
	set fullname $env(DISCOVER_SCRIPTS)/$file
	if [file exists $fullname] {
	    return $fullname
	}
    }
    set fullname $env(DIS_INSTALL)/$file
    if [file exists $fullname] {
	return $fullname
    }
    return ""
}

proc rebuild_customized_menu file {
    global disMenuBar buildCache
    unset buildCache
    build_customized_menu $file $disMenuBar
}
proc build_customized_menu { file m } {
    global dis_in disMenuBar buildCache
    set disMenuBar $m

    if [info exists buildCache($file)] {return}
    set    buildCache($file) 1

    set f [open $file r]
    set mm $m.file
    set src_flag 0

    global kindMenuMap

    while {[gets $f line]>=0} {
	set tag [lindex $line 0]
	switch -exact -- $tag {
	    "INCLUDE" {
		set fn [lindex $line 1]
		if {[file pathtype $fn] != "absolute"} {
		    set fn [find_script $fn]
		}
		build_customized_menu $fn $m
	    }
	    "MENU" {
		set menu [lindex $line 1]
		set underline [lindex $line 2]
		set n [string tolower $menu]
		set mm $m.$n

		if [info exists kindMenuMap(_$n)] {
		    set mm $kindMenuMap(_$n)
		} elseif {![winfo exists $mm]} {
		    if [llength $underline] {
			$m add cascade  -label $menu -menu $mm -underline $underline 
		    } else {
			$m add cascade  -label $menu -menu $mm
		    }
		    menu $mm -tearoff 0
		}
		set curind [$mm index end]
		if {$curind=="none"} {
		    set curind 1
		} else {
		    incr curind
		}
	    }
	    "0" {
		set menuitem [lindex $line 1]
		if {$menuitem==""} {
		    set curind 0
		} elseif ![catch {set index [$mm index $menuitem]}] {
		    set curind [$mm index $index]
		    incr curind
		}
	    }
	    "-" {
		set menuitem [lindex $line 1]
		if ![catch {set index [$mm index $menuitem]}] {
		    set curind [$mm index $index]
		    $mm delete $curind
		}
	    }
	    "+" {
		set menuitem [lindex $line 1]
		set underline [lindex $line 2]
		set callback [lindex $line 3]
		set type [string tolower [lindex $line 4]]
		if {$type == "command"} {
		    set callback "exec_unsafe_query  \{$callback\}"
		} elseif {$type == "tcl"} {
		    # locally (client side) executed callback
		} else     {
		    set callback "do_query_1  \{$callback\}"
		}
		my_menu $mm $menuitem $underline $callback $curind
		incr curind
		
	    }
	    "SEP" {
		$mm add separator
		incr curind
	    }
	    "SOURCE" {
		set src_flag 1
		break
	    }
	    "TCL" {
		set src_flag 2
		break
	    }
	    default {}
	}
    }
    while {$src_flag} {
	set new_src_flag 0
	set src_code ""
	while {![eof $f]} {
	    set line [gets $f]
	    if {$line=="TCL"} {
		set new_src_flag 2
		break
	    } elseif {$line== "SOURCE"} {
		set new_src_flag 1
		break
	    } else {
		append src_code $line\n
	    }
	}
	if {$src_flag==1} {
	    puts $dis_in $src_code
	    flush $dis_in
		read_file $dis_in
	} else {
	    eval $src_code
	}
	set src_flag $new_src_flag
    }
    close $f
}

proc build_kind_menu m {
    global kindMenuMap

    $m add cascade  -label "Dynamic" 
    set   kindMenuMap(menuindex) [$m index last]
    set   kindMenuMap(menuwidget) $m
    set   kindMenuMap(current) {}

    foreach {name kinds} {
	Defines   { proj scope module default}
	Method    { method}
	Function  { funct cfun}
	Interface { interface }
	Class     { struct class}
    } {
	set n [string tolower $name]
	set mm $m.$n
	set val [list $name $mm]

	menu $mm -tearoff 0

	foreach kind $kinds {
	    set kindMenuMap($kind) $val
	    set kindMenuMap(_$n) $mm
	}
    }
}

proc set_kind_menu kind {
    global kindMenuMap
    if ![info exists kindMenuMap($kind)] {
	set kind default
    }
    set val $kindMenuMap($kind)
    set mm  [lindex $val 1]
    set name [lindex $val 0]

    if { $name == $kindMenuMap(current) } {return 0}

    set kindMenuMap(current) $name
    set m $kindMenuMap(menuwidget) 
    set i $kindMenuMap(menuindex)
    $m  entryconfigure $i -label $name -menu $mm
    return 1
}
