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

    proc OpenDialog {} {
        global gd
        global ShowMatching HideMatching CLIExpression
	global CategoryItems
	global FormatList FormatItemList SortList SortItemList
        global OK Cancel Help
        global CreateNewFilter REnameFilter SaveFilter DeleteFilter
	global FilterItems FormatItems SortItems
	global FormatSash SortSash
	global FormatSelection SortSelection
	global AddToFormat RemoveFromFormat AddToSort RemoveFromSort
	global FormatLength SortAscending
	global fList sList FormatPosition SortPosition
	global formatExpression CLIExpression
	global Selection

# Initialize the ListViews (Necessary to allocate the list associated with the list view)

	dis_gdInitListView $gd FormatList 2 {}
	dis_gdInitListView $gd SortList 2 {}

# Initialize the Rulers

	dis_gdInitRuler $gd FormatRuler FormatList {}
	dis_gdInitRuler $gd SortRuler SortList {}

# Get the current filter

	reval_async "DISrtlFilter"  [concat "dis_filterGet" $gd [dis_getvar -global FilterRTLid]]

# Initialize the Filter page

        	dis_gdInitTextItem $gd ShowMatching {}
       	dis_gdInitTextItem $gd HideMatching {}
	dis_gdInitTextItem $gd CLIExpression {}
	dis_gdInitTextItem $gd formatExpression {}

	set ShowMatching [reval "DISrtlFilter" [concat "dis_filterGetShow" $gd [dis_getvar -global FilterRTLid]]]
	set HideMatching  [reval "DISrtlFilter" [concat "dis_filterGetHide" $gd [dis_getvar -global FilterRTLid]]]
	set CLIExpression [reval "DISrtlFilter" [concat "dis_filterGetCLI" $gd [dis_getvar -global FilterRTLid]]]
	set formatExpression [reval "DISrtlFilter" [concat "dis_filterGetFormatSpec" $gd [dis_getvar -global FilterRTLid]]]

#  Set the categories that are shared between the Format Items list and the Sort Items list

	set CategoryItems [reval "DISrtlFilter" "dis_filterGetCategories"]

#  Initialize the sashes on the Sort and Format pages

#	dis_gdInitSash $gd FormatSash NULL  NULL  #FormatList  #FormatItemList
#	dis_gdInitSash $gd SortSash NULL  NULL  #SortList  #SortItemList

# Initialize the buttons to add and remove items from the Sort and Format lists

	dis_gdInitButton $gd AddToFormat {
		if {[info exists FormatSelection]} {
			set AddItem [lindex $CategoryItems $FormatSelection]
			set fList [linsert $fList $FormatPosition $AddItem $FormatLength]
			set saveFormatPosition $FormatPosition;	# Save the FormatPosition to use later (because it gets corrupted for some unknown reason
			SetFormatList
			set FormatPosition [expr $saveFormatPosition + 2]
			dis_gdListViewSelect FormatList [expr $FormatPosition / 2]
    		}
	}

	dis_gdInitButton $gd RemoveFromFormat {
		if {[expr $FormatPosition + 2] != [llength $fList]} {
			set fList [lreplace $fList $FormatPosition [expr $FormatPosition + 1]]
			set saveFormatPosition $FormatPosition;	# Save the FormatPosition to use later (because it gets corrupted for some unknown reason)
			SetFormatList
			if {$saveFormatPosition == 0} {set FormatPosition $saveFormatPosition} else {set FormatPosition [expr $saveFormatPosition - 2]}
			dis_gdListViewSelect FormatList [expr $FormatPosition / 2]
		}
	}

	dis_gdInitButton $gd AddToSort {
		if {[info exists SortSelection]} {
			if {$SortAscending == 1} {set direction Ascending} else {set direction Descending}
			set AddItem [lindex $CategoryItems $SortSelection]
			set sList [linsert $sList $SortPosition $AddItem $direction]
			set saveSortPosition $SortPosition;		# Save the SortPosition to use later (because it gets corrupted for some unknown reason
			SetSortList
			set SortPosition [expr $saveSortPosition + 2]
 			dis_gdListViewSelect SortList [expr $SortPosition / 2]
    		}
	}

	dis_gdInitButton $gd RemoveFromSort {
		if {[expr $SortPosition + 2] != [llength $sList]} {
			set sList [lreplace $sList $SortPosition [expr $SortPosition + 1]]
			set saveSortPosition $SortPosition;	# Save the FormatPosition to use later (because it gets corrupted for some unknown reason)
			SetSortList
			if {$saveSortPosition == 0} {set SortPosition $saveSortPosition} else {set SortPosition [expr $saveSortPosition - 2]}
			dis_gdListViewSelect SortList [expr $SortPosition / 2]
		}
	}

# Initialize the FormatList information

	set FormatLength 0
	set FormatPosition 0
	setFormatFilters [reval "DISrtlFilter" [concat "dis_filterGetFormat" $gd [dis_getvar -global FilterRTLid]]]
	SetFormatList
#dis_puts "selecting item [expr [llength $fList] / 2 -1]"
	dis_gdListViewSelect FormatList [expr [llength $fList] / 2 -1]

# Initialize the SortList information

	set SortAscending 1
	set SortPosition 0
	setSortFilters [reval "DISrtlFilter" [concat "dis_filterGetSort" $gd [dis_getvar -global FilterRTLid]]]
	SetSortList
	dis_gdListViewSelect SortList [expr [llength $sList] / 2 -1]

#Initialize the FormatItemList

	set FormatItemList {{return [llength $CategoryItems]}
				{lindex $CategoryItems $Item}
				{set FormatSelection $Selection}
				{}}

#Initialize the SortItemList

	set SortItemList {{return [llength $CategoryItems]}
				{lindex $CategoryItems $Item}
				{set SortSelection $Selection}
				{}}

#  Initialize the OK and Cancel buttons

        	dis_gdInitButton $gd OK {
		CreateFilterLists
		reval_async "DISrtlFilter" [concat "dis_filterApply" [dis_getvar -global FilterRTLid] $sList $fList $FilterItems]
		dis_close_dialog
	}

        	dis_gdInitButton $gd Cancel {dis_close_dialog}

        	dis_gdInitButton $gd tagFilterHelp {dis_help -topic 1500 "[dis_getenv PSETHOME]\\lib\\hyperhelp\\DISCOVER_HELP.hlp"}

        	return OK
    }

    proc SetFormatList {} {
	global FormatList

	set FormatList  { {global fList; return [expr [llength $fList] / 2]}
			     {global fList Item; lindex $fList $Item}
			     {global FormatPosition
				if {[info exists Selection]} {
			      		set FormatPosition [expr $Selection * 2]
				} else {
			      		set FormatPosition 0
				}
			     }
			     {}}
    }

    proc SetSortList {} {
	global SortList

	set SortList 	{ {global sList; return [expr [llength $sList] / 2]}
			  {global sList Item; lindex $sList $Item}
			  {global SortPosition
			   if {[info exists Selection]} {
				set SortPosition [expr $Selection * 2]
			   } else {
				set SortPosition 0
			  }}
			{}}
    }


    proc CreateFilterLists {} {
	global ShowMatching HideMatching CLIExpression formatExpression
	global FormatItems FilterItems SortItems

	if {[info exists formatExpression] == 0} {set FE ""} else {set FE $formatExpression}
	if {[info exists ShowMatching] == 0} {set SM ""} else {set SM $ShowMatching}
	if {[info exists HideMatching] == 0} {set HM ""} else {set HM $HideMatching}
	if {[info exists CLIExpression] == 0} {set CLI ""} else {set CLI $CLIExpression}

	set FilterItems [list $FE $SM $HM $CLI]
    }

    proc setSortFilters {sortFilters} {
	global sList
	set sList $sortFilters
    }

    proc setFormatFilters {formatFilters} {
	global fList
	set fList $formatFilters
    }

    OpenDialog
    return OK
}
