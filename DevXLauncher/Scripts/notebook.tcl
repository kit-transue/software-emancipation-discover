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
# Doesn't handle focus all that well yet.  Not totally bust though...

namespace eval notebook {
    # Start by configuring the "compiled-in" default for the label to
    # look like the compiled-in defaults for the buttons, making them
    # default to something not entirely daft in the process...
    foreach {opt val} {
	highlightThickness 0
	borderWidth        0
	relief             raised
	padX               3
	padY               3
    } {
	option add *NoteBook.Label.$opt $val 30
    }

    variable re {([0-9]+)x([0-9]+)\+(-?[0-9]+)\+(-?[0-9]+)}
    proc drawPoly {w} {
	variable buttonList
	variable paneList
	variable canvas
	variable index
	variable line
	variable text
	variable re

	set p [lindex $paneList($w)   $index($w)]
	set b [lindex $buttonList($w) $index($w)]

	regexp $re [winfo geometry $p] dummy pw ph px py
	# X-Coords
	set pl [expr {$px-1}]
	set pr [expr {$px+$pw}]
	# Y-Coords
	set pt [expr {$py-1}]
	set pb [expr {$py+$ph}]

	set i 0
	set c $canvas($w)
	foreach b $buttonList($w) l $line($w) t $text($w) {
	    foreach {p l1 l2} $l {break}
	    regexp $re [winfo geometry $b] dummy bw bh bx by
	    # X-Coords
	    set bl [expr {$bx-6}]
	    set br [expr {$bx+$bw}]
	    # Y-Coords
	    set bt [expr {$by-6}]
	    set bb [expr {$by+$bh}]

	    if {$i==$index($w)} {
		$c coords $p \
			$pl $pb  $pl $pt  $bl $pt  $bl $bt  $br $bt \
			$br $pt  $pr $pt  $pr $pb  $pl $pb
		$c coords $l1 [expr {$pl+1}] $pb $pr $pb $pr [expr {$pt+1}]
		$c coords $l2 $br [expr {$pt+1}] $br [expr {$bt+1}]
	    } else {
		foreach {v +} {bl 5 br 5 bt 5 bb 6} {
		    incr $v ${+}
		}
		$c coords $p \
			$bl $bb  $bl $bt  $br $bt  $br $bb  $bl $bb
		$c coords $l1 [expr {$bl+1}] $bb $br $bb $br [expr {$bt+1}]
		$c coords $l2 -10 -10 -20 -20
	    }
	    $c coords $t [expr {($bl+$br)/2}] [expr {($bt+$bb)/2}]
	    incr i
	}
	for {incr i -1} {$i>=0} {incr i -1} {
	    $c raise note$i
	}
	$c raise note$index($w)
    }

    proc selectLabel {w idx} {
	variable labelList
	variable paneList
	variable index
	set index($w) $idx
	focusPane $w $idx
	raise [lindex $paneList($w) $idx]
	drawPoly $w
    }

    proc activeLabel {w} {
	    variable index
	    return $index($w)
    }


    proc focusIn {parent idx} {
	variable focus
	set focus($parent) $idx
    }
    proc focusOut {parent idx} {
	variable focus
	set focus($parent) {}
    }
    proc focusPane {parent idx} {
	variable focus
	variable paneList
	if {$focus($parent) != {} && $idx != $focus($parent)} {
	    set focus($parent) {}
	    focus [lindex $paneList($parent) $idx]
	}
    }

    proc makeButtons {parent labels} {
	variable buttonList
	variable labelList
	variable paneList
	variable canvas
	variable line
	variable text

	set labelList($parent) $labels
	set i 0; set j 0
	set bg [$canvas($parent) cget -bg]
	set dark  [tkDarken $bg 50]
	set light [tkDarken $bg 133]
	foreach l $labels {
	    grid column $parent $i -minsize 7
	    incr i
	    set b [label $parent.__b$j -text $l]
	    lappend buttonList($parent) $b

	    set pid [$canvas($parent) create polygon -10 -10 -20 -20 -20 -10 \
		    -fill $bg  -outline $light  -tags note$j]
	    set lid1 [$canvas($parent) create line -10 -10 -20 -20 \
		    -fill $dark  -tags note$j]
	    set lid2 [$canvas($parent) create line -10 -10 -20 -20 \
		    -fill $dark  -tags note$j]
	    lappend line($parent) [list $pid $lid1 $lid2]

	    set tid [$canvas($parent) create text -10 -10  -text $l \
		    -fill [$b cget -fg]  -font [$b cget -font]  -tags note$j]
	    lappend text($parent) $tid
	    $canvas($parent) bind note$j <1> \
		    [namespace code [list selectLabel $parent $j]]

	    lower $b
	    grid $b -row 1 -column $i -sticky ns
	    set f [frame $parent.__f$j -class NoteBookLeaf]
	    bind $f <FocusOut> [namespace code [list focusOut $parent $j]]
	    bind $f <FocusIn>  [namespace code [list focusIn  $parent $j]]
	    lappend paneList($parent) $f
	    incr i; incr j
	}
	grid column $parent $i -weight 1
	grid row    $parent 2  -weight 1
	incr i
	foreach p $paneList($parent) {
	    grid $p -row 2 -column 0 -columnspan $i -sticky nsew \
		    -padx 2 -pady 4
	}
    }

    proc cleanup {w} {
	variable buttonList
	variable labelList
	variable paneList
	variable canvas
	variable focus
	variable index
	variable line
	variable text
	unset buttonList($w)
	unset labelList($w)
	unset paneList($w)
	unset canvas($w)
	unset focus($w)
	unset index($w)
	unset line($w)
	unset text($w)
    }

    proc getNote {w label} {
	variable labelList
	variable paneList

	set idx [lsearch -exact $labelList($w) $label]
	if {$idx < 0} {
	    return -code error "unknown label \"$label\""
	}
	return [lindex $paneList($w) $idx]
    }

    proc notebook {w args} {
	variable buttonList
	variable labelList
	variable paneList
	variable canvas
	variable focus
	variable line
	variable text

	set buttonList($w) {}
	set labelList($w)  {}
	set paneList($w)   {}
	set canvas($w)     {}
	set focus($w)      {}
	set line($w)       {}
	set text($w)       {}

	frame $w -class NoteBook
	grid row $w 0 -minsize 7
	set canvas($w) [canvas $w.__canv -width 1 -height 1]
	grid $canvas($w) -rowspan 3 -sticky nsew \
		-columnspan [expr {[llength $args]*2+2}]
	makeButtons $w $args
	selectLabel $w 0
	bind $canvas($w) <Configure> [namespace code [list drawPoly $w]]
	bind $w <Destroy> [namespace code [list cleanup $w]]
	return $w
    }

    namespace export getNote notebook selectLabel activeLabel
}
namespace import notebook::*

# -----------------------------------------------------------------------
# Demo code
#

proc do_submitcheck {} {
    global oldfocus listfont
    set w .submitcheck
    toplevel $w -class Dialog
    wm geometry $w =70x30
    wm title $w "Submission Check"
    set note $w.note
    pack [notebook $note Files Result] \
	    -expand 1 -fill both -padx 1m -pady 1m
    
    set filespage [getNote $note Files]   
    listbox $filespage.list -font $listfont -yscrollcommand "$filespage.scroll set" -selectmode extended 
    scrollbar $filespage.scroll -command "$filespage.list yview"
    
    pack $filespage.scroll -in $filespage -side right -fill y 
    pack $filespage.list -in $filespage -side left -fill both -expand true

    set resultpage [getNote $note Result]
    text $resultpage.text -relief sunken -bd 2 -font $listfont -setgrid 1 -insertofftime 0 -yscrollcommand "$resultpage.scroll set"
    scrollbar $resultpage.scroll -command "$resultpage.text yview"
    pack $resultpage.scroll -in $resultpage -side right -fill y
    pack $resultpage.text -in $resultpage -expand yes -fill both
    
    set f [frame $w.buttom]
    pack $f -side bottom
    eb_button $f.start start  raised {} 10
    eb_button $f.cancel  cancel raised {} 10
    eb_button $f.browseall "brow. all" raised {} 10
    eb_button $f.browsenew "brow. new" raised {} 10
    eb_button $f.submit  submit raised {selectLabel .submitcheck.note 1} 10
    eb_button $f.close close raised {destroy .submitcheck} 10
    pack $f.start $f.cancel $f.browseall $f.browsenew $f.submit $f.close -side left -padx 3 -pady 8

    set oldfocus [focus]

    catch {grab $w}
    catch {tkwait visibility $w}
    catch {grab release $w}
    raise $w
    focus $w.note
    update
}

