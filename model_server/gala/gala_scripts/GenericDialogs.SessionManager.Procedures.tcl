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
	global gd OK
	set OK 0
	
	set fn "[dis_getenv PSETHOME]/lib/gala_scripts/sev_callbacks.tcl"
	set fn [join [split "$fn" "\\"] /]
	source "$fn"
	global CLIENT_EVAL
	global SESSIONLIST
	global SESSIONLISTLENGTH
	
	set SESSIONLIST [lsev_list_sessions]
	#dis_confirm1 asdf ok "Sessionlist =$SESSIONLIST"
	if {$SESSIONLIST == ""} {dis_confirm1 "No Sessions" OK "You haven't saved any sessions!"}
	set SESSIONLISTLENGTH [llength $SESSIONLIST]
	
	proc OpenDialog {} {
		InitWidgets
		InitState
	}

	proc ReopenDialog {} {
	}

	proc CloseDialog {} {
	}


	proc InitWidgets {} {
		global gd
		global ListSessions BtnOpen BtnRemove BtnCancel
		dis_gdInitButton $gd BtnOpen {OpenCallback}
		dis_gdInitButton $gd BtnRemove {RemoveCallback}
		dis_gdInitButton $gd BtnCancel {CancelCallback}
		
		dis_gdInitListItem $gd ListSessions {}
	}

	proc InitState {} {
		global ListSessions Item Selection		
		set ListSessions {
		{global SESSIONLISTLENGTH; return $SESSIONLISTLENGTH}
		{global SESSIONLIST; return [lindex $SESSIONLIST $Item]}
		{EnableButtons 1}
		{}
		{return "Session"}
		}
		
		global BtnOpen BtnRemove
		dis_ItemEnable BtnOpen 0
		dis_ItemEnable BtnRemove 0
		# we unfortunately need this global variable to communicate to the layer that it must redraw (YUCK!)
		dis_setvar -global SEVMustRedraw 0
	}

	proc CancelCallback {} {
		global gd
		dis_close_dialog $gd
	}
	
	#remove the item then rebuild the list.
	proc RemoveCallback {} {
		global Selection SESSIONLIST SESSIONLISTLENGTH
		set curSession [lindex  $SESSIONLIST $Selection] 
		set oldSelection $Selection
		lsev_remove_session "$curSession"
		RebuildList
		if { $SESSIONLISTLENGTH > 0 && $oldSelection != 0 } {
			set Selection [expr $oldSelection -1]
		dis_setvar -global SEVMustRedraw 1
		} 
	}
		
	proc OpenCallback {} {
	global Selection SESSIONLIST
		set curSession [lindex $SESSIONLIST $Selection]
		lsev_open_session $curSession
		global gd
		
		dis_close_dialog $gd
		dis_setvar -global SEVMustRedraw 1
	}
	proc RebuildList {} {
		global SESSIONLIST SESSIONLISTLENGTH ListSessions
		set SESSIONLIST [lsev_list_sessions]
		set SESSIONLISTLENGTH [llength $SESSIONLIST]
		set ListSessions {
		{global SESSIONLISTLENGTH; return $SESSIONLISTLENGTH}
		{global SESSIONLIST; return [lindex $SESSIONLIST $Item]}
		{}
		{}
		{return "Session"}
		}
		if {$SESSIONLISTLENGTH == 0 } {
			dis_confirm1 "No More Sessions" OK "You have no more sessions to manage!"
			EnableButtons 0
			}
		}

	proc EnableButtons {x} {
		global BtnOpen BtnRemove
		dis_ItemEnable BtnOpen $x
		dis_ItemEnable BtnRemove $x
	}
OpenDialog
return $OK
}
