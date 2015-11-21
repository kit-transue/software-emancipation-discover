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
# Let's establish some global variables
global env
global emacsPath EditorList
set startup 0
if {[info exists env(DIS_LITESTARTUP)] && $env(DIS_LITESTARTUP)==1 } {
    set startup 1
    source $env(DIS_INSTALL)/common_init.tcl
    source $env(DIS_INSTALL)/combobox.tcl
    namespace import ::combobox::*

}

set available_hosts {}
if { "[array get env DIS_HOSTS]" != "" } {
    set available_services ""
    set readable_services ""
    foreach PmodServerHost $env(DIS_HOSTS) {
		set ServiceNameList [join [exec $Dish -test $PmodServerHost]]
		foreach ServiceName $ServiceNameList {
		    if { [string index $ServiceName 0] != "." } {
				set full_service_name "$PmodServerHost@$ServiceName"
				set readable_service_name "$PmodServerHost"
				set colonIdx [string last ":" $ServiceName]
				if {$colonIdx >= 0} {
					set ServiceName [string range $ServiceName [expr $colonIdx+1] [string length $ServiceName]]
				}
				set readable_service_name "$readable_service_name@$ServiceName"
				lappend available_services $full_service_name
				lappend readable_services $readable_service_name
		    }
		}
    }
} else {
    set available_hosts [exec $Dish -all]

    if { $available_hosts != "" } {
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
					set readable_service_name "$name"
					set colonIdx [string last ":" $s]
					if {$colonIdx >= 0} {
						set s [string range $s [expr $colonIdx+1] [string length $s]]
					}
				    set readable_service_name "$readable_service_name@$s"
				    lappend available_services $full_service_name
				    lappend readable_services $readable_service_name
				}
		    }
		}
    }
}

if { $available_services == "" } {
	tk_messageBox -title "Error" -message "No services are available.  Developer XPress will now exit\n"
	catch {file delete $env(HOME)/dislite/dislite.prefs}
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

proc editor_select { index } {
	global EditorList
	global editorDisplayName editorName editorExecutable editorArguments
	global env unix_platform emacsPath Psethome
	set editorDisplayName [ lindex $EditorList $index ]
	set integratedEditors [ open $env(HOME)/dislite/integratedEditors.txt ]
	set foundEditor 0
	while { ! $foundEditor && ! [ eof $integratedEditors ] } {
		gets $integratedEditors currentLine
		regsub -all {\\} $currentLine / currentLine
		if { $editorDisplayName == [ lindex $currentLine 0 ] } {
		        set foundEditor 1
		        set editorName [ lindex $currentLine 1 ]
		        set editorExecutable [ lindex $currentLine 2 ]
		        set editorArguments [ lrange $currentLine 3 end ]
		}
	}
	# The editor might be emacs or vim
	if { $foundEditor != 1 } {
		# ASSERTION: If emacs was in EditorList, it was either found in integratedEditors.txt
		#        (windows) or we set the emacsPath (unix)
		if { $editorDisplayName == "emacs" } {
		        set foundEditor 1
		        set editorName "emacs"
		        set editorExecutable {$emacsPath}
		        set editorArguments "-l {$Psethome/lib/dislite/dislite.el}"
		}
		if { $editorDisplayName == "gvim" } {
		        set foundEditor 1
		        set editorName "gvim"
		        if { $unix_platform == 1 } {
		                set editorExecutable "{$Psethome/bin/vim}"
		        } else {
		                set editorExecutable "{$Psethome/bin/gvim.exe}"
		        }
		        set editorArguments "-g"
		}
	}
	if { $foundEditor != 1 } {
		tk_messageBox -title "Error" -message "That editor is not a supported editor." -icon error
	}
}

proc createEditorList {} {
	global EditorList env font editorDisplayName
	set EditorList ""
	# The current editor needs to be listed as the first element in the box
	if { $editorDisplayName != "" } {
		lappend EditorList $editorDisplayName
	}
	if { ! [ file exists $env(HOME)/dislite/integratedEditors.txt ] } {
		# This message really necessary?  Who cares if they have it or not.  Create it, dumbass!
		#tk_messageBox -title "Editors not found" -message "You do not have a \$HOME/dislite/integratedEditors.txt file.  A default copy of this file will be created." -icon error -type ok
		createIntegratedEditorsFile
	} else {
		set integratedEditors [ open $env(HOME)/dislite/integratedEditors.txt ]
		while { ! [ eof $integratedEditors ] } {
		        gets $integratedEditors editorInformation
		        if { $editorInformation != "" } {
					set EditorName [ lindex $editorInformation 0 ]
					# do not add an editor if it is already there
					if {[lsearch -exact $EditorList $EditorName] < 0} {
			        	lappend EditorList $EditorName
					}
		        }
		}
		close $integratedEditors
		if [llength $EditorList] {
			editor_select 0
		}
	}
}

# This function takes "emacs" or "gvim" as an argument, and determines whether such
#        an editor is present on the system or not.  If it is, it returns the nameserver
#        name, the path to the executable, and the arguments
# note: This will set emacsPath for the calling function
proc verifyBuiltInEditor { currEditorName } {
	global Psethome unix_platform emacsPath
	set returnString ""
	if { $currEditorName == "emacs" } {
		if { $unix_platform == 1 } {
		        set catchError [ catch { exec "$Psethome/bin/pset-which" emacs } ]
		        if { $catchError == 0 } {
		                lappend returnString "emacs"
		                set emacsPath [ exec "$Psethome/bin/pset-which" emacs ]
		                lappend returnString "$emacsPath"
		                lappend returnString "-l $Psethome/lib/dislite/dislite.el"
		        }
		}
		# If this is Windows, we can't verify emacs installation
	}
	if { $currEditorName == "gvim" } {
		if { $unix_platform == 1 } {
		        if { [ file executable "$Psethome/bin/vim" ] } {
		                set returnString "gvim $Psethome/bin/vim -g"
		        }
		} else {
		        if { [ file executable "$Psethome/bin/gvim.exe" ] } {
		                set returnString "gvim \"$Psethome/bin/gvim.exe\" -g"
		        }
		}
	}
	return $returnString
}

proc CM_select { ind } {
    global CM CMLabel CMInfo Transform
	if {$Transform != 2} {
		set CM $ind
	} else {
		tk_messageBox -title Warning -type ok -message "CM commands are not applicable for the \"Server source\" transformation option."
		set_CM_none
	}
}

proc servicechooser_optionMenu {w cmd varName options} {
    upvar #0 $varName var
    if ![info exists var] {
	set var [lindex $options 0]
    }
    global font

# It looks better if we don't put it in a disabled state
#    if {[llength $options] <= 1} {
#        menubutton $w -textvariable $varName -indicatoron 1 -menu $w.menu \
#                   -relief raised -font $font  -anchor w -width 11 \
#                   -state disabled
#    } else {
#        menubutton $w -textvariable $varName -indicatoron 1 -menu $w.menu \
#                   -relief raised -font $font  -anchor w -width 11
#    }
    
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
wm title .w "DISCOVER -  Service Chooser"

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

label .w.t.service.l -text "Service:" -width 9 -font $font -padx 0

set _xxx_ $ReadableService
combobox .w.t.service.e -width 35 -font $font -exportselection no -command "service_choose" -textvariable _xxx_
pack  .w.t.service.l .w.t.service.e -side left -padx 5 -pady 5
bind .w.t.service.e.top.list +{ServiceChooserInit %W}

label .w.t.option.label1 -text "CM:" -font $font 
servicechooser_optionMenu .w.t.option.cm CM_select CMLabel $CMInfo
label .w.t.option.label2 -text "  Mode:" -font $font 
servicechooser_optionMenu .w.t.option.transform transform_select TransformLabel [list "Transform" "No Transform" "Server Source"]

# keep track of the old editor
#tk_messageBox -message "editorDisplayName is $editorDisplayName"
set oldEditorDisplayName $editorDisplayName

# Let's get our valid editors
createEditorList

label .w.t.option.editorLabel -text " Editor:" -font $font
servicechooser_optionMenu .w.t.option.editor editor_select  EditorLabel $EditorList
label .w.t.option.label4 -width 0


# now we pack

pack .w.t.option.label1 .w.t.option.cm .w.t.option.label2 .w.t.option.transform .w.t.option.editorLabel .w.t.option.editor .w.t.option.label4 -side left -anchor w -padx 2  -pady 8  

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
    set init_file [tk_getOpenFile -filetypes {{"Preference Files"        {.prefs}}}  -initialdir $env(HOME)/dislite/$Model_name -parent $w]
    if [string compare $init_file ""] {
	init $init_file
    }
}

proc editor_ok {} {

# editor_ok returns:
#        0 - if a null editorName or editorExecutable has been specified
#        1 - if everything looks dandy
#        2 - if everything is dandy, and the editor settings have been modified

	global oldEditorDisplayName editorDisplayName editorName editorExecutable editorArguments

	if { $oldEditorDisplayName != $editorDisplayName } {
		set modified true
	} else {
		set modified false
	}
	set invalidEditorOptions false
	set returnValue 0

	if { $editorDisplayName == "" || $editorName == "" || $editorExecutable == "" } {
		set invalidEditorOptions true
	}

	if { $invalidEditorOptions == "true" } {
		set returnValue 0
	} else {
		if { $modified == "true" } {
		        set returnValue 2        
		} else {
		        set returnValue 1
		}
	}
	return $returnValue
}

proc service_ok {} {
    global env Service Tempdir oldfocus error CM old_CM available_services
    global readable_services ReadableService Model_name dis_in
    set ind [lsearch $readable_services $ReadableService]
    if { $ind != -1 } {
	set Service [lindex $available_services $ind]
    } else {
	set Service [lindex $available_services 0]
	set ReadableService [lindex $readable_services 0]
    }
    
    set Service [string trim $Service]
    if { [string length $Service] == 0 } {
		tk_messageBox -title "Error" -message "A service of zero length service was specified." 
		return 0
    } elseif { [lsearch -exact $available_services $Service] == -1 } {
		tk_messageBox -title "Error" -message "The specified service is not available." 
		return 0
    } else {
	if ![file exists $env(HOME)/dislite] {
	    file mkdir $env(HOME)/dislite
	}
	if ![file exists $env(HOME)/dislite/$Model_name] {
	    file mkdir $env(HOME)/dislite/$Model_name
	}
    }

  return 1
}

proc prefs_ok {} {
    global env Service Tempdir oldfocus error CM old_CM available_services
    global ModelSrcRoot SharedSrcRoot PrivateSrcRoot Transform startup 
    global dis_in Editor list Model_name
	global editorDisplayName editorName editorExecutable editorArguments

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
    
	if { [service_ok] } {
		save_pref_file $env(HOME)/dislite/$Model_name/$Model_name.prefs
		set f [open $env(HOME)/dislite/dislite.prefs w]
		puts $f $Service
		close $f
	    }

	set editorResults [ editor_ok ]
	# tk_messageBox -message "editorResults are $editorResults"
	if { $editorResults > 0 } {
		if { $editorResults == 2 } {
		        # write the new settings
				# tk_messageBox -message "Saving editor prefs"
		        set editorPreferenceFile [open $env(HOME)/dislite/editor.prefs w]
				set curEditorCfg ""
				if {[string first " " $editorDisplayName] > -1} {
					set curEditorCfg \{$editorDisplayName\}
				} else {
					set curEditorCfg $editorDisplayName
				}
				if {[string first " " $editorName] > -1} {
					set curEditorCfg "$curEditorCfg \{$editorName\}"
				} else {
					set curEditorCfg "$curEditorCfg $editorName"
				}
				if {[string first " " $editorExecutable] > -1} {
					set curEditorCfg "$curEditorCfg \{$editorExecutable\}"
				} else {
					set curEditorCfg "$curEditorCfg $editorExecutable"
				}
				set curEditorCfg "$curEditorCfg $editorArguments"

				puts $editorPreferenceFile $curEditorCfg
		        close $editorPreferenceFile
		}
	} else {
		tk_messageBox -title "Invalid Editor" -message "The editor you have chosen is invalid.\nPlease select a valid editor." -icon error        
		return
	}

    if $startup {
	   exit
    } else {
	if {$error} { # not error - this is Service_Chooser is invoked by dis_client
	    devxMessage RESTART_DEVX "$Service"
	    exit
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
    global oldfocus env startup error Service exitDevx
    focus $oldfocus
    catch {grab release .w}
    catch {destroy .w}
    if $error {
	   #I don't see why ANYTHING has to happened when cancel is hit
	   #set exitDevx 1
	   #editorMessage CLOSE_DRIVER
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
