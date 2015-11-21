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
proc eb_button { nm txt rel cmd {width 7}} {
    if [isunix] {
	set pady 2
    } else {
	set pady 0
    }
    button  $nm -width $width -padx 0 -pady $pady -bd 1 -relief $rel -anchor c -text $txt -command $cmd
}

proc eb_print {error_browser} {
    upvar #0 $error_browser eb
    global env Model_name Transform
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
	foreach el $eb(result$eb(cur_ind)) {
	    regexp {([^\*]+)\*([^\*]+)\*([^\*]+)\*(.*)} $el dummy local_file rootname linenum err_msg
	    if {$Transform==2} {
		puts $f $rootname:$linenum:$err_msg
	    } else {
		puts $f $local_file:$linenum:$err_msg
	    }
	}
	close $f
    }
}

proc eb_get_next_ind {error_browser} {
    upvar #0 $error_browser eb
    if {$eb(cur_ind) && ![llength $eb(result$eb(cur_ind))]} {
	return $eb(cur_ind)
    }
    if {$eb(cur_ind) == 10 } {
	set eb(cur_ind) 0
    }
    incr eb(cur_ind)
    if {$eb(cur_ind) > $eb(max_ind)} {
	set eb(max_ind) $eb(cur_ind)
    }
    return $eb(cur_ind)
}

proc eb_back {error_browser} {
    upvar #0 $error_browser eb
    if {$eb(cur_ind)!=0} {
	set eb(cur_sel$eb(cur_ind)) [lindex [$eb(list) curselection] 0]
	if {$eb(cur_ind) > 1} {
	    incr eb(cur_ind) -1
	} elseif { $eb(max_ind) > 0 } {
	    set eb(cur_ind) $eb(max_ind)
	}
	$eb(list) delete 0 end
	foreach el $eb(result$eb(cur_ind)) {
	    regexp {([^\*]+)\*([^\*]+)\*([^\*]+)\*(.*)} $el dummy local_file rootname linenum err_msg
	    $eb(list) insert end $rootname:$linenum:$err_msg
	}
	if {$eb(cur_sel$eb(cur_ind))!=""} {
	    $eb(list) selection set $eb(cur_sel$eb(cur_ind))
	    $eb(list) see $eb(cur_sel$eb(cur_ind))
	}
	wm title .$eb(name) "Discover $eb(title):  $eb(title$eb(cur_ind))"
	$eb(label) config -text "$eb(title): [llength $eb(result$eb(cur_ind))] found"
    }
}

proc eb_forward {error_browser} {
    upvar #0 $error_browser eb
    if {$eb(cur_ind)!=0} {
	set eb(cur_sel$eb(cur_ind)) [lindex [$eb(list) curselection] 0]
	if {$eb(cur_ind) < $eb(max_ind)} {
	    incr eb(cur_ind)
	} else {
	    set eb(cur_ind) 1
	}
	$eb(list) delete 0 end
	foreach el $eb(result$eb(cur_ind)) {
	    regexp {([^\*]+)\*([^\*]+)\*([^\*]+)\*(.*)} $el dummy local_file rootname linenum err_msg
	    $eb(list) insert end $rootname:$linenum:$err_msg
	}
	if {$eb(cur_sel$eb(cur_ind))!=""} {
	    $eb(list) selection set $eb(cur_sel$eb(cur_ind))
	    $eb(list) see $eb(cur_sel$eb(cur_ind))
	}
	wm title .$eb(name) "Discover $eb(title):  $eb(title$eb(cur_ind))"
	$eb(label) config -text "$eb(title): [llength $eb(result$eb(cur_ind))] found"
    }
}

proc eb_stop {error_browser} {
    upvar #0 $error_browser eb
    set eb(cancel) 1
    set eb(finish) 1
    set eb(running) 0
    catch {close $eb(pipe)}
    eb_filemenu_cb $eb(name) normal
    button_cb $eb(name) 1
    $eb(label) config -text "$eb(title): [llength $eb(result$eb(cur_ind))] found"
}

proc eb_describe {error_browser} {
    upvar #0 $error_browser eb
    global Transform
    set line [lindex $eb(result$eb(cur_ind)) [lindex [$eb(list) curselection] 0]]
    regexp {([^\*]+)\*([^\*]+)\*([^\*]+)\*} $line dummy local_filename rootname linenum
    $eb(label) config -text $local_filename
}

proc eb_read_pipe {error_browser} {
    upvar #0 $error_browser eb
    set input_str [gets $eb(pipe)]
    if [eof $eb(pipe)] {
	catch {close $eb(pipe)}
	eb_filemenu_cb $eb(name) normal
	$eb(label) config -text "$eb(title): [llength $eb(result$eb(cur_ind))] found"
	set eb(finish) 1
    } else {
	catch {
	    regexp {([^:]+):(.*)} $input_str dummy linenum err_msg
	    $eb(list) insert end $eb(tailname):$linenum:$err_msg
	    lappend eb(result$eb(cur_ind)) $eb(local_fn)\*$eb(tailname)\*$linenum\*$err_msg
	}
	update idletasks
    }
}
	    
proc start_process {error_browser cmd {del 1}} {
    upvar #0 $error_browser eb
    global old_bind
    if $del {
	if {$eb(cur_ind)!=0} {
	    set eb(cur_sel$eb(cur_ind)) [lindex [$eb(list) curselection] 0]
	}
	$eb(list) delete 0 end
	$eb(label) config -text ""
	set eb(cur_ind) [eb_get_next_ind $eb(name)]
	set eb(result$eb(cur_ind)) {}
    }
    if ![catch {set eb(pipe) [open "| $cmd "  w+]}] {
	fileevent $eb(pipe) readable "eb_read_pipe $eb(name)"
    }
}

proc eb_build {error_browser} {
    global listfont statfont oldfocus old_bind
    upvar #0 $error_browser eb
  
    set w .$eb(name)
    set n $w.toolbar
    set l $w.listbox
    set b $w.bottom
    catch {destroy $w}
    toplevel $w -class Dialog
    if [isunix] {
	wm geometry $w =505x500
    } else {
	wm geometry $w =487x500
    }
    #wm title $w "Discover:  $eb(title)"
    wm protocol $w WM_DELETE_WINDOW "eb_close $eb(name)"

    set n [frame $w.toolbar -bd 2]
    
    if [isunix] {
	set relief groove
    } else {
	set relief raised
    }

    set l [frame $w.listbox -width 300 -height 450 -relief $relief -bd 1]
    set b [frame $w.bottom -relief raised -bd 1]
    pack $b -side bottom -fill both
    pack $n -side top -fill x
    pack $l -side top -fill both -expand true

    eb_button $n.back "Back " $relief "eb_back $eb(name)"
    eb_button $n.forward "Forward" $relief "eb_forward $eb(name)" 8
    
    eb_button $n.previous "Prev. " $relief "eb_previous $eb(name)"
    eb_button $n.current "Current" $relief "eb_current $eb(name)"
    eb_button $n.next "Next " $relief "eb_next $eb(name)" 

    eb_button $n.find "Find " $relief "eb_dialog_find $eb(name)"
    eb_button $n.print "Print" $relief "eb_print $eb(name)"
    
    eb_button $n.close "Close" $relief "eb_close $eb(name)"
    eb_button $n.stop "Stop" $relief "eb_stop $eb(name)"

    label $n.sep1
    label $n.sep2 
    pack  $n.back $n.forward  $n.sep1 $n.find $n.print $n.sep2 $n.previous $n.current $n.next -side left -anchor w
    pack $n.stop $n.close -side right -anchor e

    set eb(list) $l.list
    listbox $l.list  -font $listfont -yscrollcommand "$l.scroll set" -selectmode extended -exportselection no
    scrollbar $l.scroll -command "$l.list yview"
    
    pack $l.scroll -in $l -side right -fill y 
    pack $l.list -in $l -side left -fill both -expand true

    bind $l.list <Button-1> "$old_bind; focus %W; eb_describe $eb(name)"
    bind $l.list <Double-Button-1> "eb_open_error $eb(name)"
    bind $l.list <Return> "eb_current $eb(name)"
    #bind $l.list <Control-Button-1> ""
    
    
    set eb(label) $b.label
    label $b.label -font $statfont -anchor w 
    pack  $b.label -side top -fill both -expand true

    #grab set .sub 

    set oldfocus [focus]

    catch {grab $w}
    catch {tkwait visibility $w}
    catch {grab release $w}
    raise $w
    focus $eb(list)

    #tkwait window .sub
}

proc eb_open_error {error_browser} {
    upvar #0 $error_browser eb
    set line [lindex $eb(result$eb(cur_ind)) [lindex [$eb(list) curselection] 0]]
    regexp {([^\*]+)\*([^\*]+)\*([^\*]+)\*} $line dummy local_filename rootname linenum
    if {$linenum==-1} {set linenum 0}
    myputs [concat {open} [list $local_filename $linenum]]
}

proc eb_previous {error_browser} {
    upvar #0 $error_browser eb
    set cur_sel [lindex [$eb(list) curselection] 0]
    if {$cur_sel=="" || $cur_sel==0} {return}
    incr cur_sel -1
    $eb(list) selection clear 0 end
    $eb(list) selection set $cur_sel $cur_sel
    $eb(list) see $cur_sel
    eb_open_error $eb(name)
} 
   
proc eb_next {error_browser} {
    upvar #0 $error_browser eb
    set cur_sel [lindex [$eb(list) curselection] 0]
    if {$cur_sel==""} {
	set cur_sel 0
    } else {
	incr cur_sel 1
    }
    if {$cur_sel<[llength $eb(result$eb(cur_ind))]} {
	$eb(list) selection clear 0 end
	$eb(list) selection set $cur_sel $cur_sel
	$eb(list) see $cur_sel
	eb_open_error $eb(name)
    }
}    

proc eb_close {error_browser} {
    upvar #0 $error_browser eb
    if {[tk_messageBox -title Message -message "Are you sure you want to close?" -type yesno -icon question]=="yes"} {
	if $eb(running) {
	    eb_stop $eb(name)
	    after 500
	}
	catch { wm withdraw .find$eb(name)}
	catch { wm withdraw .$eb(name)}
	set eb(cur_ind) 0
	set eb(max_ind) 0
    }
}

proc eb_current {error_browser} {
    upvar #0 $error_browser eb
    set cur_sel [lindex [$eb(list) curselection] 0]
    if {$cur_sel==""} {
	set cur_sel 0
    }
    $eb(list) selection clear 0 end
    $eb(list) selection set $cur_sel $cur_sel
    $eb(list) see $cur_sel
    eb_open_error $eb(name)
}

proc eb_dialog_find {error_browser} {
    upvar #0 $error_browser eb
    global oldfocus find

    set f .find$eb(name)

    if [Dialog_Create $f "Find" -borderwidth 10] {
	set l [frame $f.left]
	set r [frame $f.right -width 25]
	pack $l  -side left 
	pack $r -side left -padx 10 -anchor e
	set t [frame $l.top]
	set b [frame $l.bottom]
	pack $t $b -side top -anchor w
	label $t.lbl -text "Find what:   " 
	combobox $t.entry  -width 25 -textvariable find($eb(name)_pat) -exportselection no
	pack $t.lbl $t.entry -side left -pady 5
	
	frame $b.left 
	frame $b.right
	pack $b.left  -side left  
	pack $b.right -side left 

	checkbutton $b.left.exactmatch  -text "Exact match" -width 16 -anchor w -variable find($eb(name)_exactmatch)
	checkbutton $b.left.matchcase   -text "Match case" -width 16 -anchor w -variable find($eb(name)_matchcase)
	checkbutton $b.left.reg   -text "Reg. Expression" -width 16 -anchor w -variable find($eb(name)_reg)
	pack $b.left.exactmatch $b.left.matchcase $b.left.reg -side top -pady 2 

	radiobutton $b.right.down   -text "down" -relief flat -anchor w -variable find($eb(name)_direction) -value 0
	radiobutton $b.right.up   -relief flat -text "up  " -anchor w -variable find($eb(name)_direction) -value 1
	pack $b.right.down $b.right.up -side top  -anchor w -padx 35
	$b.right.down select

	button $r.next -text "Find next"   -command "combobox_add  .find$eb(name).left.top.entry; eb_find $eb(name) find"
	button $r.findall -text " Find all  "  -command "combobox_add  .find$eb(name).left.top.entry; eb_findall $eb(name) find"
	button $r.cancel -text "  Cancel  "  -command "eb_find_cancel $eb(name)"
	pack  $r.next $r.findall $r.cancel -side top -padx 5 -pady 5
	
	bind $t.entry.e <Return> "combobox_add .find$eb(name).left.top.entry; eb_find $eb(name) find"
    }

    $f.left.top.entry.e selection range 0 end
    set oldfocus [focus -displayof $f]
    if $eb(running) {
	$f.right.findall config -state disabled
    }

    catch {grab $f}
    catch {tkwait visibility $f}
    catch {grab release $f}
    raise $f
    focus $f.left.top.entry.e
}

proc eb_find {error_browser findarr} {
    upvar #0 $error_browser eb
    upvar #0 $findarr find
    set pat $find($eb(name)_pat)
    set exactmatch $find($eb(name)_exactmatch)
    set matchcase $find($eb(name)_matchcase)
    set reg $find($eb(name)_reg)
    set direction $find($eb(name)_direction)
    if {$pat!={}} {
	set sz [llength $eb(result$eb(cur_ind))]
	set cur_sel [lindex [$eb(list) curselection] 0]
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
		set sublist [lrange $eb(result$eb(cur_ind)) [incr cur_sel] end]
		if {[llength $sublist]>0} {
		    set i $cur_sel
		    foreach str $sublist {
			regexp {([^:]+):(.*)} $str dummy local_file str
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
			$eb(list) selection clear 0 end
			$eb(list) selection set $i $i
			$eb(list) see $i
		    } else {
			set error 1
		    }
		} else {
		    set error 1
		}
	    } else {
		if {$cur_sel==""} { set cur_sel $sz}
		set sublist [lrange $eb(result$eb(cur_ind)) 0 [incr cur_sel -1]]
		if {[llength $sublist]>0} {
		    set last -1
		    set i 0
		    foreach str $sublist {
			regexp {([^:]+):(.*)} $str dummy local_file str
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
			$eb(list) selection clear 0 end
			$eb(list) selection set $last $last
			$eb(list) see $last
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
	.find$eb(name).left.top.entry.e selection range 0 end
    }
}

proc eb_findall {error_browser findarr} {
    upvar #0 $error_browser eb
    upvar #0 $findarr find
    set pat $find($eb(name)_pat)
    set exactmatch $find($eb(name)_exactmatch)
    set matchcase $find($eb(name)_matchcase)
    set reg $find($eb(name)_reg)
    if {$pat!={}} {
	set sz [llength $eb(result$eb(cur_ind))]
	if {$reg==1 && $exactmatch==1} {set pat "^$pat$"}

	set findall_res {}
	set ii 0
	foreach str $eb(result$eb(cur_ind)) {
	    regexp {([^:]+):(.*)} $str dummy local_file str
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
	set old_ind $eb(cur_ind)
	set eb(cur_ind) [eb_get_next_ind $eb(name)]
	set eb(result$eb(cur_ind)) {}
	foreach i $findall_res {
	    lappend eb(result$eb(cur_ind)) [lindex $eb(result$old_ind) $i]
	}
	set eb(title$eb(cur_ind)) "$eb(title$old_ind) | findall \"$pat\""
	$eb(list) delete 0 end
	foreach el $eb(result$eb(cur_ind)) {
	     regexp {([^\*]+)\*([^\*]+)\*([^\*]+)\*(.*)} $el dummy local_file rootname linenum err_msg
	    $eb(list) insert end $rootname:$linenum:$err_msg
	}
	wm title .$eb(name) "Discover $eb(title):  $eb(title$eb(cur_ind))"
	$eb(label) config -text "$eb(title): [llength $eb(result$eb(cur_ind))] found"
    }
}

proc eb_find_cancel {error_browser} {
    upvar #0 $error_browser eb
    global oldfocus
    catch {grab release .find$eb(name)}
    focus $oldfocus
    Dialog_Dismiss .find$eb(name)
}

proc eb_filemenu_cb {error_browser state} {
    upvar #0 $error_browser eb
    set filemenu .d1.mbar.file
    switch -exact $eb(name) {
	egrep {set label "Grep ..."}
	inst {set label "Instances"}
	submit {set label "Submit"}
    }
    $filemenu entryconfigure [$filemenu index $label] -state $state
}

set egrep(running) 0
proc do_grep {} {
    global egrep Psethome Transform
    if $egrep(running) {
	return
    }
    set res [Dialog_grep]
    if {$res!={}} {
	set arglist [dis_prepare_selection]
	set model_fnamelist [dis_query "modules $arglist" fname]
	if {$model_fnamelist == {}} {return}
	
	set flags [lindex $res 0]
	set pat [lindex $res 1]
	set reg [lindex $res 2]

	if {$reg==1} {
	    regsub -all {\\} $pat {\\\\\\} pat
	    if {[string first \" $pat]<0} {
		set pat \"$pat\"
	    } 
	    set grep_flags "-n $flags $pat"
	} else {
	    regsub -all {\\} $pat {\\\\} pat
	    if {[string first \" $pat]<0} {
		set pat \"$pat\"
	    }
	    set grep_flags "-F -n $flags $pat"
	}

	set egrep(cancel) 0
	set egrep(running) 1
	if ![winfo exists .egrep] {
	    array set egrep {name egrep title "Grep" cur_ind 0 max_ind 0}
	    eb_build egrep
	}
	wm deiconify .egrep
	raise .egrep

	set egrep(cur_ind) [eb_get_next_ind egrep]
	$egrep(list) delete 0 end
	set egrep(result$egrep(cur_ind)) {}
	set egrep(cur_sel$egrep(cur_ind)) ""
	set egrep(title$egrep(cur_ind)) "egrep $grep_flags $arglist"
	wm title .egrep "Discover Grep:  $egrep(title$egrep(cur_ind))"
	button_cb egrep 0
	eb_filemenu_cb egrep disabled
	foreach model_fname $model_fnamelist {
	    if {$Transform==2} {
		get_index $model_fname 0
	    }
	    set egrep(tailname) [file tail $model_fname]
	    set local_fname [get_localfname $model_fname]
	    set egrep(local_fn) $local_fname
	    set cmd "$Psethome/bin/generic/egrep $grep_flags \"$local_fname\""
	    update
	    set egrep(finish) 0
	    start_process egrep $cmd 0
	    tkwait variable egrep(finish)
	    if {$egrep(cancel)==1} {return}
	} 

	button_cb egrep 1
	eb_filemenu_cb egrep normal
	set egrep(running) 0
    }
}

proc query_instances {} {
    global inst
    if !$inst(running) { 
	set arglist [dis_prepare_selection]
	set instances [dis_command "dis_instances $arglist"]
	do_instances $instances
    }
}

set inst(running) 0
proc do_instances { instances } {
    global modified model_localcopy Transform inst Psethome
    if {$instances=={}} {return}
    set inst(cancel) 0
    set inst(running) 1
    if ![winfo exists .inst] {
	array set inst {name inst title "Instances" cur_ind 0 max_ind 0}
	eb_build inst
    }
    activate_browser .inst
    set inst(cur_ind) [eb_get_next_ind inst]
    $inst(list) delete 0 end
    set inst(result$inst(cur_ind)) {}
    set inst(cur_sel$inst(cur_ind)) ""
    set inst(title$inst(cur_ind)) [lindex $instances 0]
    wm title .inst "Discover Instances:  $inst(title$inst(cur_ind))"
    eb_filemenu_cb inst disabled
    button_cb inst 0
    set instances [lrange $instances 1 end]
    foreach line $instances {
	regexp {([^:]+): (.*)} $line dummy lname linelist
	set linelist [join [lsort -integer $linelist]]
	set model_fname [change_drivename [string trim [dis_command "fname $lname"] \{\}]]
	set n [get_index $model_fname 0]
	if {$n<0} {return}
	set local_fname [get_localfname $model_fname]
	set inst(local_fn) $local_fname
	set inst(tailname) [file tail $model_fname]
	#set inst(filename) [file tail $model_fname]
	if {$Transform<2 && $modified($n)} {
	    set cmd "$Psethome/bin/mrg_update -instances $linelist $model_localcopy($n) $model_localcopy($n).diff1 \"$local_fname\""
	} else {
	    set cmd "$Psethome/bin/mrg_update -print_line $linelist \"$local_fname\""
	}	
	update
	set inst(finish) 0
	start_process inst $cmd 0
	tkwait variable inst(finish)
	if {$inst(cancel)==1} {return}
    }
    eb_filemenu_cb inst normal
    button_cb inst 1
    set inst(running) 0
}

proc button_cb {error_browser flag} {
    upvar #0 $error_browser eb
    if $flag {
	set state normal
	set state1 disabled
    } else {
	set state disabled
	set state1 normal
    }
    .$eb(name).toolbar.back config -state $state
    .$eb(name).toolbar.forward config -state $state
    .$eb(name).toolbar.close config -state $state
    .$eb(name).toolbar.print config -state $state
    .$eb(name).toolbar.stop config -state $state1
    catch {.find$eb(name).right.findall config -state $state}
}






