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
# substitute your favorite method here...
source mclistbox.tcl
source progressbar.tcl
package require mclistbox 1.02
catch {namespace import mclistbox::*}
catch {namespace import dkfprogress::*}


proc do_submitcheck {} {

    if {[winfo exists .sch]} {
	tk_messageBox -title "Submission Check" -message "Only one copy of the submission check window can be opened." -type ok -icon error 
	return
    }

    global sch_files sch_filelist 
    global sch_unparselist
    set arglist [dis_prepare_selection]
    set error 0
    if {[llength $arglist]<1} { 
	set msg "Please select files for submission check."
	set error 1
    } else {
	set sch_files [dis_cmd "apply fname $arglist"]
	if {[llength $sch_files]==1} {
	    set sch_files [lindex $sch_files 0]
	} 
	debug_var sch_files
	set len [llength $sch_files]
	set sch_files [dis_print $sch_files lname]
	if { $len == 0 || [llength $sch_files] != $len } {
	    set msg "One or more of the files selected could not be converted to its logical name."
	    set error 1
	}
    }

    if $error {
	    tk_messageBox -title Message -message $msg -type ok -icon error 
	    return
    } 

    if ![winfo exists .sch] {
	    destroy .sch
    }
    sch_build

   foreach el $sch_files {
       $sch_filelist insert end [ list $el ]
       $sch_unparselist insert end [ list $el ]
   }
}


##########################################################################################################################
# Modified Page toolbar buttons callbacks and service functions
##########################################################################################################################
proc sch_new_objects {} {
global sch_modifiedlist
global added_objects

   set w .sch

   set note $w.note
   set modifiedpage [getNote $note {Modified Objects}]   
   $modifiedpage.toolbar.new config -relief sunken
   $modifiedpage.toolbar.modified config -relief raised
   $modifiedpage.toolbar.removed config -relief raised


   $sch_modifiedlist delete 0 end   
   global sch_arr
   foreach object $added_objects {
	    if [info exists sch_arr([lindex $object 0])] {
		    set lname [lindex $sch_arr([lindex $object 0]) 2]
            $sch_modifiedlist insert end [list [file tail $lname] [lindex $object 3] [lindex $object 1] [lindex $object 2]]
		}
   }
   global modified_page
   set modified_page {Added}
}
    
proc sch_removed_objects {} {
global sch_modifiedlist
global removed_objects

   set w .sch

   set note $w.note
   set modifiedpage [getNote $note {Modified Objects}]   
   $modifiedpage.toolbar.new config -relief raised
   $modifiedpage.toolbar.modified config -relief raised
   $modifiedpage.toolbar.removed config -relief sunken


   $sch_modifiedlist delete 0 end   
   global sch_arr
   foreach object $removed_objects {
	    if [info exists sch_arr([lindex $object 0])] {
		    set lname [lindex $sch_arr([lindex $object 0]) 2]
            $sch_modifiedlist insert end [list [file tail $lname] [lindex $object 3] [lindex $object 1] [lindex $object 2]]
		}
   }
   global modified_page
   set modified_page {Removed}
}


proc sch_modified_objects {} {
global sch_modifiedlist
global modified_objects

   set w .sch

   set note $w.note
   set modifiedpage [getNote $note {Modified Objects}]   
   $modifiedpage.toolbar.new config -relief raised
   $modifiedpage.toolbar.modified config -relief sunken
   $modifiedpage.toolbar.removed config -relief raised
   $modifiedpage.toolbar.new config -relief raised
   $modifiedpage.toolbar.modified config -relief sunken
   $modifiedpage.toolbar.removed config -relief raised

   $sch_modifiedlist delete 0 end   
   global sch_arr
   foreach object $modified_objects {
	    if [info exists sch_arr([lindex $object 0])] {
		    set lname [lindex $sch_arr([lindex $object 0]) 2]
            $sch_modifiedlist insert end [list [file tail $lname] [lindex $object 3] [lindex $object 1] [lindex $object 2]]
		}
   }
   global modified_page
   set modified_page {Modified}
}
###########################################################################################################################


#--------------------------------------------------------------------------------------------------------------------------
# This procedure will build the hits page of the notebook. Hits page will contain spreadsheet for the hits and 
# toolbar with three buttons : "New Hits", "All Hits" and "Browse"
#--------------------------------------------------------------------------------------------------------------------------
proc make_hitspage {} {
global sch_hitslist

    set w .sch
    set note $w.note
    set hitspage [getNote $note Hits]

    # we want the listbox and two scrollbars to be embedded in a 
    frame $hitspage.spreadsheet -bd 2 -relief sunken

    # frame so they look like a single widget
    scrollbar $hitspage.vsb -orient vertical -command [list $hitspage.listbox yview]
    scrollbar $hitspage.hsb -orient horizontal -command [list $hitspage.listbox xview]

    # we will purposefully make the width less than the sum of the
    # columns so that the scrollbars will be functional right off
    # the bat.
    mclistbox $hitspage.listbox \
	    -bd 0 \
	    -height 10 \
	    -width 10 \
	    -columnrelief flat \
	    -labelanchor w \
	    -columnborderwidth 0 \
	    -selectcommand "" \
	    -selectmode single \
	    -labelborderwidth 2 \
	    -fillcolumn hit \
	    -xscrollcommand [list $hitspage.hsb set] \
	    -yscrollcommand [list $hitspage.vsb set] \
		-relief flat

    # add the columns we want to see
    $hitspage.listbox column add file -label "File"     -width 12
    $hitspage.listbox column add line -label "Line"     -width 6
    $hitspage.listbox column add hit  -label "Comments"

    # set up bindings to sort the columns.
    #$hitspage.listbox label bind file <ButtonPress-1> "sort %W file"
    #$hitspage.listbox label bind line <ButtonPress-1> "sort %W line"
    #$hitspage.listbox label bind hit  <ButtonPress-1> "sort %W hit"


    grid $hitspage.vsb -in $hitspage.spreadsheet -row 0 -column 1 -sticky ns
    grid $hitspage.hsb -in $hitspage.spreadsheet -row 1 -column 0 -sticky ew
    grid $hitspage.listbox -in $hitspage.spreadsheet -row 0 -column 0 -sticky nsew -padx 0 -pady 0
    grid columnconfigure $hitspage.spreadsheet 0 -weight 1
    grid columnconfigure $hitspage.spreadsheet 1 -weight 0

    grid rowconfigure    $hitspage.spreadsheet 0 -weight 1
    grid rowconfigure    $hitspage.spreadsheet 1 -weight 0

	frame $hitspage.toolbar -relief flat
    eb_button $hitspage.toolbar.all {All Hits} raised {sch_allhits} 10
    pack $hitspage.toolbar.all -in $hitspage.toolbar -side left -padx 1m -pady 1m
    eb_button $hitspage.toolbar.new {New Hits} sunken {sch_newhits} 10
    pack $hitspage.toolbar.new -in $hitspage.toolbar -side left -padx 1m -pady 1m
    eb_button $hitspage.toolbar.browse {Browse} raised {sch_open_hit} 10
    pack $hitspage.toolbar.browse -in $hitspage.toolbar -side right -padx 1m -pady 1m

    pack $hitspage.toolbar -in $hitspage -side bottom -fill both
    pack $hitspage.spreadsheet -in $hitspage -side left -fill both -expand true

    #bind $hitspage.listbox <Button-1> "$old_bind; focus %W"
    bind $hitspage.listbox <Return> sch_open_hit
    bind $hitspage.listbox <Double-Button-1> sch_open_hit    

    set sch_hitslist $hitspage.listbox
}
#--------------------------------------------------------------------------------------------------------------------------


#--------------------------------------------------------------------------------------------------------------------------
# This procedure will build the errors page of the notebook. Hits page will contain spreadsheet for the errors and 
# toolbar with one button : "Browse"
#--------------------------------------------------------------------------------------------------------------------------
proc make_errorspage {} {
global sch_errorslist

    set w .sch
    set note $w.note
    set errorspage [getNote $note Errors]

    # we want the listbox and two scrollbars to be embedded in a 
    frame $errorspage.spreadsheet -bd 2 -relief sunken

    # frame so they look like a single widget
    scrollbar $errorspage.vsb -orient vertical -command [list $errorspage.listbox yview]
    scrollbar $errorspage.hsb -orient horizontal -command [list $errorspage.listbox xview]

    # we will purposefully make the width less than the sum of the
    # columns so that the scrollbars will be functional right off
    # the bat.
    mclistbox $errorspage.listbox \
	    -bd 0 \
	    -height 10 \
	    -width 10 \
	    -columnrelief flat \
	    -labelanchor w \
	    -columnborderwidth 0 \
	    -selectcommand "" \
	    -selectmode single \
	    -labelborderwidth 2 \
	    -fillcolumn comment \
	    -xscrollcommand [list $errorspage.hsb set] \
	    -yscrollcommand [list $errorspage.vsb set] \
		-relief flat

    # add the columns we want to see
    $errorspage.listbox column add file    -label "File"        -width 12
    $errorspage.listbox column add comp    -label "Compile"   -width 12
    $errorspage.listbox column add line    -label "Line"        -width 6
    $errorspage.listbox column add comment -label "Comments"

    # set up bindings to sort the columns.
    #$errorspage.listbox label bind file <ButtonPress-1> "sort %W file"
    #$errorspage.listbox label bind line <ButtonPress-1>  "sort %W line"
    #$errorspage.listbox label bind comment  <ButtonPress-1> "sort %W comment"


    grid $errorspage.vsb -in $errorspage.spreadsheet -row 0 -column 1 -sticky ns
    grid $errorspage.hsb -in $errorspage.spreadsheet -row 1 -column 0 -sticky ew
    grid $errorspage.listbox -in $errorspage.spreadsheet -row 0 -column 0 -sticky nsew -padx 0 -pady 0
    grid columnconfigure $errorspage.spreadsheet 0 -weight 1
    grid columnconfigure $errorspage.spreadsheet 1 -weight 0
    grid rowconfigure    $errorspage.spreadsheet 0 -weight 1
    grid rowconfigure    $errorspage.spreadsheet 1 -weight 0

	frame $errorspage.toolbar -relief flat
    eb_button $errorspage.toolbar.browse {Browse} raised {sch_open_error} 10
    pack $errorspage.toolbar.browse -in $errorspage.toolbar -side right -padx 1m -pady 1m

    pack $errorspage.toolbar -in $errorspage -side bottom -fill both
    pack $errorspage.spreadsheet -in $errorspage -side left -fill both -expand true

    #bind $errorspage.listbox <Button-1> "$old_bind; focus %W"
    bind $errorspage.listbox <Return> sch_open_error
    bind $errorspage.listbox <Double-Button-1> sch_open_error    

    set sch_errorslist $errorspage.listbox
}
#--------------------------------------------------------------------------------------------------------------------------






#--------------------------------------------------------------------------------------------------------------------------
#--------------------------------------------------------------------------------------------------------------------------
proc ArrowUp {} {
global sch_filelist
global sch_unparselist
global sch_hitslist
global sch_modifiedlist
global sch_errorslist 
global sch_text
global sch_report

    set page [activeLabel .sch.note]
    if  { $page == 0 } {
        tkListboxUpDown $sch_filelist -1
    }
    if  { $page == 1 } {
        tkListboxUpDown $sch_unparselist -1
    }
    if  { $page == 2 } {
        tkListboxUpDown $sch_hitslist -1
    }
    if  { $page == 3 } {
        tkListboxUpDown $sch_errorslist -1
    }
    if  { $page == 4 } {
        tkListboxUpDown $sch_modifiedlist -1
    }
    if  { $page == 5 } {
	    $sch_text yview scroll -1 units
    }
    if  { $page == 6 } {
	    $sch_report yview scroll -1 units
    }
}
#--------------------------------------------------------------------------------------------------------------------------


#--------------------------------------------------------------------------------------------------------------------------
#--------------------------------------------------------------------------------------------------------------------------
proc PageUp {} {
global sch_filelist
global sch_unparselist
global sch_hitslist
global sch_modifiedlist
global sch_errorslist 
global sch_text
global sch_report

    set page [activeLabel .sch.note]
    if  { $page == 0 } {
	    $sch_filelist yview scroll -1 pages
	    set index [$sch_filelist index @0,0]
	    $sch_filelist see $index
	    $sch_filelist activate $index
	    $sch_filelist selection clear 0 end
	    $sch_filelist selection anchor $index
	    $sch_filelist selection set $index
    }
    if  { $page == 1 } {
	    $sch_unparselist yview scroll -1 pages
	    set index [$sch_unparselist index @0,0]
	    $sch_unparselist see $index
	    $sch_unparselist activate $index
	    $sch_unparselist selection clear 0 end
	    $sch_unparselist selection anchor $index
	    $sch_unparselist selection set $index
    }
    if  { $page == 2 } {
	    $sch_hitslist yview scroll -1 pages
	    set index [$sch_hitslist index @0,0]
	    $sch_hitslist see $index
	    $sch_hitslist activate $index
	    $sch_hitslist selection clear 0 end
	    $sch_hitslist selection anchor $index
	    $sch_hitslist selection set $index
    }
    if  { $page == 3 } {
	    $sch_errorslist yview scroll -1 pages
	    set index [$sch_errorslist index @0,0]
	    $sch_errorslist see $index
	    $sch_errorslist activate $index
	    $sch_errorslist selection clear 0 end
	    $sch_errorslist selection anchor $index
	    $sch_errorslist selection set $index
    }
    if  { $page == 4 } {
	    $sch_modifiedlist yview scroll -1 pages
	    set index [$sch_modifiedlist index @0,0]
	    $sch_modifiedlist see $index
	    $sch_modifiedlist activate $index
	    $sch_modifiedlist selection clear 0 end
	    $sch_modifiedlist selection anchor $index
	    $sch_modifiedlist selection set $index
    }
    if  { $page == 5 } {
	    $sch_text yview scroll -1 pages
    }
    if  { $page == 6 } {
	    $sch_report yview scroll -1 pages
    }
}
#--------------------------------------------------------------------------------------------------------------------------



#--------------------------------------------------------------------------------------------------------------------------
#--------------------------------------------------------------------------------------------------------------------------
proc ArrowDown {} {
global sch_filelist
global sch_unparselist
global sch_hitslist
global sch_modifiedlist
global sch_errorslist 
global sch_text
global sch_report

    set page [activeLabel .sch.note]
    if  { $page == 0 } {
        tkListboxUpDown $sch_filelist 1
    }
    if  { $page == 1 } {
        tkListboxUpDown $sch_unparselist 1
    }
    if  { $page == 2 } {
        tkListboxUpDown $sch_hitslist 1
    }
    if  { $page == 3 } {
        tkListboxUpDown $sch_errorslist 1
    }
    if  { $page == 4 } {
        tkListboxUpDown $sch_modifiedlist 1
    }
    if  { $page == 5 } {
	    $sch_text yview scroll 1 units
    }
    if  { $page == 6 } {
	    $sch_report yview scroll 1 units
    }
}
#--------------------------------------------------------------------------------------------------------------------------


#--------------------------------------------------------------------------------------------------------------------------
#--------------------------------------------------------------------------------------------------------------------------
proc PageDown {} {
global sch_filelist
global sch_unparselist
global sch_hitslist
global sch_modifiedlist
global sch_errorslist 
global sch_text
global sch_report

    set page [activeLabel .sch.note]
    if  { $page == 0 } {
	    $sch_filelist yview scroll 1 pages
	    set index [$sch_filelist index @0,0]
		if {$index != 0} {
	       $sch_filelist see $index
	       $sch_filelist activate $index
	       $sch_filelist selection clear 0 end
	       $sch_filelist selection anchor $index
	       $sch_filelist selection set $index
		}
    }
    if  { $page == 1 } {
	    $sch_unparselist yview scroll 1 pages
	    set index [$sch_unparselist index @0,0]
		if {$index != 0} {
	       $sch_unparselist see $index
	       $sch_unparselist activate $index
	       $sch_unparselist selection clear 0 end
	       $sch_unparselist selection anchor $index
	       $sch_unparselist selection set $index
		}
    }
    if  { $page == 2 } {
	    $sch_hitslist yview scroll 1 pages
	    set index [$sch_hitslist index @0,0]
		if {$index != 0} {
	       $sch_hitslist see $index
	       $sch_hitslist activate $index
	       $sch_hitslist selection clear 0 end
	       $sch_hitslist selection anchor $index
	       $sch_hitslist selection set $index
		}
    }
    if  { $page == 3 } {
	    $sch_errorslist yview scroll 1 pages
	    set index [$sch_errorslist index @0,0]
		if {$index != 0} {
	       $sch_errorslist see $index
	       $sch_errorslist activate $index
	       $sch_errorslist selection clear 0 end
	       $sch_errorslist selection anchor $index
	       $sch_errorslist selection set $index
		}
    }
    if  { $page == 4 } {
	    $sch_modifiedlist yview scroll 1 pages
	    set index [$sch_modifiedlist index @0,0]
		if {$index != 0} {
	       $sch_modifiedlist see $index
	       $sch_modifiedlist activate $index
	       $sch_modifiedlist selection clear 0 end
	       $sch_modifiedlist selection anchor $index
	       $sch_modifiedlist selection set $index
		}
    }
    if  { $page == 5 } {
	    $sch_text yview scroll 1 pages
    }
    if  { $page == 6 } {
	    $sch_report yview scroll 1 pages
    }
}
#--------------------------------------------------------------------------------------------------------------------------



#--------------------------------------------------------------------------------------------------------------------------
# This procedure will build the modified objects page of the notebook. the page will contain spreadsheet for the objects and 
# toolbar with 4 buttons : "New", "Removed","Modified" and "Browse"
#--------------------------------------------------------------------------------------------------------------------------
proc make_modifiedpage {} {
global sch_modifiedlist

    set w .sch
    set note $w.note
    set modifiedpage [getNote $note {Modified Objects}]

    # we want the listbox and two scrollbars to be embedded in a 
    frame $modifiedpage.spreadsheet -bd 2 -relief sunken

    # frame so they look like a single widget
    scrollbar $modifiedpage.vsb -orient vertical -command [list $modifiedpage.listbox yview]
    scrollbar $modifiedpage.hsb -orient horizontal -command [list $modifiedpage.listbox xview]

    # we will purposefully make the width less than the sum of the
    # columns so that the scrollbars will be functional right off
    # the bat.
    mclistbox $modifiedpage.listbox \
	    -bd 0 \
	    -height 10 \
	    -width 10 \
	    -columnrelief flat \
	    -labelanchor w \
	    -columnborderwidth 0 \
	    -selectcommand "" \
	    -selectmode single \
	    -labelborderwidth 2 \
	    -fillcolumn object \
	    -xscrollcommand [list $modifiedpage.hsb set] \
	    -yscrollcommand [list $modifiedpage.vsb set] \
		-relief flat

    # add the columns we want to see
    $modifiedpage.listbox column add file    -label "File"    -width 12
    $modifiedpage.listbox column add line    -label "Line"    -width 6
    $modifiedpage.listbox column add kind    -label "Kind"    -width 6
    $modifiedpage.listbox column add object  -label "Object"

    # set up bindings to sort the columns.
    #$modifiedpage.listbox label bind file   <ButtonPress-1> "sort %W file"
    #$modifiedpage.listbox label bind line   <ButtonPress-1> "sort %W line"
    #$modifiedpage.listbox label bind kind   <ButtonPress-1> "sort %W kind"
    #$modifiedpage.listbox label bind object <ButtonPress-1> "sort %W object"


    grid $modifiedpage.vsb -in $modifiedpage.spreadsheet -row 0 -column 1 -sticky ns
    grid $modifiedpage.hsb -in $modifiedpage.spreadsheet -row 1 -column 0 -sticky ew
    grid $modifiedpage.listbox -in $modifiedpage.spreadsheet -row 0 -column 0 -sticky nsew -padx 0 -pady 0
    grid columnconfigure $modifiedpage.spreadsheet 0 -weight 1
    grid columnconfigure $modifiedpage.spreadsheet 1 -weight 0

    grid rowconfigure    $modifiedpage.spreadsheet 0 -weight 1
    grid rowconfigure    $modifiedpage.spreadsheet 1 -weight 0

	frame $modifiedpage.toolbar -relief flat

    eb_button $modifiedpage.toolbar.modified {Modified} sunken {sch_modified_objects} 10
    pack $modifiedpage.toolbar.modified -in $modifiedpage.toolbar -side left -padx 1m -pady 1m
    eb_button $modifiedpage.toolbar.new {New} raised {sch_new_objects} 10
    pack $modifiedpage.toolbar.new -in $modifiedpage.toolbar -side left -padx 1m -pady 1m
    eb_button $modifiedpage.toolbar.removed {Removed} raised {sch_removed_objects} 10
    pack $modifiedpage.toolbar.removed -in $modifiedpage.toolbar -side left -padx 1m -pady 1m
    eb_button $modifiedpage.toolbar.browse {Browse} raised {sch_open_obj} 10
    pack $modifiedpage.toolbar.browse -in $modifiedpage.toolbar -side right -padx 1m -pady 1m

    pack $modifiedpage.toolbar -in $modifiedpage -side bottom -fill both
    pack $modifiedpage.spreadsheet -in $modifiedpage -side left -fill both -expand true

    #bind $modifiedpage.listbox <Button-1> "$old_bind; focus %W"
    bind $modifiedpage.listbox <Return> sch_open_obj
    bind $modifiedpage.listbox <Double-Button-1> sch_open_obj    

    set sch_modifiedlist $modifiedpage.listbox
}
#--------------------------------------------------------------------------------------------------------------------------


#--------------------------------------------------------------------------------------------------------------------------
# This procedure will build the files page of the notebook. 
#--------------------------------------------------------------------------------------------------------------------------
proc make_filespage {} {
global sch_filelist

    set w .sch
    set note $w.note
    set filespage [getNote $note {Files to Submit}]   

    # we want the listbox and two scrollbars to be embedded in a 
    frame $filespage.spreadsheet -bd 2 -relief sunken

    # frame so they look like a single widget
    scrollbar $filespage.vsb -orient vertical -command [list $filespage.listbox yview]
    scrollbar $filespage.hsb -orient horizontal -command [list $filespage.listbox xview]

    # we will purposefully make the width less than the sum of the
    # columns so that the scrollbars will be functional right off
    # the bat.
    mclistbox $filespage.listbox \
	    -bd 0 \
	    -height 10 \
	    -width 10 \
	    -columnrelief flat \
	    -labelanchor w \
	    -columnborderwidth 0 \
	    -selectcommand "" \
	    -selectmode single \
	    -labelborderwidth 2 \
	    -fillcolumn file \
	    -xscrollcommand [list $filespage.hsb set] \
	    -yscrollcommand [list $filespage.vsb set] \
		-relief flat

    # add the columns we want to see
    $filespage.listbox column add file    -label "File"     -width 12

    # set up bindings to sort the columns.
    #$filespage.listbox label bind file <ButtonPress-1> "sort %W file"


    grid $filespage.vsb -in $filespage.spreadsheet -row 0 -column 1 -sticky ns
    grid $filespage.hsb -in $filespage.spreadsheet -row 1 -column 0 -sticky ew
    grid $filespage.listbox -in $filespage.spreadsheet -row 0 -column 0 -sticky nsew -padx 0 -pady 0
    grid columnconfigure $filespage.spreadsheet 0 -weight 1
    grid columnconfigure $filespage.spreadsheet 1 -weight 0
    grid rowconfigure    $filespage.spreadsheet 0 -weight 1
    grid rowconfigure    $filespage.spreadsheet 1 -weight 0

    pack $filespage.spreadsheet -in $filespage -side left -fill both -expand true

    bind $filespage.listbox <Double-Button-1> sch_open_file    

    set sch_filelist $filespage.listbox
}
#--------------------------------------------------------------------------------------------------------------------------




#--------------------------------------------------------------------------------------------------------------------------
# This procedure will build the unparsed files page of the notebook. 
#--------------------------------------------------------------------------------------------------------------------------
proc make_unparsedpage {} {
global sch_unparselist

    set w .sch
    set note $w.note
    set parserpage [getNote $note {Unparsed Files}]   

    # we want the listbox and two scrollbars to be embedded in a 
    frame $parserpage.spreadsheet -bd 2 -relief sunken

    # frame so they look like a single widget
    scrollbar $parserpage.vsb -orient vertical -command [list $parserpage.listbox yview]
    scrollbar $parserpage.hsb -orient horizontal -command [list $parserpage.listbox xview]

    # we will purposefully make the width less than the sum of the
    # columns so that the scrollbars will be functional right off
    # the bat.
    mclistbox $parserpage.listbox \
	    -bd 0 \
	    -height 10 \
	    -width 10 \
	    -columnrelief flat \
	    -labelanchor w \
	    -columnborderwidth 0 \
	    -selectcommand "" \
	    -selectmode single \
	    -labelborderwidth 2 \
	    -fillcolumn file \
	    -xscrollcommand [list $parserpage.hsb set] \
	    -yscrollcommand [list $parserpage.vsb set] \
		-relief flat

    # add the columns we want to see
    $parserpage.listbox column add file    -label "File"     -width 12

    # set up bindings to sort the columns.
    #$parserpage.listbox label bind file <ButtonPress-1> "sort %W file"


    grid $parserpage.vsb -in $parserpage.spreadsheet -row 0 -column 1 -sticky ns
    grid $parserpage.hsb -in $parserpage.spreadsheet -row 1 -column 0 -sticky ew
    grid $parserpage.listbox -in $parserpage.spreadsheet -row 0 -column 0 -sticky nsew -padx 0 -pady 0
    grid columnconfigure $parserpage.spreadsheet 0 -weight 1
    grid columnconfigure $parserpage.spreadsheet 1 -weight 0
    grid rowconfigure    $parserpage.spreadsheet 0 -weight 1
    grid rowconfigure    $parserpage.spreadsheet 1 -weight 0

    pack $parserpage.spreadsheet -in $parserpage -side left -fill both -expand true

    set sch_unparselist $parserpage.listbox
}
#--------------------------------------------------------------------------------------------------------------------------



#--------------------------------------------------------------------------------------------------------------------------
# This procedure will clear progress bar and status message
#--------------------------------------------------------------------------------------------------------------------------
proc clear_progress_bar {} {
global sch_status
global progress_val
global current_progress_val
global max_value_of_progress

    set max_value_of_progress 0
    set progress_val 0
    set current_progress_val 0
	SetProgress $sch_status.progress 0 100

}
#--------------------------------------------------------------------------------------------------------------------------


#--------------------------------------------------------------------------------------------------------------------------
# This procedure will set  progress bar
#--------------------------------------------------------------------------------------------------------------------------
proc set_progress {val} {
global sch_status
global running
global current_progress_val
     
	if { $running == 1 } {
	    if { $current_progress_val < $val } {
	        set current_progress_val $val
	        SetProgress $sch_status.progress $val 100
		}
	}

}
#--------------------------------------------------------------------------------------------------------------------------


#--------------------------------------------------------------------------------------------------------------------------
# This procedure will set  progress bar
#--------------------------------------------------------------------------------------------------------------------------
proc set_progress_max {val} {
global running
global max_value_of_progress
   
	if { $running == 1 } {
	    if { $val < 100 } {
		    set max_value_of_progress $val
		} else {
		    set max_value_of_progress 99
		}
        after 10000 next_progress
	}

}
#--------------------------------------------------------------------------------------------------------------------------


#--------------------------------------------------------------------------------------------------------------------------
# This procedure will increment progress bar
#--------------------------------------------------------------------------------------------------------------------------
proc next_progress {} {
global sch_status
global running
global current_progress_val
global max_value_of_progress
  
	if { $running == 1 } {
	    if { $current_progress_val < $max_value_of_progress } {
	        incr current_progress_val
	        SetProgress $sch_status.progress $current_progress_val 100
	        after 7000 next_progress
			update
		}
	}

}
#--------------------------------------------------------------------------------------------------------------------------


proc stop {} {
global running

   if { $running == 1 } {
        if {[tk_messageBox -title "Submission Check" -message "This will stop the submission process.\n Are you sure?" -type yesno -icon question] == "no"} {
		   return
		} else {
           set running 0
           client_end
           destroy .sch
		   return
	   }
   } else {
       set running 0
       client_end
       destroy .sch
	   return
   }

}

proc user_comments {} {
global comments
global sch_report
global report_summary
global report_details
global analyse_done
global comment_status

    set old $comments
    set comments [get_comment "Comments for this submission:"]
	if { $comment_status == 0 } {
	   set comments $old
	   return
	}


    if { $analyse_done == 1 } {
	    $sch_report config -state normal
        #### report summary and details sections 
	    $sch_report delete 1.0 end
        $sch_report insert end [sch_report_header]

        $sch_report insert end [sch_report_summary]
        set report_summary [sch_report_header]
        append report_summary [sch_report_summary]

        $sch_report insert end [sch_report_details]
	    set report_details [sch_report_header]
        append report_details [sch_report_summary]
        append report_details [sch_report_details]
	    $sch_report config -state disabled
	}
}


# This procedure originally was in 'cm.tcl'.
proc text_ok { w } {
    global comment_txt comment_status
    set comment_txt [$w.text get 1.0 end]
    set comment_status 1
    destroy $w
}


# This procedure originally was in 'cm.tcl'.
proc text_cancel { w } {
    global comment_txt comment_status
    set comment_txt Cancelled
    set comment_status 0
    destroy $w
}


# This procedure originally was in 'cm.tcl'.
proc get_comment {{ title "Enter comment" } } { 
    
    set w .text
    catch {destroy $w}
    toplevel $w
     wm title $w $title
    wm iconname $w "text"
#    positionWindow $w
    
    frame $w.buttons    
    pack $w.buttons -side bottom -fill x -pady 2m
    button $w.buttons.dismiss -text "OK" -command "text_ok $w"
    button $w.buttons.code -text "Cancel" -command "text_cancel $w"
    pack $w.buttons.dismiss $w.buttons.code -side left -expand 1
                 
    text $w.text -relief sunken -bd 2 -yscrollcommand "$w.scroll set" -setgrid 1 -height 5
    scrollbar $w.scroll -command "$w.text yview"
    pack $w.scroll -side right -fill y
    pack $w.text -expand yes -fill both
    $w.text insert 0.0 ""
    $w.text mark set insert 0.0
    set oldfocus [focus]
    focus $w.text
    catch {grab $w}
    tkwait window .text
    focus $oldfocus
    global comment_txt
    return [string trim $comment_txt]
}


proc sch_build {} {
global old_bind oldfocus sch_filelist  sch_text sch_scroll sch_report listfont
global sch_unparselist
global analyse_done
global added_objects
global removed_objects
global modified_objects
global modified_page
global comments

    set comments ""
    set modified_page {Modified}
    set added_objects {}
    set removed_objects {}
    set modified_objects {}

    set analyse_done 0
    set w .sch
    toplevel $w -class Dialog
    wm geometry $w =70x30
    wm title $w "Submission Check"

    sch_build_menu $w.menu

    set note $w.note
    pack [notebook $note {Files to Submit} {Unparsed Files} Hits Errors {Modified Objects} Log Report] \
	    -expand 1 -fill both -padx 1m -pady 1m
    

    make_filespage
    make_unparsedpage
    make_hitspage
	make_errorspage
	make_modifiedpage

  

    set logpage [getNote $note Log]
    text $logpage.text  -state disabled -relief sunken -bd 2 -font $listfont -setgrid 1 -insertofftime 0 -yscrollcommand "$logpage.scroll set"
    scrollbar $logpage.scroll -command "$logpage.text yview "
    pack $logpage.scroll -in $logpage -side right -fill y
    pack $logpage.text -in $logpage -expand yes -fill both
    set sch_text $logpage.text
    set sch_scroll $logpage.scroll


    set reportpage [getNote $note Report]
    text $reportpage.text -state disabled -relief sunken -bd 2 -font $listfont -wrap none -xscrollcommand "$reportpage.scrollx set" -yscrollcommand "$reportpage.scrolly set"
    scrollbar $reportpage.scrollx -orient horizontal -command "$reportpage.text xview "
    scrollbar $reportpage.scrolly -command "$reportpage.text yview "
    pack $reportpage.scrollx -in $reportpage -side bottom -fill x
    pack $reportpage.scrolly -in $reportpage -side right -fill y
    pack $reportpage.text -in $reportpage -expand yes -fill both
    set sch_report $reportpage.text

    set f [frame $w.button -bd 1]
    pack $f -fill x
    eb_button $f.start Check raised {analyse} 10
    eb_button $f.submit Submit raised {submit} 10
    eb_button $f.comment Comments raised {user_comments} 10
    eb_button $f.close Close raised {stop} 10

    pack $f.start -side left -padx 3 -pady 3
    pack $f.start $f.submit -side left -padx 3 -pady 3
    pack $f.comment $f.submit -side left -padx 3 -pady 3
    pack $f.close -side right -padx 3 -pady 3

    global statfont sch_status


    
	# persantage of each pass - we need this to draw progress bar
    global persantage
	set persantage(cache) 5
	set persantage(analysis) 5
	set persantage(parse) 40
	set persantage(update) 10
	set persantage(finalize) 5
	set persantage(dfa_init) 5
	set persantage(dfa) 15
	set persantage(query) 15



    #### generating report header
#	set report_header ""
#	append report_header [sch_report_header]

    bind $w <Up>    ArrowUp
    bind $w <Down>  ArrowDown
    bind $w <Prior> PageUp
    bind $w <Next>  PageDown
    
    set oldfocus [focus]

    catch {grab $w}
    catch {tkwait visibility $w}
    catch {grab release $w}
    raise $w
    focus $w.note
    update
}

set running 0
proc analyse {} {
global sch_status
global sch_new_objects_list
global sch_modified_objects_list
global sch_removed_objects_list
global progress_val
global running
     

    if { $running == 1 } {
	   return
    }

    # Do not allow to press this button twice.
    set running 1

    # "Press" button.
    set w .sch
    set f $w.button
    $f.start config -relief sunken

    # reading modelbuild server host and ADMINDIR
    # the pmod_server returns this information, it takes it from the prj file.
    set host [dis_command build_info host]
    set admindir [dis_command build_info admindir]

    # connectiong to the build server
    set err [client_connect $host]
    if {$err == 0 } {
        set msg "Can't connect to the build server. Check if build server is running."
        tk_messageBox -title {Submission Check} -type ok -icon error \
                      -message $msg
        set running 0
        $f.start config -relief raised
        return
    }

    # Check existance of all files in this submission.
    global sch_files
    set msg ""
    set num 0
    foreach fn $sch_files {
        if { [ regexp {[ ]} $fn ] } {
            set fn \"$fn\"
        }
        # Get real private file name.
        set real_fn [join [ dis_command fname $fn ] ]
        set priv_fn [get_privatefname $real_fn]
        if { ![file exists $priv_fn]} {
            if {[string length $msg] > 0} {
                set msg "$msg\n"
            }
            if {$num >= 20} {
                # To avoid very big list
                set msg "$msg..."
                break
            }
            set msg "$msg$priv_fn"
            incr num 1
        }
    }
    if {[string length $msg] > 0} {
        tk_messageBox -title {Submission Check} -type ok -icon error \
                      -message "File(s) are not accessible:\n\n$msg"
        set running 0
        $f.start config -relief raised
        return
    }

    set progress_val 0

    set sch_status [frame $w.stat -relief groove]
    pack $sch_status -side bottom -fill both
	Progress $sch_status.progress
    pack  $sch_status.progress -side  left -fill both -expand true -pady 1
	clear_progress_bar

    set sch_new_objects_list      {}
    set sch_modified_objects_list {}
    set sch_removed_objects_list  {}


    global Tempdirectory sch_arr sch_text
    set subdir $Tempdirectory/workdir
    if [file exists $subdir] {
	file delete -force $subdir
    }
    file mkdir $subdir
    if [isunix] {
	file attributes $subdir -permissions 0777
    }
    #status_msg $subdir
    set fn $subdir/sch.prepare
    set fs [open $fn w]
    set ind 1
    set max_len 0
    foreach ff $sch_files {	
	    #set tag s_$ind ** tag should have file type info
	    set tag f${ind}_[file tail $ff]
	    # logical name
	    set names [dis_command prepare_names $ff]
	    set names [lindex $names 0]
	    lappend names $tag
	    set sch_arr($tag) $names 
 	    set pn [lindex $names 0]
	    set model_fname [lindex $names 1]
	    set local_fname [get_localfname $model_fname]
	    lappend names $local_fname
	    set ln [lindex $names 2]
	    set sch_arr($ln) $names 

	    set len [string length [file tail $ln]]
	    if {$len > $max_len} {
	        set max_len $len
	    }
	    puts $fs  [list $tag $pn $local_fname $ln]
	    incr ind
    }
    close $fs
    incr max_len 10
    global report_printformat
    set report_printformat "%-${max_len}s %7s    %s"


    #status_msg [list $host $admindir]

    client_submit_start $admindir $subdir $fn
    $sch_text insert end "\n**********   Submission Check started  **********"
    $sch_text insert end "\n<<===================================================>>\n\n"
    selectLabel .sch.note 5
}



#------------------------------------------------------------------------------
# Real submission - this will send the mail
#------------------------------------------------------------------------------
proc submit {} {
global report_summary
global report_details
global mails_list
global analyse_done
global parser_errors
global env
global CM

    if { $analyse_done == 1 } {
        if { $parser_errors == "" } {
            set admindir [dis_command build_info admindir]

            ### mail destination from the policy
            set details_list [get_server_report_section_key \
                                      $mails_list mail_dtl]
            set summary_list [get_server_report_section_key \
                                      $mails_list mail_stl]

            if { $details_list != "" } {
                set status [ catch {send_mail  {Submission check} $details_list $report_details} error_message ]
  		    if { $status != 0 } {
		    	tk_messageBox -title {Submission Check} -type ok -icon error -message $error_message
		    }
            }

            if { $summary_list != "" } {
                set status [ catch {send_mail  {Submission check} $summary_list $report_summary} error_message ]
  		    if { $status != 0 } {
		    	tk_messageBox -title {Submission Check} -type ok -icon error -message $error_message
		    }
            }

            ### self_mail if set
            set self_summary_list [get_server_report_section_key \
                                           $mails_list mail_stu]
            set self_details_list [get_server_report_section_key \
                                           $mails_list mail_dtu]

            set self_sum [lindex $self_summary_list 0]
            set self_det [lindex $self_details_list 0]

            set mail_addr ""
            catch {set mail_addr $env(DISCOVER_SELF_MAIL)}
            if { $mail_addr == "" } {
                catch {set mail_addr $env(USER)}
            }

            if {$self_sum == 1} {
                set status [ catch {send_mail  {Submission check} $mail_addr $report_summary} error_message ]
  		    if { $status != 0 } {
		    	tk_messageBox -title {Submission Check} -type ok -icon error -message $error_message
		    }
            }

            if {$self_det == 1 } {
                set status [ catch {send_mail  {Submission check} $mail_addr $report_details} error_message ]
  		    if { $status != 0 } {
		    	tk_messageBox -title {Submission Check} -type ok -icon error -message $error_message
		    }
            }

	    if $CM {
                # Get all files without new hits.
		global sch_files
		set local_list {}
		foreach ff $sch_files {
                    if {[has_new_hits $ff] == 0} {
                        lappend local_list $ff
                    }
		}

		set num_locals [llength $local_list]
		if {$num_locals == 0} {

		    set msg "None of the files selected for submission test"
                    set msg "$msg\nare passed the test."
		    tk_messageBox -title {Submission Check} -type ok \
                                  -icon error -message $msg

		} else {

                    # Make real submission, i.e. Check In.
		    set retval [cm_put [sch_submit_comments] $local_list]
                    if {$retval != 0} {

                        set msg "Error occured during submitting files."
                        tk_messageBox -title {Submission Check} -type ok -icon error \
                                      -message $msg

                    } else {

                        # Show confirmation message.
                        client_report $admindir
                        if {$num_locals == 1} {

                            set msg "Submission done. 1 file, which"
                            set msg "$msg\ndoesn't contain new hits,"
                            set msg "$msg was submitted."

                        } else {

                            set msg "Submission done. $num_locals files, which"
                            set msg "$msg\ndon't contain new hits,"
                            set msg "$msg were submitted."
                        }
                        tk_messageBox -title {Submission Check} -type ok \
                                      -message $msg

                        # Close 'Submission Check' window.
                        destroy .sch
                    }
		}
	    } else {
		set msg "Submission reports have been sent."
		tk_messageBox -title {Submission Check} -type ok -icon info \
                              -message $msg
	    }
        } else {
            set msg "Can't make this submission. Parser errors were detected." 
            tk_messageBox -title {Submission Error} -type ok -icon error \
                          -message $msg 
        }
    } else {
        set msg "Can't make this submission. You need to run analysis first." 
        tk_messageBox -title {Submission Error} -type ok -icon error \
                      -message $msg 
    }

} ;# submit
#------------------------------------------------------------------------------

proc has_new_hits {fn} {

    global new_violations_list
    global sch_arr

    # Go thru list of new hits.
    foreach viol $new_violations_list {

        set tag [lindex $viol 0]
        set names $sch_arr($tag)
        set lname [lindex $names 2]

        if {[string compare $fn $lname] == 0} {
            return 1
        }
    }

    return 0

} ;#has_new_hits


#------------------------------------------------------------------------------------------------------
# Open selected hit on the hits page in the editor. Will select between new hits and all hits
# using err_flag
#------------------------------------------------------------------------------------------------------
proc sch_open_hit {} {
global sch_arr
global violations_list
global new_violations_list
global err_flag
global sch_hitslist


    set selection [$sch_hitslist curselection]
	if { $selection < 0 } return

    #### do nothing if the hits list is empty
	if {[$sch_hitslist size]==0} {return}

    #### determining which list to use
    if { $err_flag == 0 }  {
	    set violations $new_violations_list
    } else {
	    set violations $violations_list
    }


    #### reading current line from the hots list
    set line [lindex $violations [lindex $selection 0]]

    #### we need file tag and line number to edit a hint
    set tag      [lindex $line 0]
    set linenum  [lindex $line 1]

    #### reading the file name
    set names $sch_arr($tag)

    
    #### opening the file
    set local_fname [get_localfname [lindex $names 1]]
    if {$linenum==-1} {set linenum 0}
    #[concat {open} [list $local_fname $linenum]]
    editorMessage OPEN "$local_fname" "$linenum"

}
#------------------------------------------------------------------------------------------------------


#------------------------------------------------------------------------------------------------------
# Open currently selected parser error on the errors page.  
#------------------------------------------------------------------------------------------------------
proc sch_open_error {} {
global sch_arr
global parser_errors
global sch_errorslist

    set selection [$sch_errorslist curselection]
	if { $selection < 0 } return

    #### do nothing if the hits list is empty
	if {[$sch_errorslist size]==0} {return}


	#### reading current line from the hots list
    set line [lindex $parser_errors [lindex $selection 0]]

    #### we need file tag and line number to edit a hint
	set can_open [lindex $line 2]
    set tag      [lindex $line 1]
    set linenum  [lindex $line 3]

    if { $can_open == 0 } {
	    tk_messageBox -title "Can't open the file" -message "This file is in the build server view\n and can't be opened in your local view." -type ok -icon error
		return
	}


    #### reading the file name
    set names $sch_arr($tag)

    	#### opening the file
    set local_fname [get_localfname [lindex $names 1]]
    if {$linenum==-1} {set linenum 0}
    #write [concat {open} [list $local_fname $linenum]]
    editorMessage OPEN "$local_fname" "$linenum"
}
#------------------------------------------------------------------------------------------------------



#------------------------------------------------------------------------------------------------------
# Open currently selected file
#------------------------------------------------------------------------------------------------------
proc sch_open_file {} {
global sch_arr
global sch_filelist
global sch_files

    set selection [$sch_filelist curselection]
    if { $selection < 0 } return

    #### do nothing if the hits list is empty
    if {[$sch_filelist size]==0} {return}

    #### reading current line from the hots list
    set line [lindex $sch_files [lindex $selection 0]]

    #### we need file tag and line number to edit a hint
    set tag  [lindex $line 0]

    set name [dis_command fname $tag]
 
    #### opening the file
    set local_fname [get_localfname $name]

    #write [concat {open} [list $local_fname 0]]
    editorMessage OPEN "$local_fname" 0
}
#------------------------------------------------------------------------------------------------------


#------------------------------------------------------------------------------------------------------
# Open currently selected modified object  
#------------------------------------------------------------------------------------------------------
proc sch_open_obj {} {
global sch_arr
global added_objects
global removed_objects
global modified_objects
global sch_modifiedlist
global modified_page

    set selection [$sch_modifiedlist curselection]
	if { $selection < 0 } return

    set objPage ""
    if { $modified_page == "Added" } {
	   set objPage $added_objects
	}
    if { $modified_page == "Removed" } {
	   set objPage $removed_objects
	}
    if { $modified_page == "Modified" } {
	   set objPage $modified_objects
	}



    #### do nothing if the hits list is empty
	if { $objPage == "" } {return}


	#### reading current line from the hots list
    set line [lindex $objPage [lindex $selection 0]]

    #### we need file tag and line number to edit a hint
    set tag      [lindex $line 0]
    set linenum  [lindex $line 3]

    #### reading the file name
    set names $sch_arr($tag)

    	#### opening the file
    set local_fname [get_localfname [lindex $names 1]]
    if {$linenum==-1} {set linenum 0}
    #write [concat {open} [list $local_fname $linenum]]
    editorMessage OPEN "$local_fname" "$linenum"
}
#------------------------------------------------------------------------------------------------------



################################
# Default page is all hits
# If you need to change this you
# must change the flag + initial
# status of the control buttons
###############################
set err_flag 0



#-----------------------------------------------------------------------------------------------------
# Modifies hits page to show only new hits
#-----------------------------------------------------------------------------------------------------
proc sch_newhits {} {
global new_violations_list 
global sch_hitslist 
global sch_arr
global err_flag

    set w .sch
    set note $w.note
    set hitspage [getNote $note {Hits}]
	   
    $hitspage.toolbar.new config -relief sunken
    $hitspage.toolbar.all config -relief raised

    $sch_hitslist delete 0 end
	foreach violation $new_violations_list {
	    set tag      [lindex $violation 0]
	    set linenum  [lindex $violation 1]
	    set err_code [lindex $violation 2]
	    set err_msg  [lindex $violation 3]
        set lname [lindex $sch_arr($tag) 2]
		if { $linenum < 0 } {
		    set linenum "N/A"
		}
        $sch_hitslist insert end [list [file tail $lname] $linenum $err_msg]
	}
    set err_flag 0
}
    

#-----------------------------------------------------------------------------------------------------
# Modifies hits page to show all hits
#-----------------------------------------------------------------------------------------------------
proc sch_allhits {} {
global violations_list 
global sch_hitslist 
global sch_arr
global err_flag


    set w .sch
    set note $w.note
    set hitspage [getNote $note {Hits}]   
    $hitspage.toolbar.new config -relief raised
    $hitspage.toolbar.all config -relief sunken


    $sch_hitslist delete 0 end
	foreach violation $violations_list {
	    set tag      [lindex $violation 0]
	    set linenum  [lindex $violation 1]
	    set err_code [lindex $violation 2]
	    set err_msg  [lindex $violation 3]
	    set lname [lindex $sch_arr($tag) 2]
		if { $linenum < 0 } {
		    set linenum "N/A"
		}
        $sch_hitslist insert end [list [file tail $lname] $linenum $err_msg]
	}
    set err_flag 1
}



proc dbg_msg args {
global progress_val
global running
global persantage

catch {
    if { $running == 1 } {
       next_progress
       global sch_text
	   $sch_text config -state normal
       set val [lindex $args 0]
	   set is_status_msg [string first "@ " $val]
	   if { $is_status_msg == 0 } {
		   set pass  [lindex $val 1]
		   set steps [lindex $val 2]
		   set step  [lindex $val 3]
		   set code  [lindex $val 4]
		   if { $step != 0 } { 
 	          set done $persantage($pass)
	          set done [expr $done/$steps]
              incr progress_val $done
	          set_progress $progress_val
		   } else {
 	          set done $persantage($pass)
	          set_progress_max [expr $progress_val+$done]
		   }
       } else {
           $sch_text insert end "[join $args]\n"
           $sch_text see end
	   }
	   $sch_text config -state disabled
	}

}
}

proc client_submit_start {admindir workdir fn} {
    client_put_file_list $workdir $fn
    client_submit $admindir $workdir
}

################################## REPORT GENERATING PROCEDURES ############################################


#-----------------------------------------------------------------------------------------------------------
# Generates report header like this:
#             Submission Check Reports
#
#  Username : <xxx>
#  Date     : <xxx>
#  Files in submission :
#     <aaa>
#     <bbb>
#-----------------------------------------------------------------------------------------------------------         
proc sch_report_header {} {
global env
global sch_files

   set header ""

   ## Title
   append header  "                                 Submission Check Report\n\n"

   ## User ID
   set msg [format "Username: %s" $env(USER) ];
   append header $msg\n

   ## Date
   set tt [format "%s" [clock format [clock seconds] -format %T]]
   set msg [format "Date: %s \[%s\]" [clock format [clock seconds] -format %D] $tt];
   append header $msg\n\n
   return $header
}

#------------------------------------------------------------------------------------------------------------



#-----------------------------------------------------------------------------------------------------------
# Generates report header like this:
#            Submission summary
#
#  Files in this submission :
#     <aaa>
#     <bbb>
#  Total violations : <xxx>
#  New violations   : <xxx>
#  Entities added   : <xxx>
#  Entities modified: <xxx>
#  Entities removed : <xxx>
#-----------------------------------------------------------------------------------------------------------         
proc sch_report_summary {} {
global sch_files
global violations_list
global new_violations_list
global modified_objects
global removed_objects
global added_objects
global err_cnt
global comments


   set sum ""
   append sum "                                 Submission Summary:\n\n"
   if {$comments != "" } {
      append sum "Purpose:\n"
      append sum $comments
      append sum "\n"
	}
       
   ## Files in submission
   set msg [format "Submitted files: %d" [llength $sch_files] ];
   append sum "$msg\n"
   set index 0
   set headers 0
   foreach ff $sch_files {
       set pos1 [string last ".h" $ff]
	   set pos2 [string last ".H" $ff]
       if { $pos1 != -1 ||  $pos2 != -1 } {
	       if { $pos1 == [expr [string length $ff]-2] ||  $pos2 == [expr [string length $ff]-2] }  {
	           incr headers;
		   }
	   }
       incr index
       set msg [format "    %d %s" $index $ff];
       append sum "$msg\n"
   }
   set msg [format "Header files: %d" $headers];
   append sum "$msg\n" 
   append sum "\n"


   if { $err_cnt != 0 } {
       append sum "Parser errors were detected.\n"
       return $sum;
   }

   ## new objects
   append sum "Entities added    : "
   set msg [format "%d" [llength $added_objects]];
   append sum "$msg\n"

   ################ MODIFIED OBJECTS #################
   ## modified objects
   append sum "Entities modified : "
   set msg [format "%d" [llength $modified_objects]];
   append sum "$msg\n"

   ## removed objects
   append sum "Entities removed  : "
   set msg [format "%d" [llength $removed_objects]];
   append sum "$msg\n"


   ############### VIOLATIONS DETECTED #####################

   set total_v [llength $violations_list]
   set new_v   [llength $new_violations_list]
   set old_v   [ expr $total_v-$new_v ]

   ## total violations
   append sum "Total violations  : "
   set msg [format "%d" $total_v];
   append sum "$msg\n"

   append sum "Old violations    : "
   set msg [format "%d" $old_v];
   append sum "$msg\n"


   ## new violations
   append sum "New violations    : "
   set msg [format "%d" $new_v];
   append sum "$msg\n"

   append sum "\n\n"
   return $sum
}

#------------------------------------------------------------------------------------------------------------


#-----------------------------------------------------------------------------------------------------------
# Generates report details:
#
# Submission details:
#
# File      Line        Description    
#-----------------------------------------------------------------------------------------------------------         
proc sch_report_details {} {
global violations_list
global new_violations_list
global sch_arr

# modified objects lists - must be filled by Alex Harlap script
global added_objects
global removed_objects
global modified_objects
global err_cnt

   if { $err_cnt != 0 } return;
   set details ""
   ## Title
   append details "                                 Submission Details:\n\n"

   ## All detected violations
   append details "Violations detected:\n\n"
   if { $violations_list != "" } {
       append details "File name              Line   Description\n"
       foreach violation $violations_list {
	       set tag      [lindex $violation 0]
	       set linenum  [lindex $violation 1]
	       set err_code [lindex $violation 2]
	       set err_msg  [lindex $violation 3]
	       ### forming hits page - inserting all errors
	       global report_printformat
	       set lname [lindex $sch_arr($tag) 2]
		   if { $linenum == -1 } {
		        set linenum "N/A"
		   }
	       set msg [format $report_printformat $lname $linenum $err_msg]
           append details $msg\n	       
       }
   } else {
      append details "     No violations detected\n"
   }
 
   append details "\n\n"


   ## New violations
   append details "New violations:\n\n"
   if { $new_violations_list != "" } {
       append details "File name              Line   Description\n"
       foreach violation $new_violations_list {
	       set tag      [lindex $violation 0]
	       set linenum  [lindex $violation 1]
	       set err_code [lindex $violation 2]
	       set err_msg  [lindex $violation 3]
           ### forming hits page - inserting all errors
  		   global report_printformat
	       set lname [lindex $sch_arr($tag) 2]
		   if { $linenum == -1 } {
		       set linenum "N/A"
		   }
	       set msg [format $report_printformat $lname $linenum $err_msg]
           append details $msg\n	       
	   }

   } else {
       append details "     No new violations detected\n"
   }    
   append details "\n\n"
 

   ## New objects
   append details "Entities added:\n\n"
   if { $added_objects != "" } {
       foreach object $added_objects {
	       append details [lindex $object 2]\n
	   }
   } else {
       append details "    No new entities\n"
   }
   append details "\n\n"

   ## Modified objects
   append details "Entities modified:\n\n"
   if { $modified_objects != "" } {
       foreach object $modified_objects {
	       append details [lindex $object 2]\n
	   }
   } else {
       append details "    No modified entities\n"
   }
   append details "\n\n"


   ## Removed objects
   append details "Entities removed:\n\n"
   if { $removed_objects != "" } {
       foreach object $removed_objects {
	       append details [lindex $object 2]\n
	   }
   } else {
       append details "    No removed entities\n"
   }
   append details "\n\n"


   return $details;
}
#------------------------------------------------------------------------------------------------------------



#-----------------------------------------------------------------------------------------------------------
# Generates submit comments:
#-----------------------------------------------------------------------------------------------------------         
proc sch_submit_comments {} {
global report_printformat
# modified objects lists - must be filled by Alex Harlap script
global added_objects
global removed_objects
global modified_objects
global sch_files
global comments

   set details ""
   append  details $comments
   append details "\n"

   append details "Files in this submission:\n\n"

   set index 0
   foreach ff $sch_files {
       incr index
       set msg [format "    %d %s" $index $ff];
       append details "$msg\n"
   }
   append sum "\n\n"

   ## New objects
   append details "Entities added:\n\n"
   if { $added_objects != "" } {
       foreach object $added_objects {
	       append details [lindex $object 2]\n
	   }
   } else {
       append details "    No new entities\n"
   }
   append details "\n\n"

   ## Modified objects
   append details "Entities modified:\n\n"
   if { $modified_objects != "" } {
       foreach object $modified_objects {
	       append details [lindex $object 2]\n
	   }
   } else {
       append details "    No modified entities\n"
   }
   append details "\n\n"


   ## Removed objects
   append details "Entities removed:\n\n"
   if { $removed_objects != "" } {
       foreach object $removed_objects {
	       append details [lindex $object 2]\n
	   }
   } else {
       append details "    No removed entities\n"
   }
   return $details;
}
#------------------------------------------------------------------------------------------------------------



#------------------------------------------------------------------------------------------------------------
# This will run "mail" executable to send mail.  We need our "mail" emulator for NT
#------------------------------------------------------------------------------------------------------------
proc send_mail { subject to text } {
global env

  set retval ""
  set retcode 0

  set _to ""

  foreach addr $to {
    if { $addr != "" } {
      append _to "\{$addr\} "
    }
  }
  set to $_to

  set text "Subject: ${subject}\n\n$text"

  if [isunix] {
      set cmd "mail $to"
  } else {
      set exe "$env(PSETHOME)/bin/mail.exe"
      if { [ regexp {[ ]} $exe ] } {
          set exe "\"$exe\""
      }
      set cmd "$exe $to"
  }
  regsub -all {\\} $cmd / cmd
  set status [ catch { eval exec $cmd << {$text} } errmsg ]
  if { $status != 0 } {
      set ec ""
      global errorCode
      if { [ info exists errorCode ] } {
         set ec $errorCode
      }
      if { (! [ regexp SIG $errmsg ]) && (! [ regexp INTR $ec ]) } {
        set retval "Error encountered while sending mail:\n\n$errmsg"
        set retcode 1
      }
  }
  return -code $retcode $retval
}
#------------------------------------------------------------------------------------------------------------

#------------------------------------------------------------------------------------------------------------
# This procedure will try to find section with the specified name in the build server results 
# file and will return this section as a TCL list. It will return empty list if it can't find
# the section with this name in the buildserver report file (username@host/submission_check.out)
#------------------------------------------------------------------------------------------------------------
proc get_server_report_section { list name } {
  set all [lindex $list 0]
  foreach section $all {
     set sname [lindex $section 0]
	 set value [lindex $section 1]
     if { "$sname" == "$name" } {
	     return $value
	 }
  }
  return {}
}
#------------------------------------------------------------------------------------------------------------

#------------------------------------------------------------------------------------------------------------
# This procedure will try to find section with the specified name in the build server results 
# file and will return this section as a TCL list. It will return empty list if it can't find
# the section with this name in the buildserver report file (username@host/submission_check.out)
#------------------------------------------------------------------------------------------------------------
proc get_server_report_section_key { list name } {
  foreach record $list {
     set key [lindex $record 0]
	 set value [lindex $record 1]
     if { "$key" == "$name" } {
	     return $value
	 }
  }
  return {}
}
#------------------------------------------------------------------------------------------------------------



#------------------------------------------------------------------------------------------------------------
# This procedure will run at the end of the submission and will read all results from the server,
# will form violations list and new violations list, will create report and modified objects lists
#------------------------------------------------------------------------------------------------------------
proc client_submit_end {admindir workdir} {
global  violations_list
global  new_violations_list
global  parser_errors
global  err_flag

global  types_list
global  mails_list
global  sch_files

global  CM
global  cm_map
global  old_source
global  sch_arr
global  sch_text
global  sch_report
global  sch_hitslist 
global  sch_errorslist 
global  sch_modifiedlist
global  sch_status

global  report_summary
global  report_details
global  analyse_done

global added_objects
global removed_objects
global modified_objects
global err_cnt
global running
global Tempdirectory


    client_get_file $workdir/submission_check.out submission_check.out


    #### reading server reply
    set f [open $workdir/submission_check.out]
    array set server_results [read $f]
    close $f


    #### reading parser output - parser errors
    client_get_file $workdir/submission_check.err submission_check.err
    set f [open $workdir/submission_check.err]
    set err_cnt 0
	set parser_errors ""
    $sch_errorslist delete 0 end
	set oname ""
    while {[gets $f line] > 0} {
	    if { [regexp {^\"([^\"]+)\" [^\"]* \"([^\"]+)\", line ([0-9]+): .*: (.*)} $line dummy unit file linenum err_msg] } {
	        regexp {^.*/([^/]+)$} $unit dummy utag
	        regexp {^.*/([^/]+)$} $file dummy ftag
	        if [info exists sch_arr($utag)] {
		        set lname [lindex $sch_arr($utag) 2]
	            if [info exists sch_arr($ftag)] {
				   set comp_file [lindex $sch_arr($ftag) 2]
				   set can_open 1
			    } else {
				   set comp_file $file
				   set can_open 0
                }
		        set rec ""
		        lappend rec $lname
		        lappend rec $comp_file
		        lappend rec $can_open
		        lappend rec $linenum
		        lappend rec $err_msg
		        lappend parser_errors $rec
		        set err_cnt [expr $err_cnt+1];
		        if { $linenum == -1 } {
		             set linenum "N/A"
		        }
				if { $oname != $lname } {
		            $sch_errorslist insert end [list [file tail $lname] [file tail $comp_file] $linenum $err_msg]
				} else {
		            $sch_errorslist insert end [list "" [file tail $comp_file] $linenum $err_msg]
			    }
				set oname $lname
	        }
	    }
    } 
    close $f

    #### reading parser output - unparsed files
	global sch_unparselist
    $sch_unparselist delete 0 end
	catch {
        client_get_file $workdir/submission_check.not submission_check.not
        set f [open $workdir/submission_check.not]

        while {[gets $f line] > 0} {
	        if [info exists sch_arr([file tail $line])] {
		    set lname [lindex $sch_arr([file tail $line]) 2]
		    $sch_unparselist insert end $lname
		    set failed_array($lname) 1
		} else {
		    $sch_unparselist insert end $line
		}
        } 
        close $f
	}



    #### extracting reply sections

    set violations_list {}
    set vlist           {}
    set types_list      {}
    set mails_list      {}
    set entities_list   {}
    set entities_subm   {}
    catch {set vlist $server_results(violations)}
	foreach v $vlist {
	    set tag     [file tail [lindex $v 0]]
	    if [info exists sch_arr($tag)] {
		   lappend violations_list  [lreplace $v 0 0 $tag]
		}
	}

    catch {set types_list      $server_results(querylib)}
    catch {set mails_list      $server_results(mail)}
    catch {set entities_list   $server_results(entities)}
    catch {set entities_subm   $server_results(old_entities)}

    #### find added / removed /modified entities

    set old_files {}
    foreach myline $entities_subm {
	set level [lindex $myline 0]
	if {$level==0} {
	    set file [lindex $myline 2]
	    lappend old_files $file
	}
	lappend ent_arr($file) $myline
    }

    catch { unset old_source }
    catch { unset cm_map }

    foreach el $old_files {
	if ![info exists failed_array($el)] {
	    set tag_name [lindex $sch_arr($el) 3]
	    set local_fname [lindex $sch_arr($el) 4]
	    set old_source($local_fname) $Tempdirectory/$tag_name
   	    client_get_file $Tempdirectory/$tag_name "$tag_name.old"
	    if $CM {
		cm_map $old_source($local_fname) $local_fname cm_map
	    }
	}
    }

    set sch_files_passed {}

    foreach checked_file $sch_files {
	if { ![info exists failed_array($checked_file)] && ![info exists ent_arr($checked_file)] } {
	    lappend sch_files_passed $checked_file
	    set tag_name [lindex $sch_arr($checked_file) 3]
	    set local_fname [lindex $sch_arr($checked_file) 4]
	    set model_fname [lindex $sch_arr($checked_file) 1]
	    set old_source($local_fname) $Tempdirectory/$tag_name
	    copy_pset_from_server $model_fname $old_source($local_fname)
	    if $CM {
		cm_map $old_source($local_fname) $local_fname cm_map
	    }
 	}
    }
    debug_var "sch_files_passed"
    if { [llength $sch_files_passed] == 0 } {
	set entities_list_old_pmod {}
    } else {
	set entities_list_old_pmod [ dis_command list_entities $sch_files_passed ]
	if { [llength $entities_list_old_pmod] == 1 } {
	    set entities_list_old_pmod [lindex $entities_list_old_pmod 0]
	}
    }

    foreach myline $entities_list_old_pmod {
	set level [lindex $myline 0]
	if {$level==0} {
	    set file [lindex $myline 2]
	    lappend old_files $file
	}
	lappend ent_arr($file) $myline
    }
    lsort old_files
    set entities_list_old {}
    foreach el $old_files {
	foreach item $ent_arr($el) {
	    lappend entities_list_old $item
	}
    }

    debug_var "entities_list_old"
    debug_var "entities_list"

    set changes [find_modified_ent $entities_list $entities_list_old]
    set added_objects [lindex $changes 0]
    set removed_objects [lindex $changes 1]
    set modified_objects [lindex $changes 2]

    debug_var "added_objects"
    debug_var "removed_objects"
    debug_var "modified_objects"

    #### filling hits page in the browser + searching for the new errors
    set err_flag 0
	set new_violations_list ""
    $sch_hitslist delete 0 end
	foreach violation $violations_list {
	    set tag      [lindex $violation 0]
	    set linenum  [lindex $violation 1]
	    set err_code [lindex $violation 2]
	    set err_msg  [lindex $violation 3]

	    set names $sch_arr($tag)
	    ### searching for the new hits
        set model_fname [lindex $names 1]
        set local_fname [get_localfname $model_fname]
        if [info exists cm_map($local_fname:$linenum)] {
		    lappend new_violations_list $violation
	        ### forming hits page - inserting all errors
		    if { $linenum == -1 } {
		        set linenum "N/A"
		    }
            $sch_hitslist insert end [list [file tail $local_fname] $linenum $err_msg]
	    }

	}

    set w .sch
    set note $w.note
    set hitspage [getNote $note Hits]
    $hitspage.toolbar.new config -relief sunken
    $hitspage.toolbar.all config -relief raised


    $sch_modifiedlist delete 0 end   
    foreach object $modified_objects {
	    if [info exists sch_arr([lindex $object 0])] {
		    set lname [lindex $sch_arr([lindex $object 0]) 2]
            $sch_modifiedlist insert end [list [file tail $lname] [lindex $object 3] [lindex $object 1] [lindex $object 2]]
		}
    }
    global modified_page
    set modifiedpage [getNote $note {Modified Objects}]
    $modifiedpage.toolbar.new config -relief raised
    $modifiedpage.toolbar.modified config -relief sunken
    $modifiedpage.toolbar.removed config -relief raised
    set modified_page {Modified}

	$sch_report config -state normal
    #### report summary and details sections 
	$sch_report delete 1.0 end
    $sch_report insert end [sch_report_header]

    $sch_report insert end [sch_report_summary]
    set report_summary [sch_report_header]
    append report_summary [sch_report_summary]


    $sch_report insert end [sch_report_details]
	set report_details [sch_report_header]
    append report_details [sch_report_summary]
    append report_details [sch_report_details]
	$sch_report config -state disabled

	#### status
    if { $parser_errors == "" } {
        selectLabel .sch.note 2
	} else {
        selectLabel .sch.note 3
	}
    set_progress 100
	set analyse_done 1
    set w .sch
    set f $w.button
    $f.start config -relief raised

    global sch_status
    destroy $sch_status
	set running 0
}
#------------------------------------------------------------------------------------------------------------




#------------------------------------------------------------------------------------------------------------
# Not used right now
#------------------------------------------------------------------------------------------------------------
proc sch_filter_errors { input_list violation_code } {
    set filtered_list ""
    foreach violation $input_list {
	    set code [lindex $violation 2]
		if { $code == $violation_code } {
		    lappend filtered_list $violation
		}
	}
	return $filtered_list
}
#------------------------------------------------------------------------------------------------------------

proc hits_pane {} {

global err_flag
global violations_list
global new_violations_list
global sch_arr

	if { $err_flag == 0 } {
				set pane $new_violations_list
			} else {
				set pane $violations_list
				}
   set details ""	
   if { $pane != "" } {
       foreach violation $pane {
	       set tag      [lindex $violation 0]
	       set linenum  [lindex $violation 1]
	       set err_msg  [lindex $violation 3]

###     forming hits page - inserting all errors

	       set lname [lindex $sch_arr($tag) 2]
		   if { $linenum == -1 } {
		       set linenum "N/A"
		   }
	       set msg "$lname\t$linenum\t$err_msg"
           append details $msg\n	       
	   }

   } else {
       append details "     No violations detected\n"
   }    
	return $details
}				

proc objects_pane {} {

global modified_objects
global removed_objects
global added_objects
global modified_page
global sch_arr

	if { $modified_page == "Added" } {
				set pane $added_objects
			} elseif { $modified_page == "Removed" } {
				set pane $removed_objects					  
			  }	else {
				set pane $modified_objects
				}
   set details ""	
   if { $pane != "" } {
       foreach violation $pane {
	       set tag      [lindex $violation 0]
	       set kind     [lindex $violation 1]
	       set object   [lindex $violation 2]
	       set linenum  [lindex $violation 3]

###     forming hits page - inserting all errors

	       set lname [lindex $sch_arr($tag) 2]
		   if { $linenum == -1 } {
		       set linenum "N/A"
		   }
	       set msg "$lname\t$linenum\t$kind\t$object"
           append details $msg\n	       
	   }

   } else {
       append details "     No objects detected\n"
   }    
	return $details
}

proc error_pane {} {

global parser_errors
global sch_arr

   set pane ""	
   if { $parser_errors != "" } {
	set p_lname ""
        foreach violation $parser_errors {
	        set tag      [lindex $violation 0]
	        set fname    [lindex $violation 1]
	        set linenum  [lindex $violation 3]
	        set comments [lindex $violation 4]

###     forming hits page - inserting all errors

	        set lname [lindex $sch_arr($tag) 2]
		if {$lname == $p_lname} {
			set lname_prn ""
		} else {
			set lname_prn $lname
			set p_lname $lname
		}	
		if { $linenum == -1 } {
		       set linenum "N/A"
	        }
	       set msg "$lname_prn\t$fname\t$linenum\t$comments"
               append pane $msg\n	       
	   }

   } else {
       append pane "     No errors detected\n"
   }    
	return $pane
}

proc save_in_file {pg file sav_pane} {

	set file_ID [ open $file w+ ]
	if {$pg < 2} {
		foreach str $sav_pane {
			puts $file_ID $str
		}
	} else {
		puts $file_ID $sav_pane 
		}
	close $file_ID
}

proc save_pane {} {

global env
global sch_files
global sch_unparselist
global sch_text
global report_details

    	set page [activeLabel .sch.note]
	switch $page {
		0 {set pane_to_file $sch_files}
		1 {set pane_to_file [$sch_unparselist get 0 end]}
		2 {set pane_to_file [hits_pane]}
		3 {set pane_to_file [error_pane]}
		4 {set pane_to_file [objects_pane]}
		5 {set pane_to_file [$sch_text get 1.0 end]}
		6 {set pane_to_file $report_details}
	}   
	set res [tk_getSaveFile -filetypes {{"Text Files"  	{.txt}}} -defaultextension .txt -initialdir $env(HOME)]
	if { $res!={} } { 
		save_in_file $page $res $pane_to_file
	}     
}

proc sch_build_menu {m} {
    menu  $m -tearoff 0  -bd 1

    set top [winfo parent $m]
    $top config -menu $m

    set mm $m.file
    $m add cascade  -label "File " -menu $mm -underline 0 
    menu $mm -tearoff 0
    my_menu $mm "Check"    0 "analyse"
    my_menu $mm "Submit"   0 "submit"
    my_menu $mm "Comments..." 2 "user_comments"
    my_menu $mm "Find in Log..." 0 "Dialog_Find 1"
    my_menu $mm "Save Pane As..." 10 "save_pane"
    $mm add separator
    my_menu $mm "Close " 2 "destroy .sch"
    
    set mm $m.filter
    $m add cascade  -label "View " -menu $mm -underline 0 
    menu $mm -tearoff 0
    my_menu $mm "Files to Submit"    0    "selectLabel .sch.note 0"
    my_menu $mm "Unparsed Files"     0    "selectLabel .sch.note 1"
    my_menu $mm "Hits"               0    "selectLabel .sch.note 2"
    my_menu $mm "Errors"             0    "selectLabel .sch.note 3"
    my_menu $mm "Modified Objects"   0    "selectLabel .sch.note 4"
    my_menu $mm "Log"                0    "selectLabel .sch.note 5"
    my_menu $mm "Report"             0    "selectLabel .sch.note 6"


}

proc collect_ent { lst arr } {
    upvar $arr this
    set file ""
    foreach line $lst {
	set level [lindex $line 0]
	if {$level==0} {
	    set file [lindex $line 2]
	} else { 
	    set knd [lindex $line 1]
	    set nm [lindex $line 2]
	    set value [list $file $knd $nm [lindex $line 3] [lindex $line 4]] 
	    set this($knd:$nm:$file) $value
	}
    }
    catch {close $f}
}

proc find_modified_ent {lnew lold} {
global cm_map sch_arr old_source
global err_cnt

    collect_ent $lnew ar1
    collect_ent $lold ar2
    set added {}
    set removed {}
    set comm {}
    set modified {}

#	if { $err_cnt != 0 } return;

    foreach {key value} [array get ar1] {
	if [info exists ar2($key)] {
	    set nlen [lindex $value 4]
	    set olen [lindex $ar2($key) 4]
	    if { $nlen == $olen } {
		lappend comm [list $value [lindex $ar2($key) 3]]
	    } else {
		lappend modified $value
	    }
	    unset ar2($key)
	} else {
	    lappend added $value
	}
    }
    foreach {key value} [array get ar2] {
	lappend removed $value
    }

    set added [lsort $added]
    set removed [lsort $removed]

    init_file_struct newfile
    init_file_struct oldfile

    foreach my_el $comm {
	set el [lindex $my_el 0]
	set oldline [lindex $my_el 1]
	set is_modified 0
	set ln [lindex $el 0]
	set model_fname [lindex $sch_arr($ln) 1]
	set local_fname [get_localfname $model_fname]
	set start [lindex $el 3]
	for {set ii 0} {$ii < [lindex $el 4]} {incr ii} {
	    set linenum [expr $start + $ii]
	    if [info exists cm_map($local_fname:$linenum)] {
		set is_modified 1
		break
	    }
	}
	if { $is_modified == 1 } {
	    set is_modified 0

	    seek_position newfile $local_fname $start
	    seek_position oldfile $old_source($local_fname) $oldline
	    
	    set end_loop [lindex $el 4]
	    for {set ii 0} {$ii < $end_loop} {incr ii} {
		set newsource [read_line_struct newfile]
		set oldsource [read_line_struct oldfile]
		if { $newsource != $oldsource } {
		    set is_modified 1
		    break
		}
	    }
	}
	if { $is_modified == 1 } {
	    lappend modified $el
	}
    }

    close_file_struct newfile
    close_file_struct oldfile

    set modified [lsort -command cmp_file_line $modified]
    list $added $removed $modified
}

proc cmp_file_line { rec1 rec2 } {
    set f1 [lindex $rec1 0]
    set f2 [lindex $rec2 0]
    if { $f1 < $f2 } { return -1 }
    if { $f1 > $f2 } { return 1 }
    set l1 [lindex $rec1 3]
    set l2 [lindex $rec2 3]
    if { $l1 < $l2 } { return -1 }
    if { $l1 > $l2 } { return 1 }
    return 0
}

proc init_file_struct ff {
  upvar $ff fp
  set fp(name) ""
  set fp(stream) 0
  set fp(line) 0
}

proc close_file_struct ff {
  upvar $ff fp
  if { $fp(stream) != 0 } {
    catch { close $fp(stream) }
  } 
} 

proc read_line_struct ff {
  upvar $ff fp
  incr fp(line)
  gets $fp(stream)
}

proc seek_position { ff fname line } {
  upvar $ff fp

  if { $fp(name) != $fname || $fp(line) > $line } {
    if { $fp(stream) != 0 } {
      catch { close $fp(stream) }
    }

    set fp(name) $fname
    set fp(stream) [open $fname "r"]
    set fp(line) 1
  }
  while { $fp(line) < $line } {
    gets $fp(stream)
    incr fp(line)
  }
}

# sort the list based on a particular column
proc sort {w id} {
    set data [$w get 0 end]
    set index [lsearch -exact [$w column names] $id]
    set result [lsort -index $index $data]
    $w delete 0 end
    # ... and add our sorted data in
    eval $w insert end $result
}
