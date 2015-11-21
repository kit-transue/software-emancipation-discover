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
    proc ReopenDialog {} {
		BROWSER_ModelPageInitProc
		BROWSER_ProjectPageInitProc
    }

    proc OpenDialog {} {
        global gd v vr
        global FunctionCallSlider DataReferenceSlider InstanceOfSlider ArgumentTypeSlider ReturnTypeSlider ContainsSlider 
        global FriendOfSlider SubclassOfSlider MemberOfSlider
        global FunctionCallOut DataReferenceOut InstanceOfOut ArgumentTypeOut ReturnTypeOut ContainsOut FriendOfOut 
        global SubclassOfOut MemberOfOut
        global FriendOfLabel SubclassOfLabel MemberOfLabel
        global FileNamePrefixSlider LogicalNameLevelSlider ThresholdSlider FileNamePrefixOut LogicalNameLevelOut ThresholdOut
        global NumberOfSubsystems FileNamePrefixCount LogicalNameLevelCount ExistingSubsystems
        global EXtractItem
        global extractDefaultFile extractLastFile
        global ExtractNowBtn ExtractUndoBtn

        dis_setvar -global "ExtractDialogID" $v

        dis_gdInitSlider $gd  FunctionCallSlider 	{set FunctionCallOut $FunctionCallSlider}
        dis_gdInitSlider $gd  DataReferenceSlider	{set DataReferenceOut $DataReferenceSlider}
        dis_gdInitSlider $gd  InstanceOfSlider 	{set InstanceOfOut $InstanceOfSlider}
        dis_gdInitSlider $gd  ArgumentTypeSlider 	{set ArgumentTypeOut $ArgumentTypeSlider}
        dis_gdInitSlider $gd  ReturnTypeSlider 	{set ReturnTypeOut $ReturnTypeSlider}
        dis_gdInitSlider $gd  ContainsSlider 		{set ContainsOut $ContainsSlider}
        dis_gdInitSlider $gd  FriendOfSlider 		{set FriendOfOut $FriendOfSlider}
        dis_gdInitSlider $gd  SubclassOfSlider 	{set SubclassOfOut $SubclassOfSlider}
        dis_gdInitSlider $gd  MemberOfSlider 	{set MemberOfOut $MemberOfSlider}

        dis_gdInitSlider $gd  FileNamePrefixSlider 		{set FileNamePrefixOut $FileNamePrefixSlider}
        dis_gdInitSlider $gd  LogicalNameLevelSlider 	{set LogicalNameLevelOut $LogicalNameLevelSlider}
        dis_gdInitSlider $gd  ThresholdSlider 		{set ThresholdOut $ThresholdSlider}

        dis_gdInitTextItem $gd NumberOfSubsystems {}
        dis_gdInitTextItem $gd FileNamePrefixCount {}
        dis_gdInitTextItem $gd LogicalNameLevelCount {}

        dis_gdInitButton $gd ExtractNowBtn {
            dis_gdDialogIDEval [dis_getvar -global "ExtractDialogID"] "doExtract"
        }
        dis_gdInitButton $gd ExtractUndoBtn {
	       reval_async "DISextract" "dis_extractUndo"
        }

        dis_gdInitExclusiveGroup $gd ExistingSubsystems {}

        dis_gdInitLabel $gd FriendOfLabel {}
        dis_gdInitLabel $gd SubclassOfLabel {}
        dis_gdInitLabel $gd MemberOfLabel {}

        if {[reval "DISextract" "dis_extractIsCppEnabled"] == 0} {
          	set FriendOfSlider 0
          	set SubclassOfSlider 0
          	set MemberOfSlider 0

          	dis_ItemEnable FriendOfOut 0
          	dis_ItemEnable SubclassOfOut 0
          	dis_ItemEnable MemberOfOut 0

          	dis_ItemEnable FriendOfSlider 0
          	dis_ItemEnable SubclassOfSlider 0
          	dis_ItemEnable MemberOfSlider 0

          	dis_ItemEnable FriendOfLabel 0
          	dis_ItemEnable SubclassOfLabel 0
          	dis_ItemEnable MemberOfLabel 0
        }

        catch {set extractDefaultFile [dis_getStringPreference "DIS_misc.SubsysSettingFile"]}
	    if {$extractDefaultFile==""} then {
	         set pset_home [dis_getenv PSETHOME]
	         regsub -all \\\\ $pset_home / extractDefaultFile
	         append extractDefaultFile "/lib/subsysPrefs.default"
	    }
	    set extractLastFile $extractDefaultFile
	    set fileBuffer [dis_file_read $extractDefaultFile]
	    set tempList [list "{" $fileBuffer "}"]
	    set joinList [join $tempList ""]
	    set tempFileList [list "{" $extractDefaultFile "}"]
	    set joinFileList [join $tempFileList ""]
	    set tempStr [concat "dis_extractRestore" $joinFileList $joinList $v]
       	reval "DISextract" $tempStr

#        dis_ViewerDisableItem $vr "EXtractItem"


         return OK
    }

    proc CloseDialog { } { }

    proc doExtract {} {
        global gd v vr
        global FunctionCallSlider DataReferenceSlider InstanceOfSlider ArgumentTypeSlider ReturnTypeSlider ContainsSlider 
        global FriendOfSlider SubclassOfSlider MemberOfSlider
        global FileNamePrefixSlider LogicalNameLevelSlider ThresholdSlider
        global NumberOfSubsystems FileNamePrefixCount LogicalNameLevelCount ExistingSubsystems
        global GLOBAL_LastServerRTL LastModelServerRTL
        global BROWSER_ProjectScope


		if { ![info exists LastModelServerRTL] } {
			dis_confirm1 "Invalid Selection" OK "You must select some entities to extract!"
		}

        if {[info exists GLOBAL_LastServerRTL]} {
            reval_async "DISextract" [ concat "dis_doExtraction" $LastModelServerRTL $LastModelServerRTL $NumberOfSubsystems $FunctionCallSlider $DataReferenceSlider $InstanceOfSlider $ArgumentTypeSlider $ReturnTypeSlider $ContainsSlider $FriendOfSlider $SubclassOfSlider $MemberOfSlider $FileNamePrefixSlider $LogicalNameLevelSlider $FileNamePrefixCount $LogicalNameLevelCount $ThresholdSlider $ExistingSubsystems ]
        } else {
            reval_async "DISextract" [ concat "dis_doExtraction" $LastModelServerRTL $LastModelServerRTL $NumberOfSubsystems $FunctionCallSlider $DataReferenceSlider $InstanceOfSlider $ArgumentTypeSlider $ReturnTypeSlider $ContainsSlider $FriendOfSlider $SubclassOfSlider $MemberOfSlider $FileNamePrefixSlider $LogicalNameLevelSlider $FileNamePrefixCount $LogicalNameLevelCount $ThresholdSlider $ExistingSubsystems ]
        }

        return OK
    }

    proc SaveSettings {filename} {
        global gd v vr
        global FunctionCallSlider DataReferenceSlider InstanceOfSlider ArgumentTypeSlider ReturnTypeSlider ContainsSlider 
        global FriendOfSlider SubclassOfSlider MemberOfSlider
        global FileNamePrefixSlider LogicalNameLevelSlider ThresholdSlider
        global NumberOfSubsystems FileNamePrefixCount LogicalNameLevelCount ExistingSubsystems

        reval_async "DISextract" [concat "dis_extractSave" $filename $NumberOfSubsystems $FunctionCallSlider $DataReferenceSlider $InstanceOfSlider $ArgumentTypeSlider $ReturnTypeSlider $ContainsSlider $FriendOfSlider $SubclassOfSlider $MemberOfSlider $FileNamePrefixSlider $LogicalNameLevelSlider $FileNamePrefixCount $LogicalNameLevelCount $ThresholdSlider $ExistingSubsystems $v]
        return OK
    }

    proc loadDialog {numSubs fcall dataref instance argtype rettype element friend subclass member filename logicname threshold file_prefix lognam_cnt exist_subs} {
        global gd
        global FunctionCallSlider DataReferenceSlider InstanceOfSlider ArgumentTypeSlider ReturnTypeSlider ContainsSlider 
        global FriendOfSlider SubclassOfSlider MemberOfSlider
        global FunctionCallOut DataReferenceOut InstanceOfOut ArgumentTypeOut ReturnTypeOut ContainsOut 
        global FriendOfOut SubclassOfOut MemberOfOut
        global FileNamePrefixSlider LogicalNameLevelSlider ThresholdSlider
        global FileNamePrefixOut LogicalNameLevelOut ThresholdOut
        global NumberOfSubsystems FileNamePrefixCount LogicalNameLevelCount ExistingSubsystems

        set NumberOfSubsystems     $numSubs

        set FunctionCallSlider     $fcall        
        set DataReferenceSlider    $dataref
        set InstanceOfSlider       $instance
        set ArgumentTypeSlider     $argtype
        set ReturnTypeSlider       $rettype
        set ContainsSlider         $element

        if {[reval "DISextract" "dis_extractIsCppEnabled"] != 0} {
          set FriendOfSlider         $friend
          set SubclassOfSlider       $subclass
          set MemberOfSlider         $member
        }

        set FileNamePrefixSlider   $filename
        set LogicalNameLevelSlider $logicname
        set ThresholdSlider        $threshold

        set FileNamePrefixCount    $file_prefix
        set LogicalNameLevelCount  $lognam_cnt
        set ExistingSubsystems     $exist_subs

        return OK
    }

    eval [dis_getTextStrRes "DialogLayers:BROWSERcode"]
    OpenDialog
    return OK
}
