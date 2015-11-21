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
proc Dialog_Create {top title args} {
	global dialog
	if [winfo exists $top] {
		switch -- [wm state $top] {
			normal {
				# Raise a buried window
				raise $top
			}
			withdrawn -
			iconified {
				# Open and restore geometry
				wm deiconify $top
				catch {wm geometry $top $dialog(geo,$top)}
			}
		}
		return 0
	} else {
		eval {toplevel $top} $args
		wm title $top $title
	        wm resizable $top 0 0
		return 1
	}
}
proc Dialog_Wait {top varName {focus {}}} {
	upvar $varName var

	# Poke the variable if the user nukes the window
	bind $top <Destroy> [list set $varName $var]

	# Grab focus for the dialog
	if {[string length $focus] == 0} {
		set focus $top
	}
	set old [focus -displayof $top]
	focus $focus
#	catch {tkwait visibility $top}
	catch {grab $top}
	catch {tkwait visibility $top}
	# Wait for the dialog to complete
	tkwait variable $varName
	catch {grab release $top}
	focus $old
}
proc Dialog_Dismiss {top} {
	global dialog
	# Save current size and position
	catch {
		# window may have been deleted
		set dialog(geo,$top) [wm geometry $top]
		wm withdraw $top
	}
}

proc Dialog_Search {} {
    global search
    set f .search
    if [Dialog_Create $f "Search" -borderwidth 10] {
	set l [frame $f.left]
	set r [frame $f.right ]
	pack $l -side left 
	pack $r -side right -padx 3 -anchor e
	set t [frame $l.top]
	set m [frame $l.middle -bd 1]
	set b [frame $l.bottom -width 10 -bd 1] 
	pack $t $m $b -side top -pady 2 -anchor nw
	
	label $t.lbl -text "Search for:    " 
	combobox $t.entry -width 28 -textvariable search(pat) -exportselection no
	pack $t.lbl $t.entry -side left -pady 10

	radiobutton $m.all -text "All " -relief flat -anchor w -variable search(type) -value 0
	radiobutton $m.function -relief flat -text "Function " -anchor w -variable search(type) -value 1
	radiobutton $m.class -relief flat -text "Class " -anchor w -variable search(type) -value 2
	radiobutton $m.string -relief flat -text "String " -anchor w -variable search(type) -value 3
	radiobutton $m.others -relief flat -text "Others" -anchor w -variable search(type) -value 4
	pack $m.all $m.function $m.class $m.string $m.others -side left -anchor w -pady 5

	checkbutton $b.exactmatch -text "Exact match " -variable search(exactmatch)
	checkbutton $b.matchcase -text "Match case  " -variable search(matchcase)
	checkbutton $b.reg -text "Reg. Expression " -variable search(reg)
	pack $b.exactmatch $b.matchcase $b.reg -side left -anchor s -pady 5

	button $r.ok -text "OK" -width 6 -command {set search(ok) 1; combobox_add .search.left.top.entry }
	button $r.cancel -text "Cancel" -width 6 -command {set search(ok) 0}
	pack  $r.ok $r.cancel -side top -padx 5 -pady 5
	bind $t.entry.e <Return> {set search(ok) 1; combobox_add .search.left.top.entry}
	set search(type) 0
    }
    set search(ok) 0
    .search.left.top.entry.e selection range 0 end
    Dialog_Wait $f search(ok) .search.left.top.entry.e
    Dialog_Dismiss $f
    if {$search(ok)} {
	set flags ""
	if {$search(pat)==""} { return {} }
	if {$search(matchcase)==0} {set flags [concat $flags "-i "]}
	if {$search(exactmatch)==0} {
	    set pat $search(pat)
	} else {
	    if {$search(reg)==1} {
		set pat "^$search(pat)$"
	    } else {
		set flags [concat $flags "-x "]
		set pat $search(pat)
	    }
	}
	set res [list "$flags" "$pat" $search(reg) $search(type)]
	return $res
    } else {
	return {}
    }
}

proc Dialog_grep {} {
    global grep
    set f .grep
    if [Dialog_Create $f "Grep" -borderwidth 10] {
	set l [frame $f.left]
	set r [frame $f.right ]
	pack $l -side left 
	pack $r -side right -padx 3 -anchor e
	set t [frame $l.top]
	#set m [frame $l.middle -bd 1]
	set b [frame $l.bottom -width 10 -bd 1] 
	pack $t $b -side top -pady 2 -anchor nw
	
	label $t.lbl -text "Grep selection for pattern:   " 
	combobox $t.entry -width 28 -textvariable grep(pat) -exportselection no
	pack $t.lbl $t.entry -side left -pady 10

	checkbutton $b.exactmatch -text "Exact match " -variable grep(exactmatch)
	checkbutton $b.matchcase -text "Match case  " -variable grep(matchcase)
	checkbutton $b.reg -text "Reg. Expression " -variable grep(reg)
	pack $b.exactmatch $b.matchcase $b.reg -side left -anchor s -pady 5

	button $r.ok -text "OK" -width 6 -command {set grep(ok) 1; combobox_add .grep.left.top.entry }
	button $r.cancel -text "Cancel" -width 6 -command {set grep(ok) 0}
	pack  $r.ok $r.cancel -side top -padx 5 -pady 5
	bind $t.entry.e <Return> {set grep(ok) 1; combobox_add .grep.left.top.entry}
	set grep(type) 0
    }
    set grep(ok) 0
    .grep.left.top.entry.e selection range 0 end
    Dialog_Wait $f grep(ok) .grep.left.top.entry.e
    Dialog_Dismiss $f
    if {$grep(ok)} {
	set flags ""
	if {$grep(pat)==""} { return {} }
	if {$grep(matchcase)==0} {set flags [concat $flags "-i "]}
	if {$grep(exactmatch)==1} { set flags [concat $flags "-w "]}
	set res [list "$flags" $grep(pat) $grep(reg)]
	return $res
    } else {
	return {}
    }
}

proc Dialog_Action {cmd str} {
    global action
    set f .$cmd
     
    if {[Dialog_Create $f $cmd -borderwidth 10]} {
	message $f.msg -text "Enter $str" -aspect 1000
	combobox $f.entry -textvariable action($cmd:result) -exportselection no
	set b [frame $f.buttons]
	pack $f.msg -side top 
	pack $f.entry $f.buttons -side top -fill x
	pack $f.entry -pady 5
	button $b.ok   -text "  Ok  "  -command "set action($cmd:ok) 1"
	button $b.cancel   -text Cancel  -command "set action($cmd:ok) 0"
	pack $b.ok -side left -pady 5
	pack $b.cancel -side right -pady 5
	bind $f.entry.e <Return> "set action($cmd:ok) 1"
    }

    set action($cmd:ok) 0
    $f.entry.e selection range 0 end
    Dialog_Wait $f action($cmd:ok) $f.entry.e
    Dialog_Dismiss $f
    if {$action($cmd:ok)} {
	return $action($cmd:result)
    } else {
	return {}
    }
}

proc Dialog_Find {sgn} {

global find_log oldfocus start_poz

	set start_poz 1.0
	set f .find
	set name "Find"
	if {$sgn} {
		set f .find_log
		set name "Find in Log"
    	}
	if [Dialog_Create $f $name -borderwidth 10] {
		set l [frame $f.left]
		set r [frame $f.right -width 25]
		pack $l  -side left 
		pack $r -side left -padx 10 -anchor e
		set t [frame $l.top]
		set b [frame $l.bottom]
		pack $t $b -side top -anchor w
		label $t.lbl   -text "Find what:   " 
		combobox $t.entry  -width 25 -textvariable find(pat) -exportselection no
		pack $t.lbl $t.entry -side left -pady 5

		frame $b.left 
		frame $b.right
		pack $b.left  -side left  
		pack $b.right -side left 

		checkbutton $b.left.exactmatch  -text "Exact match" -width 16 -anchor w -variable find(exactmatch)
		checkbutton $b.left.matchcase   -text "Match case" -width 16 -anchor w -variable find(matchcase)
		checkbutton $b.left.reg   -text "Reg. Expression" -width 16 -anchor w -variable find(reg)
		pack $b.left.exactmatch $b.left.matchcase $b.left.reg -side top -pady 2 
	
		radiobutton $b.right.down   -text "down" -relief flat -anchor w -variable find(direction) -value 0
		radiobutton $b.right.up   -relief flat -text "up  " -anchor w -variable find(direction) -value 1
		pack $b.right.down $b.right.up -side top  -anchor w -padx 35
		$b.right.down select

		if {$sgn} {
			button $r.next   -text "Find next (<F3>)"   -command { combobox_add  .find_log.left.top.entry; do_find_log $find(pat) $find(exactmatch) $find(matchcase) $find(reg) $find(direction)}
			button $r.findall   -text " Find  "  -command { combobox_add  .find_log.left.top.entry; do_find_first $find(pat) $find(exactmatch) $find(matchcase) $find(reg)}  
			button $r.cancel   -text "  Cancel  "  -command find_log_cancel
			pack  $r.next $r.findall $r.cancel -side top -padx 5 -pady 5
			bind $t.entry.e <F3> { combobox_add .find_log.left.top.entry; do_find_log $find(pat)  $find(exactmatch) $find(matchcase) $find(reg) $find(direction)}
			bind $t.entry.e <Return> { combobox_add .find_log.left.top.entry; do_find_log $find(pat)  $find(exactmatch) $find(matchcase) $find(reg) $find(direction)}
		} else {
			button $r.next   -text "Find next"   -command { combobox_add  .find.left.top.entry; do_find $find(pat) $find(exactmatch) $find(matchcase) $find(reg) $find(direction)}
			button $r.findall   -text " Find all  "  -command { combobox_add  .find.left.top.entry; do_findall $find(pat) $find(exactmatch) $find(matchcase) $find(reg)}  
			button $r.cancel   -text "  Cancel  "  -command find_cancel
			pack  $r.next $r.findall $r.cancel -side top -padx 5 -pady 5
			bind $t.entry.e <Return> { combobox_add .find.left.top.entry; do_find $find(pat)  $find(exactmatch) $find(matchcase) $find(reg) $find(direction)}
		}
	}

	$f.left.top.entry.e selection range 0 end

	set oldfocus [focus -displayof $f]

	catch {grab $f}
	catch {tkwait visibility $f}
	catch {grab release $f}
	raise $f
	focus $f.left.top.entry.e
}

proc find_cancel {} {
    global oldfocus 
    catch {grab release .find}
    focus $oldfocus
    Dialog_Dismiss .find
}

proc find_log_cancel {} {
    global oldfocus 
    catch {grab release .find_log}
    focus $oldfocus
    Dialog_Dismiss .find_log
}





