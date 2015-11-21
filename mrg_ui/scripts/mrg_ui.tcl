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
# callback procedures
proc previous_cb {} {
    global conflictsOnly
    if {$conflictsOnly} {
	showPrevConflict
    } else {
	showPrevChange
    }
}
proc next_cb  {} {
    global conflictsOnly
    if {$conflictsOnly} {
	showNextConflict
    } else {
	showNextChange
    }
}

proc toggle_conflicts_cb {} {
    global conflictsOnly
    ## conflictsOnly = 1 - conflictsOnly;  Has already been toggled.
    if {$conflictsOnly == 1} {
	focusOnConflict 1
    }
}

proc cutMenu {} {
    event generate [focus] <<Cut>>
}
proc pasteMenu {} {
    event generate [focus] <<Paste>>
}
proc copyMenu {} {
    if {[selection own] != ""} {
        event generate [selection own] <<Copy>>
    } else {
        event generate [focus] <<Copy>>
    }
}

# Choose Dialog's Merge button has been pressed.
proc merge_cb {} {
    set count [selectAll]
    if {$count == 0} {
	tk_messageBox -icon error -type ok -title Notice -message "At least one change file should be selected."
    } else {
        global dialog
        setDialogCommand merge
        destroy $dialog
    }
}

# Choose Dialog's View/Change button has been pressed.
proc change_cb {} {
    set count [buildSelectionList]
    if {$count != 1} {
	tk_messageBox -icon error -type ok -title Notice -message "Only one change can be modified/viewed."
    } else {
        setDialogCommand edit
    }
}

proc addChangeInfoWindow {name} {
    frame $name
    frame $name.changeText
    text $name.changeText.textField -height 4 -width 160
    $name.changeText.textField configure -yscrollcommand "$name.changeText.scroll set"
    scrollbar $name.changeText.scroll -command "$name.changeText.textField yview"
    pack $name.changeText.scroll -side right -fill y
    pack $name.changeText.textField -fill both
    pack $name.changeText -side bottom
    label $name.editorLabel
}

proc removeChangeInfoWindow {name} {
    destroy $name
}

proc createMainDialog {} {

    # Don't use the root for the main dialog, because the parent-child
    # relationship interferes with the stacking behavior of the 2 dialogs.
    set mainName ".main"
    set main ".main"
    toplevel $mainName

    # Specify a requested size for the main dialog window, and don't let the slaves take it over.
    pack propagate $mainName 0
    wm geometry $mainName =850x600

    wm title $mainName "mrg_ui"

    #### menu bar ####

    menu $main.menu -tearoff 0
    ## "File" menu:
    menu $main.menu.file -tearoff 0
    $main.menu add cascade -label "File" -menu $main.menu.file -underline 0
    $main.menu.file add command -label "Cancel Current Merge" -command cancelMerge
    $main.menu.file add command -label "Exit" -command exit
    ## "Edit" menu:
    menu $main.menu.edit -tearoff 0
    $main.menu add cascade -label "Edit" -menu $main.menu.edit -underline 0
    $main.menu.edit add command -label "Cut" -command cutMenu
    $main.menu.edit add command -label "Paste" -command pasteMenu
    $main.menu.edit add command -label "Copy" -command copyMenu
    $mainName configure -menu $main.menu

    #### label & push button bar ####

    frame $main.buttonBar
    pack $main.buttonBar -side top -fill x

    # frame $main.buttonBar.gap
    # pack $main.buttonBar.gap -expand 1 -side left

    ## Conflicts Only toggle checkbox
    checkbutton $main.buttonBar.conflictsOnlyToggle -text "Conflicts only" -variable conflictsOnly -command toggle_conflicts_cb

    ## "Previous" button
    button $main.buttonBar.previous -text "<< Previous" -command previous_cb

    ## "Next" button
    button $main.buttonBar.next -text "Next >>" -command next_cb

    ## "Current" button
    button $main.buttonBar.current -text "Current" -command showCurrentChange

    ## "Accept" button
    button $main.buttonBar.accept -text "Accept" -command acceptChanges

    ## "Editor" button
    button $main.buttonBar.editor -text "Editor" -command invokeEditor

    # Now post those things on the right side of the buttonbar:
    pack $main.buttonBar.editor $main.buttonBar.accept $main.buttonBar.current $main.buttonBar.next $main.buttonBar.previous $main.buttonBar.conflictsOnlyToggle -padx 3 -side right

    ## Source file label
    label $main.buttonBar.sourceFileLabel -text "No file yet"
    pack $main.buttonBar.sourceFileLabel -side left -anchor sw -expand 1

    #### text editors ####
    ## original_text_driver
    frame $main.original
    text $main.original.text -yscrollcommand "$main.original.scroll set" -height 8
    scrollbar $main.original.scroll -command "$main.original.text yview"
    pack $main.original.scroll -side right -fill y
    pack $main.original.text -expand yes -fill both
    pack $main.original -expand yes -fill both
    $main.original.text insert end \
    {This window displays the original text.}
    $main.original.text configure -state disabled

    ## modified_text_driver
    frame $main.modified
    text $main.modified.text -yscrollcommand "$main.modified.scroll set" -height 8
    scrollbar $main.modified.scroll -command "$main.modified.text yview"
    pack $main.modified.scroll -side right -fill y
    pack $main.modified.text -expand yes -fill both
    pack $main.modified -expand yes -fill both
    $main.modified.text insert end \
    {This window displays the modified text.}

    #### container ####
    global containerWin
    set containerWin $main.container
    frame $containerWin
    pack $containerWin -fill x

    ## line -- What becomes of this

    #### info label ####
    frame $main.infoBar
    pack $main.infoBar -fill x

    label $main.infoBar.infoLabel -text "No change info yet"
    pack $main.infoBar.infoLabel -anchor w
}

# DiffChooseDialog
proc createChooseDialog {dname} {

    global dialog
    set dialog $dname
    toplevel $dialog

    # Specify a requested size for the choose dialog window, and don't let the slaves take it over.
    pack propagate $dialog 0
    $dialog configure -width 600 -height 500

    wm title $dialog "mrg_ui"

    ## upper section holds lists
    set listsName "$dialog.lists"
    frame $listsName

    set sourceName "$listsName.source"
    frame $sourceName
    set gap "$listsName.gap"
    frame $gap
    set changesName "$listsName.changes"
    frame $changesName
    pack $sourceName -side left -expand 1 -fill both
    pack $gap -side left -fill y -ipadx 2m
    pack $changesName -side left -expand 1 -fill both

    ## Sources label
    label $sourceName.label -text "Sources:"
    pack $sourceName.label -side top -anchor sw

    ## Sources list
    listbox $sourceName.list -yscrollcommand "$sourceName.scroll set" -selectmode single
    bind $sourceName.list <ButtonRelease> sourceSelectObserver
    pack $sourceName.list -side left -expand 1 -fill both
    scrollbar $sourceName.scroll -command "$sourceName.list yview"
    pack $sourceName.scroll -side right -fill y

    ## Changes label
    label $changesName.label -text "Changes:"
    pack $changesName.label -side top -anchor sw

    ## Changes list
    listbox $changesName.list -yscrollcommand "$changesName.scroll set" -selectmode single
    bind $changesName.list <ButtonRelease> diffSelectObserver
    pack $changesName.list -side left -expand 1 -fill both
    scrollbar $changesName.scroll -command "$changesName.list yview"
    pack $changesName.scroll -side right -fill y

    ## Change Information label
    label $dialog.infoLabel -text "Change Information:"

    ## Change Info text   
    ## Helvetical bold 12??
    text $dialog.info -height 8
    $dialog.info insert end \
    {This window displays some information about the change.}
    $dialog.info configure -state disabled

    #### label & push button bar ####

    frame $dialog.buttonBar

    ## "Merge" button
    button $dialog.buttonBar.merge -text "Merge" -command merge_cb

    ## "View/Change" button
    button $dialog.buttonBar.change -text "View/Change" -command change_cb

    ## "Exit" button
    button $dialog.buttonBar.exit -text "Exit" -command exit

    pack $dialog.buttonBar.merge $dialog.buttonBar.change $dialog.buttonBar.exit -side left -expand 1

    pack $listsName -side top -fill both -expand 1
    pack $dialog.infoLabel -side top -anchor sw
    pack $dialog.info -side top -fill x
    pack $dialog.buttonBar -side top -fill x

    bind $dialog <Destroy> {noteDestroy %W}

    # These 4 lines are necessary to _really_ raise the dialog.
    catch {grab $dialog}
    catch {tkwait visibility $dialog}
    catch {grab release $dialog}
    raise $dialog

    focus $dialog
}

proc noteDestroy {win} {
    global dialog
    if {$win == $dialog} {
	# Here we need to check in case the window was destroyed
        # from the Close menu item, and if so there is no Merge or View
        # callback so we must exit because we don't want to leave
        # the user with a raw main dialog.
    } 
}

# For an index into a text widget, return its offset in
# chars from the beginning of the buffer.
# Finds the value by binary search.  Binary search is pretty efficient,
# but there should be a direct way in Tk.  Examining the documentation
# for the Tk text widget didn't turn it up.
proc indexToOffset {win idx} {
    set offset 0
    set prev -1
    while { [$win compare "1.0 + $offset chars" < $idx] } {
        set prev $offset
	set offset [expr $offset * 2 + 1]
    }
    # $prev < $idx and $idx <= $offset
    # $offset - $prev is a power of 2
    while { 1 } {
	if { $offset - $prev == 1 } {
	    return $offset;
	}
	set test [expr ($offset + $prev) / 2]
	if { [$win compare "1.0 + $test chars" < $idx] } {
	    set prev $test
	} else {
	    set offset $test
	}
    }
}

proc tagExists {win tag} {
    set tags [$win tag names]
    set found 0
    foreach t $tags {
	if {$t == $tag} {
	    set found 1
	}
    }
    return $found
}

proc makeTag {win tag beginning end} {
    $win tag add $tag "1.0 + $beginning chars" "1.0 + $end chars"
}
