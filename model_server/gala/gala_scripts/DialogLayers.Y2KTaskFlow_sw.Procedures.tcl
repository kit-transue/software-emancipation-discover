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
eval {
    global OK
    set OK 0

    proc FilterPageInitProc {} {
	global gd
	global  FilterName  PatternMatchingList  TypeMatchingList  FilterSave  FilterCancel
	global  CurrentFilterName  tempNameList  retval modified  retconfirm 
	global  GlobalFileName
	
	if { "$GlobalFileName" != "" } {
	    set FilterName $GlobalFileName
	    set GlobalFileName ""
	}
    }

    proc PatternsPageInitProc {} {
	global GlobalFileName
	global PatternName

	if { "$GlobalFileName" != "" } {
	    set PatternName $GlobalFileName
	    set GlobalFileName ""
	}
    }

    proc FlowPageInitProc {} {
	global SelectedNode

	if { $SelectedNode != -1 } {
	    dis_gdDrawingSelectItem TaskDraw $SelectedNode -1
	    set SelectedNode -1
	    dis_gdListViewUseInternalData PropertiesList
	}
    }

    proc OpenDialog {} {
        global gd
        global tempNameList  retval modified  retconfirm 
	global GlobalFileName

        set gd -1
	set GlobalFileName ""

	#Call procedure to init filter page
	BROWSER_FilterPageInitProc

	## enable/disable pages based on licenses
	if { ! [dis_aset_eval_cmd "license w feature"] } {
	    dis_gdNotebookPageEnable $gd Y2KFilterNotebook  FilterPage 0
	}
	if { ! [dis_aset_eval_cmd "license wb feature"] } {
	    dis_gdNotebookPageEnable $gd Y2KFilterNotebook PatternsPage 0
	}
	if { ! [dis_aset_eval_cmd "license wa feature"] } {
	    dis_gdNotebookPageEnable $gd Y2KFilterNotebook Flow 0
	}

	global PatternSetModified
	set PatternSetModified 0
	global PatternSave
	dis_gdInitButton $gd PatternSave { SaveCurrentPattern "" }
	global NewRow
	dis_gdInitButton $gd NewRow { AddNewPattern }
	dis_gdInitButton $gd DeleteRow { DeletePattern }

	global PatternSetType
	dis_gdInitExclusiveGroup $gd PatternSetType {}

        # Cancel button will call CancelPressed
	dis_gdInitButton $gd FilterCancel { CancelPressed }

	global TPMArray
	global TPMRows
	global PatternsList
	global PatternsListRuler
	dis_gdInitListView $gd PatternsList 4 {}
	set PatternsList {{ global TPMRows; return $TPMRows } { getTPMInfo $Item } {} { OpenTPMEditor $Item }}
	dis_gdListViewAddPopupItem PatternsList Cut    ListDeleteRow
	dis_gdListViewAddPopupItem PatternsList Copy   ListCopyRow
	dis_gdListViewAddPopupItem PatternsList Paste  ListPasteRow
	# dis_gdListViewAddPopupItem PatternsList Edit   ListEditRow
	dis_gdInitRuler $gd PatternsListRuler PatternsList {}
	
	global PatternRowSelected
	set PatternRowSelected -1
	global EditRow
	dis_gdInitButton $gd EditRow { 
	    global PatternRowSelected;
	    if { $PatternRowSelected >= 0 } { OpenTPMEditor [expr $PatternRowSelected * 4] }
	}

	global CurrentPattern
	global PatternName
	global PatternsNameList
	global PatternsFiles
	dis_gdInitButton $gd NewPattern CreateNewPatternSet

	set PatternsNameList "default"
	set default_pattern_name [dis_getenv PSETHOME]
	append default_pattern_name "/lib/default.ptn"
	set PatternsFiles(default) "$default_pattern_name"
	GetPatternsList
	set PatternsNameList [lsort -ascii $PatternsNameList]
	dis_gdInitPopDownComboBox $gd PatternName { ChangePattern }
	dis_gdComboBoxSetMatch PatternName 1
	dis_gdAddListToCombo $gd PatternName $PatternsNameList
	set CurrentPattern "default"
	set PatternName $CurrentPattern
	LoadPatternSet $CurrentPattern

	InitTaskEditor
    }

    #-------------------------------------------------------------------------------------------------------
    # This procedure will be called every time Y2K Filter page activated
    #-------------------------------------------------------------------------------------------------------
    proc ReopenFilterPage {} {
	global LastOpenedFilter
	global FilterName
	global CurrentFilter
	
	if { [info exists LastOpenedFilter] == 0 } {
	    return
	}
	set CurrentFilter ""
	set FilterName $LastOpenedFilter
	ChangeFilter
    }
    #-------------------------------------------------------------------------------------------------------


    #----------------------------------------------------------------------------------------------------
    # This procedure will be called to initialize filter page in Y2K notebook. gdListView C++ class is
    # used to represent SpreadSheet
    #
    # RESOURCES:
    #
    # FilterName       - combobox with all available filters resource name
    # NewFilter        - filter creation button resource name
    # SaveFilter       - current filter saving button resource name
    # TypeMatchList    - type match spreadsheet resource name
    # TypeMatchRuler   - type match spreadsheet ruler resource name
    # TextMatch        - text match spreadsheet recource name
    # TextMatchRuler   - text match spreadsheet ruler resource name
    #
    # GLOBALS:
    #
    # FilterName       - name of the initially selected filter
    # FilterSetMofified- durty flag. Will be not 0 if any of the filter spreadsheets are were modified
    # CurrentFilter    - TCL string with the name of the currently selected filter
    # FilterNameList   - TCL list with the names of all filters
    # FilterFiles      - TCL array of all files represented by names in FilterNameList
    # tagCurrentFilter - tag name of the currently selected filter
    # TypeMatchArray   - TCL ARRAY to represent TypeMatch spreadsheet rows
    # TextMatchArray   - TCL ARRAY to represent TextMatch spreadsheet rows
    # TypeMatchRows    - TCL integer to store the amount of rows in the TypeMatch spreadsheet
    # TextMatchRows    - TCL integer to store the amount of rows in the TextMatch spreadsheet
    # TypeMatchRowSelected- TCL to store  currently selected row in the TypeMatch spreadsheet
    # TextMatchRowSelected- TCL to store  currently selected row in the TextMatch spreadsheet
    # TypeMatchList    - TCL representation of the TypeMatch spreadsheet
    # TextMatchList    - TCL representation of the TextMatch spreadsheet
    # TypeMatchRuler   - TCL representation of the TypeMatch spreadsheet ruler
    # TextMatchRuler   - TCL representation of the TextMatch spreadsheet ruler
    #
    # CALLBACKS:
    #
    # ChangeFilter       - TCL procedure called when Filter Combo Box changed selection - different
    #                      filter selected
    # SaveCurrentFilter  - TCL procedure to save current filter spreadsheets. Attached to the FilterSave
    #                      button
    # AddNewFilter       - TCL procedure to xreate new filter & new file for it. Attached to the NewFilter
    #                      button
    #
    # TypeMatchCreateRow - <New>   - item was selected in the TypeMatch spreadsheet popup menu
    # TypeMatchDeleteRow - <Cut.   - item was selected in the TypeMatch spreadsheet popup menu
    # TypeMatchCopyRow   - <Copy>  - item was selected in the TypeMatch spreadsheet popup menu
    # *TypeMatchPasteRow  - <Paste> - item was selected in the TypeMatch spreadsheet popup menu
    # 
    # TextMatchCreateRow - <New>   - item was selected in the TextMatch spreadsheet popup menu
    # TextMatchDeleteRow - <Cut.   - item was selected in the TextMatch spreadsheet popup menu
    # TextMatchCopyRow   - <Copy>  - item was selected in the TextMatch spreadsheet popup menu
    # *TextMatchPasteRow  - <Paste> - item was selected in the TextMatch spreadsheet popup menu
    #
    # USES :
    #
    # GetFilterList      - will fill FilterNameList with the saved filters names and FilterFiles with 
    #                      the physical files to read/write this filters
    # LoadFilterSet      - will load all information about currently selected filter.
    #----------------------------------------------------------------------------------------------------
    proc BROWSER_FilterPageInitProc {} {
	global gd
	global FilterSetModified
	global FilterSave
	global TypeMatchArray
	global TextMatchArray
	global CurrentTypeMatchRow
	global CurrentTextMatchRow


    set CurrentTypeMatchRow 0
	set CurrentTextMatchRow 0

    global FilterCancel

    # filter spreadsheets were not modified
	set FilterSetModified 0


        #                ----------------- SAVE & NEW BUTTONS -----------------
	# TCL FilterSave callback will be started when pushing on FilterSave button with empty string
	dis_gdInitButton $gd SaveFilter { SaveCurrentFilter "" }

	# TCL NewFilter callback will be started when pushing NewFilter button with no parameters
	dis_gdInitButton $gd NewFilter  { AddNewFilter }

	# TCL TypeMatchCreateRow callback will be started when pushing NewTypeMatch button with 
	#no parameters
	dis_gdInitButton $gd NewTypeMatch  { TypeMatchCreateRow }

	# TCL TextMatchCreateRow callback will be started when pushing NewTextMatch button with 
	#no parameters
	dis_gdInitButton $gd NewTextMatch  { TextMatchCreateRow }

	# TCL TypeMatchDeleteCurrentRow callback will be started when pushing DeleteTypeMatch button with 
	# no parameters. Current type match row will be removed
	dis_gdInitButton $gd DeleteTypeMatch  { TypeMatchDeleteCurrentRow }

    # TCL TextMatchDeleteRow callback will be started when pushing DeleteTextMatch button with 
	# no parameters. Current text match row will be removed.
    dis_gdInitButton $gd DeleteTextMatch  { TextMatchDeleteCurrentRow }

    # cancel button - will close this dialog
    dis_gdInitButton $gd FilterCancel  { CancelPressed }


        #                ----------------- TYPE MATCH SPREADSHEET --------------- 
	# INTEGER. The quantity of the rows in the TypeMatch spreadsheet
	global TypeMatchRows

	# INTEGER. Currently selected TypeMatch row
	global TypeMatchRowSelected

	# represent in TCL C++ spreadsheet rows. 
	# Same name as spreadsheet resource
	global TypeMatchList
	# represent in TCL C++ spreadsheet ruler,
	# Same name as spreadsheet ruler resource.
	global TypeMatchRuler

	# TypeMatching spreadsheet will be connected to the appropriate rerource 
	#with the name TypeMatchList
	dis_gdInitListView $gd TypeMatchList 4 {}
	dis_gdListViewUseInternalData TypeMatchList
    dis_gdListViewSetChangeCallback TypeMatchList callbackTypeEditor
    dis_gdListViewSetCellChange TypeMatchList TypeMatchRowChanged


	# TypeMatch spreadsheet will contain 0 rows when initializing
	set TypeMatchRows 0
	# No selected row at the begining
	set TypeMatchRowSelected -1


        # TYPE MATCH RULER
        # TypeMatch spreadsheet ruler will be connected to the appropriate resource 
	# with the name TypeMatchRuler
	dis_gdInitRuler $gd TypeMatchRuler TypeMatchList {}


        #                ----------------- TEXT MATCH SPREADSHEET --------------- 
	# INTEGER. The quantity of the rows in the TextMatch spreadsheet
	global TextMatchRows

	# INTEGER. Currently selected TextMatch row
	global TextMatchRowSelected

	# represent in TCL C++ spreadsheet rows. 	
	# Same name as spreadsheet resource
	global TextMatchList
	# represent  in TCL C++ spreadsheet ruler
	# Same name as spreadsheet ruler resource
	global TextMatchRuler

	# TextMatching spreadsheet will be connected to the appropriate rerource 
	#with the name TextMatchList
	dis_gdInitListView $gd TextMatchList 6 {}
	dis_gdListViewUseInternalData TextMatchList
    dis_gdListViewSetChangeCallback TextMatchList callbackTextEditor
    dis_gdListViewSetCellChange TextMatchList TextMatchRowChanged


	# TextMatch spreadsheet will contain 0 rows when initializing
	set TextMatchRows 0
	# No selected row at the begining
	set TextMatchRowSelected -1



        # TEXT MATCH RULER
        # TextMatch spreadsheet ruler will be connected to the appropriate resource 
	# with the name TypeMatchRuler
	dis_gdInitRuler $gd TextMatchRuler TextMatchList {}

        #                   -------------- FILTER NAMES COMBO BOX --------------------
        # TCL string to store current filter name
	global CurrentFilter
	# TCL string wich also will contain current filter name
	global FilterName
	# list with all filters names
	global FilterNameList
	# array with all filter physical files
	global FilterFiles
	#current filter tag
	global tagCurrentFilter

        # Filter name list will be set to show only one name - <default>
	set FilterNameList "default"
	set FilterName     ""
	set CurrentFilter  "default"
        set gd -1

	# logic to get default filter filename 
	set tagCurrentFilter $CurrentFilter
	set pset_home [dis_getenv PSETHOME]
	regsub -all \\\\ $pset_home / default_filter_name
	append default_filter_name "/lib/default.flt"
	set FilterFiles(default) "$default_filter_name"

	# Server call to fill two globals :
	# <FilterNameList> - TCL LIST with all names of all filters on the server
	# <FilterFiles>    - TCL ARRAY with all physical names of the filter files 
	GetFilterList


	#sort filter names list. Use ASCII coding.
	set FilterNameList [lsort -ascii $FilterNameList]

        # connecting filter selection combo box with an appropriate C++ class.
	# resource name is <FilterName>
	# <ChangeFilter> TCL callback will be called every time selection changed.
	dis_gdInitPopDownComboBox $gd FilterName { ChangeFilter }
        

        dis_gdComboBoxSetMatch FilterName 1

	# initially, this combo box will be empty.
	dis_gdClearCombo $gd FilterName


	# all items inside this combobox will be filled from the FilterNameList
	dis_gdAddListToCombo $gd FilterName $FilterNameList

        ChangeFilter

	# All filter internals will be loaded from current filter file 
	# into spreadsheets
	LoadFilterSet $CurrentFilter
    }
    #------------------------------------------------------------------------------------------------------


    #------------------------------------------------------------------------------------------------------
    # CALLBACK will run when user press Cancel button. This callback will close all TPM Flow dialog.
    #-----------------------------------------------------------------------------------------------------
     proc CancelPressed {} {
        dis_quit
     }
    #------------------------------------------------------------------------------------------------------

    #------------------------------------------------------------------------------------------------------
    # This procedure will be called by BROWSER_FilterPageInitProc to form all available filter names in
    # TCL global FilterNameList and all available file paths in TCL global FilterFiles
    # It will call server with <filter> parameter to obtain file list.
    #
    # GLOBALS:
    #   FilterNameList - TCL LIST.  Will store all names of the available filters
    #   FilterFiles    - TCL ARRAY. Will store all filters paths.
    #------------------------------------------------------------------------------------------------------
    proc GetFilterList {} {
	global FilterFiles
	global FilterNameList
	
	# server call - take all file names in the area <filter>
	set files [dis_aset_eval_cmd "apply fname \[extfile list filter\]"]
	#filling array and list
	foreach f $files {
	    set d [file dirname $f]
	    regsub "$d." $f "" tmp
	    set e [file extension $tmp]
	    regsub $e $tmp "" filter_name
	    set FilterFiles($filter_name) "$f"
	    lappend FilterNameList $filter_name
	}
    }
    #------------------------------------------------------------------------------------------------------

    
    #------------------------------------------------------------------------------------------------------
    # This procedure will be called by BROWSER_FilterPageInitProc to load all data from currently 
    # selected filter file.
    #
    # GLOBALS:
    #   
    #   FilterNameList
    #   FilterFiles
    #   FilterSetMofified- durty flag. Will be not 0 if any of the filter spreadsheets are were modified
    #   TypeMatchArray   - TCL representation of thr TypeMatch rows
    #   TextMatchArray   - TCL representation of the TextMatch rows
    #   TypeMatchList    - TCL representation of the TypeMatch spreadsheet
    #   TextMatchList    - TCL representation of the TextMatch spreadsheet
    #   TypeMatchRows    - TCL integer to store the amount of rows in the TypeMatch spreadsheet
    #   TextMatchRows    - TCL integer to store the amount of rows in the TextMatch spreadsheet
    #   TypeMatchRowSelected- TCL to store  currently selected row in the TypeMatch spreadsheet
    #   TextMatchRowSelected- TCL to store  currently selected row in the TextMatch spreadsheet
    #   
    # USES:
    #
    #   
    #------------------------------------------------------------------------------------------------------
    proc LoadFilterSet { name } {
	global FilterNameList
	global FilterFiles
	global TypeMatchArray
	global TypeMatchRows
	global TextMatchArray
	global TextMatchRows
	global TypeMatchList
	global TextMatchList
	global TypeMatchRowSelected
	global TextMatchRowSelected
	global FilterSetModified
	
	set fname $FilterFiles($name)
	catch  { 
	    unset TypeMatchArray
	    unset TextMatchArray
	}

	set f [open $fname "r"]

	# version information readed
	gets $f ver_info

	# obtaining amount of rows in TypeMatch spreadsheet and amount of rows in TextMatch spreadsheet
	set readed [gets $f type_match_rows]
	if { $readed > 0 } {
	    set TypeMatchRows $type_match_rows
	}
	set readed [gets $f text_match_rows]
	if { $readed > 0 } {
	    set TextMatchRows $text_match_rows
	}

	# Fillint TypeMatchArray
	# readinr from the current file pointer to the end of the file or to reach TypeMatchRows
	set row 0
	while { $row != $type_match_rows } {
	    #taking the line number 
	    set num [gets $f line]
	    if { $num > 0 } {
		set TypeMatchArray($row,1) [lindex $line 0]
		set TypeMatchArray($row,2) [lindex $line 1]
		set TypeMatchArray($row,3) [lindex $line 2]
		set typeRow [expr $row + 1]
		lappend typeRow $TypeMatchArray($row,1)
		lappend typeRow $TypeMatchArray($row,2)
		lappend typeRow $TypeMatchArray($row,3)
		SetupTypeMatchRow $row $typeRow
		incr row
	    }
	}
	dis_gdListViewRefresh TypeMatchList	


	# Fillint TextMatchArray
	# readinr from the current file pointer to the end of the file or to reach TextMatchRows
	set row 0
	while { $row != $text_match_rows } {
	    #taking the line number 
	    set num [gets $f line]
	    if { $num > 0 } {
		set TextMatchArray($row,1) [lindex $line 0]
		set TextMatchArray($row,2) [lindex $line 1]
		set TextMatchArray($row,3) [lindex $line 2]
		set TextMatchArray($row,4) [lindex $line 3]
		set TextMatchArray($row,5) [lindex $line 4]
		set textRow [expr $row + 1]
		lappend textRow $TextMatchArray($row,1)
		lappend textRow $TextMatchArray($row,2)
		lappend textRow $TextMatchArray($row,3)
		lappend textRow $TextMatchArray($row,4)
		lappend textRow $TextMatchArray($row,5)
		SetupTextMatchRow $row $textRow
		incr row
	    }
	}
	dis_gdListViewRefresh TextMatchList	
	set FilterSetModified 0

	# setting the selected row for the TypeMatch spreadsheet
	if { $TypeMatchRows > 0 } {
	    set TypeMatchRowSelected 0
	} else {
	    set TypeMatchRowSelected -1	
	}

	# setting the selected row for the TextMatch spreadsheet
	if { $TextMatchRows > 0 } {
	    set TextMatchRowSelected 0
	} else {
	    set TextMatchRowSelected -1	
	}

	# closing filter file
	close $f
    }
    #----------------------------------------------------------------------------------------------------


    #----------------------------------------------------------------------------------------------------
    # CALLBACK. Selection changed in filter combo box.
    #
    # GLOBALS:
    #
    #  FilterName        - represents filter combo box
    #  CurrentFilter     - represents currently selected filter
    #  FilterSetModified - durty flag  -filter data
    #  tagCurrentFilter  - currently opened filter tag
    #  LastOpenedFilter  - used to reopen current filter page
    #
    # USES:
    #
    # SaveCurrentFilter - writes currently selected filter data to it's file
    # LoadFilterSet     - loads all data according to the currently selected filter
    #
    #----------------------------------------------------------------------------------------------------
    proc ChangeFilter {} {	
	global FilterName
	global FilterNameList
	global FilterFiles
	global CurrentFilter
	global FilterSetModified
	global tagCurrentFilter
	global CurrentTypeMatchRow
	global CurrentTextMatchRow
	global LastOpenedFilter
	global gd


	dis_gdListViewFlush TypeMatchList
	dis_gdListViewFlush TextMatchList
	if { $CurrentFilter == $FilterName } { return }
	if { $FilterSetModified != 0 && "$CurrentFilter" != "default" } {
	    set is_ok [dis_confirm2 "" "Save" "Discard" "Current filter set has been modified. Do you want to save your changes?"]
	    if { $is_ok == "LeftButton" } { SaveCurrentFilter "" }
	}

	# reading the first line from the FLT file
	set fname $FilterFiles($FilterName)
	set f [open $fname "r"]
	gets $f first_filter_word
	close $f 

	# testing for old version
	if { "$first_filter_word" != "@@FLT01"} {
	    ConvertFilter $fname
	    set f [open $fname "r"]
	    gets $f first_filter_word
	    close $f 
	    if { "$first_filter_word" != "@@FLT01"} {
		dis_confirm1 "Version Mismatch" "OK" "Unable to convert old style filter.\nPlease make sure file:\n$fname is writable."
		set gd -1
		set FilterName "default"
	    }
	} 
	set CurrentTypeMatchRow 0
	set CurrentTextMatchRow 0
	set CurrentFilter $FilterName
	set tagCurrentFilter $CurrentFilter
	dis_gdListViewUseInternalData TypeMatchList
	dis_gdListViewUseInternalData TextMatchList
	LoadFilterSet $CurrentFilter
	dis_gdListViewRefresh TypeMatchList
	dis_gdListViewRefresh TextMatchList
	set LastOpenedFilter $FilterName
    }
    #----------------------------------------------------------------------------------------------------

    # procedure to convert old style filter into new
    # we probably will not need this in future versions
    proc ConvertFilter { filename } {
	set f [open $filename "r"]
	set bytes [gets $f version]
	# new edition of the file - need not to be converted
	if { "$version" == "@@FLT01" } { return }
	
	set old_version 1
	if { "[lindex $version 0]" == "@@filter_version_2" } {
	    set old_version 0
	}
	#### looking for the TextMatch spreadsheet start
	if { [lindex $version 0] != "#" } {
	    while { [eof $f] == "0" } {
		set num [gets $f line]
		set marker [lindex $line 0]
		if { "$marker" == "#" } { break }
	    }
	}
	#### reading text match spreadsheet until #
	set text_match_rows 0
	while { 1 } {
	    set num [gets $f line]
	    if { $num > 0 } {
		set marker [lindex $line 0]
		if { "$marker" == "!" } {
		    if { $old_version == 1 } {
			set TextMatchArray($text_match_rows,1) [lindex $line 1]
			set TextMatchArray($text_match_rows,2) [lindex $line 2]
			set TextMatchArray($text_match_rows,3) [lindex $line 3]
			set TextMatchArray($text_match_rows,4) [lindex $line 4]
			set TextMatchArray($text_match_rows,5) [lindex $line 5]
		    } else {
			set TextMatchArray($text_match_rows,1) [lindex $line 1]
			set TextMatchArray($text_match_rows,3) [lindex $line 2]
			set TextMatchArray($text_match_rows,4) [lindex $line 3]
			set TextMatchArray($text_match_rows,5) [lindex $line 4]
			set llen [llength $line]
			set TextMatchArray($text_match_rows,2) [lrange $line 5 $llen]
		    }
		    incr text_match_rows
		}
		if { "$marker" == "#" } { break  }
	    } else {
		if [eof $f] {
		    close $f
		    return
		}
	    }
	}
	
	gets $f line

	#### reading type match spreadsheet until EOF
	set type_match_rows 0
	while { 1 } {
	    set num [gets $f line]
	    if { $num > 0 } {
		set marker [lindex $line 0]
		if { "$marker" == "!" } {
		    set TypeMatchArray($type_match_rows,1) [lindex $line 1]
		    set TypeMatchArray($type_match_rows,2) [lindex $line 2]
		    set TypeMatchArray($type_match_rows,3) [lindex $line 3]
		    incr type_match_rows
		}
	    } else { 
		if [eof $f] {
		    break;
		}
	    }
	}
	close $f
	set f [open $filename "w"]

	#### Saving header
	puts $f "@@FLT01"
	# saving TypeMatch rows amount
	puts $f $type_match_rows
	# saving TextMatch rows amount
	puts $f $text_match_rows
	# saving TypeMatch spreadsheet

	#### Saving TypeMatch spreadsheet
	set cur_row 0
	while { $cur_row < $type_match_rows } {
	    set line ""
	    lappend line $TypeMatchArray($cur_row,1)
	    lappend line $TypeMatchArray($cur_row,2)
	    if { "$TypeMatchArray($cur_row,3)" == "NULL" } {
		lappend line {}
	    } else {
		lappend line $TypeMatchArray($cur_row,3)
	    }
	    puts $f $line
	    incr cur_row
	}

	#### Saving TextMatch spreadsheet
	set cur_row 0
	while { $cur_row < $text_match_rows} {
	    set line ""
	    lappend line $TextMatchArray($cur_row,1)
	    if { "$TextMatchArray($cur_row,2)" == "NULL" } {
		lappend line {}
	    } else {
		lappend line $TextMatchArray($cur_row,2)
	    }
	    lappend line $TextMatchArray($cur_row,3)
	    lappend line $TextMatchArray($cur_row,4)
	    lappend line $TextMatchArray($cur_row,5)
	    puts $f $line
	    incr cur_row
	}

	close $f
    }


    #----------------------------------------------------------------------------------------------------
    # CALLBACK will save the filter with the name <name> into it's file.
    #
    # GLOBALS:
    #   
    #   FilterNameList
    #   FilterFiles
    #   FilterSetMofified- durty flag. Will be not 0 if any of the filter spreadsheets are were modified
    #   TypeMatchArray   - TCL representation of thr TypeMatch rows
    #   TextMatchArray   - TCL representation of the TextMatch rows
    #   TypeMatchList    - TCL representation of the TypeMatch spreadsheet
    #   TextMatchList    - TCL representation of the TextMatch spreadsheet
    #   TypeMatchRows    - TCL integer to store the amount of rows in the TypeMatch spreadsheet
    #   TextMatchRows    - TCL integer to store the amount of rows in the TextMatch spreadsheet
    #   TypeMatchRowSelected- TCL to store  currently selected row in the TypeMatch spreadsheet
    #   TextMatchRowSelected- TCL to store  currently selected row in the TextMatch spreadsheet
    #----------------------------------------------------------------------------------------------------
    proc SaveCurrentFilter { name } {
	global FilterNameList
	global FilterFiles
	global FilterSetModified
	global TypeMatchArray
	global TextMatchArray
	global TypeMatchList
	global TextMatchList
	global TypeMatchRows
	global TextMatchRows
	global CurrentFilter

    if { $FilterSetModified == 0 } {
	   return
	}

	# if any table was in the editor mode we need to save changes and close editors
	dis_gdListViewFlush TypeMatchList
	dis_gdListViewFlush TextMatchList
	# if name is empty we will use our currently selected filter name
	if { "$name" == "" } { set name $CurrentFilter }

	# can't re-write our default filter
	if { "$name" == "default" || "$name" == "Default" } {
	    dis_confirm1 "Save" "OK" [concat Default filter can not be changed!]
	    return
	}

	set fname $FilterFiles($name)
	set type_match_rows $TypeMatchRows
	set text_match_rows $TextMatchRows

	set f [open $fname "w"]
	puts $f "@@FLT01"
	# saving TypeMatch rows amount
	puts $f $TypeMatchRows
	# saving TextMatch rows amount
	puts $f $TextMatchRows

	# saving TypeMatch spreadsheet
	set cur_row 0
	while { $cur_row < $type_match_rows } {
	    set line ""
	    lappend line $TypeMatchArray($cur_row,1)
	    lappend line $TypeMatchArray($cur_row,2)
	    lappend line $TypeMatchArray($cur_row,3)
	    puts $f $line
	    incr cur_row
	}

	#saving TextMatch spreadsheet
	set cur_row 0
	while { $cur_row < $text_match_rows} {
	    set line ""
	    lappend line $TextMatchArray($cur_row,1)
	    lappend line $TextMatchArray($cur_row,2)
	    lappend line $TextMatchArray($cur_row,3)
	    lappend line $TextMatchArray($cur_row,4)
	    lappend line $TextMatchArray($cur_row,5)
	    puts $f $line
	    incr cur_row
	}

	# clear our durty flag
	set FilterSetModified 0
	close $f
    }
    #----------------------------------------------------------------------------------------------------

    #----------------------------------------------------------------------------------------------------
    # CALLBACK will ask server to create the filter saving file with given name in the filter files area
    #
    # GLOBALS :
    #
    #   FilterName        - TCL representation of the filter combo box
    #   FilterNameList    - TCL LIST with all available filter names in it
    #   FilterFiles       - TCL ARRAY with all available filter files in it 
    #   FilterSetModified - our filter durty flag
    #
    # USES:
    #
    #   SaveCurrentFilter 
    #----------------------------------------------------------------------------------------------------
    proc AddNewFilter {} {
	global FilterName
	global FilterNameList
	global FilterFiles
	global FilterSetModified
	global gd

       set gd -1
       dis_gdListViewFlush TypeMatchList
	   dis_gdListViewFlush TextMatchList
	   set is_ok [dis_prompt GetStringDialog "New Filter" "Enter the name for new filter"]
	   if { $is_ok != "OK" } { return }
	   set tmp_name [dis_getvar -global ConDValue]
	   regsub -all {[^A-Za-z0-9]+} $tmp_name "_" name
           dis_aset_eval_cmd "filter_save $name"
	   set ind [lsearch $FilterNameList $name]
	   if { $ind > 0 } { return }

	   #saving our old filter
	   SaveCurrentFilter $FilterName

       # server request to create new filter file
	   set file_name [dis_aset_eval_cmd "fname \[extfile new filter $name\]"]

	   set f [open $file_name "w"]
       puts $f "@@FLT01"
	   set TypeMatchRows  0
	   set TextMatchRows  0
	   puts $f $TypeMatchRows
	   puts $f $TextMatchRows
	   close $f

	   # adding our new name to the filter names list
	   set FilterNameList [concat $FilterNameList $name]

	   #adding our new filter filename to the filter files array
	   set FilterFiles($name) "$file_name"

	   # sorting our new filter names list
	   set FilterNameList [lsort -ascii $FilterNameList]

	   # filter combo box will be empty
	   dis_gdClearCombo $gd FilterName

	   # and filled with our new filter list
	   dis_gdAddListToCombo $gd FilterName $FilterNameList

	   dis_gdListViewUseInternalData TypeMatchList
	   dis_gdListViewUseInternalData TextMatchList

	   # saving our new filter name in the FilterName global
	   set FilterName $name

	   #clearing our durty flag
	   set FilterSetModified 0
    }
    #-----------------------------------------------------------------------------------------------------
	#-----------------------------------------------------------------------------------------------------
	# CALLBACK will create new empty row in the TypeMatch spreadsheet.
	#
	# GLOBALS:
	#
	#   TypeMatchArray - TCL representation of the spreadsheet data
	#   TypeMatchRows  - amount of rows in the TypeMatch spreadsheet
	#   TypeMatchList  - TCL representation of the TypeMatch C++ spreadsheet 
	#
	# USES:
	#
	#-----------------------------------------------------------------------------------------------------
    proc TypeMatchCreateRow {} {
	global TypeMatchArray
	global TypeMatchRows
	global CurrentTypeMatchRow
	global FilterSetModified
	
	   dis_gdListViewChangeCheckStatus  TypeMatchList $CurrentTypeMatchRow 0 0
	   set copy_from $TypeMatchRows
	   set copy_to $TypeMatchRows
	   if { $TypeMatchRows != 0 } { incr CurrentTypeMatchRow 1 }
	   incr copy_from -1
	   while { $copy_from >= $CurrentTypeMatchRow } {
	       set TypeMatchArray($copy_to,1) $TypeMatchArray($copy_from,1)
	       set TypeMatchArray($copy_to,2) $TypeMatchArray($copy_from,2)
	       set TypeMatchArray($copy_to,3) $TypeMatchArray($copy_from,3)

           set typeRow $copy_to
	       lappend typeRow $TypeMatchArray($copy_to,1)
	       lappend typeRow $TypeMatchArray($copy_to,2)
	       lappend typeRow $TypeMatchArray($copy_to,3)
	       SetupTypeMatchRow $copy_to $typeRow

	       incr copy_from -1
	       incr copy_to -1
	   }
	   set TypeMatchArray($CurrentTypeMatchRow,1) "1"
	   set TypeMatchArray($CurrentTypeMatchRow,2) ""
	   set TypeMatchArray($CurrentTypeMatchRow,3) ""

       set typeRow $CurrentTypeMatchRow
	   lappend typeRow $TypeMatchArray($CurrentTypeMatchRow,1)
	   lappend typeRow $TypeMatchArray($CurrentTypeMatchRow,2)
	   lappend typeRow $TypeMatchArray($CurrentTypeMatchRow,3)
	   SetupTypeMatchRow $CurrentTypeMatchRow $typeRow
        
	   incr TypeMatchRows
	   dis_gdListViewChangeCheckStatus  TypeMatchList $CurrentTypeMatchRow 0 1
	   dis_gdListViewRefresh TypeMatchList	
	   set FilterSetModified 1
    }
	#-----------------------------------------------------------------------------------------------------


	#-----------------------------------------------------------------------------------------------------
	# CALLBACK will create new empty row in the TextMatch spreadsheet.
	#
	# GLOBALS:
	#
	#   TextMatchArray - TCL representation of the spreadsheet data
	#   TextMatchRows  - amount of rows in the TypeMatch spreadsheet
	#   TextMatchList  - TCL representation of the TypeMatch C++ spreadsheet 
	#
	# USES:
	#
	#   
	#-----------------------------------------------------------------------------------------------------
    proc TextMatchCreateRow {} {
	global TextMatchArray
	global TextMatchRows
	global FilterSetModified
	global CurrentTextMatchRow
	
	   set copy_from $TextMatchRows
	   set copy_to $TextMatchRows
	   dis_gdListViewChangeCheckStatus  TextMatchList $CurrentTextMatchRow 0 0
	   if { $TextMatchRows != 0 } { incr CurrentTextMatchRow 1 }
	   incr copy_from -1
	   while { $copy_from >= $CurrentTextMatchRow } {
	       set TextMatchArray($copy_to,1) $TextMatchArray($copy_from,1)
	       set TextMatchArray($copy_to,2) $TextMatchArray($copy_from,2)
	       set TextMatchArray($copy_to,3) $TextMatchArray($copy_from,3)
	       set TextMatchArray($copy_to,4) $TextMatchArray($copy_from,4)
	       set TextMatchArray($copy_to,5) $TextMatchArray($copy_from,5)
           set textRow $copy_to
	       lappend textRow $TextMatchArray($copy_to,1)
	       lappend textRow $TextMatchArray($copy_to,2)
	       lappend textRow $TextMatchArray($copy_to,3)
	       lappend textRow $TextMatchArray($copy_to,4)
	       lappend textRow $TextMatchArray($copy_to,5)
	       SetupTextMatchRow $copy_to $textRow
	       incr copy_from -1
	       incr copy_to -1
	   }
	   set TextMatchArray($CurrentTextMatchRow,1) "1"
	   set TextMatchArray($CurrentTextMatchRow,2) ""
	   set TextMatchArray($CurrentTextMatchRow,3) "1"
	   set TextMatchArray($CurrentTextMatchRow,4) "1"
	   set TextMatchArray($CurrentTextMatchRow,5) "1"
       set textRow $CurrentTextMatchRow
	   lappend textRow $TextMatchArray($CurrentTextMatchRow,1)
	   lappend textRow $TextMatchArray($CurrentTextMatchRow,2)
	   lappend textRow $TextMatchArray($CurrentTextMatchRow,3)
	   lappend textRow $TextMatchArray($CurrentTextMatchRow,4)
	   lappend textRow $TextMatchArray($CurrentTextMatchRow,5)
	   SetupTextMatchRow $CurrentTextMatchRow $textRow
	   incr TextMatchRows
	   dis_gdListViewChangeCheckStatus  TextMatchList $CurrentTextMatchRow 0 1
	   dis_gdListViewRefresh TextMatchList	
	   set FilterSetModified 1
    }
	#-----------------------------------------------------------------------------------------------------


    #-----------------------------------------------------------------------------------------------------
    # CALLBACK will remove currently selected row from the TypeMatch spreadsheet.
    #
    # GLOBALS:
    #
    #   TypeMatchArray - TCL representation of the spreadsheet data
    #   TypeMatchRows  - amount of rows in the TypeMatch spreadsheet
    #   TypeMatchList  - TCL representation of the TypeMatch C++ spreadsheet 
    #   Clipboard      - system clipboard
    #
    # USES:
    #
    #   
    #-----------------------------------------------------------------------------------------------------
    proc TypeMatchDeleteRow { row col } {
	global TypeMatchArray
	global TypeMatchRows
	global Clipboard
	
	set Clipboard "@" 
	set copy_row $row
	lappend Clipboard $TypeMatchArray($copy_row,1)
	lappend Clipboard $TypeMatchArray($copy_row,2)
	lappend Clipboard $TypeMatchArray($copy_row,3)
	incr copy_row
	if { $copy_row < $TypeMatchRows } {
	    while { $copy_row < $TypeMatchRows } {

		set TypeMatchArray($row,1) $TypeMatchArray($copy_row,1)
		set TypeMatchArray($row,2) $TypeMatchArray($copy_row,2)
		set TypeMatchArray($row,3) $TypeMatchArray($copy_row,3)

		set typeRow $row
		lappend typeRow $TypeMatchArray($row,1)
		lappend typeRow $TypeMatchArray($row,2)
		lappend typeRow $TypeMatchArray($row,3)
		SetupTypeMatchRow $row $typeRow

		incr row
		incr copy_row
	    }
	}
	incr TypeMatchRows -1
	dis_gdListViewRemoveRow TypeMatchList $TypeMatchRows
	dis_gdListViewRefresh TypeMatchList
    }
    #------------------------------------------------------------------------------------------------------

    #-----------------------------------------------------------------------------------------------------
    # CALLBACK will remove currently selected row from the TextMatch spreadsheet.
    #
    # GLOBALS:
    #
    #   TextMatchArray - TCL representation of the spreadsheet data
    #   TextMatchRows  - amount of rows in the TypeMatch spreadsheet
    #   TextMatchList  - TCL representation of the TypeMatch C++ spreadsheet 
    #   Clipboard      - system clipboard
    #
    # USES:
    #
    #   
    #-----------------------------------------------------------------------------------------------------
    proc TextMatchDeleteRow { row col } {
	global TextMatchArray
	global TextMatchRows
	global Clipboard
	
	set Clipboard "$" 
	set copy_row $row
	lappend Clipboard $TextMatchArray($copy_row,1)
	lappend Clipboard $TextMatchArray($copy_row,2)
	lappend Clipboard $TextMatchArray($copy_row,3)
	lappend Clipboard $TextMatchArray($copy_row,4)
	lappend Clipboard $TextMatchArray($copy_row,5)
	incr copy_row
	if { $copy_row < $TextMatchRows } {
	    while { $copy_row < $TextMatchRows } {

		set TextMatchArray($row,1) $TextMatchArray($copy_row,1)
		set TextMatchArray($row,2) $TextMatchArray($copy_row,2)
		set TextMatchArray($row,3) $TextMatchArray($copy_row,3)
		set TextMatchArray($row,4) $TextMatchArray($copy_row,4)
		set TextMatchArray($row,5) $TextMatchArray($copy_row,5)

		set textRow $row
		lappend textRow $TextMatchArray($row,1)
		lappend textRow $TextMatchArray($row,2)
		lappend textRow $TextMatchArray($row,3)
		lappend textRow $TextMatchArray($row,4)
		lappend textRow $TextMatchArray($row,5)
		SetupTextMatchRow $row $textRow

		incr row
		incr copy_row
	    }
	}
	incr TextMatchRows -1
	dis_gdListViewRemoveRow TextMatchList $TextMatchRows
	dis_gdListViewRefresh TextMatchList	
    }
    #------------------------------------------------------------------------------------------------------

	#-----------------------------------------------------------------------------------------------------
	# CALLBACK will set current row in the TypeMatch spreadsheet and modify the state of the 0 column to
	# show arrow - selected row.
	#
	# GLOBALS:
	#
	#     CurrentTypeMatchRow - selected row in the TypeMatch spreadsheet
	#
	# USES:
	#
	#-----------------------------------------------------------------------------------------------------
    proc TypeMatchRowChanged { row col oldrow oldcol} {
	global CurrentTypeMatchRow

       # CurrentTypeMatchRow will be used when user removes current row by "Delete row"button
       if { $oldrow != $row } {
	      set old $CurrentTypeMatchRow
		  dis_gdListViewChangeCheckStatus TypeMatchList $old 0 0
	      set CurrentTypeMatchRow $row
		  dis_gdListViewChangeCheckStatus TypeMatchList $row 0 1
	   }
	
    }
	#------------------------------------------------------------------------------------------------------

	#-----------------------------------------------------------------------------------------------------
	# CALLBACK will set current row in the TextMatch spreadsheet and modify the state of the 0 column to
	# show arrow - selected row.
	#
	# GLOBALS:
	#
	#   CurrentTextMatchRow      -  selected row in the TextMatch spreadsheet
	#
	# USES:
	#
	#-----------------------------------------------------------------------------------------------------
    proc TextMatchRowChanged { row col oldrow oldcol } {
	global CurrentTextMatchRow

       # CurrentTextMatchRow will be used when user removes current row by "Delete row"button
       if { $oldrow != $row } {
	      set old $CurrentTextMatchRow
		  dis_gdListViewChangeCheckStatus TextMatchList $old 0 0
	      set CurrentTextMatchRow $row
		  dis_gdListViewChangeCheckStatus TextMatchList $row 0 1
	   }

	
    }
	#------------------------------------------------------------------------------------------------------

	#-----------------------------------------------------------------------------------------------------
	# CALLBACK will remove current row from the TypeMatch spreadsheet.
	#
	# GLOBALS:
	#     CurrentTypeMatchRow - selected row in the TypeMatch spreadsheet
	#
	# USES:
	#
	#     TypeMatchDeleteRow - removes row passed as a parameter
	#-----------------------------------------------------------------------------------------------------
    proc TypeMatchDeleteCurrentRow {} {
	global CurrentTypeMatchRow
	global TypeMatchRows

	   if { $TypeMatchRows == 0 } { return }
	   dis_gdListViewFlush TypeMatchList
	   dis_gdListViewChangeCheckStatus TypeMatchList $CurrentTypeMatchRow 0 0
	   TypeMatchDeleteRow $CurrentTypeMatchRow 0
	   if { $CurrentTypeMatchRow == $TypeMatchRows } { 
	     if { $CurrentTypeMatchRow != 0 } {
	        incr CurrentTypeMatchRow -1 
		 }
	   }
	   if { $TypeMatchRows != 0 } {
          dis_gdListViewChangeCheckStatus TypeMatchList $CurrentTypeMatchRow 0 1
	   }
	
    }
	#------------------------------------------------------------------------------------------------------

	#-----------------------------------------------------------------------------------------------------
	# CALLBACK will remove current row from the TextMatch spreadsheet.
	#
	# GLOBALS:
	#
	#   CurrentTextMatchRow      -  selected row in the TextMatch spreadsheet
	#
	# USES:
	#
	#   TextMatchDeleteRow - removes row passed as a parameter
	#-----------------------------------------------------------------------------------------------------
    proc TextMatchDeleteCurrentRow {} {
	global CurrentTextMatchRow
	global TextMatchRows

	   if { $TextMatchRows == 0 } { return }
	   dis_gdListViewFlush TextMatchList
	   dis_gdListViewChangeCheckStatus TextMatchList $CurrentTextMatchRow 0 0
       TextMatchDeleteRow $CurrentTextMatchRow 0
	   if { $CurrentTextMatchRow == $TextMatchRows } { 
	      if { $CurrentTextMatchRow != 0 } {
	         incr CurrentTextMatchRow -1 
		  }
	   }
	   if { $TextMatchRows != 0 } {
          dis_gdListViewChangeCheckStatus TextMatchList $CurrentTextMatchRow 0 1
	   }
	}
	#------------------------------------------------------------------------------------------------------


    #-----------------------------------------------------------------------------------------------------
    # CALLBACK will copy currently selected row from the TypeMatch spreadsheet to clipboard
    #
    # GLOBALS:
    #
    #   TypeMatchArray - TCL representation of the spreadsheet data
    #   TypeMatchRows  - amount of rows in the TypeMatch spreadsheet
    #   TypeMatchList  - TCL representation of the TypeMatch C++ spreadsheet 
    #   Clipboard      - system clipboard
    #-----------------------------------------------------------------------------------------------------
    proc TypeMatchCopyRow { row col } {
	global TypeMatchArray
	global TypeMatchRows
	global Clipboard
	
	set Clipboard "@"
	lappend Clipboard $TypeMatchArray($row,1)
	lappend Clipboard $TypeMatchArray($row,2)
	lappend Clipboard $TypeMatchArray($row,3)
    }
    #-----------------------------------------------------------------------------------------------------

    #-----------------------------------------------------------------------------------------------------
    # CALLBACK will copy currently selected row from the TextMatch spreadsheet to clipboard
    #
    # GLOBALS:
    #
    #   TextMatchArray - TCL representation of the spreadsheet data
    #   TextMatchRows  - amount of rows in the TypeMatch spreadsheet
    #   TextMatchList  - TCL representation of the TypeMatch C++ spreadsheet 
    #   Clipboard      - system clipboard
    #-----------------------------------------------------------------------------------------------------
    proc TextMatchCopyRow { row col } {
	global TextMatchArray
	global TextMatchRows
	global Clipboard
	
	set Clipboard "$"
	lappend Clipboard $TextMatchArray($row,1)
	lappend Clipboard $TextMatchArray($row,2)
	lappend Clipboard $TextMatchArray($row,3)
	lappend Clipboard $TextMatchArray($row,4)
	lappend Clipboard $TextMatchArray($row,5)
    }
    #-----------------------------------------------------------------------------------------------------

    #-----------------------------------------------------------------------------------------------------
    # CALLBACK will paste the data from the clipboard into current row of the TypeMatch spreadsheet
    #
    # GLOBALS:
    #
    #   TypeMatchArray - TCL representation of the spreadsheet data
    #   TypeMatchRows  - amount of rows in the TypeMatch spreadsheet
    #   TypeMatchList  - TCL representation of the TypeMatch C++ spreadsheet 
    #   Clipboard      - system clipboard
    #
    # USES:
    #
    #   
    #------------------------------------------------------------------------------------------------------
    proc TypeMatchPasteRow { row col } {
	global TypeMatchArray
	global TypeMatchRows
	global Clipboard
	
	if { [info exists Clipboard] == 0 } { return }
	if { [lindex $Clipboard 0] != "@" } { return }
	set copy_from $TypeMatchRows
	set copy_to $TypeMatchRows
	incr copy_from -1
	lappend Clipboard $TypeMatchArray($row,1)
	lappend Clipboard $TypeMatchArray($row,2)
	lappend Clipboard $TypeMatchArray($row,3)
	while { $copy_from >= $row } {
	    set TypeMatchArray($copy_to,1) $TypeMatchArray($copy_from,1)
	    set TypeMatchArray($copy_to,2) $TypeMatchArray($copy_from,2)
	    set TypeMatchArray($copy_to,3) $TypeMatchArray($copy_from,3)

	    set typeRow $copy_to
	    lappend typeRow $TypeMatchArray($copy_to,1)
	    lappend typeRow $TypeMatchArray($copy_to,2)
	    lappend typeRow $TypeMatchArray($copy_to,3)
	    SetupTypeMatchRow $copy_to $typeRow

	    incr copy_from -1
	    incr copy_to -1
	}
	set TypeMatchArray($row,1) [lindex $Clipboard 1]
	set TypeMatchArray($row,2) [lindex $Clipboard 2]
	set TypeMatchArray($row,3) [lindex $Clipboard 3]

	set typeRow $row
	lappend typeRow $TypeMatchArray($row,1)
	lappend typeRow $TypeMatchArray($row,2)
	lappend typeRow $TypeMatchArray($row,3)
	SetupTypeMatchRow $row $typeRow
        
	incr TypeMatchRows
	dis_gdListViewRefresh TypeMatchList	
    }
    #------------------------------------------------------------------------------------------------------


    #-----------------------------------------------------------------------------------------------------
    # CALLBACK will paste the data from the clipboard into current row of the TextMatch spreadsheet
    #
    # GLOBALS:
    #
    #   TextMatchArray - TCL representation of the spreadsheet data
    #   TextMatchRows  - amount of rows in the TypeMatch spreadsheet
    #   TextMatchList  - TCL representation of the TypeMatch C++ spreadsheet 
    #   Clipboard      - system clipboard
    #
    # USES:
    #
    #   
    #------------------------------------------------------------------------------------------------------
    proc TextMatchPasteRow { row col } {
	global TextMatchArray
	global TextMatchRows
	global FilterSetModified
	global Clipboard
	
	if { [info exists Clipboard] == 0 } { return }
	if { [lindex $Clipboard 0] != "$" } { return }
	set copy_from $TextMatchRows
	set copy_to $TextMatchRows
	incr copy_from -1
	lappend Clipboard $TextMatchArray($row,1)
	lappend Clipboard $TextMatchArray($row,2)
	lappend Clipboard $TextMatchArray($row,3)
	lappend Clipboard $TextMatchArray($row,4)
	lappend Clipboard $TextMatchArray($row,5)
	while { $copy_from >= $row } {
	    set TextMatchArray($copy_to,1) $TextMatchArray($copy_from,1)
	    set TextMatchArray($copy_to,2) $TextMatchArray($copy_from,2)
	    set TextMatchArray($copy_to,3) $TextMatchArray($copy_from,3)
	    set TextMatchArray($copy_to,4) $TextMatchArray($copy_from,4)
	    set TextMatchArray($copy_to,5) $TextMatchArray($copy_from,5)
	    set textRow $copy_to
	    lappend textRow $TextMatchArray($copy_to,1)
	    lappend textRow $TextMatchArray($copy_to,2)
	    lappend textRow $TextMatchArray($copy_to,3)
	    lappend textRow $TextMatchArray($copy_to,4)
	    lappend textRow $TextMatchArray($copy_to,5)
	    SetupTextMatchRow $copy_to $textRow
	    incr copy_from -1
	    incr copy_to -1
	}
	set TextMatchArray($row,1) [lindex $Clipboard 1]
	set TextMatchArray($row,2) [lindex $Clipboard 2]
	set TextMatchArray($row,3) [lindex $Clipboard 3]
	set TextMatchArray($row,4) [lindex $Clipboard 4]
	set TextMatchArray($row,5) [lindex $Clipboard 5]
	set textRow $row
	lappend textRow $TextMatchArray($row,1)
	lappend textRow $TextMatchArray($row,2)
	lappend textRow $TextMatchArray($row,3)
	lappend textRow $TextMatchArray($row,4)
	lappend textRow $TextMatchArray($row,5)
	SetupTextMatchRow $row $textRow
	incr TextMatchRows
	dis_gdListViewRefresh TextMatchList	
    }
    #------------------------------------------------------------------------------------------------------

    #------------------------------------------------------------------------------------------------------
    # This function will initialize internal TypeMatch spreadsheet row.
    # You need to set row to use and array with 4 elements to represent cell values in the following form:
    # <row_name> <criteria_name> <type_value> <size_value>
    #------------------------------------------------------------------------------------------------------
    proc SetupTypeMatchRow {row values} {
	global CurrentTypeMatchRow

    # column 0 - row number, read-only
    set val [lindex $values 0]
    dis_gdListViewSetData TypeMatchList $row 0 $val R
    if { "$row" == "$CurrentTypeMatchRow" } { 
       set checkval 1 
    } else { 
       set checkval 0 
    }
    set num $row
    incr num 1
    dis_gdListViewAddCheckCell TypeMatchList $row 0 "" $checkval $num $num
    dis_gdListViewChangeCheckIcons TypeMatchList $row 0 "SpreadSheetImages:SelectImage" "SpreadSheetImages:UnelectImage"


	set enum_values ""
	lappend enum_values "New"
	lappend enum_values "Cut"
	lappend enum_values "Copy"
	lappend enum_values "Paste"
	dis_gdListViewSetCellMenu TypeMatchList $row 0 callbackTypeMenu $enum_values

	# column 1 - criteria. 2-state bool, can be include or exclude
	set val [lindex $values 1]
	dis_gdListViewSetData TypeMatchList $row 1 $val R 
	dis_gdListViewAddCheckCell TypeMatchList $row 1 callbackTypeCheckbox $val Include Exclude
	dis_gdListViewChangeCheckIcons TypeMatchList $row 1 "SpreadSheetImages:CheckInImage" "SpreadSheetImages:CheckOutImage"

	# column 2 - C or C++ type. This field  editable
	set val [lindex $values 2]
	dis_gdListViewSetData TypeMatchList $row 2 $val W

	# column 3 - Array size. This field can be editable
	set val [lindex $values 3]
	dis_gdListViewSetData TypeMatchList $row 3 $val W
    }
    #------------------------------------------------------------------------------------------------------


    #------------------------------------------------------------------------------------------------------
    # CALLBACK. This procedure will react on the TypeList popup-menu selection
    #------------------------------------------------------------------------------------------------------
    proc callbackTypeMenu { title row col } {
	global FilterSetModified
	global CurrentTypeMatchRow
	global TypeMatchRows

    dis_gdListViewFlush TypeMatchList
	if  {$CurrentTypeMatchRow != -1 } {
       dis_gdListViewChangeCheckStatus TypeMatchList $CurrentTypeMatchRow 0 0
	}
	if { $title == "New"   } { 
	    TypeMatchCreateRow
	    set FilterSetModified 1
	}
	if { $title == "Cut"   } { 
	    TypeMatchDeleteRow $row $col
	    set FilterSetModified
	}
	if { $title == "Copy"  } { 
	    TypeMatchCopyRow $row $col
	}
	if { $title == "Paste" } { 
	    TypeMatchPasteRow $row $col
	    set FilterSetModified 1
	}
	if {$TypeMatchRows != 0 } {
       if { $CurrentTypeMatchRow == $TypeMatchRows } { incr CurrentTypeMatchRow -1 }
	   dis_gdListViewChangeCheckStatus TypeMatchList $CurrentTypeMatchRow 0 1
	}
    }
    #------------------------------------------------------------------------------------------------------

    #------------------------------------------------------------------------------------------------------
    # CALLBACK. This procedure will react every time TypeMatch spreadsheet editor closed.
    # It updates TCL array to store new text in it
    #------------------------------------------------------------------------------------------------------
    proc callbackTypeEditor { title row col } {
	global TypeMatchArray
	global FilterSetModified

	set TypeMatchArray($row,$col) $title
	set FilterSetModified 1
    }
    #------------------------------------------------------------------------------------------------------


    #------------------------------------------------------------------------------------------------------
    # CALLBACK. This procedure will react every time TypeMatch spreadsheet chekbox changed state.
    # It updates TCL array to store new text in it
    #------------------------------------------------------------------------------------------------------
    proc callbackTypeCheckbox { value row col } {
	global TypeMatchArray
	global FilterSetModified

	set TypeMatchArray($row,$col) $value
	set FilterSetModified 1
    }
    #------------------------------------------------------------------------------------------------------


    #------------------------------------------------------------------------------------------------------
    # This function will initialize internal TextMatch spreadsheet row.
    # You need to set row to use and array with 6 elements to represent row values in the following form:
    # <row_name> <criteria_name> <base_string_value> <mixed_case> <entiti names> <strings>
    #------------------------------------------------------------------------------------------------------
    proc SetupTextMatchRow {row values} {

	global CurrentTextMatchRow

	# column 0 - row number, read-only
	set val [lindex $values 0]
	dis_gdListViewSetData TextMatchList $row 0 $val R
	if { "$row" == "$CurrentTextMatchRow" } { 
	   set checkval 1 
	} else { 
	   set checkval 0 
	}
	set num $row
	incr num 1

  	dis_gdListViewAddCheckCell TextMatchList $row 0 "" $checkval $num $num
    dis_gdListViewChangeCheckIcons TextMatchList $row 0 "SpreadSheetImages:SelectImage" "SpreadSheetImages:UnelectImage"
	set enum_values ""
	lappend enum_values "New"
	lappend enum_values "Cut"
	lappend enum_values "Copy"
	lappend enum_values "Paste"
	dis_gdListViewSetCellMenu TextMatchList $row 0 callbackTextMenu $enum_values

	# column 1 - criteria. 2-state bool, can be include or exclude
	set val [lindex $values 1]
	dis_gdListViewSetData TextMatchList $row 1 $val R 
	dis_gdListViewAddCheckCell TextMatchList $row 1 callbackTextCheckbox $val Include Exclude
	dis_gdListViewChangeCheckIcons TextMatchList $row 1 "SpreadSheetImages:CheckInImage" "SpreadSheetImages:CheckOutImage"

	# column 2 - Base string. This field  editable
	set val [lindex $values 2]
	dis_gdListViewSetData TextMatchList $row 2 $val W

	# column 3 - Mixed Cases. 2-state bool, can be On or Off
	set val [lindex $values 3]
	dis_gdListViewSetData TextMatchList $row 3 $val R 
	dis_gdListViewAddCheckCell TextMatchList $row 3 callbackTextCheckbox $val On Off


	# column 4 - Entity name. 2-state bool, can be On or Off
	set val [lindex $values 4]
	dis_gdListViewSetData TextMatchList $row 4 $val R 
	dis_gdListViewAddCheckCell TextMatchList $row 4 callbackTextCheckbox $val On Off

	# column 5 - Strings. 2-state bool, can be On or Off
	set val [lindex $values 5]
	dis_gdListViewSetData TextMatchList $row 5 $val R 
	dis_gdListViewAddCheckCell TextMatchList $row 5 callbackTextCheckbox $val On Off
    }
    #------------------------------------------------------------------------------------------------------

    #------------------------------------------------------------------------------------------------------
    # CALLBACK. This procedure will react on the TypeList popup-menu selection
    #------------------------------------------------------------------------------------------------------
    proc callbackTextMenu { title row col } {
	global FilterSetModified
	global CurrentTextMatchRow
	global TextMatchRows

	dis_gdListViewFlush TextMatchList
	if  {$CurrentTextMatchRow != -1 } {
	    dis_gdListViewChangeCheckStatus TextMatchList $CurrentTextMatchRow 0 0
	}
	if { $title == "New"   } { 
	    TextMatchCreateRow
	    set FilterSetModified 1
	}
	if { $title == "Cut"   } { 
	    TextMatchDeleteRow $row $col
	    set FilterSetModified 1
	}
	if { $title == "Copy"  } { 
	    TextMatchCopyRow $row $col
	}
	if { $title == "Paste" } { 
	    TextMatchPasteRow $row $col
	    set FilterSetModified 1
	}
    if { $TextMatchRows != 0 } {
        if { $CurrentTextMatchRow == $TextMatchRows } { incr CurrentTextMatchRow -1 }
	    dis_gdListViewChangeCheckStatus  TextMatchList $CurrentTextMatchRow 0 1
	}
    }
    #------------------------------------------------------------------------------------------------------

    #------------------------------------------------------------------------------------------------------
    # CALLBACK. This procedure will react every time TextMatch spreadsheet editor closed.
    # It updates TCL array to store new text in it
    #------------------------------------------------------------------------------------------------------
    proc callbackTextEditor { title row col } {
	global TextMatchArray
	global FilterSetModified

	set TextMatchArray($row,$col) $title
	set FilterSetModified 1
    }
    #------------------------------------------------------------------------------------------------------


    #------------------------------------------------------------------------------------------------------
    # CALLBACK. This procedure will react every time TextMatch spreadsheet chekbox changed state.
    # It updates TCL array to store new text in it
    #------------------------------------------------------------------------------------------------------
    proc callbackTextCheckbox { value row col } {
	global TextMatchArray
	global FilterSetModified

	set TextMatchArray($row,$col) $value
	set FilterSetModified 1
    }
    #------------------------------------------------------------------------------------------------------


    proc TaskEditorASETNotify { notification } {
	global CurrentRunningNode
	global SelectedNode
	global FlowTask
	global TaskSuspendFlag
	
	set n_type [lindex $notification 0]
	if { $n_type == "StepEndNotify" } {
	    set error_msg [lindex $notification 1]
	    if { $error_msg != {} } {
		dis_confirm1 {Execution Error} OK $error_msg
		return
	    } 
	    if { $SelectedNode != -1 } {
		dis_gdDrawingSelectItem TaskDraw $SelectedNode 0
	    }
	    #erase run pointer
	    dis_gdDrawingSelectItem TaskDraw $CurrentRunningNode -2
	    incr CurrentRunningNode
	    if { $CurrentRunningNode == [llength $FlowTask] } {
		set CurrentRunningNode -1
		dis_gdListViewUseInternalData PropertiesList
	    } else {
		set SelectedNode $CurrentRunningNode
		dis_gdDrawingSelectItem TaskDraw $SelectedNode 1
		dis_gdDrawingSelectItem TaskDraw $CurrentRunningNode 2
		DisplayProperties $SelectedNode
	    }
	}
	if { $n_type == "RunMode" } {
	    set error_msg [lindex $notification 1]
	    if { $error_msg != {} } {
		dis_confirm1 {Execution Error} OK $error_msg
		return
	    } 
	    if { $SelectedNode != -1 } {
		dis_gdDrawingSelectItem TaskDraw $SelectedNode 0
		dis_gdListViewUseInternalData PropertiesList
	    }
	    #erase run pointer
	    dis_gdDrawingSelectItem TaskDraw $CurrentRunningNode -2
	    incr CurrentRunningNode
	    if { $CurrentRunningNode == [llength $FlowTask] } {
		set CurrentRunningNode -1
		dis_Message "Done."
	    } else {
		set SelectedNode $CurrentRunningNode
		dis_gdDrawingSelectItem TaskDraw $SelectedNode 1
		dis_gdDrawingSelectItem TaskDraw $CurrentRunningNode 2
		DisplayProperties $SelectedNode
		if { $TaskSuspendFlag == 0 } {
		    TaskDoRun
		} else {
		    set TaskSuspendFlag 2
		    dis_Message "Suspended. Press \"Run\" to resume."
		}
	    }
	}
    }

    proc TaskDoStep {} {
	global CurrentRunningNode
	global SelectedNode
	global CurrentTask
	global TaskSuspendFlag

	TaskSaveComponents
	set runnable [dis_aset_eval_cmd "wftask validate $CurrentTask"]
	if { "$runnable" == "0" } {
	    dis_confirm1 "Flow" "OK" "Current task could not be executed. Check order of nodes." 
	    return
	}

	set TaskSuspendFlag 0
	if { $SelectedNode != -1 } {
	    dis_gdDrawingSelectItem TaskDraw $SelectedNode 0
	}
	if { $CurrentRunningNode == -1 } {
	    set CurrentRunningNode 0
	}
	set SelectedNode $CurrentRunningNode
	dis_gdDrawingSelectItem TaskDraw $SelectedNode 1
	dis_gdDrawingSelectItem TaskDraw $CurrentRunningNode 2
	DisplayProperties $SelectedNode
	set cmd "catch { wftask run "
	append cmd $CurrentTask
	append cmd " -from "
	append cmd $CurrentRunningNode
	append cmd " -n 1 }; set result \[wftask error_status $CurrentTask\]; "
	append cmd "send_to_client \"StepEndNotify {\$result}\""
	dis_aset_eval_cmd_async $cmd
    }

    proc TaskDoRun {} {
	global CurrentRunningNode
	global SelectedNode
	global CurrentTask
	global TaskSuspendFlag

	TaskSaveComponents
	set runnable [dis_aset_eval_cmd "wftask validate $CurrentTask"]
	if { "$runnable" == "0" } {
	    dis_confirm1 "Flow" "OK" "Current task could not be executed. Check order of nodes." 
	    return
	}

	set TaskSuspendFlag 0
	if { $SelectedNode != -1 } {
	    dis_gdDrawingSelectItem TaskDraw $SelectedNode 0
	}
	if { $CurrentRunningNode == -1 } {
	    set CurrentRunningNode 0
	}
	dis_Message "Running..."
	set SelectedNode $CurrentRunningNode
	dis_gdDrawingSelectItem TaskDraw $SelectedNode 1
	dis_gdDrawingSelectItem TaskDraw $CurrentRunningNode 2
	DisplayProperties $SelectedNode
	set cmd "catch { wftask run "
	append cmd $CurrentTask
	append cmd " -from "
	append cmd $CurrentRunningNode
	append cmd " -n 1 }; set result \[wftask error_status $CurrentTask\]; "
	append cmd "send_to_client \"RunMode {\$result}\""
	dis_aset_eval_cmd_async $cmd
    }

    proc TaskDoSuspend {} {
	global TaskSuspendFlag

	if { $TaskSuspendFlag == 0 } {
	    set TaskSuspendFlag 1
	    dis_Message "Suspending..."
	}
    }

    proc TaskDoContinueFrom {} {
	global CurrentRunningNode
	global FlowTask
	global SelectedNode
	global TaskSuspendFlag

	if { $SelectedNode == -1 } {
	    return 
	}
	if { $TaskSuspendFlag == 0 } {
	    set TaskSuspendFlag 1
	}
	if { $CurrentRunningNode != -1 } {
	    dis_gdDrawingSelectItem TaskDraw $CurrentRunningNode -2
	}
	set CurrentRunningNode $SelectedNode
	dis_gdDrawingSelectItem TaskDraw $CurrentRunningNode 2
    }

    #this is dormant procedure for futher use (maybe)
    proc TaskDoRestart {} {
	global CurrentRunningNode
	global FlowTask
	global SelectedNode
	global TaskSuspendFlag

	if { $TaskSuspendFlag == 0 } {
	    set TaskSuspendFlag 1
	}
	set CurrentRunningNode -1
	if { $SelectedNode != -1 } {
	    dis_gdDrawingSelectItem TaskDraw $SelectedNode 0
	}
	if { [llength $FlowTask] != 0 } {
	    set SelectedNode 0
	    dis_gdDrawingSelectItem TaskDraw $SelectedNode 1
	}	
    }

    proc TaskSaveComponents {} {
	global PatternSetModified
	global FilterName 
	global CurrentPattern

	if { $PatternSetModified == 1 } {
	    if { $CurrentPattern != "default" && $CurrentPattern != "Default" } {
		SaveCurrentPattern $CurrentPattern
	    }
	}        
	if { $FilterName == "Default" || "$FilterName" == "" || $FilterName == "New" } {
	    return
	}
	SaveCurrentFilter $FilterName
    }

    proc InitTaskEditor {} {
	global AvailableNodes
	global FlowTask
	global CurrentTask
	global TaskModified
	global NodeNames
	global SelectedNode
	global CurrentRunningNode
	global TaskSuspendFlag

	set gd -1
	set AvailableNodes { CollectImage FilterImage PatternImage InstancesImage AssignmentsImage ReportImage QueryImage }
	set NodeNames      { Collect      Filter      Pattern      Instances      Assignments      Report      Query }

	set all_nodes ""
	if { [dis_aset_eval_cmd "license wa feature"] } {
	    set all_nodes [dis_aset_eval_cmd "wfnode info"]
	}
	set FlowTask       {}
	set CurrentTask    ""
	set TaskModified   0
	set SelectedNode   -1
	set CurrentRunningNode -1
	set TaskSuspendFlag 0

	dis_register_aset_callback TaskEditorASETNotify

	dis_gdInitDrawing $gd TaskDraw TaskDrawClick TaskHandleMove
	dis_gdInitDrawing $gd AvailableItems AvailableItemsClick
	dis_gdDrawingSetDragProhibited AvailableItems 1
	dis_gdDrawingSetSingleSelection AvailableItems 1
	dis_gdDrawingSetSingleSelection TaskDraw 1
	dis_gdDrawingSetGrid TaskDraw 55 45 H
	dis_gdDrawingSetGrid AvailableItems 50 45 H
	dis_gdDrawingSetGridDimensions AvailableItems [llength $all_nodes] 1
	set pos 0
	foreach item $AvailableNodes {
	    dis_gdDrawingAddItem AvailableItems $item
	    dis_gdDrawingSetIconTip AvailableItems $pos [lindex $NodeNames $pos]
	    incr pos
	}
	# draw user defined nodes
	foreach item $all_nodes { 
	    if { [lsearch $NodeNames $item] == -1 } {
		set pos [dis_gdDrawingAddItem AvailableItems UserImage]
		lappend NodeNames $item
		lappend AvailableNodes UserImage
		dis_gdDrawingSetIconTitle AvailableItems $pos $item
	    }
	}

	global TasksNameList
	if { [dis_aset_eval_cmd "license wa feature"] } {
	    set TasksNameList [dis_aset_eval_cmd "wftask info"]
	} else {
	    set TasksNameList ""
	}
	dis_gdInitPopDownComboBox $gd TaskName { ChangeTask }
	dis_gdComboBoxSetMatch TaskName 1

	global TaskPropertyRuler
	dis_gdInitListView $gd PropertiesList 2 {}
	dis_gdInitRuler $gd TaskPropertyRuler PropertiesList {}

	dis_gdInitButton $gd NewTask CreateNewTask
	dis_gdInitButton $gd TaskSave DoTaskSave

	dis_gdInitButton $gd TaskStep TaskDoStep
	dis_gdInitButton $gd TaskSuspend TaskDoSuspend
	#dis_gdInitButton $gd TaskRestart TaskDoRestart
	dis_gdInitButton $gd TaskContinueFrom TaskDoContinueFrom
	dis_gdInitButton $gd TaskRun TaskDoRun

	dis_gdInitButton $gd TaskRemoveNode TaskDoRemoveNode
	
	dis_gdClearCombo $gd TaskName
	if { "$TasksNameList" != "" } {
	    set TasksNameList [lsort -ascii $TasksNameList]
	    dis_gdAddListToCombo $gd TaskName $TasksNameList
	}
    }

    proc TaskDoRemoveNode {} {
	global CurrentRunningNode
	global CurrentTask
	global SelectedNode
	global FlowTask

	if { $SelectedNode != -1 } {
	    dis_aset_eval_cmd "wftask delnode $CurrentTask $SelectedNode"
	    dis_gdListViewUseInternalData PropertiesList
	    dis_gdDrawingRemoveItem TaskDraw $SelectedNode
	    set FlowTask [lreplace $FlowTask $SelectedNode $SelectedNode]
	    set SelectedNode -1
	    set CurrentRunningNode -1
	}
    }

    proc ChangeTask {} {
	global TaskName
	global CurrentTask
	global TaskModified

	if { $CurrentTask == $TaskName } { return }
	if { $TaskModified != 0 } {
	    set is_ok [dis_confirm2 "" "Save" "Discard" "Current task has been modified. Do you want to save your changes?"]
	    if { $is_ok == "LeftButton" } { SaveCurrentTask "" }
	}
	set CurrentTask $TaskName
	LoadTask $CurrentTask
    }

    proc DoTaskSave {} {
	global CurrentTask
	
	if { "$CurrentTask" != "" } {
	    dis_aset_eval_cmd "wftask save $CurrentTask"
	}
    }

    proc CreateNewTask {} {
	global TaskName
	global TasksNameList
	global FlowTask
	global CurrentRunningNode
	global SelectedNode
	global gd

	set is_ok [dis_prompt GetStringDialog "New Task Flow" "Enter the name for new task"]
	if { $is_ok != "OK" } { return }
	set tmp_name [dis_getvar -global ConDValue]
	regsub -all {[^A-Za-z0-9]+} $tmp_name "_" name
	set ind [lsearch $TasksNameList $name]
	if { $ind > 0 } { return }
	dis_aset_eval_cmd "wftask new $name"
	set TasksNameList [concat $TasksNameList $name]
	set TasksNameList [lsort -ascii $TasksNameList]
	dis_gdClearCombo $gd TaskName
	dis_gdAddListToCombo $gd TaskName $TasksNameList
	set FlowTask {}
	set CurrentRunningNode -1
	set SelectedNode       -1
	set TaskName $name
    }

    proc LoadTask { name } {
	global NodeNames
	global AvailableNodes
	global SelectedNode
	global CurrentRunningNode
	global FlowTask

	dis_gdDrawingClear TaskDraw
	dis_gdListViewUseInternalData PropertiesList
	set FlowTask {}
	set SelectedNode -1
	set CurrentRunningNode -1
	set task_list [dis_aset_eval_cmd "wftask print $name"]
	foreach node $task_list {
	    set ind [lsearch $NodeNames $node]
	    set image [lindex $AvailableNodes $ind]
	    AddTaskToFlow $image $node
	}
	dis_aset_eval_cmd "wftask revive $name"

	dis_gdListViewSetChangeCallback PropertiesList SetValueCallback
	dis_gdListViewSetDblClickCallback PropertiesList CellDblClickCallback
    }

    proc CellDblClickCallback { row col } {
	global GlobalFileName
	
	set cell_type [dis_gdListViewGetCellData PropertiesList $row $col]
	set cell_value [dis_gdListViewGetCellValue PropertiesList $row 2]
	if { "$cell_type" == "filter" } {
	    set GlobalFileName $cell_value
	    dis_activate_notebook_page FilterPage
	} 
	if { "$cell_type" == "pattern" } {
	    set GlobalFileName $cell_value
	    dis_activate_notebook_page PatternsPage
	} 
	if { "$cell_type" == "entity" } {
	    set cell_data [dis_gdListViewGetCellData PropertiesList $row 2]
	    dis_aset_eval_cmd "set_result $cell_data"
	}
    }

    proc SetValueCallback { title row col } {
	global CurrentTask
	global SelectedNode

	set cell_type [dis_gdListViewGetCellData PropertiesList $row 1]
	set prop_name [dis_gdListViewGetCellData PropertiesList $row 0]
	dis_aset_eval_cmd "wftask setparam $CurrentTask -n $SelectedNode -p $prop_name -v {$title}"
	dis_gdListViewSetData PropertiesList $row 2 $title
	dis_gdListViewRefresh PropertiesList
    }

    proc ChangeBoolCallback { new_value row col } {
	global CurrentTask
	global SelectedNode

	set cell_type [dis_gdListViewGetCellData PropertiesList $row 1]
	set prop_name [dis_gdListViewGetCellData PropertiesList $row 0]
	dis_aset_eval_cmd "wftask setparam $CurrentTask -n $SelectedNode -p $prop_name -v {$new_value}"
    }

    proc TaskHandleMove { from_row from_col from_cell to_row to_col to_cell } {
	global FlowTask
	global SelectedNode
	global CurrentTask

	
	dis_gdDrawingSelectItem TaskDraw $SelectedNode 0
	set SelectedNode -1
	dis_gdListViewUseInternalData PropertiesList
	dis_aset_eval_cmd "wftask move $CurrentTask $from_cell $to_cell"
	return 1
    }

    proc SetupValueCell { row property } {
	set full_type [lindex $property 3]
	set type [lindex $full_type 0]
	set val [lindex $property 5]
	if { $type == "enum" } {
	    set enum_values [lindex $full_type 1]
	    dis_gdListViewSetData PropertiesList $row 2 $val
	    dis_gdListViewSetCellMenu PropertiesList $row 2 SetValueCallback $enum_values
	} else {
	    if { $full_type == "set" } {
		regsub _DI_ $val "Set # " val_str
		dis_gdListViewSetData PropertiesList $row 2 $val_str Readonly $val
		set browsers_list {"Browser" "Mini Browser"}
		set custom_list [dis_aset_eval_cmd "get_custom_set_browsers"]
		if { "$custom_list" != "" }  {
		    lappend browsers_list $custom_list
		}
		dis_gdListViewSetCellMenu PropertiesList $row 2 BrowseSetCallback $browsers_list
	    } else {
		if { $full_type == "bool" } {
		    dis_gdListViewSetData PropertiesList $row 2 " " Readonly
		    dis_gdListViewAddCheckCell PropertiesList $row 2 ChangeBoolCallback $val
		} else {
		    dis_gdListViewSetData PropertiesList $row 2 $val
		}
	    }
	}
    }

    proc BrowseSetCallback { title row col } {
	set cell_data [dis_gdListViewGetCellData PropertiesList $row 2]
	if { $title == "Browser" } {
	    dis_aset_eval_cmd "set_result $cell_data"
	    return
	}
	if { $title == "Mini Browser" } {
	    dis_aset_eval_cmd "set_mini $cell_data"
	    return
	}
	dis_aset_eval_cmd "execute_custom_browser $cell_data \"$title\""
    }

    proc DisplayProperties { node } {
	global CurrentTask

	dis_gdListViewUseInternalData PropertiesList
	set task_list [dis_aset_eval_cmd "wftask dump $CurrentTask $node"]
	set task_list [lindex $task_list 0]
	# cut first element from list
	set task_list [lrange $task_list 1 999]
	set row 0
	foreach property $task_list {
	    #get property type
	    set ptype [lindex $property 2]
	    set property_name [lindex $property 0]
	    set property_desc [dis_aset_eval_cmd "wfparam describe $property_name"]
	    #display property name
	    dis_gdListViewSetData PropertiesList $row 0 $property_desc Readonly $property_name
	    #display property type
	    dis_gdListViewSetData PropertiesList $row 1 [lindex $property 1] Readonly $ptype
	    SetupValueCell $row $property
	    incr row
	}
    }

    proc TaskDrawClick {row col cell type} {
	global FlowTask
	global SelectedNode

	if { $type == "click" } {
	    if { $SelectedNode >= 0 } {
		dis_gdDrawingSelectItem TaskDraw $SelectedNode -1
	    }
	    if { [llength $FlowTask] > $cell } {
		dis_gdDrawingSelectItem TaskDraw $cell 1
		set SelectedNode $cell
		DisplayProperties $cell
	    } else {
		set SelectedNode -1
	    }
	}
	if { $type == "dblclick" } {
	    set node [lindex $FlowTask $cell]
	}
    }

    proc AvailableItemsClick {row col cell type} {
	global AvailableNodes
	global NodeNames
	global TaskName
	global FlowTask
	
	if { $type == "click" } {
	    set image [lindex $AvailableNodes $cell]
	    set node_name [lindex $NodeNames $cell]
	    set node_number [llength $FlowTask]
	    incr node_number -1
	    dis_aset_eval_cmd "wftask addnode $TaskName $node_number $node_name"
	    AddTaskToFlow $image $node_name
	}	
    }

    proc AddTaskToFlow { imagename node_name } {
	global FlowTask
	global TaskName
	global NodeNames

	lappend FlowTask $imagename
	set pos [dis_gdDrawingAddItem TaskDraw $imagename]
	if { $imagename == "UserImage" } {
	    dis_gdDrawingSetIconTitle TaskDraw $pos $node_name
	}
	dis_gdDrawingSetIconTip TaskDraw $pos $node_name
    }

    proc ListInitialize {} {
	global PatternsList
	global PatternRowSelected

	set PatternsList {
	    { global TPMRows; return $TPMRows } { getTPMInfo $Item } 
	    { global PatternRowSelected; global Selection; set PatternRowSelected $Selection;
	    dis_Message $TPMArray($Selection,3)
	}
	{ OpenTPMEditor $Item }}
	dis_gdListViewRefresh PatternsList	
    }

    proc ListPasteRow { row col } {
	global TPMArray
	global TPMRows
	global PatternSetModified
	global PatternsList
	global Clipboard
	
	if { [info exists Clipboard] == 0 } { return }
	if { [lindex $Clipboard 0] != "*" } { return }
	set copy_from $TPMRows
	set copy_to $TPMRows
	incr copy_from -1
	lappend Clipboard $TPMArray($row,1)
	lappend Clipboard $TPMArray($row,2)
	lappend Clipboard $TPMArray($row,3)
	while { $copy_from >= $row } {
	    set TPMArray($copy_to,1) $TPMArray($copy_from,1)
	    set TPMArray($copy_to,2) $TPMArray($copy_from,2)
	    set TPMArray($copy_to,3) $TPMArray($copy_from,3)
	    incr copy_from -1
	    incr copy_to -1
	}
	set TPMArray($row,1) [lindex $Clipboard 1]
	set TPMArray($row,2) [lindex $Clipboard 2]
	set TPMArray($row,3) [lindex $Clipboard 3]
	incr TPMRows
	ListInitialize
	set PatternSetModified 1
    }


    proc ListDeleteRow { row col } {
	global TPMArray
	global TPMRows
	global PatternSetModified
	global PatternsList
	global Clipboard
	
	set Clipboard "*"
	lappend Clipboard $TPMArray($row,1)
	lappend Clipboard $TPMArray($row,2)
	lappend Clipboard $TPMArray($row,3)
	set copy_row $row
	incr copy_row	
	if { $copy_row < $TPMRows } {
	    while { $copy_row < $TPMRows } {
		set TPMArray($row,1) $TPMArray($copy_row,1)
		set TPMArray($row,2) $TPMArray($copy_row,2)
		set TPMArray($row,3) $TPMArray($copy_row,3)
		incr row
		incr copy_row
	    }
	}
	incr TPMRows -1
	ListInitialize
	set PatternSetModified 1
    }

    proc DeletePattern {} {
	global TPMArray
	global TPMRows
	global PatternSetModified
	global PatternsList
	global Selection
	
	if { "$Selection" == "" } {
	    dis_confirm1 "Removing selected pattern" "OK" [concat Please,first select pattern to remove.]
	    return
	}
	set  row $Selection
	set  copy_row $Selection
	incr copy_row	
	if { $copy_row < $TPMRows } {
	    while { $copy_row < $TPMRows } {
		   set TPMArray($row,1) $TPMArray($copy_row,1)
		   set TPMArray($row,2) $TPMArray($copy_row,2)
		   set TPMArray($row,3) $TPMArray($copy_row,3)
		   incr row
		   incr copy_row
	    }
	}
	incr TPMRows -1
	ListInitialize
	set PatternSetModified 1
    }

    proc ListCopyRow { row col } {
	global TPMArray
	global TPMRows
	global PatternSetModified
	global PatternsList
	global Clipboard
	
	set Clipboard "*"
	lappend Clipboard $TPMArray($row,1)
	lappend Clipboard $TPMArray($row,2)
	lappend Clipboard $TPMArray($row,3)
    }

    proc GetPatternsList {} {
	global PatternsFiles
	global PatternsNameList

	set files [dis_aset_eval_cmd "apply fname \[extfile list pattern\]"]
	foreach f $files {
	    set d [file dirname $f]
	    regsub "$d." $f "" tmp
	    set e [file extension $tmp]
	    regsub $e $tmp "" pattern_name
	    set PatternsFiles($pattern_name) "$f"
	    lappend PatternsNameList $pattern_name
	}
    }

    proc ChangePattern {} {	
	global PatternName
	global CurrentPattern
	global PatternSetModified

	if { $CurrentPattern == $PatternName } { return }
	if { $PatternSetModified != 0 } {
	    set is_ok [dis_confirm2 "" "Save" "Discard" "Current pattern set has been modified. Do you want to save your changes?"]
	    if { $is_ok == "LeftButton" } { SaveCurrentPattern "" }
	}
	set CurrentPattern $PatternName
	LoadPatternSet $CurrentPattern
	dis_gdListViewRefresh PatternsList
    }

    proc getTPMInfo { item } {
	if { [expr $item % 4] == 0 } { return [expr $item / 4 + 1] }
	set row [expr $item / 4]
	set col [expr $item % 4]
	global TPMArray
	return $TPMArray($row,$col)
    }

    proc OpenTPMEditor { item } {
	global TPMArray
	global TPMEditRow
	set row [expr $item / 4]	
	lappend str $TPMArray($row,1)
	lappend str $TPMArray($row,2)
	lappend str $TPMArray($row,3)
	set TPMEditRow $row
	dis_launch_dialog PatternProperty -modal $str
    }

    proc TPMEditorDone {} {
	global TPMArray
	global TPMEditRow
	global TPMRows
	global PatternSetModified
	global PatternsList

	set result [dis_getvar -global TPMEditorResult]
	if { "$result" == "" } {
	    #set rows [expr $TPMRows - 1] #Check if we where adding new row and it's empty remove it
	    if { "$TPMArray($TPMEditRow,1)" == "" && "$TPMArray($TPMEditRow,2)" == "" && "$TPMArray($TPMEditRow,3)" == "" } {
		incr TPMRows -1
	    }
	    ListInitialize
	    return
	}
	set TPMArray($TPMEditRow,1) [lindex $result 0]
	set TPMArray($TPMEditRow,2) [lindex $result 1]
	set TPMArray($TPMEditRow,3) [lindex $result 2]
	dis_gdListViewRefresh PatternsList
	set PatternSetModified 1
    }

    proc CreateNewPatternSet {} {
	global PatternName
	global PatternsNameList
	global PatternsFiles
	global PatternSetModified
	global gd

	set is_ok [dis_prompt GetStringDialog "New Pattern Set" "Enter the name for new pattern set"]
	if { $is_ok != "OK" } { return }
	set tmp_name [dis_getvar -global ConDValue]
	regsub -all {[^A-Za-z0-9]+} $tmp_name "_" name
	set ind [lsearch $PatternsNameList $name]
	if { $ind > 0 } { return }
	set file_name [dis_aset_eval_cmd "fname \[extfile new pattern \"$name\"\]"]
	if { "$file_name" == "" } {
	    return
	}
	set PatternsNameList [concat $PatternsNameList $name]
	set PatternsFiles($name) "$file_name"
	SaveCurrentPattern $name
	set PatternsNameList [lsort -ascii $PatternsNameList]
	dis_aset_eval_cmd "add_new_pattern_to_menus $name"
	dis_gdClearCombo $gd PatternName
	dis_gdAddListToCombo $gd PatternName $PatternsNameList
	set PatternName $name
	set PatternSetModified 0
    }

    proc SaveCurrentPattern { name } {
	global TPMArray
	global CurrentPattern
	global PatternsNameList
	global PatternsFiles
	global PatternSetModified
	global TPMRows
	global PatternSetType

	if { "$name" == "" } { set name $CurrentPattern }
	if { "$name" == "default" || "$name" == "Default" } {
	    dis_confirm1 "Save" "OK" [concat Default pattern can not be changed!]
	    return
	}
	set fname $PatternsFiles($name)
	set rows $TPMRows
	set cur_row 0
	set f [open $fname "w"]
	puts $f $PatternSetType
	while { $cur_row < $rows } {
	    if { "$TPMArray($cur_row,3)" != "" } {
		set line ""
		lappend line $TPMArray($cur_row,1)
		lappend line $TPMArray($cur_row,2)
		set tpm_exp $TPMArray($cur_row,3)
		regsub -all \n $tpm_exp \01 tmp
		lappend line $tmp
		puts $f $line
	    }
	    incr cur_row
	}
	set PatternSetModified 0
	close $f
    }

    proc LoadPatternSet { name } {
	global TPMArray
	global TPMRows
	global PatternName
	global PatternsNameList
	global PatternsFiles
	global PatternsList
	global PatternSetModified
	global PatternSetType
	global PatternRowSelected
	global gd

	set fname $PatternsFiles($name)
	set result [catch {set f [open $fname "r"]}]
	if { $result == 1 } {
	    dis_Message "Unable to open pattern set file. Switching to default"
	    dis_gdClearCombo $gd PatternName
	    dis_gdAddListToCombo $gd PatternName $PatternsNameList
	    set PatternName "default"
	    return
	}
	catch { unset TPMArray }
	set row 0
	set num [gets $f set_type]
	if { $num > 0 } {
	    set PatternSetType $set_type
	}
	while { [eof $f] == 0 } {
	    set num [gets $f line]
	    if { $num > 0 } {
		set TPMArray($row,1) [lindex $line 0]
		set TPMArray($row,2) [lindex $line 1]
		set tmp [lindex $line 2]
		regsub -all \01 $tmp \n tpm_expr
		set TPMArray($row,3) $tpm_expr
		incr row
	    }
	}
	set TPMRows $row
	ListInitialize
	set PatternSetModified 0
	if { $TPMRows > 0 } {
	    set PatternRowSelected 0
	} else {
	    set PatternRowSelected -1	
	}
	close $f
    }

    proc AddNewPattern {} {
	global TPMArray
	global TPMRows
	global PatternsList

	set TPMArray($TPMRows,1) ""
	set TPMArray($TPMRows,2) ""
	set TPMArray($TPMRows,3) ""
	incr TPMRows
	ListInitialize
	OpenTPMEditor [expr ( $TPMRows - 1 ) * 4] 
    }

    ### Procedure to Sort a list and remove any non-unique entries.
    proc luniq {inList} {
	set outList [lsort $inList]
	set index 0
	set next 1
	while { $next < [llength $outList] } {
	    if { [lindex $outList $index] == [lindex $outList $next] } then {
		### Delete the next entry in the list.
		set outList [lreplace $outList $next $next]
	    } else {
		incr index
		incr next
	    }
	}
	return $outList
    }

    global FilterPage
    global PatternsPage
    set FilterPage ""
    set PatternsPage ""
    set gd -1
    dis_gdInitNotebookPage $gd FilterPage { FilterPageInitProc }
    dis_gdInitNotebookPage $gd PatternsPage { PatternsPageInitProc }
    dis_gdInitNotebookPage $gd Flow { FlowPageInitProc }

    OpenDialog

    # snap to appropriate page
    if { [dis_aset_eval_cmd "license w feature"] } {
	dis_activate_notebook_page FilterPage
    } elseif { [dis_aset_eval_cmd "license wb feature"] } {
	dis_activate_notebook_page PatternsPage
    } elseif { [dis_aset_eval_cmd "license wa feature"] } {
	dis_activate_notebook_page Flow
    } 
    
    return OK
}
