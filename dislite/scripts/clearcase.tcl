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
set driver "C"
if {![isunix] && !$Transform} {
    if [string length $SharedSrcRoot] {
	set driver [string index $SharedSrcRoot 0]
    }
}
   

proc cm_get_src { cm_sysfile cm_workfile comment } {
    cm_exec_command cleartool checkout -c $comment -unreserved $cm_workfile
}

proc cm_unget_src { cm_sysfile cm_workfile } {
    cm_exec_command cleartool uncheckout -rm $cm_workfile
}

proc cm_put_src { cm_sysfile cm_workfile comment } {
    cm_exec_command cleartool ci -c $comment $cm_workfile
}

proc cm_local_list {} {
    global driver Transform
    if {![isunix] && !$Transform} {
	cd ${driver}:/
    }
    set filelist [cm_pipe_list cleartool lspriv -co -short]
    cm_list_command cm_file_filter $filelist
}  

proc cm_pipe_list { args } {
    global Tempdirectory
    set cm_err $Tempdirectory/cm_err.out
    set pp [open "| $args 2>$cm_err "] 
    set linelist {}
	while { ! [eof $pp] } {
	    set line [gets $pp]
		if ![isunix] {
		   set line [string toupper $line]
		}
	    lappend linelist $line
	}
	set err [catch { close $pp }]
	if { $err } {
		set f [open $cm_err]
		set msg [read $f]
		close $f
		tk_messageBox -title "CM error" -message "$msg\nPlease,check the \$SharedSrcRoot and \$PrivateSrcRoot in the Service Chooser." -type ok -icon error		
		return -1
	}
    return $linelist
}

proc cm_get_view {} {
    set err [cm_exec_command cleartool pwv -short -set]
    if {!$err} {
	global cm_exec_msg
	return $cm_exec_msg
    }
    return ""
}


