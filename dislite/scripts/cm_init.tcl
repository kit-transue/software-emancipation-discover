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
#
# This file 'cm_init.tcl' only contains procedures
#     that are needed on initial phase.
# The "main cm-stuff" is placed to 'cm_generic.tcl'.
# So it is loaded when necessary.
#


# Global variables:
set CmDish             ""
set CmSystemStr        ""
set CmSystemList       ""
set CmSystemListInited 0
set CmInited           0


proc cm_init {} {

    global Psethome CmDish CmInited

    if {$CmInited == 0} {
	
	if ![isunix] {
		exec "$Psethome/bin/cmdriver.exe" &
	}

	# This is program, which helps us to communicate with the 'cmdriver'.
	set CmDish "$Psethome/bin/cmdish"

	# Wait until the cmdriver starts.
	set errc [catch {set res [exec $CmDish -query wait]} res]

	# If error -> show message in the dialog.
	if $errc {

	    cm_show_error_message $res

	} else {

	    set CmInited 1
	}
    }

} ;# cm_init


proc cm_get_systems {} {

    global CmDish CmSystemStr CmSystemList CmInited CmSystemListInited

    if {$CmSystemListInited == 0} {

	if {$CmInited == 0} {

		set CmSystemStr  ""
		set CmSystemList {"No CM"}

	} else {

	    # Try to get name of all supported CM systems from the 'cmdriver'.
	    set errc [catch {set res [exec $CmDish -query get_cm_systems]} res]

	    # If error -> show message in the dialog.
	    if $errc {

		cm_show_error_message $res

		set CmSystemStr  ""
		set CmSystemList {"No CM"}

	    } else {

		# Make list of supported CM systems.
		set CmSystemStr  $res
		set CmSystemList [split $CmSystemStr "\n"]
		set CmSystemList [linsert $CmSystemList 0 "No CM"]
	    }
	}

	set CmSystemListInited 1
    }

    return $CmSystemList

} ;# cm_get_systems


proc cm_get {comment files} {

    global CM
    global CmInited
    global CmCmdKeyList
    global CmAttrValueArr

    # CM has not been initialized --> can't process file(s).
    if {$CmInited == 0} {
	return -1
    }

    # No CM is selected --> can't process file(s).
    if {$CM == 0} {
	return -1
    }

    # Check existance of the given command.
    if {[lsearch -exact $CmCmdKeyList "autoco"] == -1} {
	return -1
    }

    # Replace current comment in the global array.
    set CmAttrValueArr(autocomment) $comment

    # Let's process automatically all given files.
    set ret_code [cm_process_items "autoco" $files]

    # Propagate error code further.
    return $ret_code

} ;# cm_get


proc cm_put {comment files} {

    global CM
    global CmInited
    global CmCmdKeyList
    global CmAttrValueArr

    # CM has not been initialized --> can't process file(s).
    if {$CmInited == 0} {
	return -1
    }

    # No CM is selected --> can't process file(s).
    if {$CM == 0} {
	return -1
    }

    # Check existance of the given command.
    if {[lsearch -exact $CmCmdKeyList "autoci"] == -1} {
	return -1
    }

    # Replace current comment in the global array.
    set CmAttrValueArr(autocomment) $comment

    # Let's process automatically all given files.
    set ret_code [cm_process_items "autoci" $files]

    # Propagate error code further.
    return $ret_code

} ;# cm_put


proc cm_get_view {} {

    global CmInited CMLabel

    # CM has not been initialized --> can't get current view.
    if {$CmInited == 0} {
	return ""
    }

    # This command works in ClearCase only.
    # It is needed in submission check, which works with ClearCase only.
    if {[string compare $CMLabel "ClearCase"] != 0} {
	return ""
    }

    # Run external utility.
    set errc [catch {set res [exec cleartool pwv -short -set]} res]

    # If error occured or view is not set.
    if {$errc != 0 || [string compare $res "** NONE **"] == 0} {
	return ""
    }

    return $res

} ;# cm_get_view


array set integers { 1 1 2 2 3 3 4 4 5 5 6 6 7 7 8 8 9 9 }

# This procedure originally was in 'cm.tcl'.
proc cm_map { f1 f2 map } {
    global env integers
    upvar #0 $map this
    set diff "$env(PSETHOME)/bin/diff"
    regsub -all {\\} $diff {/} diff
    if { [ regexp {[ ]} $diff ] } {
        set diff "\"$diff\""
    }
    set f [open "| $diff -w $f1 $f2"]
    while {![eof $f]} {
	set line [gets $f]
	set c [string index $line 0]
	if {[info exists integers($c)]} {
	    if [regexp {(.*)([ac])(.*)} $line m x y z ] {
		set zz [split $z ,]
		if {[llength $zz] == 1} {
		    set start $zz
		    set end $zz
		} else {
		    set start [lindex $zz 0]
		    set end [lindex $zz 1]
		}
		for {set i $start} {$i <= $end} {incr i } { 
		    set this($f2:$i) $y
		}
	    } 
	}
    }
    catch {close $f}
} ;# cm_map


proc cm_show_error_message {msg} {

    # Sort out '\nchild process exited abnormally' sentence from message.
    # This string was added by Tcl/Tk.
    if { [regexp {^(.*)\nchild process exited abnormally} $msg dummy our]} {

	set res $our

    } else {

	set res $msg
    }

    # Sort out 'Error\n' string, which was added in cmdriver.
    if { [regexp {^Error\n(.*)} $res dummy our]} {

	set res $our
    }

    # If we have meaningfull error message,
    #   i.e. not only spaces, tabs, new lines.
    if { [string length $res] > 0 && [regexp {^[\n \t]*$} $res] == 0 } {

	# Show specific error message in the dialog.
	tk_messageBox -title "CM Error" -message $res -type ok -icon error
    }
} ;# cm_show_error_message


# END OF FILE

