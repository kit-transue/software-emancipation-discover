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
	
    proc ReopenDialog {} {
		BROWSER_ModelPageInitProc
		BROWSER_ProjectPageInitProc
    }

	### This procedure does all of the work.
	proc OpenDialog {} {
	global OK  gd
    global Roots RootsRTL  RootsServerRTL  rootsFilter
    global DORMANTExtractNowBtn  DORMANTExtractHelpBtn

		### Init the DORMANTsw application interface
		reval_async DISdormant dis_dormant_startup
         
        ### Init Root Entities and this is done only once
        set RootsRTL  [dis_rtl_get_id  Roots]
        set RootsServerRTL  [dis_rtl_get_id  -server Roots] 

        set Roots {{} {} {} {reval_async DISmain "dis_view -text -rtl $RootsServerRTL"} }

        dis_rtl_post_init $RootsRTL 	rootsFilter

        set gd -1
        dis_gdInitNotebookPage $gd ExtractPage  InitDORMANTExtractPageProc

		### Init the Extract Button
 		dis_gdInitButton $gd DORMANTExtractNowBtn {
			 DORMANTExtractBtnProc
		}

		### Init the Help Button
 		#dis_gdInitButton $gd DORMANTExtractHelpBtn {
		#		dis_help -link 220 "DORMANTsw.hlp"
		#}

        	return $OK
    	}	; ### End of OpenDialog procedure for DORMANTsw.


	### Initialize the Extract Page 
	proc InitDORMANTExtractPageProc {}  {  
		global OK
		global Roots  
		global RootsServerRTL     LastModelServerRTL

		if { [ info exists LastModelServerRTL]  != 0 } then {
			set RootsServerRTL  [dis_rtl_get_id  -server Roots] 
              		reval_async DISdormant  "dis_dormant_update_roots  $RootsServerRTL  $LastModelServerRTL"
		}
	}

	### Start/Update Extract procedure
	proc  DORMANTExtractBtnProc {}  {   
		global OK
		global Roots RootsServerRTL  

		set RootsServerRTL  [dis_rtl_get_id  -server Roots]
		reval_async DISdormant  "dis_dormant_extract $RootsServerRTL" 
	}

    proc CloseDialog {} {       }


	### This is where the processing actually starts.
	### All we do is open the dialog and exit.

	eval [dis_getTextStrRes  "DialogLayers.BROWSERcode" ]

	set status [ OpenDialog ] 


	return $status
}
