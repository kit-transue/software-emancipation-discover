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
    proc is_ms_windows {} {
        set ret 0
        global tcl_platform
        if { [info exists tcl_platform(platform)]  && $tcl_platform(platform) == "windows" } {
            set ret 1
        }
        return $ret
    }

    proc native_sep { path_string } {
        set ret $path_string
        if {[is_ms_windows]} {
            regsub -all "/" $path_string {\\} ret
        }
        return $ret
    }


    proc ReopenDialog {} {
    }

    proc BuildCheckSum {} {
        global tagPDF tagBuildMetrics ADMINDIRval privateSrcRootval
        global sharedSrcRootval privateModelRootval sharedModelRootval
        global tagDepProjects tagProjProjects tagHeadersProjects tagForgive
        global tagDepEnable tagProjEnable tagHeadersEnable tagSaveProj
        global tagDFAInfo
        global tagDefectLinkEnable 

        set res [concat $tagPDF $tagBuildMetrics $ADMINDIRval $privateSrcRootval $sharedSrcRootval $privateModelRootval $sharedModelRootval $tagDepProjects $tagProjProjects $tagHeadersProjects $tagForgive $tagDepEnable $tagProjEnable $tagHeadersEnable $tagSaveProj $tagDFAInfo $tagDefectLinkEnable]     
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
        global IsSettingsOpen checkSum showMakeRule
        set ret 0
        if {$IsSettingsOpen} {
            set ret [SetVarFromSettings tagPDF pdfFileBuild 0]
            if {!$ret} {
                set ret [SetVarFromSettings tagPDF defaultPDF 0]
            }
            set VarPrefPairs {  
                {tagBuildMetrics DIS_model.SaveMetrics 1} 
                {ADMINDIRval ADMINDIR 0}
                {privateSrcRootval privateSrcRoot 0}
                {sharedSrcRootval sharedSrcRoot 0}
                {privateModelRootval privateModelRoot 0}
                {sharedModelRootval sharedModelRoot 0}
                {tagDepProjects projectListDependency 0}
                {tagProjProjects projectList 0}
                {tagHeadersProjects projectListHeaders 0}
                {tagForgive acceptSyntaxErrors 1}
                {tagDepEnable doPassDependency 1}
                {tagProjEnable doPassProj 1}
                {tagHeadersEnable doPassHeaders 1}
                {tagSaveProj doPassSaveProj 1}
                {tagDFAInfo doBuildDFA 1}
                {tagDefectLinkEnable doPassDefectLink 1}
                {showMakeRule debugShowMakeRuleOutput 1}
            }
            foreach pair $VarPrefPairs {
                set fIsFlag 0
                catch {set fIsFlag [lindex $pair 2]}
                set ret [expr $ret && [SetVarFromSettings [lindex $pair 0] [lindex $pair 1] $fIsFlag]]
            }
            
        }
        set checkSum [BuildCheckSum]
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

        set items {tagPDF ADMINDIRval privateSrcRootval sharedSrcRootval privateModelRootval
        sharedModelRootval tagDepProjects tagProjProjects tagHeadersProjects  tagLogFile
       }
        foreach textitem $items {
            catch { dis_update_gdTextItem $textitem }
        }
    }

    proc WriteSettings {} {
        global IsSettingsOpen   checkSum
        set ret 0
        UpdateTextItems
        if {$IsSettingsOpen} {
            set VarPrefPairs {  
                {tagPDF pdfFileBuild 0}
                {tagBuildMetrics DIS_model.SaveMetrics 0} 
                {ADMINDIRval ADMINDIR 0}
                {privateSrcRootval privateSrcRoot 0}
                {sharedSrcRootval sharedSrcRoot 0}
                {privateModelRootval privateModelRoot 0}
                {sharedModelRootval sharedModelRoot 0}
                {tagDepProjects projectListDependency 0}
                {tagProjProjects projectList 0}
                {tagHeadersProjects projectListHeaders 0}
                {tagForgive acceptSyntaxErrors 1}
                {tagDepEnable doPassDependency 1}
                {tagProjEnable doPassProj 1}
                {tagHeadersEnable doPassHeaders 1}
                {tagSaveProj doPassSaveProj 1}
                {tagDFAInfo doBuildDFA 1}
                {tagDefectLinkEnable doPassDefectLink 1}
            }
            foreach pair $VarPrefPairs {
                set fIsYesNoFlag 0
                catch {set fIsYesNoFlag [lindex $pair 2]}
                set ret [expr $ret && [WriteSettingsVar [lindex $pair 0] [lindex $pair 1] $fIsYesNoFlag]]
            }
            
        }
        return $ret
    }
    
    
    proc BuildUpdateDepOptions { projects do_expand } {
        global tagDepCustom tagDepOptions tagDepProjects

        if {$tagDepCustom} {
            set ret $tagDepOptions
        } else {
            global tagPDF tagPREFS
            set ret ""
            if { $tagPDF!="" } {
                set ret [concat $ret -pdf]
                if {$do_expand} {
                    set ret [concat $ret [dis_expand_envvars $tagPDF]]
                } else {
                    set ret [concat $ret $tagPDF]
                }
            }
            if { $tagPREFS!="" } {
                set ret [concat $ret -prefs]
                if {$do_expand} {
                    set ret [concat $ret [dis_expand_envvars $tagPREFS]]
                } else {
                    set ret [concat $ret $tagPREFS]
                }
            }
            
            set ret [concat $ret "-dependency"]
            set ret [concat $ret $projects]
            
            set tagDepOptions $ret
        }
        return $ret
    }

    proc BuildProjOptions { projects do_expand } {
        global tagForgive tagProjCustom tagProjOptions tagProjProjects

        if {$tagProjCustom} {
            set ret $tagProjOptions
        } else {
            global tagPDF tagPREFS
            set ret ""
            if { $tagPDF!="" } {
                set ret [concat $ret -pdf]
                if {$do_expand} {
                    set ret [concat $ret [dis_expand_envvars $tagPDF]]
                } else {
                    set ret [concat $ret $tagPDF]
                }
            }
            if { $tagPREFS!="" } {
                set ret [concat $ret -prefs]
                if {$do_expand} {
                    set ret [concat $ret [dis_expand_envvars $tagPREFS]]
                } else {
                    set ret [concat $ret $tagPREFS]
                }
            }
            if {$projects != "" } {
                set ret [concat $ret "-proj"]
                set ret [concat $ret $projects]
            }
            global tagForgive
            if { "$tagForgive" != "" } {
                set ret [concat $ret "-forgive"]
            }

            set tagProjOptions $ret
        }
        return $ret
    }
    
	#note: this is a replacement--doesn't use text boxes.
	proc BuildDFAOptions { dfa_option } {
        global tagForgive tagProjProjects
		global tagPDF tagPREFS
		set ret ""
		if { $tagPDF!="" } {
			set ret [concat $ret -pdf \"[dis_expand_envvars $tagPDF]\" ]
		}
		if { $tagPREFS!="" } {
			set ret [concat $ret -prefs \"[dis_expand_envvars $tagPREFS]\" ]
		}
	    global tagForgive
		if { "$tagForgive" != "" } {
			set ret [concat $ret "-forgive"]
		}
		global tagProjProjects
		if {$tagProjProjects != ""} {
			set ret [concat $ret $dfa_option $tagProjProjects]
		}
		#dis_confirm1 asdf OK "$ret"
		return $ret

	}

    proc BuildDefectLinkOptions { do_expand } {
        global tagPDF tagPREFS 
 
        set ret ""
        if { $tagPDF!="" } {
            set ret [concat $ret -pdf]
            if {$do_expand} {
                set ret [concat $ret [dis_expand_envvars $tagPDF]]
            } else {
                set ret [concat $ret $tagPDF]
            }
        }
        if { $tagPREFS!="" } {
            set ret [concat $ret -prefs]
            if {$do_expand} {
                set ret [concat $ret [dis_expand_envvars $tagPREFS]]
            } else {
                set ret [concat $ret $tagPREFS]
            }
        }
        
	set ret [concat $ret -defect_link]
	
        return $ret
    }

    proc BuildHeadersOptions { projects do_expand } {
        global tagForgive tagHeadersCustom tagHeadersOptions
        if {$tagHeadersCustom} {
            set ret $tagHeadersOptions
        } else {
            global tagPDF tagPREFS
            set ret ""
            if { $tagPDF!="" } {
                set ret [concat $ret -pdf]
                if {$do_expand} {
                    set ret [concat $ret [dis_expand_envvars $tagPDF]]
                } else {
                    set ret [concat $ret $tagPDF]
                }
            }
            if { $tagPREFS!="" } {
                set ret [concat $ret -prefs]
                if {$do_expand} {
                    set ret [concat $ret [dis_expand_envvars $tagPREFS]]
                } else {
                    set ret [concat $ret $tagPREFS]
                }
            }
            
            set ret [concat $ret "-headers"]
            set ret [concat $ret $projects]
            
            if {"$tagForgive" != ""} {
                set ret [concat $ret "-forgive"]
            }
            set tagHeadersOptions $ret
        }
        return $ret
    }

    proc BuildSaveProjOptions { opt do_expand } {
        
        global tagPDF tagPREFS
        set ret ""
        if { $tagPDF!="" } {
            set ret [concat $ret -pdf]
            if {$do_expand} {
                set ret [concat $ret [dis_expand_envvars $tagPDF]]
            } else {
                set ret [concat $ret $tagPDF]
            }
        }
        if { $tagPREFS!="" } {
            set ret [concat $ret -prefs]
            if {$do_expand} {
                set ret [concat $ret [dis_expand_envvars $tagPREFS]]
            } else {
                set ret [concat $ret $tagPREFS]
            }
        }
        set ret [concat $ret $opt]
        return $ret
    }


    proc InitDialog {} {

        #Init Settings Page
        global tagPDF tagSaveProj tagBuildMetrics tagForgive showMakeRule tagDFAInfo 
        global ADMINDIRval sharedSrcRootval privateSrcRootval sharedModelRootval privateModelRootval
        global HaveRunModelBuild
		set HaveRunModelBuild 0    

		set tagPDF ""
        set tagSaveProj 1
        set tagBuildMetrics 1
        set tagForgive 1
        set ADMINDIRval ""
        set sharedSrcRootval ""
        set privateSrcRootval ""
        set sharedModelRootval ""
        set privateModelRootval ""
        set showMakeRule 0
        set tagDFAInfo 0

        #init Update Dependencies Page
        global tagDepEnable tagDepProjects tagDepOptions tagDepCustom
        set tagDepEnable 0
        set tagDepCustom 0
        set tagDepProjects ""
        
        #Init Build Projects Page
        global tagProjEnable tagProjOptions tagProjProjects tagProjCustom
        set tagProjProjects ""
        set tagProjCustom 0
        set tagProjEnable 1

        #Init Build Included Page
        global tagHeadersEnable tagHeadersOptions tagHeadersProjects
        global tagHeadersCustom
        set tagHeadersEnable 0
        set tagHeadersProjects ""
        set tagHeadersCustom 0
	
	    #Init DefectLink Pass
        global tagDefectLinkEnable
        set tagDefectLinkEnable 0

        #Load Preferences
        set loadSuccess [LoadSettings]
        BuildUpdateDepOptions $tagDepProjects 0
        BuildProjOptions $tagProjProjects 0
        BuildHeadersOptions $tagHeadersProjects 0
        dis_ItemEnable tagUpdateGrp $tagDepEnable
        dis_ItemEnable tagProjGrp $tagProjEnable
        dis_ItemEnable tagHeadersGrp $tagHeadersEnable
        global PrefFileName     tagPrefsLabel
        set tagPrefsLabel $PrefFileName
    }

    
    ##return values from this function:
    #0 ok to exit
    #1 not ok to exit
    #2 abort (cancel
    
    proc ExitAdmin {} {
        global IsSettingsOpen PrefFileName checkSum fBatchIsRunning     fIsNewFile
        global tmp_fname tagPREFS
        set doExit 1
        set is_quit 0
        set is_running 0
        if {$fBatchIsRunning} {
            set conf [dis_confirm2 "Stop Model Build?" Yes No "Are you sure you want to terminate the model build?"]
            if {$conf == "LeftButton"} {
                set doExit 1
                set is_running 1
            } else {return 2}
        }
        if {$doExit} {
            set saved 1
            if {$IsSettingsOpen } {
                set persistent 0
                UpdateTextItems
                if {$fIsNewFile || [BuildCheckSum] != $checkSum} {
                    set conf [dis_confirm3 "Save changes?" Yes No Cancel [concat "Would you like to save changes in" $PrefFileName?]]
                    if {$conf == "LeftButton"} {
                        set persistent 1
                    } elseif {$conf=="RightButton"} {
                        #dialog should not be shut down and we return quickly
                        set is_quit 2
                        return $is_quit
                    } else {
                        set is_quit 1
                        return $is_quit
                    }
                }       
                if {$persistent} {
                    #dis_confirm1 asdf OK "p=$persistent"
                    set saved [DoSave]
                }
                if {$saved} {
                    dis_CloseSettings 0 #DoSave takes care of flushing
                }
            }
            if {$saved} {
                if {$fIsNewFile && $tmp_fname != "-1"} {
                    dis_unlink $tmp_fname
                }
                catch { dis_putStringPreference DIS_prefs.dis_ADMINPREFS $tagPREFS }
                set is_quit 0
                
            }
        }
        if {$is_running !=0} { return $is_running}
        
        return $is_quit
    }
    
    proc DoOpen {} {
        global checkSum IsSettingsOpen fIsNewFile       PrefFileName
        global tagPrefsLabel tagPREFS showDFAMessage
        global tmp_fname
        set ret 0
        set saved 1
        set do_unlink 1
        if {$IsSettingsOpen} {
            UpdateTextItems
            if {$fIsNewFile || [BuildCheckSum] != $checkSum} { 
                set conf [dis_confirm2 "Save Changes?" Yes No [concat "Would you like to save changes in" $PrefFileName?]]
                if {$conf == "LeftButton"} {
                    set do_unlink 0
                    set saved [DoSave]
                }
            }
        }
        if {$saved} {   
            global open_file prefFileDir
            if {![info exists prefFileDir]} {set prefFileDir "."}
            dis_setvar -global {open_file(path)} $prefFileDir
            dis_setvar -global {open_file(defaultFilter)} {Preference Files}
            set open_file [dis_prompt_file open_file -open {Open}]
            if {$open_file != ""} {
                set prefFileDir [native_sep [file dirname $open_file]]
                if {[file exists $open_file]} {
                    set ret [dis_OpenSettings psetPrefs $open_file]
                    if {$ret} {
                        if {$do_unlink && $fIsNewFile && $tmp_fname != "-1"} {
                            dis_unlink $tmp_fname
                            set tmp_fname "-1"
                        }
                        set IsSettingsOpen 1
                        set PrefFileName $open_file
                        set tagPREFS $open_file 
                        set tagPrefsLabel $PrefFileName
                        set fIsNewFile 0
                        set showDFAMessage 0
                        InitDialog
                        set showDFAMessage 1
                    } else {
                        dis_confirm1 "File Error!" OK [concat "Could not open"  \"$open_file\"]
                    }
                } else {
                    dis_confirm1 "File Error!" OK [concat "The file"    \"$open_file\" "does not exist!"]
                }

            }
        }
        return $ret
    }
    
    proc DoSave {} {
        set ret 0
        global IsSettingsOpen fIsNewFile PrefFileName checkSum
        if { $fIsNewFile } {
            set ret [DoSaveAs]
        } else {
            if {$IsSettingsOpen} { 
                WriteSettings
                if {![dis_FlushSettings]} {
                    dis_confirm1 "Save Failed!" OK [concat "Could not save settings into" \"$PrefFileName\"]
                } else {
                    set ret 1
                    set checkSum [BuildCheckSum]
                    dis_Message [concat "Saved" $PrefFileName]
                }
            }
        }
        return $ret
    }

    proc DoSaveAs {} {
        global checkSum PrefFileName IsSettingsOpen     fIsNewFile
        global tagPrefsLabel tagPREFS tmp_fname
        global new_file prefFileDir
        set ret 0
        if {$IsSettingsOpen} {
            if {![info exists prefFileDir]} {set prefFileDir "."}
            dis_setvar -global {new_file(path)} $prefFileDir
            dis_setvar -global {new_file(defaultFilter)} {Preference Files}
            set new_file [dis_prompt_file new_file -new {Save As}]
            if {$new_file != ""} {
                set prefFileDir [native_sep [file dirname $new_file]]
                WriteSettings
                if {[dis_FlushSettings $new_file] && [dis_OpenSettings psetPrefs $new_file]} {
                    set ret 1
                    dis_Message [concat "Saved" $new_file]
                    if {$fIsNewFile && $tmp_fname != "-1"} {
                        dis_unlink $tmp_fname
                    }
                    set PrefFileName $new_file
                    set tagPREFS $new_file
                    set tagPrefsLabel $PrefFileName
                    set fIsNewFile 0
                    InitDialog
                } else {
                    dis_confirm1 "Save Failed!" OK [concat "Could not save settings into"       \"$new_file\"]
                }
            }
        }
        return $ret
    }

    proc DoNew {} {
        global IsSettingsOpen fIsNewFile PrefFileName showDFAMessage
        global tmp_fname        tagPREFS tagPrefsLabel checkSum
        set ret 0
        set saved 1
        set do_unlink 1
        if {$IsSettingsOpen} {
            UpdateTextItems
            if {$fIsNewFile || [BuildCheckSum] != $checkSum} { 
                set conf [dis_confirm2 "Save Changes?" Yes No [concat "Would you like to save changes in" $PrefFileName?]]
                if {$conf == "LeftButton"} {
                    set do_unlink 0
                    set saved [DoSave]
                }
            }
        }
        if {$saved} {
            set new_tmp_name "-1"       
            catch {set new_tmp_name [dis_tempnam "." "dis"]}
            if {$new_tmp_name != "-1"} {
                catch {
                    set file_id [open $new_tmp_name w]
                    close $file_id
                }
                set def_prefs [concat [dis_GetAppDirectory]/../lib/psetPrefs.default]
                if {[file exists $def_prefs]} {
                    catch {
                        
                        set contents [dis_file_read $def_prefs]
                        dis_file_write $new_tmp_name $contents  
                    }
                }
                set ret [dis_OpenSettings psetPrefs $new_tmp_name]
                if {$ret} {
                    if {$do_unlink && $fIsNewFile && $tmp_fname != "-1"} {
                        dis_unlink $tmp_fname
                    }
                    set tmp_fname $new_tmp_name
                    set IsSettingsOpen 1
                    set PrefFileName "New Settings"
                    set tagPREFS "" 
                    set tagPrefsLabel $PrefFileName
                    set fIsNewFile 1
                    set showDFAMessage 0
                    InitDialog
                    set showDFAMessage 1
                }
            }
        }
        return $ret
    }


    proc OpenDialog {} {
        global gd
        global PrefFileName fIsNewFile tagPREFS
        global tmp_fname pdfFileDir
        global tagLogFile tagDepEnable tagProjEnable tagHeadersEnable
        global tagDepProjects tagProjProjects tagHeadersProjects
        global tagSaveProj
        
        # this is a "magic" variable hardcoded into gala--don't change its name
        # DO NOT MAKE IT GLOBAL, EITHER--otherwise really random characters will periodically
        # be inserted into text boxes.
        set updateTextValue ""
        
        
        
        global lastLogFile
        global showDFAMessage
        set showDFAMessage 0
        set fIsNewFile 0
        set PrefFileName "New Application Settings"
        set tmp_fname "-1"
        set lastLogFile ""
        global fBatchIsRunning
        set fBatchIsRunning 0
        if {![info exists pdfFileDir]} {set pdfFileDir "."}
        dis_setvar -global {tagPDF(defaultFilter)} {PDF Files}
        dis_gdInitButton $gd tagPDFChooser {
            global pdfFileDir
            dis_setvar -global {tagPDF(path)} $pdfFileDir
            set tagPDFlocal [dis_prompt_file tagPDF -open {Project Definition File}]
            if {$tagPDFlocal != ""} {
                set tagPDF $tagPDFlocal
                set pdfFileDir [native_sep [file dirname $tagPDFlocal]]
            }
        }
        dis_gdInitTextItem $gd tagPDF {
            global tagDepProjects tagProjProjects tagHeadersProjects
            BuildUpdateDepOptions $tagDepProjects 0
            BuildProjOptions $tagProjProjects 0
            BuildHeadersOptions $tagHeadersProjects 0
        }
        
        dis_gdInitToggle $gd tagDFAInfo {
            UnsupportedFeatureLauncher
        }
        
        dis_gdInitToggle $gd tagForgive {
            global tagProjProjects tagHeadersProjects
            BuildProjOptions $tagProjProjects 0
            BuildHeadersOptions $tagHeadersProjects 0
        }
        dis_gdInitTextItem $gd tagDepProjects {} {
            BuildUpdateDepOptions $updateTextValue 0
        } 

        dis_gdInitToggle $gd tagDepEnable {
            global tagDepEnable
            dis_ItemEnable tagUpdateGrp $tagDepEnable
        }
        dis_gdInitToggle $gd tagDepCustom {
            global tagDepCustom tagDepProjects
            dis_TextItemSetModifiable tagDepOptions $tagDepCustom
            BuildUpdateDepOptions $tagDepProjects 0
        }

        dis_gdInitTextItem $gd tagProjProjects {} {
            BuildProjOptions $updateTextValue 0
        } 
        
        dis_gdInitToggle $gd tagProjEnable {
            global tagProjEnable
            dis_ItemEnable tagProjGrp $tagProjEnable
        }

        dis_gdInitToggle $gd tagProjCustom {
            global tagProjCustom tagProjProjects
            dis_TextItemSetModifiable tagProjOptions $tagProjCustom
            BuildProjOptions $tagProjProjects 0
        } 

        
        dis_gdInitTextItem $gd tagHeadersProjects {} {
            BuildHeadersOptions $updateTextValue 0
        }               
        dis_gdInitToggle $gd tagHeadersEnable {
            global tagHeadersEnable
            dis_ItemEnable tagHeadersGrp $tagHeadersEnable
        }
        dis_gdInitToggle $gd tagHeadersCustom {
            global tagHeadersCustom tagHeadersProjects
            dis_TextItemSetModifiable tagHeadersOptions $tagHeadersCustom
            BuildHeadersOptions $tagHeadersProjects 0
        }


        dis_gdInitToggle $gd tagDefectLinkEnable {
        }

        ### redoing logfile.
        global tagLogFile logFileDir

        set tagLogFile [native_sep [dis_tempnam "" "log."]]
        set logFileDir [native_sep [file dirname ${tagLogFile}]] 

        dis_ItemEnable tagStop 0
        dis_gdInitButton $gd tagLogChooser {
            global logFileDir
            dis_setvar -global {tagLogFile(path)} $logFileDir
            catch {set tagLoglocal [dis_prompt_file tagLogFile -new {Log File}]}
            if {$tagLoglocal!=""} {
                global tagLogFile
                set tagLogFile $tagLoglocal
                set logFileDir [native_sep [file dirname $tagLoglocal]]
            }
        }

        dis_gdInitButton $gd tagStart {
            StartModelBuild
        }
		dis_gdInitButton $gd tagViewLogfile {
			ViewLogFile
		}
		dis_ItemEnable tagViewLogfile 0


        dis_gdInitButton $gd tagStop {
            set conf [dis_confirm2 "Stop Model Build?" Yes No "Are you sure you want to terminate the model build?"]
            if {$conf == "LeftButton"} {
                dis_CancelBatch
            }
        }

        #Try to open MRU settings file. If the one is not found, open new settings
        global IsSettingsOpen PrefFileName tagPrefsLabel
        
        set tagPREFS ""
        set IsSettingsOpen 0
        set PrefFileName "New Settings"

        catch {set tagPREFS [dis_getStringPreference DIS_prefs.dis_ADMINPREFS]}
        if {$tagPREFS != ""} {
            set IsSettingsOpen [dis_OpenSettings psetPrefs $tagPREFS]
        }
        if {$IsSettingsOpen} {
            set PrefFileName $tagPREFS
            set tagPrefsLabel $PrefFileName
            
        } else {
            DoNew
            
        }
        InitDialog
        
        set showDFAMessage 1
    }
    
    proc CloseDialog {} {
    }

	proc ViewLogFile {} {
		UpdateTextItems
		global HaveRunModelBuild tagLogFile
		#I HATE DEALING WITH FILENAMES.
		
		set tn2 [join [split $tagLogFile \\ ] /]

		#set cmd [concat notepad.exe $tn2 ]
		#dis_confirm1 cmd OK "cmd=$cmd"
		#exec "notepad.exe $tagLogFile"
		#exec notepad.exe
		#exec $cmd
		#dis_confirm1 "adsf" OK "tn=$tn2"
		set pd [exec cmd /c start notepad.exe $tn2 "&"]
	}	
    proc StartModelBuild {} {
		global HaveRunModelBuild
		set HaveRunModelBuild 1
        global tagLogFile tagDepEnable tagProjEnable tagHeadersEnable
        global tagDepProjects tagProjProjects tagHeadersProjects 
	global  tagSaveProj 
        global lastLogFile tagDefectLinkEnable
        
        set fdir [file dirname $tagLogFile]
        set lastchar [native_sep [string index $tagLogFile [expr [string length $tagLogFile] -1 ]] ]
        set sepchar [native_sep "/"]
        
        if { ![file isdirectory $fdir] || [file isdirectory $tagLogFile] || $lastchar == $sepchar} {
            set conf ""
            
            set newLogFile [native_sep [dis_tempnam "" "log."] ]
            set conf [dis_confirm2 "Warning" Continue Cancel "The file \"$tagLogFile\" is not a valid filename.\nUse the file \"$newLogFile\" instead?"]
            if {$conf =="RightButton"} { 
                return -4 
            } else {
                set tagLogFile $newLogFile
            } 
        } elseif { ![string compare $tagLogFile $lastLogFile]} {
            set conf ""
            set conf [dis_confirm2 "Warning" Continue Cancel "You are about to overwrite the log file\n\"$lastLogFile\"."]
            if {$conf =="RightButton"} {return -3}
        }
        
        set lastLogFile $tagLogFile
        set bDoCmd 1
       
        #this is done so that it won't dump the contents of a huge log file 
        # into memory.
  	    set resultsbox "{}" 
        if {[is_ms_windows]&& [dis_build_dead_dirs -n] >0} {
            dis_launch_dialog  CleanupDirs -modal -block
        }
        set cmd "dis_CreateBatch {dis_ItemEnable tagStop 0      
        dis_ItemEnable tagStart 1
        global fBatchIsRunning
        set fBatchIsRunning 0} $resultsbox {$tagLogFile}"
        
        global env
        set pset_home "."
        catch {set pset_home $env(PSETHOME)}
        if {[is_ms_windows]} {
            set pset [concat $pset_home\\bin\\pset_server]
        } else {
            set pset [concat $pset_home/bin/discover]
        }
        set pset [concat $pset "-batch"]

        set nCmds 0
		set status_comments {}
        #set environment vars
        global sharedSrcRootval privateSrcRootval ADMINDIRval
        global privateModelRootval sharedModelRootval   tagBuildMetrics
        dis_putenv ADMINDIR [dis_expand_envvars $ADMINDIRval]
        dis_putenv sharedSrcRoot [dis_expand_envvars $sharedSrcRootval] 
        dis_putenv privateSrcRoot [dis_expand_envvars $privateSrcRootval]
        dis_putenv privateModelRoot [dis_expand_envvars $privateModelRootval] 
        dis_putenv sharedModelRoot [dis_expand_envvars $sharedModelRootval]
        dis_putenv PSET_SILENCE_STUBS 1
        ### this should be removed as soon as we get rid of the CPP_DEBUG env variable
        global showMakeRule
        catch {
            if ($showMakeRule) {
                dis_putenv "CPP_DEBUG" "1"
            }
        }

        if {$tagBuildMetrics} {
            dis_putenv DIS_MODEL_SAVEMETRICS 1
        } else {
            dis_putenv DIS_MODEL_SAVEMETRICS 0
        }

        if {$tagSaveProj} {
            incr nCmds
			append status_comments "{Saving Project Structure...}"
            
			set cmd $cmd\ {$pset\ [BuildSaveProjOptions "-save_proj" 1]} 
            set cmd [concat $cmd {"Save Project Structure
=======================================================
"                               }]
        }

        if {$tagDepEnable} {
            incr nCmds
            append status_comments "{Updating Dependencies...}"
			set cmd $cmd\ {$pset\ [BuildUpdateDepOptions $tagDepProjects 1]} 
            set cmd [concat $cmd {"Update Dependencies
=======================================================
"                               }]
        }

        if {$tagProjEnable} {
			global tagProjProjects
			catch {
				foreach proj $tagProjProjects {
					incr nCmds
					append status_comments "\{Building Project $proj...\}"
				}
			}
			set AddtlCmd [BuildProjOptions $tagProjProjects 1] 
            if { [lsearch -exact $AddtlCmd "-proj"] != -1} {
                set cmd [concat $cmd {$pset\ $AddtlCmd}]
                #set cmd $cmd\ {$pset\ $AddtlCmd} 
                set cmd [concat $cmd {"Build Projects
=======================================================
"                                       }]
            } else {
                set bDoCmd 0
                dis_activate_notebook_page tagAdminNotebook tagBuildProjPage
                dis_confirm1 "No Projects Specified!" OK "No projects were specified on this page.\nPlease enable some or disable this pass."
            }
        }

        if {$tagHeadersEnable} {
            incr nCmds
			append status_comments "{Building Headers...}"
            set cmd $cmd\ {$pset\ [BuildHeadersOptions $tagHeadersProjects 1]} 
            set cmd [concat $cmd {"Build Headers
=======================================================
"                               }]
        }
		global tagDFAInfo
        if {$tagDFAInfo == 1 && $tagProjEnable} {
            incr nCmds
			append status_comments "{Compiling DFA Information...}"
            set cmd $cmd\ {$pset\ [BuildDFAOptions "-dfa_init"]}
            set cmd [concat $cmd {"Compile DFA Across Projects
=======================================================
"                               }]
        }
		global tagDFAInfo
        if {$tagDFAInfo == 1 && $tagProjEnable} {
            incr nCmds
			append status_comments "{Linking DFA Information...}"
            set cmd $cmd\ {$pset\ [BuildDFAOptions "-dfa"]}
            set cmd [concat $cmd {"Link DFA Across Projects
=======================================================
"                               }]
        }

        if {$tagDefectLinkEnable} {
	    set dl_flags [BuildDefectLinkOptions "1"]
	    if { "$dl_flags" != "" } {
		incr nCmds
		append status_comments "{Defect Link Pass...}"
		set cmd $cmd\ {$pset\ $dl_flags}
		set cmd [concat $cmd {"DefectLink Pass
=======================================================
"                               }]
            }
	}

        set was_built 0
        if {$tagProjEnable || $tagHeadersEnable} {
            set was_built 1
        }
        if {$tagSaveProj && $was_built} {
            incr nCmds
			append status_comments "{Saving Project Roots...}"
            set cmd $cmd\ {$pset\ [BuildSaveProjOptions "-save_root" 1]} 
            set cmd [concat $cmd {"Save Project Roots
=======================================================
"                               }]
        }
	#	dis_confirm1 "comments are" OK "$status_comments"
	
		# be VERY careful, as the parser for the status comments
		# is pretty fragile.
		append cmd " \"$status_comments\" UseProgressBar"

        if {$nCmds && $bDoCmd } {
            global fBatchIsRunning
            dis_ItemEnable tagStop 1
            dis_ItemEnable tagStart 0
			#dis_confirm1 adsf OK "cmd=$cmd"
			eval $cmd
			global tagViewLogfile
			dis_ItemEnable tagViewLogfile 1
            set fBatchIsRunning 1
        } else {
            if { !$nCmds} {
                dis_confirm1 "Model Administrator" OK "Please enable at least one pass!"
            } else {dis_confirm1 "Model Administrator" OK "Please select a valid project to build!"}
        }
    }


    proc UnsupportedFeatureLauncher {} {
        global showDFAMessage tagDFAInfo
        #note: it appears that it updates the TCL variable _BEFORE_
        #running the script associated with the change of a checkbox item
        #so we have to check the "to" state rather than the "from" state
        if {$tagDFAInfo == 1 && $showDFAMessage == 1} {
            dis_launch_dialog DFAMessage -modal
            set showDFAMessage 0
        }
    }

    
    ##return values from this function:
    #0 ok to exit
    #1 not ok to exit
    #2 abort (cancel



    proc CloseHandler {} {
        global IsSettingsOpen
        set ret 0
        if {$IsSettingsOpen} {
            set ret [ExitAdmin]
        }
        return $ret
    }
    
    OpenDialog
    return OK
}
