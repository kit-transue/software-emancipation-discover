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
# callbacks for menu items and toolbar buttons

proc list_activate cur_ind {
    global result list
    $list delete 0 end
    foreach el $result($cur_ind) {
	$list insert end $el
    }
    set sel $result(sel_$cur_ind)

    if [llength $sel] {
	if [info exists result(kind_$cur_ind)] {
	    set_kind_menu $result(kind_$cur_ind)
	}
	set s [lindex $sel 0]
	$list see $s
	$list activate $s
	foreach el $sel {$list selection set $el}
    }
    status_line  [llength $result($cur_ind)] 
}

proc do_back { } {
    global cur_ind max_ind
  
    if {$cur_ind!=0} {
	if {$cur_ind > 1} {
	    incr cur_ind -1
	} elseif { $max_ind > 0 } {
	    set cur_ind $max_ind
	}	
	list_activate $cur_ind
    }
}

proc do_forward { } {
    global cur_ind max_ind

    if {$cur_ind!=0} {
	if {$cur_ind < $max_ind} {
	    incr cur_ind
	} else {
	    set cur_ind 1
	}
	list_activate $cur_ind
    }
}

proc do_empty {} {
    query_command new_set 
}

proc do_print {} {
    global dis_sel Service result cur_ind env aseticons20 Model_name
    if {$dis_sel!=2} { do_clear }
    if [isunix] {
	set file [tk_getSaveFile -initialdir $env(HOME) -title "print to file..." -defaultextension .txt -filetypes {{{print files} {.txt}}}]
    } else {
	set file [Dialog_Action print Filename]
	set dirname [file dirname $file]
	if {$dirname == "."} {
	   set file $env(HOME)/dislite/$Model_name/$file.txt
	} else {
	    if ![file exists $dirname] {
		set msg "Directory $dirname does not exist"
		tk_messageBox -title Message -message $msg -type ok -icon error
		return
	    }
	}
	combobox_add .print.entry
    }
    if {$file!=""} {
	set f [open $file w]
	foreach el $result($cur_ind) {
	    if $aseticons20 {
		puts $f [string range $el 2 end]
	    } else {
		puts $f $el
	    }
	}
	close $f
    }
}

proc do_prefs {} {
    global env Model_name
    uplevel #0 source \"$env(DIS_INSTALL)/ServiceChooser.tcl\"
    tkwait window .w
    init_setting $env(HOME)/dislite/$Model_name/$Model_name.prefs
}

set shouldExit 0
proc do_exit {{restart 0}} {
    global Tempdirectory Service env dis_in error_file grp
    global shouldExit
    if [ info exists grp ] {
	if [group_cleanup] {
	    group_save Clipboard
	} else {
	    return
	}
    }
    file_delete $Tempdirectory
    if ![ catch { set f [open $env(HOME)/dislite/dislite.prefs w] } ] {
        puts $f $Service
        close $f
    }
    file_delete $error_file
    if $restart {
	   devxMessage RESTART_DEVX "$Service"
    }
    # send a message to close the editor driver
	# if the editor driver is running - we'll exit when get an acknowledgement from it
	# that it's closed (ReadPipe function)
	set shouldExit 1
	if {[editorMessage CLOSE_DRIVER]==0} {
		# otherwise just exit
		exit
	}
}

proc startup_error {} {
    global Tempdirectory env error_file Service
    file_delete $Tempdirectory
	set Service ""
    set f [open $env(HOME)/dislite/dislite.prefs w]
	puts $f $Service
    close $f
    devxMessage RESTART_DEVX "$Service"
    exit
}


proc do_emacsexit {} {
    global Tempdirectory env Service dis_in error_file
    set res 0
    if [group_cleanup] {
	    group_save Clipboard
	    file_delete $Tempdirectory
	set f [open $env(HOME)/dislite/dislite.prefs w]
	puts $f $Service
	close $f
	close $dis_in
	file_delete $error_file
	set res 1
    }
    #set res "Emacs_Exit $res"
    editorMessage CLOSE_DRIVER
}

proc do_search { } {
    set res [Dialog_Search]
    change_cursor watch
    update
    if {$res!={}} {
	pattern_grep $res
    }
    change_cursor ""
}  

proc do_find {pat exactmatch matchcase reg direction} {
    global result dis_sel aseticons20 cur_ind list
    
    if {$dis_sel!=2} {do_clear}
    if {$pat!={}} {
	set sz [llength $result($cur_ind)]
	set cur_sel [lindex [$list curselection] 0]
	if {$reg && $exactmatch==1} {set pat "^$pat$"}

	set error 0
	if {$direction==0 && $cur_sel==[expr $sz-1]} {
	    set error 1
	}
	if {$direction==1 && $cur_sel==0} {
	    set error 1
	}
	if {$error==0} {
	  if {$direction==0} {
	    if {$cur_sel==""} { set cur_sel -1}
	    set sublist [lrange $result($cur_ind) [incr cur_sel] end]
	    if {[llength $sublist]>0} {
		set i $cur_sel
		foreach str $sublist {
		    if  $aseticons20 {
			set str [string range $str 2 end]
		    }
		    if {$reg==1} {
			if {$matchcase==1} { 
			    if {[regexp $pat $str]==1} {break}
			} else {
			    if {[regexp -nocase $pat $str]==1} {break}
			}
		    } else {
			if {$matchcase==0} {
			     set str1 [string toupper $str]
			     set pat1 [string toupper $pat]
			 } else {
			     set str1 $str
			     set pat1 $pat
			 }
			 if {$exactmatch==1} {
			     if {[string compare $pat1 $str1]==0} {break}
			 } else {
			     if {[string first $pat1 $str1]!=-1} {break}
			 }
		    }
		    incr i
		}
		if {$i<$sz} {
		    $list selection clear 0 end
		    $list selection set $i $i
		    $list see $i
		} else {
		   set error 1
		}
	    } else {
		set error 1
	    }
	} else {
	    if {$cur_sel==""} { set cur_sel $sz}
	    set sublist [lrange $result($cur_ind) 0 [incr cur_sel -1]]
	    if {[llength $sublist]>0} {
		set last -1
		set i 0
		foreach str $sublist {
		    if $aseticons20 {
			set str [string range $str 2 end]
		    }
		    if {$reg==1} {
			if {$matchcase==1} { 
			    if {[regexp $pat $str]==1} {set last $i}
			} else {
			    if {[regexp -nocase $pat $str]==1} {set last $i}
			}
		    } else {
			if {$matchcase==0} {
			     set str1 [string toupper $str]
			     set pat1 [string toupper $pat]
			 } else {
			     set str1 $str
			     set pat1 $pat
			 }
			if {$exactmatch==1} {
			    if {[string compare $pat1 $str1]==0} {set last $i}
			 } else {
			    if {[string first $pat1 $str1]!=-1} {set last $i}
			 }
		    }
		    incr i
		}
		if {$last>=0} {
		    $list selection clear 0 end
		    $list selection set $last $last
		    $list see $last
		} else {
		    set error 1
		}
	    } else {
		set error 1
	    }
	}
    }
    if {$error==1} {
	if {$reg==0} {
	    set msg "Cannot find string \"$pat\""	
	} else {
	    set msg "Cannot find regular expression \"$pat\""
	}
	tk_messageBox -title Message -message $msg -type ok -icon error
    }
    .find.left.top.entry.e selection range 0 end
    }
}

proc do_find_log {pat exactmatch matchcase reg direction} {

global sch_text start_poz

    	set page [activeLabel .sch.note]
	if {$page !=5} {
		tk_messageBox -title Warning -message "Please, go to the Log pane." -type ok -icon warning
	}	
	$sch_text tag delete x
	set case "-"
	if {!$matchcase} {set case "nocase"}
	if {!$reg && !$direction} {
		set poz [$sch_text search -$case $pat $start_poz]
	} elseif {!$reg && $direction} {
		set poz [$sch_text search -backwards -$case $pat $start_poz]
	} elseif {$reg && !$direction} {
		set poz [$sch_text search -regexp -count cnt -$case $pat $start_poz]
	} else {
		set poz [$sch_text search -regexp -count cnt -backwards -$case $pat $start_poz]
	}
	if {$poz != ""} {
		if ($reg) {
			$sch_text tag add x $poz "$poz + $cnt chars"
		} else {
			$sch_text tag add x $poz "$poz+[string length $pat] chars"
		}
		$sch_text tag configure x -background bisque
		$sch_text see $poz
		if {$direction} {
			set start_poz "$poz-1 chars"
		} else {
			if ($reg) {
				set start_poz "$poz+$cnt chars"
			} else {
				set start_poz "$poz+1 chars"
			}
		}
	} else {
		if {!$reg} {
	    		set msg "Cannot find string \"$pat\""	
		} else {
	    		set msg "Cannot find regular expression \"$pat\""
		}
		tk_messageBox -title Message -message $msg -type ok -icon error
	}
}

proc do_find_first {pat exactmatch matchcase reg} {

global sch_text start_poz

    	set page [activeLabel .sch.note]
	if {$page !=5} {
		tk_messageBox -title Warning -message "Please, go to the Log pane." -type ok -icon warning
	}
	$sch_text tag delete x
	set start_poz 1.0
	set case "-"
	if {!$matchcase} {set case "nocase"}
	if {!$reg} {
		set poz [$sch_text search -$case $pat $start_poz]
	} else {
		set poz [$sch_text search -regexp -count cnt -$case $pat $start_poz]
	} 
	if {$poz != ""} {
		if ($reg) {
			$sch_text tag add x $poz "$poz + $cnt chars"
			set start_poz "$poz+$cnt chars"
		} else {
			$sch_text tag add x $poz "$poz+[string length $pat] chars"
			set start_poz "$poz+1 chars"
		}
		$sch_text tag configure x -background bisque
		$sch_text see $poz
	} else {
		if {!$reg} {
	    		set msg "Cannot find string \"$pat\""	
		} else {
	    		set msg "Cannot find regular expression \"$pat\""
		}
		tk_messageBox -title Message -message $msg -type ok -icon error
	}
}

proc do_findall {pat exactmatch matchcase reg } {
    global result aseticons20
    global cur_ind
    global list stat
    
    if {$pat!={}} {
	set sz [llength $result($cur_ind)]
	if {$reg==1 && $exactmatch==1} {set pat "^$pat$"}

	set findall_res {}
	set ii 0
	foreach str $result($cur_ind) {
	    if $aseticons20 {
		set str [string range $str 2 end]
	    }
	    if {$reg==1} {
		if {$matchcase==1} { 
		   if {[regexp $pat $str]==1} {
			lappend findall_res $ii
		   }
		} else {
		   if {[regexp -nocase $pat $str]==1} {
		       lappend findall_res $ii
		    }
		}
	    } else {
		if {$matchcase==0} { 
		    set str1 [string toupper $str]
		    set pat1 [string toupper $pat]
		} else {
		    set str1 $str
		    set pat1 $pat
		}
		if {$exactmatch==1} {
		    if {[string compare $pat1 $str1]==0} {
			lappend findall_res $ii
		    }
		} else {
			if {[string first $pat1 $str1]!=-1} {
			    lappend findall_res $ii
			}
		}
	    }
	    incr ii
	}
	$list selection clear 0 end
	if {$findall_res!={}} {
		foreach num $findall_res {	
			$list selection set $num $num
		}
	} else {
		if {!$reg} {
	    		set msg "Cannot find string \"$pat\""	
		} else {
	    		set msg "Cannot find regular expression \"$pat\""
		}
		tk_messageBox -title Message -message $msg -type ok -icon error
	}
    }
}

proc do_exec { } {
    set pat [string trim [Dialog_Action exec Command]]
    if {$pat!={}} {
	if {[do_exec_1 $pat]==0} {
	    if {[combobox_add .exec.entry]} {
		set new_menu .d1.mbar.access
		set num [expr [$new_menu index "Query..."]-1]
		$new_menu insert $num command -label $pat -command "do_exec_1 \"$pat\""
    	    }
	}
    }
}  
proc do_exec_1 {pat} {
    exec_unsafe_query $pat
}

proc do_filter { } {
    global cur_ind
    set pat [string trim [Dialog_Action filter Filter]]
    if {$pat!={}} { 
    	if {[do_filter_1 $pat]==0} {
	    if {[combobox_add .filter.entry]} {
		.d1.mbar.access add command -label $pat -command "do_filter_1 \"$pat\""
	    }
	}
    }
}
proc do_filter_1 {pat} {
    set query "filter {$pat} [dis_prepare_current_list]"
    exec_unsafe_query $query
}


proc do_query { } {
    set pat [string trim [Dialog_Action query Query]]
    if {$pat!={}} { 
    	if ![do_query_1 $pat] {
	    if [combobox_add .query.entry] {
		set new_menu .d1.mbar.access
		set num [expr [$new_menu index "Filter..."]-1]
		$new_menu insert $num command -label $pat -command "do_query_1 \"$pat\""
	    }
	}
    }
}

proc do_query_1 {pat} {
    set cur_selection [dis_prepare_selection]
    if {"$pat"=="where referenced"} {
	set cur_selection $cur_selection
    }
    set query "$pat $cur_selection"
    exec_unsafe_query $query
}


proc do_clear {} {
    global dis_sel list clipboardlist grp
    if {$dis_sel==1} {
	Tree:unselect Tree
    } else {
	$list selection clear 0 end
	$clipboardlist selection clear 0 end
	$grp.list selection clear 0 end
    }
    set dis_sel 2
}

# The following procedures used in search utility
set all_symbols_flag 1
proc symbol_file {} {
    global all_symbols all_symbols_flag Tempdirectory
    if $all_symbols_flag {
	    set all_symbols [tempfile $Tempdirectory symbols]
	    if {[copy_result_to_file "get_all_symbols" $all_symbols]!=0} {
	        set all_symbols_flag 0
		}
    }
    return $all_symbols
}

proc grep_one_file { fn flags pat reg} {
    global Psethome
    if {$reg==1} {
	regsub -all {\\} $pat {\\\\\\} pat
	if {[string first \" $pat]<0} {
	    set pat \"$pat\"
	} 
	set f [open "| {$Psethome/bin/generic/egrep} -n $flags $pat $fn" r]
    } else {
	regsub -all {\\} $pat {\\\\} pat
	if {[string first \" $pat]<0} {
	    set pat \"$pat\"
	}
	set f [open "| {$Psethome/bin/generic/egrep} -F -n $flags $pat $fn" r]
    }
    set ll ""
    while { [gets $f line] >= 0 } {
	lappend ll [lindex [split $line :] 0]
    }
    catch "close $f"
    return $ll
}

proc pattern_grep { varlist } {
    global list
    set symbols [symbol_file]
    set flags [lindex $varlist 0]
    set pat [lindex $varlist 1]
    set reg [lindex $varlist 2]
    set type [lindex $varlist 3]
    set lines [grep_one_file $symbols $flags $pat $reg]
    set sz [llength $lines]
    if $sz {
	switch -exact $type {
	    0 { query_command "get_symbol_cache $lines" }
	    1 { query_command "filter function \[ get_symbol_cache $lines \]" }
	    2 { query_command "filter class \[ get_symbol_cache $lines \]" }
	    3 { query_command "filter string \[ get_symbol_cache $lines \]" }
	    4 { query_command "filter {!(function||class||string)} \[ get_symbol_cache $lines \]" }
	    default { return }
	}
	status_line "Search: [$list size] matches"
    } else {
	status_line "Search: 0 matches"
    }
}

# The followings are misc. procedures used throughout the program

# This seems to be an emacs-specific function that replaces the title in an emacs window
# I'm taking it out ... I don't know what the wm_extern function does, but I do not think
# 	it is necessary.
set editor_not_init 1
proc write msg {
    global env 
    myputs $msg
    if [isunix] {
	if ![info exists env(DIS_VIM_DEVXPRESS)] {
	    global editor_not_init win_tag
	    if $editor_not_init {
		set editor_not_init 0
		if { [info commands wm_extern]=="" } {
		    proc wm_extern {} {}
		} else {
		    set win_tag [pid]
		    myputs "Dislite_Service $win_tag 1"
		}
	    } else {
		wm_extern
	    }
	}
    } 
}

proc file_delete {file } {

    # we will try to remove file multiple times - in case dish2 havn't finish with it
    set attempt 0
	while { $attempt<50  && [file exists $file]} {
        catch {file delete -force $file}
		incr attempt
    }

	if { $attempt>=50 } { return 0 }
	return 1
}

proc tempfile {dir prefix} {
    set base "$dir/$prefix"
    set ind 1
    while {[file exists $base.$ind]} {
	incr ind
    }
    return $base.$ind
}

proc change_drivename {fn} {
    if {[string index $fn 1] == ":"} {
	set drivename [string toupper [string index $fn 0]]
	return $drivename[string range $fn 1 end]
    } else {
	return $fn
    }
}

proc activate_browser {{w .d1}} {
    wm deiconify $w
    if ![isunix] {
	activate
    }
    raise $w
    focus -force $w
}
