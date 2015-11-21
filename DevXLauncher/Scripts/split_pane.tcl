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

proc Pane_Create {f1 f2 args} {

	set t(-percent) 0.5
	set t(-in) [winfo parent $f1]
	array set t $args

	# Keep state in an array associated with the master frame
	set master $t(-in)
	upvar #0 Pane$master pane
	array set pane [array get t]

	
	set pane(1) $f1
	set pane(2) $f2
	
    
        if [catch {set pane(grip) [frame $master.grip -background gray50 \
		-width 2 -cursor sb_h_double_arrow]}] {
	    set pane(grip) [frame $master.grip -background #888 \
		    -width 2 -cursor sb_h_double_arrow ]
	}
 
	place $pane(1) -in $master -relx 0.0 -y 0 -anchor nw \
			-relheight 1.0 -width -1
	place $pane(2) -in $master -relx 1.0 -y 0 -anchor ne \
			-relheight 1.0 -width -1
	place $pane(grip) -in $master -anchor c -relheight 2.0 

#	$master configure -background black

	bind $master <Configure> [list PaneGeometry $master]
	bind $pane(grip) <ButtonPress-1> \
		[list PaneDrag $master %X]
	bind $pane(grip) <B1-Motion> \
		[list PaneDrag $master %X]
	bind $pane(grip) <ButtonRelease-1> \
		[list PaneStop $master]

	PaneGeometry $master
}


proc PaneDrag {master D} {
	upvar #0 Pane$master pane
	if [info exists pane(lastD)] {
		set delta [expr double($pane(lastD) - $D) / $pane(size)]
		set pane(-percent) [expr $pane(-percent) - $delta]
		if {$pane(-percent) < 0.05} {
			set pane(-percent) 0.05
		} elseif {$pane(-percent) > 0.95} {
			set pane(-percent) 0.95
		}
		# PaneGeometry $master
		place $pane(grip) -relx $pane(-percent)
	}
	set pane(lastD) $D
}

proc PaneStop {master} {
	upvar #0 Pane$master pane
        PaneGeometry $master
	catch {unset pane(lastD)}
}

proc PaneGeometry {master} {
	upvar #0 Pane$master pane
	place $pane(1) -relwidth $pane(-percent)
	place $pane(2) -relwidth [expr 1.0 - $pane(-percent)]
	place $pane(grip) -relx $pane(-percent) 
	set pane(size) [winfo width $master]
}














