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
set startup 0
if {[info exists env(DIS_LITESTARTUP)] && $env(DIS_LITESTARTUP)==1 } {
    set startup 1
    source $env(DIS_INSTALL)/common_init.tcl
}
source $env(DIS_INSTALL)/combobox.tcl
namespace import ::combobox::*


set available_hosts {}
set available_hosts [exec $Dish -all]
if { $available_hosts == "" } {
    myputs "info [error_msg NO_SERVICES]"
    exit
} else {
   # this will fill a list of all services on the network
   # in the following format : host@service_name
   set available_services ""
   set readable_services ""
   foreach computer $available_hosts {
       set ip   [lindex $computer 0]
	   set name [lindex $computer 1]
       set service_list [join [exec $Dish -test $ip]]
       foreach s $service_list {
	      if { [string index $s 0] != "." } {
	          set full_service_name "$ip@$s"
	          set readable_service_name "$name@$s"
	          lappend available_services $full_service_name
	          lappend readable_services $readable_service_name
		   }
       }
   }
   if { $available_services == "" } {
      myputs "info [error_msg NO_SERVICES]"
      exit
   }
   if {![info exists Service] || $Service == "" } {
       set Service [lindex $available_services 0]
       set ReadableService [lindex $readable_services 0]
   } else {
       set ind [lsearch $available_services $Service]
       if { $ind != -1 } {
	   set ReadableService [lindex $readable_services $ind]
       } else {
	   set Service [lindex $available_services 0]
	   set ReadableService [lindex $readable_services 0]
       }
   }
}


if [isunix] {
    set font -Adobe-Helvetica-Medium-R-Normal--*-120-*-*-*-*-*-*
    set pady 4
} else {
    set font {{MS Sans Serif} 8}
    set pady 2
}

proc set_CM_none {} {
    global CM CMLabel CMInfo
    set CM 0
    set CMLabel [lindex $CMInfo 0]
}

proc transform_select { ind } {
    global Transform opened_modelflist opened_localflist
    set Transform $ind
    if {$ind==1 && ![isunix]} {
	set_CM_none
    } elseif {$ind==2} {
	set_CM_none
    }
    set opened_modelflist {}
    set opened_localflist {}
    prefs_transform
}

proc CM_select { ind } {
    global CM CMLabel CMInfo Transform
    set enable_CM 0
    if [isunix] {
	if {$Transform != 2} {
	    set enable_CM 1
	}
    } else {
	if !$Transform {
	    set enable_CM 1
	}
    }
    if $enable_CM {
	set CM $ind
    } else {
	set_CM_none
    }
}

proc editor_select { ind } {
    global Editor
    set Editor $ind 
}

proc servicechooser_optionMenu {w cmd varName options} {
    upvar #0 $varName var
    if ![info exists var] {
	set var [lindex $options 0]
    }
    global font
    menubutton $w -textvariable $varName -indicatoron 1 -menu $w.menu \
	    -relief raised -font $font  -anchor w -width 11
    
    menu $w.menu -tearoff 0
    
    set ind 0
    foreach i $options {
	$w.menu add radiobutton -label $i -variable $varName -command [list $cmd $ind]
	incr ind
    }
    
    return $w.menu
}

proc service_choose {w val} {
}

set oldService $Service
catch {destroy .w}
wm withdraw .
toplevel .w -class Dialog
wm resizable .w 0 0
wm title .w "Discover -  Service Chooser"

frame .w.app 
frame .w.t
frame .w.model  
frame .w.share 
frame .w.private
frame .w.temp
frame .w.b
pack .w.app .w.t .w.temp .w.model .w.share .w.private  .w.b -side top -fill x 

label .w.app.l -text "Application Setting:" -width 15 -font $font -anchor w -padx 0

label .w.app.lab -font $font
button .w.app.b -font $font -width 8 -pady $pady -command "file_dialog .w"
pack  .w.app.l .w.app.lab  -side left -anchor w -padx 10 -pady 8 
pack .w.app.b -side right -padx 10 -pady 8

frame .w.t.service 
frame .w.t.option 
pack .w.t.service .w.t.option -side left

label .w.t.service.l -text "Service:" -width 15 -font $font -anchor w -padx 0

set _xxx_ $ReadableService
combobox .w.t.service.e -width 35 -font $font -exportselection no -command "service_choose" -textvariable _xxx_
pack  .w.t.service.l .w.t.service.e -side left  -anchor w -padx 10 -pady 5
bind .w.t.service.e.top.list +{ServiceChooserInit %W}

label .w.t.option.label1 -text "CM:" -font $font 
servicechooser_optionMenu .w.t.option.cm CM_select CMLabel $CMInfo
label .w.t.option.label2 -text "  Mode:" -font $font 
servicechooser_optionMenu .w.t.option.transform transform_select TransformLabel [list "Transform" "No Transform" "Server Source"]
label .w.t.option.label3 -text " Editor:" -font $font 
servicechooser_optionMenu .w.t.option.editor editor_select EditorLabel $EditorInfo
label .w.t.option.label4 -width 0
pack .w.t.option.label1 .w.t.option.cm .w.t.option.label2 .w.t.option.transform .w.t.option.label3 .w.t.option.editor .w.t.option.label4 -side left -anchor w -padx 2  -pady 8  
eval combobox_setlist .w.t.service.e $readable_services

label .w.temp.l -text "Temp. Directory:" -width 15 -font $font -anchor w -padx 0
entry .w.temp.e  -font $font -textvariable Tempdir 
pack .w.temp.l -side left  -anchor w -padx 10 -pady 5
pack .w.temp.e -side right -anchor w -padx 10 -pady 5 -fill x -expand true

label .w.model.l -text "ServerSrcRoot:" -width 15 -font $font -anchor w -padx 0
entry .w.model.e  -font $font -textvariable ModelSrcRoot
pack .w.model.l  -side left  -anchor w -padx 10 -pady 5 
pack .w.model.e -side  right -anchor w -padx 10 -pady 5 -fill x -expand true

label .w.share.l -text "SharedSrcRoot:" -width 15 -font $font -anchor w -padx 0
entry .w.share.e -font $font -textvariable SharedSrcRoot
pack .w.share.l -side left  -anchor w -padx 10 -pady 5 
pack .w.share.e -side  right -anchor w -padx 10 -pady 5 -fill x -expand true

label .w.private.l -text "PrivateSrcRoot:" -width 15 -font $font -anchor w -padx 0
entry .w.private.e  -font $font -textvariable PrivateSrcRoot
pack .w.private.l -side left  -anchor w -padx 10 -pady 5 
pack .w.private.e -side  right -anchor w -padx 10 -pady 5 -fill x -expand true

button .w.b.start -width 10 -pady $pady -font $font -command prefs_ok
button .w.b.exit -width 10 -pady $pady -font $font -command prefs_cancel
pack .w.b.start -side left -anchor w -padx 30 -pady 10
pack .w.b.exit -side right -anchor e -padx 30 -pady 10

proc service_choose {w val} {
    if {$val == ""} {return}
    global ReadableService
    if {$val == $ReadableService} {return}
    debug_msg "$ReadableService $val"
    set ReadableService $val

    if [service_ok] {
	global env Model_name
	set_service_tag 0
	set init_file $env(HOME)/dislite/$Model_name/$Model_name.prefs
	init_setting $init_file
    }
}
proc init { init_file } {
    global CM old_CM 
    .w.app.lab configure -text $init_file
    init_setting $init_file
    set old_CM $CM
    prefs_transform
}

proc file_dialog {w} {
	global env Model_name
    set init_file [tk_getOpenFile -filetypes {{"Preference Files"	{.prefs}}}  -initialdir $env(HOME)/dislite/$Model_name -parent $w]
    if [string compare $init_file ""] {
	init $init_file
    }
}
proc service_ok {} {
    global env Service Tempdir oldfocus error CM old_CM available_services
    global readable_services ReadableService Model_name dis_in Editor
    set ind [lsearch $readable_services $ReadableService]
    if { $ind != -1 } {
	set Service [lindex $available_services $ind]
    } else {
	set Service [lindex $available_services 0]
	set ReadableService [lindex $readable_services 0]
    }
    
    set Service [string trim $Service]
    if { [string length $Service] == 0 } {
	myputs "info [error_msg ZERO_SERVICE]"
	return 0
    } elseif { [lsearch -exact $available_services $Service] == -1 } {
	myputs "info service \'$Service\'; [error_msg WRONG_SERVICE]"
	return 0
    } else {
	if ![file exists $env(HOME)/dislite] {
	    file mkdir $env(HOME)/dislite
	}
	if ![file exists $env(HOME)/dislite/$Model_name] {
	    file mkdir $env(HOME)/dislite/$Model_name
	}
    }
	if {[info exists env(DIS_EDITOR)] && $env(DIS_EDITOR)!=$Editor} {
				    tk_messageBox -title "Warning" -message "To work with the new editor you have\n to quit and restart dislite (DevXpress)"
	}
	return 1
}

proc prefs_ok {} {
    global env Service Tempdir oldfocus error CM old_CM available_services
    global ModelSrcRoot SharedSrcRoot PrivateSrcRoot Transform startup dis_in Editor
    global list listfont result cur_ind clipboardlist active_grp Model_name ReadableService readable_services

    regsub -all {\\} $ModelSrcRoot / ModelSrcRoot
    regsub -all {\\} $SharedSrcRoot / SharedSrcRoot
    regsub -all {\\} $PrivateSrcRoot / PrivateSrcRoot

	set name [string trimright $ModelSrcRoot "/"]
	set ModelSrcRoot "$name/"

	set name [string trimright $SharedSrcRoot "/"]
	set SharedSrcRoot "$name/"
	if {![file exists $SharedSrcRoot]} {
		tk_messageBox -title "Error" -message "Directory $SharedSrcRoot does not exist.\nPlease,check the \$SharedSrcRoot preference." -type ok -icon error
		return
	}
	set name [string trimright $PrivateSrcRoot "/"]
	set PrivateSrcRoot "$name/"
	if {![file exists $PrivateSrcRoot]} {
		tk_messageBox -title "Error" -message "Directory $PrivateSrcRoot does not exist.\nPlease,check the \$PrivateSrcRoot preference." -type ok -icon error
		return
	}

    
	if [service_ok] {
	save_pref_file $env(HOME)/dislite/$Model_name/$Model_name.prefs
	set f [open $env(HOME)/dislite/dislite.prefs w]
	puts $f $Service
	close $f
	set edtr 0
	set f [open $env(HOME)/dislite/editor.prefs w+]
	gets $f $edtr
	if {[info exists Editor]} {
	    set edtr $Editor
	}
	seek $f 0	
	puts $f $edtr
	close $f
    }

    if $startup {
	   exit
    } else {
	if {$error} {
	    set msg "Dislite_Restart $Service"
	    myputs $msg
	} else {
	    if { $old_CM != $CM } {
		   do_exit 1
	    } else {
		set servicename [dis_command "puts \$cli_service"]
		
		if {$servicename != $Service } {
		    do_exit 1
		}
	    }
	}
	focus $oldfocus
	catch {grab release .w}
	catch {destroy .w}
    }
}

proc prefs_cancel {} {
    global oldfocus env startup error Service
    focus $oldfocus
    catch {grab release .w}
    catch {destroy .w}
    if $error {
	   set msg "Dislite_Exit $Service" 
	   myputs $msg
    }
    if $startup {
	catch {file delete $env(HOME)/dislite/dislite.prefs}
	exit
    } 
    global oldService Service env Model_name
    if {$oldService!=$Service} {
	set Service $oldService

	set_service_tag 0
	set init_file $env(HOME)/dislite/$Model_name/$Model_name.prefs
	init_setting $init_file
    }
}

proc prefs_transform {} {
    global Transform CM
    if !$Transform {
	if [isunix] {
	    .w.model.e configure -state normal -bg #d9d9d9
	    .w.share.e configure -state normal -bg #d9d9d9
	    .w.private.e configure -state normal -bg #d9d9d9
	} else {
	    .w.model.e configure -state normal -bg white
	    .w.share.e configure -state normal -bg white
	    .w.private.e configure -state normal -bg white
	}
    } else {
	.w.model.e configure -state disabled -bg gray
	.w.share.e configure -state disabled -bg gray
	.w.private.e configure -state disabled -bg gray
    }
    
}
	
.w.app.b configure  -text "Browse..."
.w.b.start configure -text OK
.w.b.exit configure -text Cancel

if {![info exists Service] || $Service==""} {
    if {![info exists env(DIS_SERVICE)] || $env(DIS_SERVICE)==""} {
	set Service [lindex $available_services 0]
    } else {
	set Service $env(DIS_SERVICE)
    }
}

set_service_tag 1

set oldfocus [focus]
focus .w.t.service.e
catch {grab .w}
