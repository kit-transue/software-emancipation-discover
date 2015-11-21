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
proc cm_get_src { cm_sysfile cm_workfile comment } {
    global Tempdirectory
    if [ file exists $cm_workfile ] {
	set answer [tk_messageBox -title "CM question" -message "File $cm_workfile already exists.\nReplace existing file?"  -type okcancel -icon question]
	if { $answer != "ok"} {return 77}
	file delete $cm_workfile
    }
    set cm_description $Tempdirectory/cm_description
    catch { file delete $cm_description }
    set ff [open  $cm_description w]
    puts $ff $comment
    close $ff
    set cm_revision_file [cm_get_revision_file $cm_sysfile]
    set res [cm_exec_command co -l $cm_workfile $cm_revision_file "<" $cm_description]
    return $res
}

proc cm_unget_src { cm_sysfile cm_workfile } {
    set err [cm_exec_command rcs -I -u $cm_sysfile]
    file delete $cm_workfile
    return $err
}

proc cm_put_src { cm_sysfile cm_workfile comment } {
    global Tempdirectory env
    if ![info exists env(RCSHOME)] {
	set env(RCSHOME) $env(PSETHOME)/bin
    }
    set cm_description $Tempdirectory/cm_description
    catch { file delete $cm_description }
    set ff [open  $cm_description w]
    puts $ff $comment
    close $ff
    
    set cm_revision_file [cm_get_revision_file $cm_sysfile]
    set res [cm_exec_command ci  $cm_workfile  $cm_revision_file "<" $cm_description]
#    if { !$res } { 
#	cm_exec_command co -f $cm_sysfile 
#    }
    return $res
}

proc cm_local_list {} {
    global PrivateSrcRoot
    if { [string length $PrivateSrcRoot]>0 } {
	set filelist [cm_find $PrivateSrcRoot]
	cm_list_command cm_file_filter $filelist
    }
}  

proc cm_find { cm_root } {
    global cm_find_list
    set cm_find_list {}
    cm_find_recursively $cm_root
    return $cm_find_list
}

proc cm_find_recursively { cm_root } {
    global cm_find_list
    set list [glob -nocomplain $cm_root/*]
    foreach ent $list {
	if { [file isfile $ent] } {
	    lappend cm_find_list $ent
	} elseif { [file isdirectory $ent] } {
	    cm_find_recursively $ent
	}
    }
}

proc cm_get_revision_file { fname } {
    if ![cm_exec_command rlog -R $fname] {
	global cm_exec_msg
	return $cm_exec_msg
    } else {
	return ""
    }
}

proc cm_get_view {} {
    return ""
}





