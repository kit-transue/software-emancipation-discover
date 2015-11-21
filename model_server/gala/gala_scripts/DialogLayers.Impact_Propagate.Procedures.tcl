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
eval  {

##### Impact/Propagate Procedures


	global OK
	set OK 0

	proc ReopenDialog {} {
		InitIASelectionPageProc
		InitIAReportPageProc
	}

	proc CloseDialog { } {
	}

	proc OpenDialog { } { 

		#####
		##### Global Initialization
		#####

		global gd
		set gd -1



		### List of Catagories of objects to do queries on.
		### Used by  Rename and Change Arguments Pages.

		### There are 11 categories that require sync. with the Server List
		### Since we don't use change propagation, not all of them are used.
		###----------------------------------------------------------------
		### 1.	{Objects Selected}
		### 2.	{Selections Unchanged}
		### 3.	{Objects Affected}
		### 4.	{Files Affected}
		### 5.	{Files Needing Update}
		### 6.	{Files Needing Checkout}
		### 7.	{Files Propagated}
		### 8.	{Files Partially Propagated}
		### 9.	{Files Not Yet Propagated}
		###10.	{Files Requiring Assistance}
		###11.	{Files Needing Recompilation}
		###----------------------------------------------------------------

		global Categories
		set Categories {
			{ {Objects Selected}		{0} }
			{ {Objects Affected}		{2} }
			{ {Files Affected}		{3} }
			{ {Files Needing Update}	{4} }
			{ {Files Needing Checkout}		{5} }
			{ {Files Needing Recompilation}	{10}}
		}



		### List of Queries that can be asked.
		### Used by  Rename and Change Arguments Pages.
		### There are 8 queries that require sync. with the Server List
		###----------------------------------------------------------------
		### 1.	{All Instances}
		### 2.	{Actual Instances}
		### 3.	{Linked/Assoc'd Instances}
		### 4.	{Progagated Instances}
		### 5.	{User-required Instances}
		### 6.	{Instances Affected}
		### 7.	{Changing to}
		### 8.	{Where Defined}
		###----------------------------------------------------------------

		global Ask
		set Ask {
			{ {All Instances}		{0} }
			{ {Actual Instances}		{1} }
			{ {Linked/Assoc'd Instances}	{2} }
			{ {Instances Affected}		{5} }
			{ {Where Defined}		{7} }
		}


		### Init the type of operation to neither Rename nor ChangeArgs.
		global IAOperation
		set IAOperation ""



		### Init the I/A Selection Page
		global IASelectionPageInit
		set IASelectionPageInit 0
		dis_gdInitNotebookPage $gd IASelectionPage InitIASelectionPageProc


		### Init the I/A Rename Page
		global IARenamePageInit
		set IARenamePageInit 0
		dis_gdInitNotebookPage $gd IARenamePage InitIARenamePageProc


		### Init the I/A Report Page
		global IAReportPageInit
		set IAReportPageInit 0
		dis_gdInitNotebookPage $gd IAReportPage InitIAReportPageProc

	}	; ### End of OpenDialog



##### 
##### Selection Page
#####	Contains SelectedRTL, SuggestedRTL, CombinedRTL,
#####	and two buttons to add and remove items from the
#####	Combined RTL.
#####

	### Procedure to init the I/A Selection Page
	proc InitIASelectionPageProc {} {
		global OK
		global gd


		### The PageInit variable is used to catch one-time init stuff..
		global IASelectionPageInit


		### List of entities selected when screen was brought up.
		global SelectedList
		if {$IASelectionPageInit == 0} {
			set SelectedList {
				{} 
				{} 
				{}
				{ reval_async DISmain "dis_view -text -rtl [dis_rtl_get_id -server SelectedList]"}
			}
			dis_rtl_post_init [dis_rtl_get_id SelectedList]	SelectedListFilter
		}

		### List of related entities automatically suggested.
		global SuggestedList
		if {$IASelectionPageInit == 0} {
			set SuggestedList {
				{} 
				{} 
				{}
				{ reval_async DISmain "dis_view -text -rtl [dis_rtl_get_id -server SuggestedList]" }
				}
			dis_rtl_post_init [dis_rtl_get_id SuggestedList] SuggestedListFilter
		}

		### Final, combined list of entities selected by the user.
		global CombinedList
		if {$IASelectionPageInit == 0} {
			set CombinedList {
				{} 
				{} 
				{}
				{ reval_async DISmain "dis_view -text -rtl [dis_rtl_get_id -server CombinedList]"}
			}
			dis_rtl_post_init [dis_rtl_get_id CombinedList] CombinedListFilter
		}


		### Add Btn moves items seleted in either Selected or Suggested lists
		### into the Combined List.
		global Add
		if {$IASelectionPageInit == 0} {
			dis_gdInitButton $gd Add { reval_async DISia dis_ia_add }
		}

		### Remove Btn moves items selected in the Combined list into either the
		### Combined RTL or the Selected RTL.
		global Remove
		if {$IASelectionPageInit == 0} {
			dis_gdInitButton $gd Remove { reval_async DISia  dis_ia_remove }
		}
 


		###
		### Initialize the server.
		###

		### Input RTL must be set before OpenDialog is called.
		global InputRTL
		if { $IASelectionPageInit == 0 } then {
			set selected  [dis_rtl_get_id -server SelectedList]
			set suggested [dis_rtl_get_id -server SuggestedList]
			set combined  [dis_rtl_get_id -server CombinedList]
			if { "$InputRTL" >= 0} then {
				reval_async DISia "dis_ia_init $selected $suggested $combined -rtl $InputRTL"
			} else {
				reval_async DISia "dis_ia_init $selected $suggested $combined"
			}
		}
 

		### Tell server to save all files.
		if {$IASelectionPageInit == 0} {
			reval_async DISia "dis_ia_save_files 1"
		}

		### Set the PageInit variable.
		set IASelectionPageInit 1

		return $OK
	}	; ### End of Selection Page Init.




#####
##### I/A Rename Page
#####    Contains a Browser with 2 lists and 2 RTLs,
#####    Change From and Change To text fields,
#####    And 2 buttons. (Rename (selected), and Rename All)
#####


	### Procedure to init the I/A Rename Page
	proc InitIARenamePageProc {} {
		global OK
		global gd

		### The PageInit variable is used to catch one-time init stuff..
		global IARenamePageInit
 

		### List of Catagories.
		global Categories RenameCategoriesList RenameCatSelection Selection Item
		global RenameElementsRTLServer RenameResultsRTLServer
		if {$IARenamePageInit == 0} {
			set RenameCategoriesList {
				{ llength $Categories } 
				{ lindex [lindex $Categories $Item] 0 }
				{ set RenameCatSelection [lindex [lindex $Categories $Selection] 1]
				  reval_async DISia "dis_ia_query -categories $RenameCatSelection $RenameElementsRTLServer $RenameResultsRTLServer"
				}
				{ }
			}
			set RenameCatSelection 0
		}


 
		### List of Elements.
		global RenameAskSelection
		global RenameElementsList
		global RenameResultsList
		if {$IARenamePageInit == 0} {
			set RenameElementsList {
				{ } 
				{ }
				{ }
				{ reval_async DISmain "dis_view -text -rtl $RenameElementsRTLServer" }
			}
			dis_rtl_post_init [dis_rtl_get_id RenameElementsList] RenameElementsListFilter
			set RenameElementsRTLServer [dis_rtl_get_id -server RenameElementsList]
			set RenameAskSelection "-1"
		}




		### List of Ask/Queries.
		global RenameAskSelection RenameAskList Ask
		if {$IARenamePageInit == 0} {
			set RenameAskList {
				{ llength $Ask } 
				{ lindex [lindex $Ask $Item] 0 }
				{ set RenameAskSelection [lindex [lindex $Ask $Selection] 1]
				  reval_async DISia "dis_ia_query -ask $RenameAskSelection $RenameElementsRTLServer $RenameResultsRTLServer"
				}
				{ }
			}
		}
  


		### List of results.
		if {$IARenamePageInit == 0} {
			set RenameResultsList {
				{} 
				{} 
				{ reval_async DISmain "dis_SetStatusInfo $RenameResultsRTLServer" }
				{ reval_async DISmain "dis_view -text -rtl $RenameResultsRTLServer" }
			}
		 	dis_rtl_post_init [dis_rtl_get_id RenameResultsList] RenameResultsListFilter
			set RenameResultsRTLServer [dis_rtl_get_id -server RenameResultsList]
		}



		### Tell server to set up for a rename operation.
		global IAOperation
		if {$IARenamePageInit == 0} {
			set IAOperation "Impact"
			reval_async DISia "dis_ia_impact_analysis Impact $RenameElementsRTLServer $RenameResultsRTLServer"
		}



		### Set the PageInit variable.
		set IARenamePageInit 1

		return $OK
	}




##### I/A Report Page

	### Procedure to init the I/A Report Page
	proc InitIAReportPageProc {} {
		global OK
		global gd

		### The PageInit variable is used to catch one-time init stuff..
		global IAReportPageInit


		global ReportName
		if {$IAReportPageInit == 0}  then {
			dis_gdInitTextItem $gd ReportName { }
			set ReportName "impact.dat"
		}

		global ReportNamelocal
		set ReportNamelocal $ReportName

		global ReportBrowseBtn
		if {$IAReportPageInit == 0} then {
			dis_gdInitButton $gd ReportBrowseBtn {
				set promptOK 0 
				catch {
					set ReportNamelocal [dis_prompt_file ReportName -new {Report File}]
					set promptOK 1
				}
				if {$promptOK} {
					if {$ReportNamelocal!=""} then {
						set ReportName $ReportNamelocal
						dis_ViewFile ReportContents "$ReportNamelocal" 0
					}
				}
			}
		}

		global  ReportContents
		if {$IAReportPageInit == 0} then {
	                dis_gdInitTextItem $gd ReportContents { }
		}

		global  ReportOKBtn
		if {$IAReportPageInit == 0} then {
		        dis_gdInitButton $gd  ReportOKBtn  {
				reval  DISia "dis_ia_generate  ReportContents {$ReportName}"
			}
		}

		### Set the PageInit variable.
		set IAReportPageInit 1
		dis_ViewFile ReportContents "$ReportNamelocal" 0

		return $OK
	}	; ### End of Report Page Init.



### Now that the procedures have been defined, open the dialog.

	### The RTL containing the selection should be the only argument.
	### Input RTL is used in the Selection Page initialization procedure.
	global Args InputRTL
	if { [info exists Args] == 0 } then {
		set InputRTL  "-1"
		dis_Message "No input RTL given to Impact/Propagate."
	} else {
		set InputRTL [lindex $Args 0]
	}

	OpenDialog 
	return $OK

}
