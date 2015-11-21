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
# This file includes common initialization codes for both ServiceChooser
# dialogbox and dev. express main browser.

source $env(DIS_INSTALL)/filecomplete.tcl
source $env(DIS_INSTALL)/cm_init.tcl

set Psethome $env(PSETHOME)

proc isunix {} {global unix_platform; return $unix_platform}

# Unix - 1 , Windows - 0
set unix_platform 0
if [info exists tcl_platform(platform)] {
    if {$tcl_platform(platform)=="unix"} {
	set unix_platform 1
	proc myputs msg {
	    puts $msg 
	    flush stdout
	}
    }
}

# Initialize GUI settings: fonts, colors, etc.
if {$unix_platform == 1} {
    set listfont -Adobe-Helvetica-Bold-R-Normal--*-120-*-*-*-*-*-*
    set statfont -Adobe-Helvetica-Medium-R-Normal--*-120-*-*-*-*-*-*
    option add *font -Adobe-Helvetica-Bold-R-Normal--*-120-*-*-*-*-*-*
    option add *background #d9d9d9
    option add *foreground Black
    option add *highlightBackground #d9d9d9
    option add *highlightColor Black
    option add *selectBackground #c3c3c3
    option add *selectForeground Black
} else {
    set listfont {{Arial} 8}
    set statfont {{MS Sans Serif} 8 }
    option add *font {{MS Sans Serif} 8}
}

# Initialize 'CM:Generic'. 'cmdriver' may be loaded here if it is not running.
cm_init

# needed for our editor
global editorName
global editorExecutable
global editorArguments

proc createIntegratedEditorsFile {} {
	global env Psethome
	set integratedEditorsFile [ open "$env(HOME)/dislite/integratedEditors.txt" w ]
	close $integratedEditorsFile
}

proc loadEditorSettings {} {
# this function is to be used when it is anticipated that an editor.prefs exist
	global editorDisplayName editorName editorExecutable editorArguments
	global env error Psethome
	set returnValue 0
	if { ! [ file exists $env(HOME)/dislite/integratedEditors.txt ] } {
		createIntegratedEditorsFile
	}
	if { [file exists $env(HOME)/dislite/editor.prefs] } {
		set editorPreferenceFile [ open "$env(HOME)/dislite/editor.prefs" r ]
		if { [ eof $editorPreferenceFile ] } {
                tk_messageBox -title "Error" -message "Error: The \$HOME/dislite/editor.prefs file should specify a display name, editor name, its path, and its arguments." -icon error
		} else {
		        gets $editorPreferenceFile editorInformation
		        close $editorPreferenceFile
		        # then we separate the name, and then the arguments to the name
		        set editorDisplayName [ lindex $editorInformation 0 ]
		        set editorName [ lindex $editorInformation 1 ]
		        set editorExecutable [ lindex $editorInformation 2 ]
		        set editorArguments [ lrange $editorInformation 3 end ]
		        if { $editorDisplayName == "" || $editorName == "" || $editorExecutable == ""} {
		        	tk_messageBox -title "Error" -message "Error: The \$HOME/dislite/editor.prefs file should specify a display name, editor name, its path, and its arguments." -icon error
		        } else {
					set returnValue 1
				}
		}
	} 

	if { $returnValue == 0 } {
        tk_messageBox -title "Warning" -message "Default editor is not specified.\nSelect it in File->Service." -icon warning
        set editorDisplayName ""
        set editorName ""
        set editorExecutable ""
		set editorArguments ""
	}
}

# Let's set up the editor
loadEditorSettings

proc tempdirectory_init {} {
    global Tempdir Transform env
    
    if {![info exists Tempdir] || $Tempdir=="" || ![file exists $Tempdir]} {
	if [info exists env(DIS_SHARED_TMP)] {
	    set Tempdir $env(DIS_SHARED_TMP)
	} elseif [info exists env(TMPDIR)] {
	    set Tempdir $env(TMPDIR)
	} elseif [info exists env(TEMP)] {
	    set Tempdir $env(TEMP)
	} elseif [info exists env(TMP)] {
	    set Tempdir $env(TMP)
	} else {
	    if [isunix] {
		set Tempdir "/usr/tmp"
	    } else {
		set Tempdir "C:/Temp"
	    }
	}
	regsub -all {\\} $Tempdir / Tempdir
    }
    if {$Transform==2} {
	    cd $Tempdir
	    set Tempdir [pwd]
	    cd $env(DIS_INSTALL)
    }
}

array set PrefsTemplate {
    CM  0
    Transform 1
    Tempdir {}
    ModelSrcRoot {}
    SharedSrcRoot {}
    PrivateSrcRoot {}
    CurrentScope {}
}

proc save_pref_file {{pf {}}} {
    if {$pf==""} {
	global CurrentPrefFile
	set pf $CurrentPrefFile
    }
    global PrefsTemplate
    set f [open $pf w]
    foreach n [array names PrefsTemplate] {
	upvar #0 $n var
	if [info exists var] {
	    puts $f [list *dislite.$n [string trim $var]]
	}
    }
    close $f
}

proc read_pref_file init_file {
    global PrefsTemplate
    global CurrentPrefFile
    set CurrentPrefFile $init_file
    #set defaults and map
    foreach {var val} [array get PrefsTemplate] {
	global $var
	set $var $val
	set PrefsTemplateMap(*dislite.[string tolower $var]) $var
    }
    #read the file

    if {![file exists $init_file]} { return}

    set f [open $init_file r]
    while {[gets $f line]>=0} {
	if {[llength $line] != 2} {continue}
	set tag [string tolower [lindex $line 0]]
	if [info exists PrefsTemplateMap($tag)] {
	    set var $PrefsTemplateMap($tag)
	    set $var [lindex $line 1]
	}
    }
    close $f
}

# Fill variable with all supported CM systems.
set CMInfo [cm_get_systems]

proc init_setting {init_file} {
	    global Service Tempdir ModelSrcRoot TransformLabel CM CMLabel CMInfo
	    global SharedSrcRoot PrivateSrcRoot Transform startup env
	    global CurrentScope scope_flag 

	# If there's no dislite directory, then settings should not be loaded

	    read_pref_file $init_file

	    switch -exact $Transform {
		0 {set TransformLabel Transform}
		1 {set TransformLabel "No Transform"}
		2 {set TransformLabel "Server Source"}
	    }

	    if {$CM >= [llength $CMInfo]} {
		set CM 0
	    }         
	    set CMLabel [lindex $CMInfo $CM]

	    if {!$startup && ![isunix] && !$Transform} {
		set ModelSrcRoot [change_drivename $ModelSrcRoot]
		set SharedSrcRoot [change_drivename $SharedSrcRoot]
		set PrivateSrcRoot [change_drivename $PrivateSrcRoot]
	    }
	    if {$CurrentScope != ""} {
		set scope_flag 1
	    } else {
		set scope_flag 0
	    }

	    tempdirectory_init
}

set old_bind [bind Listbox <Button-1>]
set dis_sel 2
set error 0
if [isunix] {
    if ![catch {exec xlsfonts aseticons-20}] {
	set aseticons20 1
    } else {
	set aseticons20 0
    }
} else {
    set aseticons20 0
    regsub -all {\\} $Psethome / Psethome
}

if {[info exists env(DIS_DISH)] && $env(DIS_DISH)!=""} {
    set Dish $env(DIS_DISH)
}  else {
    if [isunix] {
	set Dish $Psethome/bin/dish2
    } else {
	set Dish $Psethome/bin/dish2.exe
    }
}


proc error_msg tag {
    switch -exact -- $tag {
	ZERO_SERVICE {
	    set res "{Zero length service specified}"
	}
	WRONG_SERVICE {
	    set res "{The service is not available}"
	}
	NO_SERVICES {
	    set res "{There are no Model servers running}"
	}
	INST_NO_SYM {
	    set res "{The current selection doesn't contain any symbols}"
	}
	INST_NO_DEF {
	    set res "{The current selection doesn't have definition file}"
	}
	SYM_NOT_EXIST {
	    set res "{This symbol doesn't exist in the model}"
	}
	NO_CM {
	    set res "{Developer Xpress isn't integrated with source control system}"
	}
	NOT_FROM_SERVER {
	    set res "{This file doesn't come from server}"
	}
	SYM_NO_DEF {
	    set res "{The current selection doesn't have definition file}"
	}
	SYM_NO_FNAME {
	    set res "{This symbol doesn't have physical file name}"
	}
	default {
	    set res "{}"
	}
    }
    return $res
}




proc do_source { } {
    global env 
    #set file  [tk_getOpenFile -initialdir $env(DIS_INSTALL) -title "Source file..." -filetypes {{"Tcl Files"    {.tcl}} {{Data Files} {.dat}}}]
	set file [fileselect "Source file ..." "$env(DIS_INSTALL)/.tcl"]
	if { $file == "" } { 
	status_line "Source Cancelled"
	return 0 
    }
    set ext [file extension $file]
    if {$ext == ".dat"} {
	set command  [list rebuild_customized_menu $file]
    } else {
	set command [list source $file]
    }
    set res [catch $command msg]
    if !$res {
	set msg "Sourced $file"
    }
    status_line $msg

    return $res
}

proc set_service_tag {{init 1}} {
    global Service Model_name env

    set st [string first "@" $Service]
    incr st
    set Model_name [string range $Service $st end]
    regexp {^[a-zA-Z0-9]+} $Model_name Model_name

    if $init {
	init $env(HOME)/dislite/$Model_name/$Model_name.prefs
    }
}



if [info exists env(DIS_DEBUG)] {
    global debug_output
    if [info exists debug_output] {return }
    set debug_output [open $env(DIS_DEBUG) "w"]

    proc debug_var var {
	global debug_output
	upvar $var ref
	puts $debug_output "*** $var:"
	puts $debug_output [join $ref "\n"]
	flush $debug_output
    }
    proc debug_msg msg {
	global debug_output
	puts $debug_output $msg
	flush $debug_output
    }
} else {
    proc debug_var var {}
    proc debug_msg msg {}
}
