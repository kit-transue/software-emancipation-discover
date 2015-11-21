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
# This file 'cm_generic.tcl' contains all main procedures
#     for work with 'CM:Generic' interface, which implemeted
#     in the 'cmdriver'.
#


# Global variables:

# CmAttrStr  ;#
# CmAttrList ;#
# CmCmdStr   ;#
# CmCmdList  ;#

# CmAttrKeyList    ;#
# CmAttrDisplayArr ;#
# CmAttrTypeArr    ;#
# CmAttrValueArr   ;#
# CmAttrAskArr     ;#

# CmCmdKeyList     ;#
# CmCmdDisplayArr  ;#
# CmCmdReturnArr   ;#
# CmCmdAttrArr     ;#
# CmCmdFileArr     ;#

# CmCheckboxRes ;#
# CmComboboxRes ;#

# CmDlgExitCode ;#


proc cm_add_menuitems {mm} {

    global CMLabel
    global CmDish
    global CmAttrStr CmAttrList
    global CmCmdStr  CmCmdList

    global CmAttrKeyList  CmAttrDisplayArr
    global CmAttrTypeArr  CmAttrValueArr
    global CmCmdKeyList   CmCmdDisplayArr
    global CmCmdReturnArr CmCmdAttrArr
    global CmCmdFileArr
   
    # Get all supported attributes from the 'cmdriver'. 
    set CmAttrStr [exec $CmDish -query get_attributes -cm $CMLabel]
    set CmAttrList [split $CmAttrStr "\n"]

    # Get all supported commands from the 'cmdriver'. 
    set CmCmdStr [exec $CmDish -query get_commands -cm $CMLabel]
    set CmCmdList [split $CmCmdStr "\n"]

    # Parse all atributes and commands.
    cm_parse_attributes
    cm_parse_commands

    # Add displayable names to menu.
    foreach key $CmCmdKeyList {
	set display_str $CmCmdDisplayArr($key)
	if [string length $display_str] {
	    my_menu $mm $display_str -1 "cm_action $key"
	}
    }

} ;# cm_add_menuitems


proc cm_parse_attributes {} {

    global CmAttrList    CmAttrAskArr
    global CmAttrKeyList CmAttrDisplayArr
    global CmAttrTypeArr CmAttrValueArr

    # Go trough specifications of all attributes.
    foreach attr $CmAttrList {

	# Tokenize attribute specification.
	set token_list [split $attr "\t"]

	# Here will be key, displayable name, type and value of this attribute.
	set key_str     ""
	set display_str ""
	set type_str    ""
	set value_str   ""
	set ask_str     ""

	# Find tokens 'key=".."', 'display=".."', 'type=".."' and 'value=".."'.
	foreach token $token_list {

	    set token_len [string length $token]

	    if [string match key="* $token] {

		set key_str [string range $token 5 [expr $token_len - 2]]

	    } elseif [string match display="* $token] {

		set display_str [string range $token 9 [expr $token_len - 2]]

	    } elseif [string match type="* $token] {

		set type_str [string range $token 6 [expr $token_len - 2]]

	    } elseif [string match value="* $token] {

		set value_str [string range $token 7 [expr $token_len - 2]]

	    } elseif [string match ask="* $token] {

		set ask_str [string range $token 5 [expr $token_len - 2]]
		if {[string length $ask_str] == 0} {

		    set ask_str "true"
		}
	    }
	}

	# Save all parameters of this attribute in the global list and arrays.
	lappend CmAttrKeyList $key_str
	set CmAttrDisplayArr($key_str) $display_str
	set CmAttrTypeArr($key_str) $type_str
	set CmAttrValueArr($key_str) $value_str
	set CmAttrAskArr($key_str) $ask_str
    }

} ;# cm_parse_attributes


proc cm_parse_commands {} {

    global CmCmdList
    global CmCmdKeyList   CmCmdDisplayArr
    global CmCmdReturnArr CmCmdAttrArr
    global CmCmdFileArr

    # Go trough specifications of all commands.
    foreach cmd $CmCmdList {

	# Tokenize command specification.
	set token_list [split $cmd "\t"]

	# Here will be key, displayable name and return type of this command.
	set key_str     ""
	set display_str ""
	set return_str  ""
	set file_str  ""

	# Here will be list of all attributes of the current command.
	set attr_list ""

	# Find tokens 'key=".."', 'display=".."', 'return=".."' and 'attr=".."'.
	foreach token $token_list {

	    set token_len [string length $token]

	    if [string match key="* $token] {

		set key_str [string range $token 5 [expr $token_len - 2]]

	    } elseif [string match display="* $token] {

		set display_str [string range $token 9 [expr $token_len - 2]]

	    } elseif [string match return="* $token] {

		set return_str [string range $token 8 [expr $token_len - 2]]

	    } elseif [string match attr="* $token] {

		set attr_str [string range $token 6 [expr $token_len - 2]]

		if {[string compare $attr_str "file"] == 0
		        || [string compare $attr_str "files"] == 0
		        || [string compare $attr_str "dir"] == 0
		        || [string compare $attr_str "dirs"] == 0} {
		    set file_str $attr_str
		} else {
		    lappend attr_list $attr_str
		}
	    }
	}

	# Save all parameters of this command in the global list and arrays.
	lappend CmCmdKeyList $key_str
	set CmCmdDisplayArr($key_str) $display_str
	set CmCmdReturnArr($key_str) $return_str
	set CmCmdFileArr($key_str) $file_str

	# Save attributes list as simple string with attributes divided by '\t'.
	set attr_str [join $attr_list "\t"]
	set CmCmdAttrArr($key_str) $attr_str
    }

} ;# cm_parse_commands


proc cm_action {key} {

    global CmCmdFileArr

    # Get current selected files (directories).
    set sel_items [dis_prepare_selection]
    set sel_size  [llength $sel_items]

    # Determine if user has selected directory(s), not file(s).
    set is_dir 0
    if {$sel_size > 0} {
	if [dis_command project [lindex $sel_items 0]] {
	    set is_dir 1
	}
    }

    # Get parameters of the current command.
    set cmd_file_str $CmCmdFileArr($key)

    # Check how the current selection corresponds to arguments of this command.
    if [string match file* $cmd_file_str] {

	# User should have selected file(s).
	if {$sel_size == 0} {
	    set msg "Nothing is selected. Unable to perform command."
	    status_line $msg
	    tk_messageBox -title "CM Error" -message $msg -type ok -icon error
	    return
	} elseif {$is_dir == 1} {
	    set msg "No file(s) are selected. Unable to perform command."
	    status_line $msg
	    tk_messageBox -title "CM Error" -message $msg -type ok -icon error
	    return
	}
    } elseif [string match dir* $cmd_file_str] {

	# User should have selected directorie(s).
	if {$is_dir == 0} {
	    set msg "No directory is selected. Unable to perform command."
	    status_line $msg
	    tk_messageBox -title "CM Error" -message $msg -type ok -icon error
	    return
	}
    }

    cm_process_items $key $sel_items

} ;# cm_action


proc cm_process_items {cmd_key items} {

    global CmCmdDisplayArr
    global CmCmdAttrArr

    # Here will be resulting error code.
    set ret_code 0

    # Get parameters of the current command.
    set cmd_disp_str $CmCmdDisplayArr($cmd_key)
    set cmd_attr_str $CmCmdAttrArr($cmd_key)

    # Transform string with attributes specification to list.
    set attr_list [split $cmd_attr_str "\t"]

    # M.b. we should fill some attruibiutes with latest values from 'cmdriver'.
    if {[cm_retrieve_attribute_values $attr_list] != 0} {

	# We couldn't get necessary information from 'cmdriver' -> exit.
	return
    }

    # Flag: should we ask user to enter attributes or not.
    set ask_user 1

    # Here will be exit code of 'Enter Attributes' dialog.
    global CmDlgExitCode

    # We will combine query string in this variable.
    set query_str ""

    # Process all files (directories) in selection.
    foreach fn $items {

	# Get real private file name.
	set real_fn [ join [ dis_command fname $fn ] ]
	set priv_fn [get_privatefname $real_fn]

	if {$ask_user == 1} {

	    # Ask user to enter (verify) attributes.
	    cm_enter_attributes $cmd_disp_str $priv_fn $attr_list

	    # What the button was pressed in the dialog.
	    if {$CmDlgExitCode == 0} {

		# User pressed 'Cancel' -> stop process the rest files.
		status_line "Command '$cmd_disp_str' was canceled by user."
		return
	    }

	    # Construct query to 'cmdriver'.
	    set attr_spec [cm_combine_attributes $attr_list]
	    set query_str "$cmd_key\t$attr_spec\t$priv_fn"

	    if {$CmDlgExitCode == 1} {

		# If user pressed 'OK' -> perform query for the current file.
		set ec [cm_exec_query $cmd_key $query_str]

		# Propagate error code further.
		if {$ec != 0} {
		    set ret_code $ec
		}

	    } elseif {$CmDlgExitCode == -1} {

		# If user pressed 'Apply to All' -> concatenate all file names.
		set ask_user 0
	    }

	} else {

	    # Don't ask user to enter attribute. Just concatenate file names.
	    set query_str "$query_str\t$priv_fn"
	}
    }

    # If we were collecting all file names -> perform query to 'cmdriver' now.
    if {$ask_user == 0} {
	set ec [cm_exec_query $cmd_key $query_str]

	# Propagate error code further.
	if {$ec != 0} {
	    set ret_code $ec
	}
    }

    # Return error code: 0 - all OK, !0 - some error occured.
    return $ret_code

} ;# cm_process_items


proc cm_enter_attributes {cmd_name file_name attr_list} {

    global CmAttrDisplayArr
    global CmAttrTypeArr  CmAttrValueArr

    global CmCheckboxRes  CmComboboxRes
    global CmTextRes

    # We will save here exit code. It is actually pressed button.
    global CmDlgExitCode

    # If nothing to ask -> just exit as 'Apply to All'.
    set len [llength $attr_list]
    if {$len == 0} {

	set CmDlgExitCode -1
	return
    }

    # If there are only 'autocomment' -> really nothing to ask.
    if {$len == 1
	    && [string compare [lindex $attr_list 0] "autocomment"] == 0} {

	set CmDlgExitCode -1
	return
    }

    # If there are only one-value lists -> nothing to ask -> just set variable.
    set must_ask 0
    foreach attr $attr_list {

	# Check type of this attribute.
	set attr_type $CmAttrTypeArr($attr)
	if [cm_is_pure_list_attr $attr_type] {

	    # Get all displayable values of this list.
	    set disp_list [cm_display_list_attr $attr_type]

	    # Check number of values.
	    if { [llength $disp_list] == 1 } {

		# Assign the single available value to this attribute.
		set CmAttrValueArr($attr) [lindex $disp_list 0]

	    } else {

		set must_ask 1
	    }
	} else {

	    set must_ask 1
	}
    }

    # Must we ask to enter attributes?
    if {$must_ask == 0} {

	set CmDlgExitCode -1
	return
    }

    # Create window.
    set w .inp_attrs_dlg
    toplevel $w -class Dialog
    wm title $w $cmd_name 
    wm iconname $w Dialog

    # Create three frames:
    # Top frame for file name and general message.
    frame $w.top -relief raised -bd 1
    pack $w.top -side top -fill x -ipadx 5 -ipady 5

    # Middle frame for all attributes that need to be entered.
    frame $w.mid -relief raised -bd 1
    pack $w.mid -side top -fill both -ipadx 5 -ipady 5

    # Bottom frame for buttons 'OK', 'Apply to All', and 'Cancel'.
    frame $w.bot -relief raised -bd 1
    pack $w.bot -side top -fill x -ipadx 5 -ipady 5

    label $w.top.file -anchor w -text $file_name
    if { [llength $attr_list] == 1 } {
	label $w.top.msg -anchor w -text "Enter the following parameter:"
    } else {
	label $w.top.msg -anchor w -text "Enter the following parameters:"
    }
    pack $w.top.file $w.top.msg -side top -fill x -padx 5 -pady 5

    # Middle frame contains all attributes.

    # Add all text attributes at first.
    foreach attr $attr_list {

	# Check type of this attribute.
	set attr_type $CmAttrTypeArr($attr)
	if [cm_is_text_attr $attr_type] {

	    # Get displayable name and default value of this attribute.
	    set attr_disp $CmAttrDisplayArr($attr)
	    set attr_val $CmAttrValueArr($attr)

	    # Create invisible frame for this attribute (label and text).
	    frame $w.mid.fr_$attr
	    pack $w.mid.fr_$attr -side top -fill both -padx 5 -pady 5

	    # Create label.
	    label $w.mid.fr_$attr.l -anchor w -text "$attr_disp:"
	    pack $w.mid.fr_$attr.l -side top -fill x

	    # Create text, where user will enter value of this attribute.
	    text $w.mid.fr_$attr.t -relief sunken -bd 2 -height 4 -width 60
	    pack $w.mid.fr_$attr.t -side top -fill both

	    # Add default text to pane.
	    set CmTextRes($attr) $w.mid.fr_$attr.t
	    $CmTextRes($attr) insert end $attr_val
	}
    }

    # Add all pure list attributes.
    foreach attr $attr_list {

	# Check type of this attribute.
	set attr_type $CmAttrTypeArr($attr)
	if [cm_is_pure_list_attr $attr_type] {

	    # Here combobox will save user's choice.
	    set CmComboboxRes($attr) [cm_search_list_attr_by_key \
		                             $attr_type $CmAttrValueArr($attr)]

	    # Displayable name of this attribute. We will show it near combobox.
	    set attr_disp $CmAttrDisplayArr($attr)

	    # Get all displayable values.
	    set disp_list [cm_display_list_attr $attr_type]

	    # Create invisible frame for this attribute (label and combobox).
	    frame $w.mid.fr_$attr
	    pack $w.mid.fr_$attr -side top -fill x -padx 5 -pady 5

	    # Create label.
	    label $w.mid.fr_$attr.l -anchor w -text "$attr_disp: "
	    pack $w.mid.fr_$attr.l -side left -anchor w -fill none

	    # Calculate proper width of combobox.
	    set combo_width 16
	    foreach elem $disp_list {
		set elem_len [string length $elem]
		incr elem_len 2
		if {$elem_len > $combo_width} {
		    set combo_width $elem_len
		}
	    }

	    # Create combobox.
	    combobox $w.mid.fr_$attr.c -width $combo_width -editable no \
		     -textvariable CmComboboxRes($attr) -exportselection no
	    pack $w.mid.fr_$attr.c -side left -anchor w -fill x
	    eval combobox_setlist $w.mid.fr_$attr.c $disp_list
	}
    }

    # Add all boolean attributes.
    foreach attr $attr_list {

	# Check type of this attribute.
	set attr_type $CmAttrTypeArr($attr)
	if [cm_is_boolean_attr $attr_type] {

	    # Get default value for this attribute.
	    set cur_val [cm_search_list_attr_by_key \
		               $attr_type $CmAttrValueArr($attr)]

	    # Here checkbox will save user's choice.
	    if {[string compare $cur_val "true"] == 0} {
		set CmCheckboxRes($attr) 1
	    } else {
		set CmCheckboxRes($attr) 0
	    }

	    # Get displayable name of this attribute.
	    set attr_disp $CmAttrDisplayArr($attr)

	    # Create checkbox.
	    checkbutton $w.mid.chk_$attr -text $attr_disp \
		        -variable CmCheckboxRes($attr) -anchor w
	    pack $w.mid.chk_$attr -side top -fill x -padx 5 -pady 3
	}
    }

    # Add three buttons in the bottom part of the window.
    button $w.bot.ok -text OK -command "set CmDlgExitCode 1"
    button $w.bot.all -text "Apply to All" -command "set CmDlgExitCode -1"
    button $w.bot.cancel -text Cancel -command "set CmDlgExitCode 0"
    pack $w.bot.ok $w.bot.all $w.bot.cancel -side left -fill x -padx 5 -pady 5

    # Backup previous focus.
    set old_focus [focus]

    # Show the dialog.
    grab set $w
    focus $w

    # Wait for response from user.
    tkwait variable CmDlgExitCode

    # Don't save changes if user pressed 'Cancel'.
    if {$CmDlgExitCode != 0} {

	# Retrieve all results and save it to default values array.
	foreach attr $attr_list {

	    # Check type of this attribute.
	    set attr_type $CmAttrTypeArr($attr)
	    if [cm_is_text_attr $attr_type] {

		# Retrieve value from text pane.
		set text_val [$CmTextRes($attr) get 1.0 end]

		# Remove trailing character '\n'. It was added by Tcl/Tk.
		if { [regexp {^(.*)\n$} $text_val dummy new_text_val] } {

		    set text_val $new_text_val
		}

		set CmAttrValueArr($attr) $text_val

	    } elseif [cm_is_pure_list_attr $attr_type] {

		# We should save key of selected element.
		set CmAttrValueArr($attr) [cm_search_list_attr_by_disp \
		                              $attr_type $CmComboboxRes($attr)]

	    } elseif [cm_is_boolean_attr $attr_type] {

		# Look at user's choice and convert it to our internal format.
		set chk_res $CmCheckboxRes($attr)
		if {$chk_res == 1} {

		    set CmAttrValueArr($attr) [cm_search_list_attr_by_disp \
		                                             $attr_type "true"]
		} else {
		    set CmAttrValueArr($attr) [cm_search_list_attr_by_disp \
		                                            $attr_type "false"]
		}
	    }
	}
    }

    # Close window and restore previous focus.
    destroy $w
    focus $old_focus

} ;# cm_enter_attributes


proc cm_is_text_attr {attr_type} {

    if {[string compare $attr_type "text"] == 0} {
	return 1
    }

    return 0

} ;# cm_is_text_attr


proc cm_is_pure_list_attr {attr_type} {

    if {[string compare $attr_type "text"] != 0} {
	if {[cm_is_boolean_attr $attr_type] == 0} {
	    return 1
	}
    }

    return 0

} ;# cm_is_pure_list_attr


proc cm_is_boolean_attr {attr_type} {

    if {[string compare $attr_type "text"] != 0} {

	if {[string first ";" $attr_type] != -1} {

	    set len [expr [string length $attr_type] - 2]
	    set values_list [split [string range $attr_type 1 $len] ";"]

	    if {[llength $values_list] == 2} {

		set val0_list [split [lindex $values_list 0] ","]
		set val1_list [split [lindex $values_list 1] ","]

		if {[llength $val0_list] == 2 && [llength $val1_list] == 2} {

		    if {[string compare [lindex $val0_list 1] "false"] == 0
		           && [string compare [lindex $val1_list 1] "true"] == 0} {

		        return 1
		    }

		    if {[string compare [lindex $val0_list 1] "true"] == 0
		           && [string compare [lindex $val1_list 1] "false"] == 0} {

		        return 1
		    }
		}
	    }
	}
    }

    return 0

} ;# cm_is_boolean_attr


proc cm_display_list_attr {spec} {

    # Here will be list with all displayable values of the given attribute.
    set disp_list ""

    # Transform list specification from string to list.
    set spec_len [expr [string length $spec] - 2]
    set spec_str [string range $spec 1 $spec_len]
    set spec_list [split $spec_str ";"]

    # Process all list elements.
    foreach elem_str $spec_list {

	# This is the current list element.
	set elem_list [split $elem_str ","]

	if {[llength $elem_list] == 2} {

	    # It is general case "key,display".
	    set elem_disp [lindex $elem_list 1]
	    lappend disp_list $elem_disp

	} else {

	    # It is simplified case "key". Use key as displayable name.
	    set elem_disp [lindex $elem_list 0]
	    lappend disp_list $elem_disp
	} 
    }

    return $disp_list

} ;# cm_display_list_attr


proc cm_search_list_attr_by_key {spec key} {

    # Here will be displayable name of the given list element.
    set disp ""

    # Transform list specification from string to list.
    set spec_len [expr [string length $spec] - 2]
    set spec_str [string range $spec 1 $spec_len]
    set spec_list [split $spec_str ";"]

    # Process all list elements.
    foreach elem_str $spec_list {

	# This is the current list element.
	set elem_list [split $elem_str ","]

	if {[llength $elem_list] == 2} {

	    # It is general case "key,display".
	    set elem_key [lindex $elem_list 0]
	    if {[string compare $elem_key $key] == 0} {
		set disp [lindex $elem_list 1]
		break
	    }
	} else {

	    # It is simplified case "key". Use key as displayable name.
	    set elem_key [lindex $elem_list 0]
	    if {[string compare $elem_key $key] == 0} {
		set disp $elem_key
		break
	    }
	}
    }

    return $disp

} ;# cm_search_list_attr_by_key


proc cm_search_list_attr_by_disp {spec disp} {

    # Here will be key of the given list element.
    set key ""

    # Transform list specification from string to list.
    set spec_len [expr [string length $spec] - 2]
    set spec_str [string range $spec 1 $spec_len]
    set spec_list [split $spec_str ";"]

    # Process all list elements.
    foreach elem_str $spec_list {

	# This is the current list element.
	set elem_list [split $elem_str ","]

	if {[llength $elem_list] == 2} {

	    # It is general case "key,display".
	    set elem_disp [lindex $elem_list 1]
	    if {[string compare $elem_disp $disp] == 0} {
		set key [lindex $elem_list 0]
		break
	    }
	} else {

	    # It is simplified case "key". Use key as displayable name.
	    set elem_key [lindex $elem_list 0]
	    if {[string compare $elem_key $disp] == 0} {
		set key $elem_key
		break
	    }
	}
    }

    return $key

} ;# cm_search_list_attr_by_disp


proc cm_combine_attributes {attr_list} {

    global CmAttrValueArr

    set res ""

    foreach attr $attr_list {

	set val $CmAttrValueArr($attr)

	if {[string length $res] > 0} {

	    set res "$res\t$attr=\"$val\""

	} else {

	    set res "$attr=\"$val\""
	}
    }

    return $res

} ;# cm_combine_attributes


proc cm_exec_query {cmd_key query_str} {

    global CmDish CMLabel
    global CmCmdDisplayArr
    global CmCmdReturnArr

    # Get parameters of this command.
    set cmd_disp $CmCmdDisplayArr($cmd_key)
    set cmd_ret  $CmCmdReturnArr($cmd_key)
    if {[string length $cmd_disp] == 0} {
	set cmd_disp $cmd_key
    }

    # Set "wait cursor".
    change_cursor watch
    update idletasks

    # Try to execute 'cmdish' with the given argument.
    set errc [catch {set res [exec $CmDish -query $query_str -cm $CMLabel]} res]

    # Restore normal cursor.
    change_cursor ""

    # If error -> show message in the dialog and in the status line.
    if $errc {

	# Show general message in the status line.
	status_line "Command '$cmd_disp' failed!"

	# And full error description in the message box.
	cm_show_error_message $res

    } else {

	# May be this command returns text or files.
	if {[string compare $cmd_ret "text"] == 0} {

	    status_line "Command '$cmd_disp' completed successfully."
	    tk_messageBox -title $cmd_disp -message $res -type ok -icon info

	} elseif {[string compare $cmd_ret "file"] == 0
		       || [string compare $cmd_ret "files"] == 0} {

	    # Convert result to list of files.
	    set file_list [split $res "\n"]

	    # Here we will save all files that are belong to the model.
	    set local_list {}

	    # Process all files in list and get etag of "local" files.
	    foreach fn $file_list {
		if {[string length $fn] > 0 && [dis_command module $fn] == 1} {
		    lappend local_list $fn
		}
	    }

	    # This is amount of files in "local" list.
	    set am [llength $local_list]

	    # Display this list in the browser.
	    display_set [dis_print $local_list]

	    if {$am == 1} {
		status_line "Command '$cmd_disp' returned 1 file."
	    } else {
		status_line "Command '$cmd_disp' returned $am files."
	    }

	} else {

	    status_line "Command '$cmd_disp' completed successfully."
	}
    }

    return $errc

} ;# cm_exec_query


proc  cm_retrieve_attribute_values attr_list {

    global CMLabel CmDish
    global CmAttrAskArr
    global CmAttrDisplayArr CmAttrTypeArr

    # Process all attributes in the list.
    foreach attr $attr_list {

	# Should we ask 'cmdriver' about this attribute?
	set ask $CmAttrAskArr($attr)

	if {[string length $ask] > 0} {

	    # Make query string.
	    set query_str "get_attr_value\t$attr"

	    # Exec query.
	    set errc [catch {set res [exec $CmDish -query $query_str \
		                                   -cm $CMLabel]} res]

	    # If error -> show message in the dialog and in the status line.
	    if $errc {

		set attr_disp $CmAttrDisplayArr($attr)

		set status_msg "Unable to retrieve attribute '$attr_disp'"
		set status_msg "$status_msg from 'cmdriver'!"

		# Show general message in the status line.
		status_line $status_msg

		# And full error description in the message box.
		cm_show_error_message $res

		# We can't proceed without having this attribute.
		return -1

	    } else {

		# Save result.
		set CmAttrTypeArr($attr) $res
	    }
	}
    }

    return 0

} ;# cm_retrieve_attribute_values


# END OF FILE

