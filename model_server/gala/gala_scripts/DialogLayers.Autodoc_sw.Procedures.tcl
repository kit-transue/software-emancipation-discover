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

	### This procedure is called when switching to an existing REPORT app.
	proc ReopenDialog {} {
		InitREPORTBrowserPageProc
		InitREPORTTemplatePageProc
		InitREPORTQueryPageProc
		InitREPORTGeneratePageProc
	}



	### This procedure is called the first time the REPORT app is opened.
	proc OpenDialog { BrowserRTLServerId } {
		global OK

		global gd
		set gd -1

		### Init the REPORTsw application interface
		reval DISreport dis_report_startup  

		### Init the Browser Pages.
		global REPORTBrowserPageInit
		set REPORTBrowserPageInit 0
		InitREPORTBrowserPageProc

		### Init the Template Page
		global REPORTTemplatePageInit
		set REPORTTemplatePageInit 0
		global gd
		dis_gdInitNotebookPage $gd REPORTTemplatePage { InitREPORTTemplatePageProc }

		### Init the Query Page.
		global REPORTQueryPageInit
		set REPORTQueryPageInit 0
		global gd
		dis_gdInitNotebookPage $gd REPORTQueryPage { InitREPORTQueryPageProc }

		### Init the Generate Page.
		global REPORTGeneratePageInit
		set REPORTGeneratePageInit 0
		global gd
		dis_gdInitNotebookPage $gd REPORTGeneratePage { InitREPORTGeneratePageProc }

       	return OK
    	}	; ### End of OpenDialog procedure for REPORTsw.





	### Initialize the Browser  Pages of the REPORT Notebook
	proc InitREPORTBrowserPageProc {} {
		global OK
		### The init variable is used to catch first time initialization.
		global REPORTBrowserPageInit

		if { $REPORTBrowserPageInit == 0} then {
   			eval [dis_getTextStrRes "DialogLayers:BROWSERcode"]
		} else {
			BROWSER_ModelPageInitProc
			BROWSER_ProjectPageInitProc
		}

		### Mark this page as initialized.
		set REPORTBrowserPageInit 1

		return OK
	}	; ### End of  InitREPORTBrowserPageProc  procedure.





	### Initialize the Template Page
	proc InitREPORTTemplatePageProc {} {
		global OK
		global REPORTTemplatePageInit
		global gd


		### Init the Template Name Choice.
		global REPORTTemplateName
		global CurrentTemplateName
		global REPORTPartData
		global CurrentPart OriginalPartText 
		global REPORTPartText REPORTPartTextChanged
		if {$REPORTTemplatePageInit == 0} then {
			set REPORTTemplateName ""
			set CurrentTemplateName ""
			set CurrentPart ""
			set OriginalPartText ""
			set REPORTPartTextChanged 0
			dis_gdInitPopDownComboBox $gd REPORTTemplateName {
				global CurrentTemplateName
				### Update only if template name changes.
				if { "$REPORTTemplateName" != "$CurrentTemplateName" } then {
					### Update the current part text (if necessary).
					UpdateREPORTTemplateText

					### Update the current template.
					set CurrentTemplateName        "$REPORTTemplateName"

					### Update the Section Data
					set REPORTPartData [ reval DISreport "dis_report_part -getList" ]

				}
			}
			### Clear the combo list. get the list of names, and add them to the combo.
			dis_gdClearCombo $gd REPORTTemplateName

			### Get the list of template names from the server.
			set tempNameList [reval DISreport "dis_report_template -getList" ]
			### Because the list comes from scanning directories, we need to sort and remove duplicates.
			### luniq is defined later in this file.
			set tempNameList [luniq $tempNameList]
			dis_gdAddListToCombo $gd REPORTTemplateName $tempNameList
		}



		### Init the Template Type Choice.
		global REPORTTemplateType
		global CurrentTemplateType
		if {$REPORTTemplatePageInit == 0} then {
			set CurrentTemplateType ""
			dis_gdInitPopDownComboBox $gd REPORTTemplateType {
				### Update only if template type changes.
				if { "$REPORTTemplateType" != "$CurrentTemplateType" } then {
					### Update the current section text (if necessary).
					UpdateREPORTTemplateText
					
					### Update the current template type choice.
					set CurrentTemplateType "$REPORTTemplateType"

					### Update the Part Data
					if { [llength "$CurrentPart"] > 0 } then {
						### Load in the Part text.
						set cmd [ list dis_report_part -load $CurrentTemplateName $CurrentPart $CurrentTemplateType ]
						set OriginalPartText [reval DISreport "$cmd"]
						dis_ViewText REPORTPartText -overwrite "$OriginalPartText"
					}
				}
			}
			### Clear the combo list, get the list of types, and add them to the combo.
			dis_gdClearCombo $gd REPORTTemplateType
			set typeList [reval DISreport "dis_report_template -getTypes"]
			dis_gdAddListToCombo $gd REPORTTemplateType $typeList
		}




		### Init Part Data, List, and Text.
		###    Part-Data is an invisible text item used only to hold Section-Type data.
		###    Part-List is the list of Section-Types
		###    Part-Text is the text of the selected Section-Types.
		global REPORTPartList REPORTPartData REPORTPartText
		global gd v Selection Item
		global origTemplateName
		set origTemplateName "$CurrentTemplateName"
		if { $REPORTTemplatePageInit == 0} then {
			dis_gdInitTextItem $gd REPORTPartData {
				### Update the Part list with the new data.
				set REPORTPartList {
					{ llength $REPORTPartData }
					{ lindex $REPORTPartData $Item }
					{
						### Get the new Part name.
						if { [llength "$Selection"] > 0 } then {
							set newPart [ lindex $REPORTPartData $Selection ]
						} else {
							set newPart ""
						}
						if { ("$origTemplateName" != "$CurrentTemplateName") || ("$newPart" != "$CurrentPart") } then {
							### Update the current text, if necessary.
							UpdateREPORTTemplateText

							### If a new Part is selected, load in the text.
							if { [llength "$newPart"] > 0 } then {
								set CurrentPart $newPart
								### Load in the text for this selection.
								set cmd [ list dis_report_part -load $CurrentTemplateName $CurrentPart $CurrentTemplateType ]
								set OriginalPartText [reval DISreport "$cmd"]
								dis_ViewText REPORTPartText -overwrite "$OriginalPartText"
								### Enable the Save button, if templates are writable.
								dis_ItemEnable REPORTTemplateSaveBtn $REPORTTemplatesWritable

							### If no selection, clear the existing text.
							} else {    ;### New Part is null.
								set CurrentPart ""
								set OriginalPartText ""
								dis_ViewText REPORTPartText -clear
								dis_ItemEnable REPORTTemplateSaveBtn 0
							}
							set origTemplateName "$CurrentTemplateName"
						}	;### Done handling section change.
					}
					{ }
				}
			}
			dis_gdInitTextItem $gd REPORTPartText { }
					
			### Init the Section Data
			set REPORTPartData [ reval DISreport "dis_report_part -getList" ]
		}




		### Init the Save Btn
		global REPORTTemplatesWritable
		if { $REPORTTemplatePageInit == 0 } then {
			### Find out if the user can write out new templates.
			set REPORTTemplatesWritable [ reval DISreport "dis_report_template -getWritable" ]
			dis_gdInitButton $gd REPORTTemplateSaveBtn {
				set OriginalPartText [ dis_GetText REPORTPartText ]
				set cmd [ list dis_report_part -save $CurrentTemplateName $CurrentPart $CurrentTemplateType "$OriginalPartText" ]
				reval_async DISreport "$cmd"
			}
		}


		### Mark this page as Initialized.
		set REPORTTemplatePageInit 1

		return OK
	} ;### End of REPORTTemplateInitProc procedure.



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



	### Procedure to handle checking and saving the template text.
	proc UpdateREPORTTemplateText {} {
		global OK

		global OriginalPartText
		global REPORTTemplateName REPORTPartText REPORTPartData
		global CurrentTemplateName CurrentPart CurrentTemplateType
		set currentPartText [dis_GetText REPORTPartText]
		if { "$OriginalPartText" != "$currentPartText" } then {
			set answer [dis_prompt YesNoDialog "Report" "Do you want to save changes to Part '$CurrentPart'?" ]
			if { "$answer" == "Yes" } then {
				set cmd [ list dis_report_part -save $CurrentTemplateName $CurrentPart $CurrentTemplateType "$currentPartText" ]
				reval_async DISreport "$cmd"
			}
			set OriginalPartText $currentPartText
		}

		return OK
	}





	### Initialize the Query Page of the REPORT Notebook
	proc InitREPORTQueryPageProc {} {
		global OK

		### The init variable is used to catch one-time initialization.
		global REPORTQueryPageInit
		global gd


		### Init the Query Clipboard. (Used for copy/cut/paste operations.)
		global REPORTQueryClipboard


		### Init Query-List widget.
		###    Query-Data is an invisible text item used only to hold data for 
		### the Query-List widget.
		###	  Query-Save-Data is the data that is changed and written out.
		###		It was created because changing the data being viewed requires re-entrancy.
		###
		global REPORTQueryData 
		global REPORTQuerySaveData
		global REPORTQueryIndex
		
		
		

		if { $REPORTQueryPageInit == 0} then {
			set REPORTQueryClipboard ""
			set REPORTQueryIndex ""
			dis_gdInitTextItem $gd REPORTQueryData {
				global Selection Item
				global REPORTQueryEntityList
				global REPORTQueryQuestionList AskListItems
				### Update the Query Entity list with the new data.
				### Note - The entity field is from the beginning to the first ':'.
				set REPORTQueryEntityList {	{ llength $REPORTQueryData } 
				                            { set tmpQuery [ lindex $REPORTQueryData $Item] 
					                          set colonIndex [ string first ":" $tmpQuery ]
					                          string range $tmpQuery 0 [ incr colonIndex -1 ] } 
											{ set REPORTQueryIndex "$Selection"
					                          if { "$REPORTQueryIndex" != ""} then {
						                         set selectedQuery [lindex $REPORTQuerySaveData $REPORTQueryIndex]
						                         ### Update AskListItems based on what was selected.
						                         ### Metrics -> Metrics ask list
						                         ### All others -> Get list from server.
						                         ### Get list of Questions to select in the QueryQuestion List.
										         set AskListItems [reval DISreport "dis_report_query -loadQuestionTypes $selectedQuery"]
						                         InitREPORTQueryListProc
						                         regsub "^.*:" $selectedQuery "" questionString
						                         regsub -all "," $questionString " " questionList
						                         set selList ""
						                         foreach question "$questionList" {
							                        set index [lsearch -glob $AskListItems $question]
							                        if { $index >= 0 } then {
								                       lappend selList $index
							                        } else {
								                       dis_Message unable to find question '$question'."
							                        }
						                         }
						                         regsub -all " " $selList "," selection
						                         dis_gdListItemSelection REPORTQueryQuestionList $selection
					                          } else {
						                         dis_gdListItemSelection REPORTQueryQuestionList ""
					                          }
					                         } 
											 { }
				                         }
			}
		}

		### Init the Template name.
		global REPORTTemplateName
		global REPORTQueryTemplateName
		global REPORTQueryData
		if {$REPORTQueryPageInit == 0} then {
            set REPORTQueryTemplateName $REPORTTemplateName
			dis_gdInitPopDownComboBox $gd REPORTQueryTemplateName {
				if {"$REPORTQueryTemplateName" != "$REPORTTemplateName"} then {
					### Update the Template name on Template screen.
					set REPORTTemplateName $REPORTQueryTemplateName
					### Load in new queries.
					set REPORTQueryData [ reval DISreport "dis_report_query -load $REPORTQueryTemplateName" ]
					set REPORTQuerySaveData $REPORTQueryData
				}
			}
			### Clear the list of template names.
			dis_gdClearCombo $gd REPORTQueryTemplateName
			### Load in the list of template names.
			set tempNameList [ reval DISreport "dis_report_template -getList" ]
			set tempNameList [luniq $tempNameList]
			dis_gdAddListToCombo $gd REPORTQueryTemplateName $tempNameList

			### Init the query list.
			set REPORTQueryData [ reval DISreport "dis_report_query -load $REPORTQueryTemplateName" ]
			set REPORTQuerySaveData $REPORTQueryData
		}

		### Always update this combo box to match the Template Page combo box.
		set REPORTGenerateTemplateName $REPORTTemplateName
		set REPORTQueryTemplateName $REPORTTemplateName

		### Init Query Type Choice.
		global REPORTQueryType
		global NextQueryType
		if {$REPORTQueryPageInit == 0 } then {
			set NextQueryType ""
			dis_gdInitPopDownComboBox $gd REPORTQueryType {
				if { "$REPORTQueryType" != "$NextQueryType" } then {
					set NextQueryType "$REPORTQueryType"
				}
			}
			### Clear the list.
			dis_gdClearCombo $gd REPORTQueryType
			### Load in the list of query types.
			set qList [ reval DISreport "dis_report_query -loadQueryTypes" ]
			dis_gdAddListToCombo $gd REPORTQueryType $qList
		}

		### Init Insert Query Button
		global REPORTQueryInsertBtn
		if { $REPORTQueryPageInit == 0} then {
			dis_gdInitButton $gd REPORTQueryInsertBtn {
				set REPORTQueryClipboard "$NextQueryType:"
				REPORTQueryInsert
			}
		}


		### Init Save Query Button
		global REPORTQuerySaveBtn
		global REPORTTemplatesWritable
		if { $REPORTQueryPageInit == 0} then {
			dis_gdInitButton $gd REPORTQuerySaveBtn {
				set cmd [list dis_report_query -save "$REPORTQueryTemplateName" "$REPORTQuerySaveData"]
				reval_async DISreport "$cmd"
			}
			dis_ItemEnable REPORTQuerySaveBtn $REPORTTemplatesWritable
		}


		### Init AskListItems used in the Query Question list.
		global AskListItems
		if {$REPORTQueryPageInit == 0} then {
			set AskListItems [reval DISreport "dis_report_query -loadQuestionTypes Files"]
		}

		### Init Query Question List.
		if {$REPORTQueryPageInit == 0} then {
			InitREPORTQueryListProc
		}


		### mark this page as initialized.
		set REPORTQueryPageInit 1

		return OK
	}	; ### End of InitREPORTQueryPageProc procedure.


	#### Init Query Question List.
	#### Used when the Query type is selected.
	proc InitREPORTQueryListProc {} {
		global REPORTQueryQuestionList
		global REPORTSelectedQuestions
		global REPORTQueryIndex
		global REPORTQuerySaveData
		global AskListItems
		global SelectedQuestion
		global Selection Item
		set REPORTSelectedQuestions ""
		set REPORTQueryQuestionList {
			{ llength $AskListItems }
			{ lindex $AskListItems $Item} 
			{
			  if { $Selection != "" } then {
				if { $REPORTQueryIndex != "" } then {
					set REPORTSelectedQuestions [dis_SelectionFromList "$AskListItems" $Selection]
					### convert spaces to commas
					regsub -all " " $REPORTSelectedQuestions "," question
					### update the query value.
					set oldQuery [lindex $REPORTQuerySaveData $REPORTQueryIndex]
					set queryString ""
					regsub ":.*$" $oldQuery ":$question" newQuery
					set REPORTQuerySaveData [lreplace $REPORTQuerySaveData $REPORTQueryIndex $REPORTQueryIndex "$newQuery"]
				}
			}
			}
			{ }
		}	
	}	; ### End if REPORTQueryListInit procedure.



	#### Query Copy.
	proc REPORTQueryCopy {} {
		global REPORTQueryClipboard REPORTQueryData REPORTQueryIndex
		if { "$REPORTQueryIndex" != "" } then {
			set REPORTQueryClipboard [lindex $REPORTQueryData $REPORTQueryIndex]
		} else {
			set REPORTQueryClipboard ""
			dis_Message "Nothing to Copy."
		}

		return ""
	}



	#### Query Cut.
	proc REPORTQueryCut {} {
		global REPORTQueryClipboard REPORTQueryData REPORTQuerySaveData REPORTQueryIndex REPORTQueryEntityList
		if { "$REPORTQueryIndex" != "" } then {
			REPORTQueryCopy
			set REPORTQuerySaveData [lreplace $REPORTQuerySaveData $REPORTQueryIndex $REPORTQueryIndex]
			set REPORTQueryData $REPORTQuerySaveData
		} else {
			dis_Message "Nothing to Cut."
		}

		return ""
	}



	#### Query Paste.
	proc REPORTQueryPaste {} {
		global REPORTQueryClipboard REPORTQueryData REPORTQuerySaveData REPORTQueryIndex
		if { "$REPORTQueryClipboard" != "" } then {
			if { "$REPORTQueryIndex" != "" } then {
				set REPORTQuerySaveData [lreplace $REPORTQuerySaveData $REPORTQueryIndex $REPORTQueryIndex "$REPORTQueryClipboard"]
				set REPORTQueryData $REPORTQuerySaveData
			} else {
				dis_Message "Select item to Paste into."
			}
		} else {
			dis_Message "Nothing to Paste."
		}

		return ""
	}



	#### Query Insert.
	proc REPORTQueryInsert {} {
		global REPORTQueryClipboard REPORTQueryData REPORTQuerySaveData REPORTQueryIndex
		if { "$REPORTQueryClipboard" != "" } then {
			if { "$REPORTQueryIndex" != "" } then {
				set REPORTQuerySaveData [linsert $REPORTQuerySaveData $REPORTQueryIndex "$REPORTQueryClipboard"]
			} else {
				### Nothing selected, add to the end of the list.
				lappend REPORTQuerySaveData $REPORTQueryClipboard
			}
			set REPORTQueryData $REPORTQuerySaveData
		} else {
			dis_Message "Nothing to Insert."
		}

		return ""
	}





	### Initialize the Generate Page of the REPORT Notebook
	proc InitREPORTGeneratePageProc {} {
		global OK
		### The init variable is used to catch first time initialization.
		global REPORTGeneratePageInit
		global gd


		### Init the Template Choice.
		### There is the same looking choice on three pages.
		### This one updates Queries and Queries will update Template.
		global REPORTTemplateName
		global REPORTGenerateTemplateName
		if {$REPORTGeneratePageInit == 0} then {
			dis_gdInitPopDownComboBox $gd REPORTGenerateTemplateName {
				set REPORTTemplateName $REPORTGenerateTemplateName
			}
			### Clear the list.
			dis_gdClearCombo $gd REPORTGenerateTemplateName
			### Load in the list of template names.
			set tempNameList [ reval DISreport "dis_report_template -getList"]
			### Make sure that the list has unique members.
			set tempNameList [luniq $tempNameList]
			dis_gdAddListToCombo $gd REPORTGenerateTemplateName $tempNameList
		}

		### Always update this combo box to match the Template Page combo box.
		set REPORTGenerateTemplateName $REPORTTemplateName

		### Init Report Title
		global gd
		global REPORTGenerateTitle
		if { $REPORTGeneratePageInit == 0} then {
			dis_gdInitTextItem $gd REPORTGenerateTitle { }
			set REPORTGenerateTitle "DISCOVER Report"
		}


		### Init Report Name
		global gd
		global REPORTName
		if { $REPORTGeneratePageInit == 0} then {
			dis_gdInitTextItem $gd REPORTName { }
			set REPORTName "Report.dat"
		}


		### Init Browse... Button for Report File
		global REPORTGenerateBrowseBtn
		if { $REPORTGeneratePageInit == 0} then {
			dis_setvar -global {REPORTName(path)} {C:\\}
			dis_gdInitButton $gd REPORTGenerateBrowseBtn {
				set REPORTNamelocal [dis_prompt_file REPORTName -new {Report File}]
				if {$REPORTNamelocal!=""} then {
					set REPORTName $REPORTNamelocal
				}
			}
		}

		### Init Report Text.
		###    This text item is cleared when the Generate button is pressed, and
		### and filled in by the server when the report has been generated.
		global REPORTGenerateText
		global ReportStatus
		if { $REPORTGeneratePageInit == 0} then {
			dis_gdInitTextItem $gd REPORTGenerateText {
				if {  $REPORTGenerateText == "-clear"} then {
					set ReportStatus 1
				    UpdateGenerateBtnsProc
				} else {
					set ReportStatus 0
					UpdateGenerateBtnsProc
				}
			}
		}

		### Init Generate-Now Button.
		global REPORTGenerateNowBtn
		global BROWSERSourceServerRTL
		if { $REPORTGeneratePageInit == 0} then {
	 		dis_gdInitButton $gd REPORTGenerateNowBtn {
				### Clear the report text.
				dis_ViewText REPORTGenerateText -clear
				### Make sure we have a Selection RTL.
				set rtlId $BROWSERSourceServerRTL 
				### Start generating the report
				reval_async DISreport "dis_report_generate -start REPORTGenerateText {$REPORTName} {$REPORTGenerateTitle} {$REPORTTemplateName} $rtlId"
			}
		}

		### Init Cancel Button.
		global REPORTGenerateCancelBtn
		if { $REPORTGeneratePageInit == 0} then {
	 		dis_gdInitButton $gd REPORTGenerateCancelBtn {
				### Update the report text.
				dis_ViewText REPORTGenerateText -overwrite "Report Canceled"
				### Send the cancel command to the Server.
				reval_async DIS_report "dis_report_generate -cancel"
			}
		}

		### Initialize Generate/Cancel button status.
		set ReportStatus 0
		UpdateGenerateBtnsProc
		

		### Mark this page as initialized.
		set REPORTGeneratePageInit 1

		return OK
	}	; ### End of InitREPORTGeneratePageProc procedure.







	### Enable/disable REPORTGenerate buttons based on Status variable.
	### Current states are :	 0 == Not Running, Disable Cancel Btn.
	###				-1 == Running, Disable Generate and Print Btns.
	proc UpdateGenerateBtnsProc {} {
		global OK

		### Update the Btn status.
		global ReportStatus
		if { $ReportStatus  == 0 } then {
			dis_ItemEnable REPORTGenerateNowBtn 1
			dis_ItemEnable REPORTGenerateCancelBtn 0
		} else {
			dis_ItemEnable REPORTGenerateNowBtn 0
			dis_ItemEnable REPORTGenerateCancelBtn 1
		}

		return $OK
	}	; ### End of UpdateGenerateBtnsProc procedure.




       ###
       ### Close Dialog - currently empty.
       ###

	proc CloseDialog {} {
	}




	### This is where the processing actually starts.
	### All we do is open the dialog and exit.


	### Make sure we have an argument list.
	if { [info exists Args] == 0 } then {
		 set Args "-1"
	}

	 ### Open the dialog.
	 set status [ OpenDialog $Args ]

	 return $status
}
