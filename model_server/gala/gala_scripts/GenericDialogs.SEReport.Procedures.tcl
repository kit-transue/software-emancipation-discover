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
#big warning: you must have sourced sev.dis on the psetserver to run this..

eval {
	global gd OK
	set OK 0
	
	set fn "[dis_getenv PSETHOME]/lib/gala_scripts/sev_callbacks.tcl"
	set fn [join [split "$fn" "\\"] /]
	source "$fn"
	global CLIENT_EVAL
			
	
	proc OpenDialog {} {
		InitWidgets
		InitState
	}

	proc InitWidgets {} {
		global gd
		global tagFilename tagRun tagCancel tagFileType tagBrowse tagVerbose

		dis_gdInitTextItem $gd tagFilename {} {}
		dis_ItemEnable tagFilename 1
		dis_gdInitButton $gd tagRun {RunCallback}
		dis_gdInitButton $gd tagCancel {CancelCallback}
		dis_gdInitButton $gd tagBrowse {BrowseCallback}
		dis_gdInitPopDownComboBox $gd tagFileType {}
		dis_gdInitToggle $gd tagVerbose {}
	
	}
	
	proc InitState {} {
		
		
		global comboList tagFileType gd
		dis_gdClearCombo $gd tagFileType

		set comboList [lsev_get_report_types]
		foreach i $comboList {
			dis_gdAddToCombo $gd tagFileType $i
		}
		dis_gdComboSelect $gd tagFileType 0
		set tagFileType [lindex $comboList 0]

		global tagFilename
		set tagFilename ""
		global tagVerbose
		set tagVerbose 1
	}



	proc CancelCallback {} {
		global gd
		dis_close_dialog $gd
	}
	
	proc RunCallback {} {
		global gd tagFilename tagFileType

		if {"$tagFilename" == ""} {
			dis_confirm1 "Incorrect Filename!" OK "Please choose a filename\nusing the browse button."
			return
		}

		global tagFileType tagFilename tagVerbose
		
		regsub -all {\\} $tagFilename "/" tempfilename
		set tagFilename $tempfilename

		set x [lsev_report "$tagFileType" "$tagFilename" $tagVerbose]
		if {"$x" != ""} { 
			dis_confirm1 "Error Creating Report!" OK "Discover encountered the following error:\n $x"
		} 

		dis_close_dialog $gd
	}


	proc BrowseCallback {} {
		global tagFilename
		set cmd "filesb"
		global CLIENT_EVAL
		set n [$CLIENT_EVAL "$cmd"]
		if {"$n" != ""} {set tagFilename "$n"}
	}
	
	proc ReopenDialog {} {
	}
	
	proc CloseDialog {} {
	}
	OpenDialog

	return $OK
}	
