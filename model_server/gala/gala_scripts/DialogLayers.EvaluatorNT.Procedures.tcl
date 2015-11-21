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
        #flag marking that the page is opened, this is tested before "Exit"ing
        global QACockpit_opened
        set QACockpit_opened "1"   

	global gd OK
	set OK 0	
	global sevsel
	set sevsel ""
		
	global Selection
	set Selection ""

	#first we source the communications abstraction /utilities file
	global env

        #flag to tell if a session needs to be saved
        global save_flag
        set save_flag "0"
	
	set f [dis_getenv PSETHOME]
	#dis_confirm1 "psethome is" OK "f= $f"

	source "$f/lib/gala_scripts/sev_callbacks.tcl"

	proc OpenDialog {} {
		InitWidgets
		InitState 1
		}

	proc SaveQACSession {}  {
	        global save_flag
	        if { "$save_flag" == "1" } {
		   set x [ dis_confirm2 "QA Cockpit" "Save" "Continue" "You have an unsaved QA Cockpit session, do you want to save ?" ]
		   if { "$x" == "LeftButton"  } {
		      MenuSaveAs
		   }
	       }
	    }
	

	proc InitWidgets {} {
		global gd
		
		### first the results page
		global tagResultsText
		dis_gdInitTextItem $gd tagResultsText {}
		
		###  then the regular page
		global tagInput tagPath tagNodes tagWeight tagScore tagEnabled
		
		global RunAllBtn ResetBtn ReportBtn UpBtn RunSelectedBtn
		#global this is used as the current index for a number of functions

		dis_gdInitButton $gd RunAllBtn {RunAllButtonCallback}
		dis_gdInitButton $gd RunSelectedBtn {RunSelectedButtonCallback}
		dis_gdInitButton $gd  ResetBtn {ResetButtonCallback}
		dis_gdInitButton $gd ReportBtn {ReportButtonCallback}
		dis_gdInitButton $gd UpBtn {UpButtonCallback}

		dis_gdInitTextItem $gd tagPath {}
		#dis_confirm1 "adsf" OK "asdfasdfasdf"
		dis_gdInitExclusiveGroup $gd tagEnabled {EnabledCallback}
		#dis_gdInitTextItem $gd tagThreshold {ThresholdCallback}
		dis_gdInitTextItem $gd tagScore {}
		dis_gdInitTextItem $gd tagTotal {}

		#dis_gdInitNumberSpinner $gd tagSpinner {SpinnerCallback}
		dis_gdInitTextItem $gd tagWeight {WeightCallback}
		dis_gdInitListItem $gd tagNodes {}
		dis_gdInitPopDownComboBox $gd tagInput {ComboBoxCallback}
		
		global tagShowInstances
		dis_gdInitButton $gd tagShowInstances {ShowInstancesCallback}	
	}

	proc InitState {do_startup} {
		if {$do_startup == 1} {
		lsev_startup}
		global lastitem gd
		set lastitem ""		
	
		global tagInput tagPath tagNodes tagWeight  tagScore
	
		global nodeList iconList
		set tagPath "/"
		BuildCenterList
		global UpBtn
		dis_ItemEnable UpBtn 0
		global tagInput
		
		dis_gdClearCombo $gd tagInput
		# get the stuff in the drop down combo list
		set comboList [lsev_get_input_types]
		foreach i $comboList {
			dis_gdAddToCombo $gd tagInput $i
		}
		set tagInput [lindex $comboList 0]
		dis_gdComboSelect $gd tagInput 0
		
		global tagResultsText
		set tagResultsText ""
		global tagInstances tagEnabled
		dis_ItemEnable tagWeight 0
		dis_ItemEnable tagScore 0
		dis_ItemEnable tagInstances 0
		dis_ItemEnable tagEnabled 0
		global tagShowInstances
		dis_ItemEnable tagShowInstances 0
	}
	
	proc BuildIconList {nodes path} {
		
		#dis_confirm1 {OK} {OK} "Nodes are $nodes path=$path"
		set icons ""

		set curicon ""
		foreach i $nodes {
		set newlist [list $path $i]

		if { $path != "/"} {
			set fullname [join $newlist "/"]
			} else {set fullname [join $newlist ""]
		}
			
		set x [lsev_get_type "$fullname"]
		
		if {$x == "folder"} {
			set curicon "Folder"
		} elseif {$x=="query_0" } {
			set curicon "Query"
		} elseif {$x =="query_1"} {
			set curicon "Query"
		} elseif {$x == "query_2"} {
			set curicon "Query"
		}	else {set curicon "Folder"}
		
		set en [lsev_get_enabled "$fullname"]
		if {$en == 0} {set curicon Disabled$curicon}
		lappend icons $curicon
		}
		return $icons
	}
	
	proc SelectionCallback {index} {
		global tagEnabled
		dis_ItemEnable tagEnabled 1
		global tagPath nodeList
		set curitem [GetCurrentSelection]
		
		
		set curquerytype [lsev_get_type "$curitem"]
		global lastitem
		
		if {$lastitem != $curitem} {
		set lastitem $curitem
		global tagWeight tagScore tagInstances tagTotal
		
		#Q: why the tremendously complicated set <UIobject> <value>
		#statements?  A: to prevent unnecessary redrawing.
		# I believe that the overhead of going through the parsing and such
		# is considerably less than that of updating the tcl variables, updating the UI
		# and exercising the associated tcl scripts, but I haven't timed it (how???)

		global tagEnabled
		set newenabled [lsev_get_enabled "$curitem"]	
		if {![info exists tagEnabled] ||$newenabled != $tagEnabled} {
			set tagEnabled $newenabled
		}
		set newweight [ lsev_get_weight $curitem ]
                if {![info exists tagWeight] || $newweight != $tagWeight} {
		        set tagWeight ""
                        set tagWeight $newweight
		}
                set tagWeight $newweight
		set newscore [lsev_get_score "$curitem"]
		if {![info exists tagScore] || $newscore != $tagScore} {
			set tagScore ""
			set tagScore $newscore
		}
		global tagShowInstances 
		if {$curquerytype =="query_0" ||$curquerytype == "query_1"} {
			set tagInstances [lsev_get_num_inst "$curitem"]	
			if {$tagInstances >0} {dis_ItemEnable tagShowInstances 1
			} else {dis_ItemEnable tagShowInstances 0}
		} else {
			set tagInstances "N/A"
			dis_ItemEnable tagShowInstances 0
		}
		set newtotal [lsev_get_score "/"]
		if {![info exists tagTotal] || $newtotal !=$tagTotal} {
			set tagTotal $newtotal
		}
		
		}
		set x [lsev_get_enabled "$curitem"]
		#dis_confirm1 adsf OK "Curitem =$curitem x=$x"
		if {$x==0} {
			dis_ItemEnable tagWeight 0
			dis_ItemEnable tagScore 0
			dis_ItemEnable tagInstances 0
		} else {
			dis_ItemEnable tagWeight 1
			dis_ItemEnable tagScore 1
			dis_ItemEnable tagInstances 1
			#don't do tagThreshold because it has already been done
		}	
	}
		
	proc DoubleClickCallback {index} {
		global tagPath nodeList
		set nextitem [lindex $nodeList $index]
		set newlist [list $tagPath $nextitem]
		
		if {$tagPath != "/"} {
			set newpath [join $newlist "/"]
		} else {
			set newpath [join $newlist ""]
		}
	
		set x [lsev_get_enabled "$newpath"]
		# can't open a deselected item
		if {$x==0} {return}
		#set newnodes [lsev_get_children "$newpath"]
		#if we have no children we are at a leaf--can't do anything
		
		if {[lsev_get_type "$newpath"] == "folder"} {
			global UpBtn
			if {$tagPath == "/" } {
				dis_ItemEnable UpBtn 1
			}
			set tagPath $newpath
			BuildCenterList
		} else {
			DoubleClickQueryCallback "$newpath"
		}
	}
	
	proc GetCurrentIndex {} {
		global sevsel
		return $sevsel
	}

	proc GetCurrentSelection {} {
		global sevsel tagPath nodeList curSelection
		set curSelection ""
		if {[info exists sevsel]} {	
			if {$tagPath =="/"} {
			set curSelection "$tagPath[lindex $nodeList $sevsel]"
			} else {
			set curSelection "$tagPath/[lindex $nodeList $sevsel]"
			}
		}
		return $curSelection
	}
	

	proc BuildCenterList {} {
		global tagNodes nodeList iconList tagPath
		set nodeList ""
		set nl [lsev_get_children "$tagPath"]
		foreach i $nl {
			if {"[string index "$i" 0]" !="."} {lappend nodeList $i}
		}			
		set iconList [BuildIconList "$nodeList" "$tagPath"]
		global Selection sevsel Item
		set tagNodes  {
		{global nodeList; return [llength $nodeList]}
		{global nodeList iconList; lindex $nodeList $Item}
		{global Selection sevsel; set sevsel $Selection; SelectionCallback $sevsel}
		{global sevsel; DoubleClickCallback $sevsel}
		{global nodeList iconList; lindex $iconList $Item}
		}
	}

	proc UpButtonCallback {} {
		global tagPath
		if {$tagPath =="/"} {return}
		set newpath [file dirname $tagPath]
		global UpBtn
		if { $newpath == "/"} {dis_ItemEnable UpBtn 0}
		set tagPath $newpath
		BuildCenterList	
		}

	proc WeightCallback {} {
	        global tagWeight
			if { [lsev_license VI2 co] <=0} {
				set q 43;
				# a bunch of no-ops
				set q $q$q
				dis_confirm1 "QFS Master" OK "You need a QFS Master license to perform this operation!"
				return
			}
	        set curitem [ GetCurrentSelection ]
                set val 0
			if { [ catch { expr $tagWeight } val ] != 0 } { set val 0 }
                set tagWeight $val
	        lsev_set_weight $curitem $tagWeight
                lsev_recalculate /
                global tagTotal
                set newtotal [ lsev_get_score / ]
	        if {![info exists tagTotal] ||$newtotal != $tagTotal} {set tagTotal $newtotal}
	}

	proc ComboBoxCallback {} {
		global tagInput
		lsev_set_input "$tagInput"
	}

	proc RunAllButtonCallback {} {
		set x [dis_confirm2 "Confirm Run Queries" OK Cancel "Running your queries could take a long time.\nAre you sure that you want to do this?"]
		if {$x =="LeftButton"} {
			RunQuery "/" 
		}
	}
	
	proc RunSelectedButtonCallback {} {
		set curitem [GetCurrentSelection]
		if {"$curitem" == ""} {dis_confirm1 "No Selected Query" OK "Please select a query to run!"} else {
		RunQuery "$curitem"
		}
		
	}
	proc ReportButtonCallback {} {
		if { [lsev_license VI7 co] <=0 } {
			dis_confirm1 "License Error" OK "You need a QFS Report license to use this feature!"
			return
			}
		dis_launch_dialog SEReport -modal
	}

	proc ResetButtonCallback {} {
		set x [dis_confirm2 "Confirm Reset" OK Cancel "Are you sure that you\nwant to reset all weights and thresholds?"]
		if {$x=="LeftButton"} {
			lsev_reset
			InitState 0
		}
		global tagTotal
		set newtotal [lsev_get_score "/"]
		if {![info exists tagTotal] || $newtotal !=$tagTotal} {
			set tagTotal $newtotal
		}
	}

	proc DoubleClickQueryCallback {curitem} {
		set qtype [lsev_get_type "$curitem"]

		set qname \"[file tail "$curitem"]\"
		
		if {$qtype == "query_0" } {
			return
		} elseif {$qtype =="query_1"} {
			#we ask them if they want to set the value, or do nothing

			set qval [lsev_get_threshold  "$curitem"]
			set title "Change Threshold?"
			set msg "Enter the new value for the threshold of  query\n$qname"
			set x [dis_prompt_string2 $title OK Cancel $msg $qval]

			if {[lindex $x 0] ==0} {
				set rhs [lindex $x 1]
				if  {"$rhs" != "$qval"} {
					set isnogood [ catch {
						expr $rhs + 0

						} nval]

					if {$isnogood}  {
						dis_confirm1 "Incorrect Value!" OK "\"$rhs\" is not a recognized integer!" 
					} else {
						lsev_set_threshold "$curitem" "$nval"
					}
				}
			}
		}
	}

	proc EnabledCallback {} {
		global tagEnabled
		#dis_confirm1 {} {} "enabledcallback"
		global sevsel
		if {$sevsel == ""} {return}
		set cs [GetCurrentSelection]
		set x [lsev_get_enabled "$cs"]
	
		#do nothing if the user clicks twice
		if {$x == $tagEnabled } {return} 

		lsev_set_enabled "$cs" $tagEnabled
		global tagPath nodeList
		global Selection
		set lastsel $sevsel
		BuildCenterList
		set Selection $lastsel
	}
	
	proc ShowInstancesCallback {} {
		if { [lsev_license VI4 co]  <=0} {
			set qq 0
			set foobar $q
			dis_confirm1 "License Error" OK "You need a Browse Instances license\nto use this feature"
			return
		}
		set x [GetCurrentSelection]
		lsev_browse_results "$x"
	}

	proc RunQuery {path} {
		global tagInput
		global tagResultsText

	        global save_flag
	        set save_flag "1"

		set res [lsev_run_query "$path" "$tagInput"]
	   
		set spacer "------------------------------------\n"
		if {"$res"!=""} {
			global tagResultsText
			if {[info exists tagResultsText] && $tagResultsText !=""} {
				set tagResultsText "$res\n$spacer$tagResultsText"
			} else {
			set tagResultsText "$res"
			}
			dis_activate_notebook_page tagResultsPage
		}
				
	# update the UI bearing in mind that tha path may not be the current
	# selection
		global tagTotal
		set tagTotal [lsev_get_score "/"]
		global sevsel
		if {[info exists sevsel]} {
		set sel [GetCurrentSelection]
		set t [lsev_get_type "$sel"]
		if { "$t" != "folder" } {
		  global tagInstances
		  set numinst [lsev_get_num_inst "$sel"]
		  set tagInstances $numinst
		  if {$numinst >0 } { set x 1} else {set x 0}
		  global tagShowInstances
		  dis_ItemEnable tagShowInstances $x
		}
		global tagWeight tagScore 
		set tagWeight [lsev_get_weight "$sel"]
		set tagScore [lsev_get_score "$sel"]
		}
	}

		
	proc ReopenDialog {} {
		BROWSER_ModelPageInitProc
		BROWSER_ProjectPageInitProc
		BROWSER_GrpPageInitProc
	}

	# Initialize the groups page (two RTLs and a name field):
       
    eval [ dis_getTextStrRes "DialogLayers:GROUPcode" ]
	set returnValue [eval [dis_getTextStrRes "DialogLayers:BROWSERcode"]]
	global GLOBAL_LastServerRTL

	proc SEVInitProc {} {
		global CLIENT_EVAL

		global GLOBAL_LastServerRTL
		set proceed [info exists GLOBAL_LastServerRTL]
		if { $proceed != "0" } {	
			$CLIENT_EVAL "set_gala_val GLOBAL_GALA_RTL $GLOBAL_LastServerRTL"
		}
	
		global GLOBAL_ProjectScope
		set GLOBAL_ProjectScope [dis_getvar -global GLOBAL_ProjectScope]
		$CLIENT_EVAL "set_gala_val GLOBAL_GALA_SCOPE $GLOBAL_ProjectScope"
    
	}

	# the following simply open the dialog
	proc MenuOpen {} {
	set junk [dis_launch_dialog SessionManager -modal -block "Session Manager"]
	MaybeRedrawScreen
	}
	proc MenuRemove {} {
	set junk [dis_launch_dialog SessionManager -modal -block "Session Manager"]
	MaybeRedrawScreen
	}
	proc MenuSave {} {
	        global save_flag
		set x [lsev_get_current_session]
		if {$x == "" } {
			MenuSaveAs
			return
		} else {
		set save_flag "0"
		lsev_save_session "$x"
		}
		BuildCenterList
	}
	proc MenuSaveAs {} {
	        global save_flag
		set x [dis_prompt_string2 "Session name:" OK Cancel "Please enter name for this session:"]
		if {[lindex $x 0] == 1} {return}
		#corresponds to the cancel button being pressed.
		set nm [lrange $x 1 [llength $x]]
		#dis_confirm1 asdf OK "name is \"$nm\" length is [llength $nm]"
		set save_flag "0"
		lsev_save_session "$nm"	
		BuildCenterList
	}
	proc MaybeRedrawScreen {} {
		#dis_confirm1 asdf OK "redraw screen"
		if { [dis_getvar -global SEVMustRedraw] } {
		#make it redraw
		BuildCenterList
		#reset the variable
		dis_setvar -global SEVMustRedraw 0
		}
	}


	proc CloseDialog {} {}

	dis_gdInitNotebookPage $gd tagSEV {SEVInitProc }
    SEVInitProc
	OpenDialog
	return $OK
}
