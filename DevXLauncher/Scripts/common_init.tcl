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
source $env(DIS_INSTALL)/filecomplete.tcl
# This file includes common initialization codes for both ServiceChooser
# dialogbox and dev. express main browser.

# unix - 1 , windows - 0

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

proc isunix {} {global unix_platform; return $unix_platform}

proc tempdirectory_init {} {
    global Tempdir Transform env
    
    if {![info exists Tempdir] || $Tempdir=="" || ![file exists $Tempdir]} {
	if [isunix] {
            if [info exists env(TMPDIR)] {
                set Tempdir $env(TMPDIR)
            } elseif [info exists env(TMP)] {
                set Tempdir $env(TMP)
            } else {
                set Tempdir "/usr/tmp"
            }
	} else {
            if [info exists env(TEMP)] {
                set Tempdir $env(TEMP)
            } elseif [info exists env(TMP)] {
                set Tempdir $env(TMP)
            } else {
                set Tempdir "C:/Temp"
            }
            regsub -all {\\} $Tempdir / Tempdir
	}
    }
    if {$Transform==2} {
	if [isunix] {
	    cd $Tempdir
	    set Tempdir [pwd]
	    cd $env(DIS_INSTALL)
	} else {
	    set Tempdir [string toupper $Tempdir]
	}
    }
}

array set PrefsTemplate {
    CM  0
    Transform 1
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

if [isunix] {
   set CMInfo [list "No CM"  ClearCase RCS SCCS]
   set EditorInfo [list Emacs Xemacs VIM Viper]	
} else {
   set CMInfo [list "No CM"  ClearCase SourceSafe Continuus CustomCM]
   set EditorInfo [list MSDev Emacs VIM] 		
}	

proc init_setting {init_file} {
    global Service Tempdir ModelSrcRoot TransformLabel CM CMLabel CMInfo
    global SharedSrcRoot PrivateSrcRoot Transform startup env
    global CurrentScope scope_flag EditorLabel EditorInfo Editor

    if {[info exists env(default_editor)] && $env(default_editor)!=""} {
	set default_ed $env(default_editor)
    } else {
	set default_ed 0
    }

    if {[file exists $env(HOME)/dislite/editor.prefs]} {
	set f [open $env(HOME)/dislite/editor.prefs]
	if {[gets $f Editor] <= 0} {
	    set Editor $default_ed
	}
	close $f
    } else {
	set Editor $default_ed
    }

    read_pref_file $init_file

    switch -exact $Transform {
	0 {set TransformLabel Transform}
	1 {set TransformLabel "No Transform"}
	2 {set TransformLabel "Server Source"}
    }
    set CMLabel [lindex $CMInfo $CM]
	set EditorLabel [lindex $EditorInfo $Editor]
    
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

set Psethome $env(PSETHOME)
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
	    set res "{Current selection doesn't contain any symbols}"
	}
	INST_NO_DEF {
	    set res "{Current selection doesn't have definition file}"
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
	    set res "The current selection doesn't have definition file"
	}
	SYM_NO_FNAME {
	    set res "This symbol doesn't have physical file name"
	}
	default {
	    set res ""
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
