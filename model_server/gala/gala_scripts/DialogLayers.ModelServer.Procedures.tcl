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

	proc UpdateTextItems {} {
		global PDFItem ServiceNameItem SettingsFileItem ExtendedServerInfo
	
		set items {PDFItem ServiceNameItem SettingsFileItem ExtendedServerInfo }
		foreach textitem $items {
			catch { dis_update_gdTextItem $textitem }
		}
	}
	
	#please put all initial values of text boxes here to shrink
	#OpenDialog, and also to minimize the chance of a really fatal TCL error

	proc InitTextItems {} {
		global  PDFItem ServiceNameItem  Selection SettingsFileItem
		
		set SettingsFileItem ""
		set PDFItem ""
		set ServiceNameItem ""
		set Selection ""
		UpdateTextItems
		}

	proc is_ms_windows {} {
		set ret 0
		global tcl_platform
		if { [info exists tcl_platform(platform)]  && $tcl_platform(platform) == "windows" } {
			set ret 1
		}
 		return $ret
	}


	# this is a utility function append a filename to a path
	# without worrying about whether the path has a trailing (back)slash
	# StringOne is a path StringTwo is a directory or filename
	proc PathJoin {StringOne StringTwo} {
		set LastChar [string index $StringOne [expr [string length $StringOne] - 1]]
		if {$LastChar == "\\" && [is_ms_windows]} {set c1 1} else {set c1 0} 
		set c2 0
		if {$LastChar == "/" && (![is_ms_windows])} {set c2 1} else {set c2 0} 
		if { $c1 || $c2} {
			set ret $StringOne$StringTwo
			} elseif {[is_ms_windows]} {
			set ret $StringOne\\$StringTwo
			} else {
			set ret $StringOne/$StringTwo
		}
		return $ret
	}	
	

	#returns / or \\ depending on platform.
	proc GetSeparatorString {} {
		if {[is_ms_windows]} {
			return {\\}
		} else {
		return {/}
		}
		}
	
	proc SetupPrefs {} {
		global SettingsFileItem PDFItem ServiceNameItem
		set state 0
		if {$SettingsFileItem==""} {set state -1}
		if {!$state} { set state [expr [file exists $SettingsFileItem] -1] }
		if {!$state} {
			set state [GetEnvVariablesFromPrefs]
		}
		if {!$state} { 
			global PDFItemTemp
			catch {set bIsSettingsOpen [global SettingsFileItem; dis_OpenSettings "psetPrefs" $SettingsFileItem]
			set PDFItemTemp [dis_ReadSettingsVar pdfFileBuild]
			dis_CloseSettings 0	}
		global PDFItem
	set PDFItem [dis_expand_envvars $PDFItemTemp]
		catch {dis_update_gdTextItem $PDFItem}
		#now suggest a name for the model server
		
		if {$ServiceNameItem==""} {
			set TmpName "Model_Server_on_"
			set blah [split $PDFItem [GetSeparatorString]]
			set blah [lindex $blah [expr [llength $blah]-1]]
			#with luck blah should now have the file name...
			set blah [join [split $blah "."] "_"]
			set blah [concat $TmpName $blah]
			set blah [join $blah ""]
			set ServiceNameItem $blah
			catch {dis_update_gdTextItem $ServiceNameItem}
			}
 
		}
	}



	proc OpenDialog  {}  {
		global gd
		set gd -1
	
		### init the pdf, settings, and service name boxes
		global PDFItem SettingsFileItem
		global ServiceNameItem
		dis_gdInitTextItem $gd PDFItem {}
		dis_gdInitTextItem $gd ServiceNameItem {}
		dis_gdInitTextItem $gd SettingsFileItem {SetupPrefs}
		#   init the results box
		global ResultsItem
		dis_gdInitTextItem $gd ResultsItem
		set ResultsItem ""
			
		
		### next init the browse buttons because they are easy to do...

		global PDFItemBtn
		dis_gdInitButton $gd PDFItemBtn {
			global PDFItem
			dis_setvar -global {PDFItem(path)} {c:\\}
			dis_setvar -global {PDFItem(defaultFilter)} {PDF Files}
			catch {set PDFtemp [dis_prompt_file PDFItem -open {Project Definition File}]
				if {$PDFtemp != ""} {
					set PDFItem $PDFtemp
				}
			} 
		}	
		
		
		global SettingsFileItemBtn
		dis_gdInitButton $gd SettingsFileItemBtn {
			global SettingsFileItem
			set start_dir [dis_getenv PSETHOME]
			dis_setvar -global {SettingsFileItem(path)} {c:\\}
			dis_setvar -global {SettingsFileItem(defaultFilter)} {Preference Files}
			catch {
				set SettingsFileItemTemp [dis_prompt_file SettingsFileItem -open {Settings File}]
				if {$SettingsFileItemTemp!=""} {
					set SettingsFileItem $SettingsFileItemTemp
				}
			}
		}

		#init the pmod server list and the extended server info
		global ServiceList 
		dis_gdInitListItem $gd ServiceList {}
		dis_gdInitTextItem $gd ExtendedServerInfoItem {}

		
		
		
		# init the shutdown button and then disable it because we don't know if we can use it yet...
		global ShutDownBtn
		dis_gdInitButton $gd ShutDownBtn {
			ShutDownPmodServer
						}
		dis_ItemEnable ShutDownBtn 0
		

		#init the refresh button
		global RefreshBtn
		dis_gdInitButton $gd RefreshBtn {
			RefreshServerList
			
		}
	
		#init the start button
		global StartBtn
		dis_gdInitButton $gd StartBtn {
			 if {[is_ms_windows]&& [dis_build_dead_dirs -n] >0} {
			 dis_launch_dialog CleanupDirs -modal -block}
			StartNewServer
		}

		#set the clicking the server list page to update the server list
		dis_gdInitNotebookPage $gd AMSPage {
			global ExtendedServerInfo
			set ExtendedServerInfo "Updating server list. Please stand by..."
			UpdateTextItems
			UpdateServerList
		}

		if {![dis_vdasserv isalive]} {
		set title "Communications Server Not Found!"
		set msg "Discover was not able to find a communications server (vdasserv).\n Models served from this host will not be exposed to the network."
		dis_confirm1 $title "OK" $msg
		
		} 

		#populate the server list, set initial values.
		UpdateServerList
		InitTextItems
		return 0
	}

#end of OpenDialog
	
	# this is put here to be callable from the menu...
	proc RefreshServerList {} {
	 global ExtendedServerInfoItem
	 set ExtendedServerInfoItem  "Now updating server list. This may take a while..."
	 UpdateTextItems
	 dis_vdasserv purge
	 UpdateServerList
	 }


	# this procedure will update the list of active model servers
	proc UpdateServerList {} {

		global PMODS_LIST
		set PMODS_LIST [dis_build_pmods_list	"pmod:.*"]

		global ServiceListSize
		set  ServiceListSize [llength $PMODS_LIST]

		global ServiceList Item Selection
		
		set ServiceList {
			{ return $ServiceListSize }
			{ lindex [lindex $PMODS_LIST $Item] 0}
			{ 
				global PMODS_LIST
				set info [lindex $PMODS_LIST $Selection]
				
				set msg ""
				catch {set msg [lindex $info 1]}
				global ExtendedServerInfoItem
				set ExtendedServerInfoItem $msg
			}
			{}
		}		
		if {$ServiceListSize >0 } {
			dis_ItemEnable ShutDownBtn 1
			global ExtendedServerInfoItem
			set ExtendedServerInfoItem "Please select a Model Server..."} else {
			dis_ItemEnable ShutDownBtn 0
			global ExtendedServerInfoItem
			set ExtendedServerInfoItem "No active Model Servers found ..."}	
	}



		# this function starts a new server	

	proc StartNewServer {} {
		global PDFItem
		global ServiceNameItem
		global ServiceList
		global ServiceListSize
		global ResultsItem
	
		global SettingsFileItem

		#update service list to make sure data is fresh
		UpdateServerList
		# next update all text items
		UpdateTextItems
		set state 0

		#make sure that files exist
		if {$PDFItem == ""} {
			dis_puts "PDF File Error--no file"
			set title "PDF File Error"
			set msg "Please choose a Project Definition File"
			set state -1
		} elseif {![file exists $PDFItem]} {
			set state -1
			set title "PDF File Error"
			dis_puts "PDF File Error--Invalid PDF"
			set msg "$PDFItem is not a readable Project Definition File. Please check the filename and try again."
			
		} elseif {$SettingsFileItem!="" && ![file exists $SettingsFileItem]} {
			set state -1
			set title "Settings File Error"
			set msg "$SettingsFileItem is not a readable Settings (.prefs) file.  Please check the filename and try again."
			
		} elseif {[string length $ServiceNameItem] >60}  {
			set state -1
			set title "Service Name Error"
			set msg "Please choose a shorter Service Name"

		} elseif {[llength $ServiceNameItem]>1} {
			set state -1
			set title "Service Name Error"
			set msg "Service Names may not contain spaces or special characters"

		 } elseif {$ServiceNameItem == ""} {
			
			#assuming that it exists, make sure that the the service name is unique and valid
			set title "Service Name Error"
			dis_puts "ServiceName Error--no name"
			set msg "You must enter a Service Name!"
			set state -1
		}
		if {$state!=0} {
		dis_confirm1 $title "OK" $msg}
		set i 0

		while {$state == 0 && $i < $ServiceListSize} {
			global PMODS_LIST
			set curserver [lindex [lindex $PMODS_LIST $i] 0]
		
			if  { $ServiceNameItem == $curserver  } {
				set state -1
				dis_puts "Service Name Error--name in use"
				set title "Service Name Error"
				set msg "The Service Name you have chosen is presently in use.  \n Please choose a different one."
				dis_confirm1 $title "OK" $msg
			} else { set i [expr $i+1] }
		}
		
		# check for and delete the log file if it exists

		 set LogFileName [dis_tempnam {} $ServiceNameItem ]
		  #dis_confirm1 "logfileName" OK "$LogFileName"
		if {$state==0} {
			
			#set LogFileName [PathJoin $LogFileDir $ServiceNameItem]
			##dis_confirm1 $LogFileName OK "$LogFileName"
			set LogExt ".log"
			set LogFileName $LogFileName$LogExt

			#dis_confirm1 LogFileName OK $LogFileName
			if { [file exists $LogFileName]} {
				#dis_confirm1 "LogFileFound" OK "Found file $LogFileName"
				set x [dis_unlink $LogFileName]
				##dis_confirm1 "LogFile Deletion" OK $x
				 
			}
		 } else {
			set state -1
			set title "Log File Directory Error"
			##set msg "$LogFileDir" is not a valid directory name.\n Please check the spelling and try again."
			set msg "General LogFileDirectory error around line 280"
			#dis_confirm1 $title "OK" $msg
		}

		#now try and start the damn thing
		if { $state == 0} {
		
			global ResultsItem
		
			dis_ItemEnable StartBtn 0
			
			#settings should already have been validated...
			if {$SettingsFileItem != ""} {
				#set environment variables
				set state [GetEnvVariablesFromPrefs]

				set OptionalPart " -prefs $SettingsFileItem "
				} else {
				set OptionalPart ""
			}
		}
		if {$state == 0} {

			set msg1 "---------Attempting to start Model Server----------------"
			set msg2 "Service Name:\t \"$ServiceNameItem\""
			set msg3 "Settings File:\t \"$SettingsFileItem\""
			set msg4 "Project Definition File: \t \"$PDFItem\""
			set msg5 "For more information about this model server, please see the following log file:"
			set msg6 "Log File:\t \"$LogFileName\""
			set msg7 "------------------Log File Follows------------------"
			
			
			set SepString [GetSeparatorString]
			set SubCmd [list [dis_getenv PSETHOME] bin pmod_server]
			set SubCmd [join $SubCmd "$SepString"]
			set cmd  "dis_CreateBatch {
				UpdateServerList
				} ResultsItem {$LogFileName}  {} {$msg1} {} {$msg2} {} {$msg3} {} {$msg4} {} {$msg5} {
				} {$msg6} {} {$msg7
				} {$SubCmd -service $ServiceNameItem  $OptionalPart -pdf $PDFItem} {}"
			eval $cmd
			dis_ItemEnable StartBtn 1
			
			dis_activate_notebook_page PmodServerNotebook ResultsPage  
			UpdateServerList

		}

		return $state
	}
	
	
	# should get and set required variables from preferences previously specified
	proc GetEnvVariablesFromPrefs {} {
		global SettingsFileItem env
		global ADMINDIRval sharedSrcRootval privateSrcRootval sharedModelRootval
		
		set state 0
		
		set bIsSettingsOpen [global SettingsFileItem; dis_OpenSettings "psetPrefs" $SettingsFileItem]
		
		if {$bIsSettingsOpen <= 0} {
			set state -1
			set title "Settings File Error" 
			set msg "Couldn't open setings file $SettingsFileItem"
			dis_confirm1 $title OK $msg
		}
		
		#dis_confirm1 Status OK "settings status is $bIsSettingsOpen"

		if {$state == 0} {
			# first get admindir
			set bVarFound  [SetVarFromSettings ADMINDIRval ADMINDIR 0]
			if {$bVarFound == 1 } {
					dis_putenv ADMINDIR [dis_expand_envvars $ADMINDIRval];
					#dis_confirm1 "ADMINDIR" OK [dis_getenv ADMINDIR]
				}
			} else {
				set title "Setting File Error"
				set msg "Could not read value of ADMINDIR from $SettingsFileItem"
				set state -1
				dis_confirm1 $title OK $msg

			}

		
		if {$state == 0} {
			if {[SetVarFromSettings sharedSrcRootval sharedSrcRoot 0]} {
				catch { dis_putenv sharedSrcRoot [dis_expand_envvars $sharedSrcRootval] }
				#dis_confirm1 "Env variable set" OK "shared src root is [dis_getenv sharedSrcRoot] attempted setting was $sharedSrcRootval"

			}
			if {[SetVarFromSettings privateSrcRootval privateSrcRoot 0]} {
				catch { dis_putenv privateSrcRoot [dis_expand_envvars $privateSrcRootval] }
			}
			if {[SetVarFromSettings sharedModelRootval sharedModelRoot 0]} {
				catch { dis_putenv sharedModelRoot [dis_expand_envvars $sharedModelRootval] }
			}
		}
	 	if {$state == -1} {
			dis_confirm1 $title OK $msg
		}
		dis_CloseSettings 0
	 return $state
	 
	 }

	proc ShutDownPmodServer {} {
		global Selection PMODS_LIST  ServiceListSize
		set conf ""
		UpdateServerList
		set state 0
		# the first two lindexes produce a list which contains the
		# extended server info--the first item of which is the number of clients
		# there is some command way to do this as well.
		if {$ServiceListSize ==0 } {
			set state -1
			set title "Service List Error"
			set msg "Please select a running model server"
			dis_confirm1 $title "OK" $msg
		}
		if {$state == 0} {
			set CurClients [lindex [lindex [lindex $PMODS_LIST $Selection] 1] 0]
			
			if {$CurClients != 0} {
				set title "Warning"
				if {$CurClients >1} {set ClientsPlural "s"} else {set ClientsPlural ""}
				set msg "You are about to shut down a Model Server which is presently in use by $CurClients client${ClientsPlural}.\n"
				set msg [concat $msg "Doing so could cause all client DISCOVER sessions to crash.\n"]
				set msg [concat $msg "Do you really want to shut down this Model Server?"]
				set conf [dis_prompt YesNoDialog $title $msg]
				if {$conf == "No"} {set state -1}
			}
		}
		if {$state ==0} {
			dis_shutdown_pmodserver [lindex [lindex $PMODS_LIST $Selection] 0]
			UpdateServerList
		}

		
	
		return $state
	}

	proc SetVarFromSettings {VarName PrefName is_flag} {
		set ret 0
		catch {
			global $VarName
			set $VarName ""
			if { $is_flag } {
				set $VarName 0
			}
			set val [dis_ReadSettingsVar $PrefName]
			if { $is_flag && $val != "0" && $val != "1"} {
				set val [switch -exact $val {
					yes {format 1}
					default {format 0}
				}]			
			}
			set $VarName $val
			set ret 1 
		}
		return $ret
	}


	proc CloseDialog {}  {
	}

	proc CloseHandler {} {
	#dis_confirm1 ASDf OK "I'm in ModelSErver right now!"
	return 0
	}



	proc ReopenDialog {} {
	}


OpenDialog
return $OK
}
