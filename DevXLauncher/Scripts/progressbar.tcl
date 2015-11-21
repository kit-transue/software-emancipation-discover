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
option add *Progress.undoneForeground black  widgetDefault
option add *Progress.undoneBackground white widgetDefault
option add *Progress.doneForeground   white widgetDefault
option add *Progress.doneBackground   black widgetDefault
option add *Progress.borderWidth      1      widgetDefault
option add *Progress.relief           sunken widgetDefault

namespace eval dkfprogress {
    namespace export Progress SetProgress

    proc Progress {w args} {
	uplevel 1 [list frame $w -class Progress] $args

	foreach {val} {
	    undoneForeground doneForeground
	    undoneBackground doneBackground
	} {
	    set class [string toupper [lindex $val 0]\
		    ][string range $val 1 end]
	    set $val [option get $w $val $class]
	}

	set varname [namespace current]::progressPercent($w)

	frame $w.l -borderwidth 0 -background $undoneBackground
	label $w.l.l -textvariable $varname -borderwidth 0 \
		-foreground $undoneForeground -background $undoneBackground
	$w.l configure -height [expr {int([winfo reqheight $w.l.l]+2)}]
	frame $w.l.fill -background $doneBackground
	label $w.l.fill.l -textvariable $varname -borderwidth 0 \
		-foreground $doneForeground -background $doneBackground

	bind $w.l <Configure> [namespace code [list ProgressConf $w "%w"]]

	pack $w.l -fill both -expand 1
	place $w.l.l -relx 0.5 -rely 0.5 -anchor center
	place $w.l.fill -x 0 -y 0 -relheight 1 -relwidth 0
	place $w.l.fill.l -x 0 -rely 0.5 -anchor center

	SetProgress $w 0
	return $w
    }

    proc ProgressConf {w width} {
	place conf $w.l.fill.l -x [expr {int($width/2)}]
    }

    proc SetProgress {win value {range 100}} {
	set progress [expr {int(100*$value)/int($range)}]
	set relwidth [expr {double($value)/double($range)}]

	variable progressPercent
	place conf $win.l.fill -relwidth $relwidth
	set progressPercent($win) "${progress}%"
    }
}
# ----------------------------------------------------------------------
