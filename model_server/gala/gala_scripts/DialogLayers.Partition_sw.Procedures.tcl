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

    proc  OpenDialog {} {
 	global OK  gd
       global ManualManipulation  AutoDeclaration 
       global ManualManipulationRTL  AutoDeclarationRTL
       global ManualManipulationServerRTL  AutoDeclarationServerRTL  BROWSERSourceServerRTL
       global ManualManipulationFilter  AutoDeclarationFilter
       global TargetFileName  BROWSER_ProjectScope 
       global FilesNeedFetch  FilesNeedFetchRTL  FilesNeedFetchServerRTL  FilesNeedFetchFilter
       global CheckConflictsButton  BrowseButton MoveButton  CopyButton  CancelButton
       global tmpfilename  flag
       set ManualManipulationRTL   [dis_rtl_get_id  ManualManipulation]
       set AutoDeclarationRTL      [dis_rtl_get_id  AutoDeclaration]
       set FilesNeedFetchRTL       [dis_rtl_get_id  FilesNeedFetch]
       set ManualManipulationServerRTL   [dis_rtl_get_id  -server  ManualManipulation]
       set AutoDeclarationServerRTL      [dis_rtl_get_id  -server  AutoDeclaration]
       set FilesNeedFetchServerRTL       [dis_rtl_get_id  -server  FilesNeedFetch]
              
       set ManualManipulation {{} {} {}
             {reval_async DISmain  "dis_view  -test  -rtl  $ManualManipulationServerRTL"}
	}

       set AutoDeclaration  {{} {} {}
             {reval_async DISmain  "dis_view  -test  -rtl  $AutoDeclarationServerRTL"}
       }

       set FilesNeedFetch  {{} {} {}
             {reval_async DISmain  "dis_view  -test  -rtl  $FilesNeedFetchServerRTL"}
       }

       dis_rtl_post_init  $ManualManipulationRTL  ManualManipulationFilter
       dis_rtl_post_init  $AutoDeclarationRTL     AutoDeclarationFilter
       dis_rtl_post_init  $FilesNeedFetchRTL      FilesNeedFetchFilter


       set gd -1

       dis_gdInitTextItem  $gd  TargetFileName {}

       dis_setvar -global {TargetFileName(path)} {C:\\}
       dis_gdInitButton $gd BrowseButton {
	set TargetFileNamelocal [dis_prompt_file TargetFileName -new {Target File}]
	if {$TargetFileNamelocal!=""} {
	    set TargetFileName $TargetFileNamelocal
	}
       }

       dis_gdInitNotebookPage $gd   PartitionPage  {
          set  TargetFileName  $BROWSER_ProjectScope
       }

       dis_gdInitButton $gd CheckConflictsButton {
          reval_async DISmain "dis_partition -check  $BROWSERSourceServerRTL  $ManualManipulationServerRTL $AutoDeclarationServerRTL"
       }

       dis_gdInitButton $gd MoveButton {
          set tmpfilename $TargetFileName
          reval_async DISmain "dis_partition -physical $tmpfilename -view $v TargetFileName"
          reval_async DISmain "dis_partition -move  $BROWSERSourceServerRTL   $ManualManipulationServerRTL $AutoDeclarationServerRTL $TargetFileName  1 $FilesNeedFetchServerRTL"
      }

       dis_gdInitButton $gd CancelButton {
          reval_async DISmain "dis_partition -clear  $FilesNeedFetchServerRTL"
       }
    }

     proc  CloseDialog {} { }

     eval  [dis_getTextStrRes  "DialogLayers:BROWSERcode" ]  

     OpenDialog

     CloseDialog

     return  OK
}	
