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
	global  OK 
	set OK 0

	proc ReopenDialog {} {
	}

	proc BuildInputCheckSum {} {
		global tagPDF tagHOME ADMINDIRval privateSrcRootval
		global sharedSrcRootval privateModelRootval sharedModelRootval

		set res [concat $tagPDF $tagHOME $ADMINDIRval $privateSrcRootval $sharedSrcRootval $privateModelRootval $sharedModelRootval]
		return $res
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

	proc LoadSettings {} {
		global IsSettingsOpen	checkSum
		set ret 0
		if {$IsSettingsOpen} {
			set ret [SetVarFromSettings tagPDF pdfFileUser 0]
			if {!$ret} {
				set ret [SetVarFromSettings tagPDF defaultPDF 0]
			}
			set VarPrefPairs {	{tagBuildMetrics DIS_model.SaveMetrics 1} 
						{ADMINDIRval ADMINDIR 0}
						{privateSrcRootval privateSrcRoot 0}
						{sharedSrcRootval sharedSrcRoot 0}
						{privateModelRootval privateModelRoot 0}
						{sharedModelRootval sharedModelRoot 0}
						{tagHOME projectHome 0}
					      }
			foreach pair $VarPrefPairs {
				set fIsFlag 0
				catch {set fIsFlag [lindex $pair 2]}
				set ret [expr $ret && [SetVarFromSettings [lindex $pair 0] [lindex $pair 1] $fIsFlag]]
			}
		}
		set checkSum [BuildInputCheckSum]
		return $ret
	}

	proc WriteSettingsVar {VarName PrefName is_yes_no_flag} {
		set ret 0
		global $VarName
		eval set val $$VarName
		if { $is_yes_no_flag } {
			set val [switch -exact $val {
				1 {format yes}
				default {format no}
			}]			
		}
		set ret [dis_WriteSettingsVar $PrefName $val]
		return $ret
	}

	proc UpdateTextItems {} {
		set items {tagPDF tagHOME ADMINDIRval privateSrcRootval sharedSrcRootval privateModelRootval sharedModelRootval}
		foreach textitem $items {
			catch { dis_update_gdTextItem $textitem }
		}
	}

		proc WriteSettings {} {
		global IsSettingsOpen	checkSum
		set ret 0
		UpdateTextItems
		if {$IsSettingsOpen} {
			set VarPrefPairs {	{tagPDF pdfFileUser 0}
						{tagBuildMetrics DIS_model.SaveMetrics 0} 
						{ADMINDIRval ADMINDIR 0}
						{privateSrcRootval privateSrcRoot 0}
						{sharedSrcRootval sharedSrcRoot 0}
						{privateModelRootval privateModelRoot 0}
						{sharedModelRootval sharedModelRoot 0}
						{tagHOME projectHome 0}
			}
			foreach pair $VarPrefPairs {
				set fIsYesNoFlag 0
				catch {set fIsYesNoFlag [lindex $pair 2]}
				set ret [expr $ret && [WriteSettingsVar [lindex $pair 0] [lindex $pair 1] $fIsYesNoFlag]]
			}
	
		}
		return $ret
	}


	proc DoSave {} {
		set ret 0
		global IsSettingsOpen checkSum tagPREFS
		if {$IsSettingsOpen} { 
			WriteSettings
			if {![dis_FlushSettings]} {
				dis_prompt OKDialog DISCOVER [concat "Could not save settings into"	\"$tagPREFS\"]
			} else {
				set ret 1
				set checkSum [BuildInputCheckSum]
				dis_Message [concat "Saved" $tagPREFS]
			}
		}
		return $ret
	}

	proc FinishChooser { call_quit } {
		global IsSettingsOpen checkSum tagPREFS
		if {$IsSettingsOpen } {
			set persistent 0
			UpdateTextItems
			if {[BuildInputCheckSum] != $checkSum} {
				set conf [dis_prompt YesNoDialog DISCOVER [concat "Would you like to save changes in" $tagPREFS?]]
				if {$conf == "Yes"} {
					set persistent 1
				} 
			}
			if {$persistent} {
				DoSave
			}
			dis_CloseSettings 0 #DoSave takes care of flushing
		}
		catch { dis_putStringPreference DIS_prefs.dis_PREFS $tagPREFS }
		if {$call_quit} {
			dis_quit
		}
		return 0
	}

	proc DoOpen {file_to_open save_pdf} {
		global checkSum	IsSettingsOpen
		global tagPREFS
		set ret 0
		if {$IsSettingsOpen} {
			UpdateTextItems
			if {[BuildInputCheckSum] != $checkSum} { 
				set conf [dis_prompt YesNoDialog DISCOVER [concat "Would you like to save changes in" $tagPREFS?]]
				if {$conf == "Yes"} {
					DoSave
				}
			}
		}
		global open_file
		if {$file_to_open == ""} {	
			dis_setvar -global {open_file(path)} {C:\\}
			dis_setvar -global {open_file(defaultFilter)} {Preference Files}
			set open_file [dis_prompt_file open_file -open {Open Application Settings}]
		} else {
			set open_file $file_to_open
		}
		if {$open_file != ""} {
			if {[file exists $open_file]} {
				set ret [dis_OpenSettings psetPrefs $open_file]
				if {$ret} {
						if {$save_pdf} {
							global new_pdf_file
							dis_WriteSettingsVar pdfFileUser $new_pdf_file
							dis_FlushSettings
						}
						set IsSettingsOpen 1
						set tagPREFS $open_file 
						LoadSettings
						dis_Message [concat "Loaded settings from" $open_file]
					} else {
						dis_prompt OKDialog DISCOVER [concat "Could not open"	\"$open_file\"]
					}
			} else {
				dis_prompt OKDialog DISCOVER [concat "The file"	\"$open_file\" "does not exist!"]
			}
		}
		return $ret
	}

	proc ValidatePDF {} {
		set ret 1
		return $ret
	}

	proc OpenDialog {} { 
		global gd
		set gd -1

### Init the Quit button first, so user will have an easy way out if
### (heaven forbid!) a TCL error occurs.
		dis_gdInitButton $gd QuitDISCOVERBtn { FinishChooser 1 }

		global tagLogFile
		dis_gdInitTextItem $gd tagLogFile {}
		set tagLogFile "The Service Chooser Log feature displays any startup errors that might occur.\nIt is useful for debugging your preference files, PDFs, and user environment.\nTo enable this feature, set the environment variable\n\nDISCOVER_XTERM\n\nto a valid filename."
		dis_gdInitButton  $gd  tagChooserHelp {
			dis_help -topic 1000 "[dis_getenv PSETHOME]\\lib\\hyperhelp\\DISCOVER_HELP.hlp"

		}
		dis_gdInitButton  $gd  StartDISCOVERServerBtn  {
			 			
# save the variables
# set environment variables
			catch { dis_putenv PSET_SILENCE_STUBS 1 }
			catch { dis_putenv ADMINDIR [dis_expand_envvars $ADMINDIRval] }
			catch { dis_putenv sharedSrcRoot [dis_expand_envvars $sharedSrcRootval] }
			catch { dis_putenv privateSrcRoot [dis_expand_envvars $privateSrcRootval] }
			catch { dis_putenv sharedModelRoot [dis_expand_envvars $sharedModelRootval] }
			catch { dis_putenv privateModelRoot [dis_expand_envvars $privateModelRootval] }

#### this is temporary--remove when we finally get rid of the CPP_DEBUG env variable
			global showMakeRule
			SetVarFromSettings showMakeRule debugShowMakeRuleOutput 0
			catch { if (![string compare [string toupper $showMakeRule] "YES"]) {
					dis_putenv "CPP_DEBUG" "1"
					}
			}
#### end  of temporary stuff
			set command [concat [dis_GetAppDirectory]/pset_server]
			if { $tagPDF!="" && $tagHOME!="" } {
				set command [concat $command -pdf $tagPDF]
				if {$tagPREFS!=""} {
					set command [concat $command -prefs $tagPREFS]
				}
				set command [concat $command -server -home $tagHOME]
				set command [concat $command -group [dis_GetMyGroupName] -service DISmain]
				set command [concat $command -client [dis_GetMyServiceName]]
				if {[ValidatePDF]} {
					dis_setPDFname [dis_expand_envvars $tagPDF]
					FinishChooser 0
					dis_SystemCommand "$command" [dis_GalaLogFile] { 
						dis_Message "Could not start application server. Check log files for more information."
						dis_layer -close
						dis_SetViewVariables
						dis_layer -open DialogLayer ServiceChooser
						DoDisplayLogFile
					}
					dis_Message "Locating application server..."
					dis_layer -close
					dis_SetViewVariables
					dis_layer -open DialogLayer SplashScreen
				}

			} else {
					dis_prompt OKDialog DISCOVER "PDF name and home project parameters are required"
			}
		}

		global tagPDF
		dis_setvar -global {tagPDF(path)} {C:\\}
		dis_setvar -global {tagPDF(defaultFilter)} {PDF Files}
		dis_gdInitButton $gd tagPDFChooser {
			set tagPDFlocal [dis_prompt_file tagPDF -open {Project Definition File}]
			if {$tagPDFlocal != ""} {
				set tagPDF $tagPDFlocal
			}
		}

		dis_gdInitButton $gd tagPREFSChooser {
			DoOpen "" 0		
		}

		global IsSettingsOpen tagPREFS
		set IsSettingsOpen 0
		set tagPREFS ""
		set def_prefs [dis_getenv PSETHOME]/lib/psetPrefs.default
		catch { set tagPREFS [dis_getenv defaultPREFS] }
		if { $tagPREFS=="" } {
			catch {set tagPREFS [dis_getStringPreference DIS_prefs.dis_PREFS]}
		}
		if {$tagPREFS == ""} {
			set tagPREFS $def_prefs
		}
		set IsSettingsOpen [dis_OpenSettings psetPrefs $tagPREFS]
		if {$IsSettingsOpen} {
			LoadSettings
		} else {
			set IsSettingsOpen [dis_OpenSettings psetPrefs $def_prefs]
			if {$IsSettingsOpen} {
				set tagPREFS $def_prefs
				LoadSettings
			} else {
				dis_prompt OKDialog DISCOVER "Could not open default settings file"
				dis_exit
			}
		}
		set def_sel $serviceListSize
		incr def_sel -1
		dis_gdListItemSelect ServiceList $def_sel


	}

	proc DoDisplayLogFile {} {
		set lf [dis_getenv "DISCOVER_XTERM"]
		set x [string trim [string tolower "$lf"]]
		#dis_confirm1 asdf OK "x= $x, lf=$lf"
		catch {
		if {[string compare "$x" "console"] != 0 && "$x" != "" } {
			dis_activate_notebook_page SvcChooserLog
			dis_ViewFile tagLogFile "$lf" 0
		}
		}
		#dis_confirm1 "Aasdf" OK "end"
	} 




	proc LayerSystemMenuCloseHandler {} {
		global IsSettingsOpen
		set ret 0
		if {$IsSettingsOpen} {
			set ret [FinishChooser 0]
		}
		return $ret
	}

	proc CloseDialog {} {
		eval {
			proc LayerSystemMenuCloseHandler {} {
				return 0
			}	
		}
	}

	OpenDialog
	return OK
}
