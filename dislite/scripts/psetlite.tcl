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
proc copy_pset_from_server { from to } {
    copy_result_to_file "print_file_from_pset \"$from\"" $to
}

set opened_modelflist {}
set opened_localflist {}

proc check_serversource { model_fname } {
    global ModelSrcRoot
    set retval ""
    
    set ModelSrcRoot_poz [string first $ModelSrcRoot $model_fname]
    if {$ModelSrcRoot_poz == 0} {
	set retval [string range $model_fname [expr $ModelSrcRoot_poz +[string length $ModelSrcRoot]] end]
    } else {
	set ModelSrcRoot_poz [string last $ModelSrcRoot $model_fname]
	if {$ModelSrcRoot_poz == -1} {
	    tk_messageBox -title "Error Message" -message "ServerSrcRoot preference was set to\n\"$ModelSrcRoot\"\nin Service Chooser.\nServer has sent file name\n\"$model_fname\".\nThere are no common parts in preference and file name.\nDISCOVER can't transform them.\nPlease set the appropriate ServerSrcRoot\npreference in Service Chooser." -type ok -icon error
	} else {
	    set server_src [string range $model_fname 0 [expr $ModelSrcRoot_poz +[string length $ModelSrcRoot] -1]] 
	    tk_messageBox -title "Error Message" -message "ServerSrcRoot preference was set to\n\"$ModelSrcRoot\"\nin Service Chooser.\nServer has sent file name\n\"$model_fname\".\nPerhaps ServerSrcRoot preference should be set to\n\"$server_src\"\nin Service Chooser." -type ok -icon error
	}
    }
    return $retval
}

proc get_privatefname { model_fname } {
    global SharedSrcRoot PrivateSrcRoot Transform Tempdirectory
    switch -exact $Transform {
	0 {
	    set sub_fname [check_serversource $model_fname]
	    if {[string length $PrivateSrcRoot]>0 } {
		set local_fname $PrivateSrcRoot$sub_fname 
	    } else {
		set local_fname $SharedSrcRoot$sub_fname
	    }
	}
	1 {
	    set local_fname $model_fname 
	}
	2 {
	    regsub -all {/|\\|:| |	} $model_fname _ fname
	    set local_fname $Tempdirectory/$fname
	}
    }
    return $local_fname
}

proc get_sharedfname { model_fname } {
    global SharedSrcRoot PrivateSrcRoot Transform Tempdirectory
    switch -exact $Transform {
	0 {
	    set sub_fname [check_serversource $model_fname]
	    set local_fname $SharedSrcRoot$sub_fname
	  }
	1 {
	    set local_fname $model_fname 
	}
	2 {
	    regsub -all {/|\\|:| |	} $model_fname _ fname
	    set local_fname $Tempdirectory/$fname
	}
    }
    return $local_fname
}

proc get_localfname { model_fname } {
    global SharedSrcRoot PrivateSrcRoot Transform Tempdirectory
    switch -exact $Transform {
	0 {
	    set sub_fname [check_serversource $model_fname]
	    set local_fname $SharedSrcRoot$sub_fname
	    if {[string length $PrivateSrcRoot]>0 } {
		set fname $PrivateSrcRoot$sub_fname 
		if [file exists $fname] {
		    set local_fname $fname
		}
	    }
	}
	1 {
	    set local_fname $model_fname 
	}
	2 {
	    regsub -all {/|\\|:| |	} $model_fname _ fname
	    set local_fname $Tempdirectory/$fname
	}
    }
    return $local_fname
}

proc get_modelfname { local_fname } {
    global ModelSrcRoot SharedSrcRoot PrivateSrcRoot Transform
    if ![isunix] {
	    set msr [string toupper $ModelSrcRoot]
		set ssr [string toupper $SharedSrcRoot]
		set psr [string toupper $PrivateSrcRoot]
		set local_fname [string toupper $local_fname]
	} else {
	    set msr $ModelSrcRoot
		set ssr $SharedSrcRoot
		set psr $PrivateSrcRoot
	}


    if !$Transform {
	    if {[string length $psr]>0 } {
	        if {[string first $psr $local_fname]==0} {
		        set sub_fname [string range $local_fname [string length $psr] end]
	        } else {
	            return ""
		    }
	    } elseif {[string first $ssr $local_fname]==0} {
	        set sub_fname [string range $local_fname [string length $ssr] end]
	    } else {
	        return ""
	    }
	    set model_fname $msr$sub_fname
    } else {
	    set model_fname $local_fname
    }
    return $model_fname
}

proc get_index {fname flag} {
    global Tempdirectory Psethome Transform
    global model_localcopy modified opened_modelflist opened_localflist
    if {$flag==0} {
	set model_fname $fname
	set n [lsearch $opened_modelflist $fname]
    } else { 
	set local_fname $fname
	set n [lsearch $opened_localflist $fname]
    }
    if {$n<0} {
	set n [llength $opened_modelflist]
	if {$flag==0} {
	    set local_fname [get_localfname $model_fname]
	} else {
	    if {$Transform==2} {
		#write "info [error_msg NOT_FROM_SERVER]"
		set message "info [error_msg NOT_FROM_SERVER]"
		editorMessage INFO $message
		return -1
	    }
	    set model_fname [get_modelfname $local_fname]
	}

	if {$Transform<2} {
	    set file_exist 1
	    if ![file exists $local_fname] {
		set file_exist 0
	    } else {
		if {![isunix] && [string index $local_fname 1] != ":"} {
		    set file_exist 0
		}
	    }
	    if !$file_exist {
		set msg "Cannot open $local_fname.\nProbably, you have to choose Server Source\nor Transform option in Service Chooser.\nCheck ServerSrcRoot, SharedSrcRoot and PrivateSrcRoot preferences\nif you have chosen Transform option."
		tk_messageBox -title "Error Message" -message $msg -type ok -icon error
		return -1
	    }
	    set model_localcopy($n) $Tempdirectory/modelsrc.$n
	    copy_pset_from_server $model_fname $model_localcopy($n)

	    if [isunix] {
			set modified($n) [catch {exec $Psethome/bin/cmp -s $model_localcopy($n) $local_fname}]
	    } else {
			set modified($n) [catch {exec $Psethome/bin/diff $model_localcopy($n) $local_fname}]
	    }
	    if $modified($n) {
			exec $Psethome/bin/mrg_diff  $model_localcopy($n) $local_fname $model_localcopy($n).diff1
			exec $Psethome/bin/mrg_diff  $local_fname $model_localcopy($n) $model_localcopy($n).diff2
	    }
	} else {
	    copy_pset_from_server $model_fname $local_fname
	    if [isunix] {
		file attributes $local_fname -permissions 0444
	    } else {
		file attributes $local_fname -readonly 1
	    }
	}
	lappend opened_modelflist $model_fname
	lappend opened_localflist $local_fname
    }
    return $n  
}

proc get_model_position {local_fname n offset line col } {
    global model_localcopy modified Psethome Transform
    if [isunix] {
	if {$Transform<2 && $modified($n)} {
	    set pos [exec $Psethome/bin/mrg_update -mapP_to_P [expr $offset-1] $model_localcopy($n).diff2]
	    if {$pos==-1} {
		set errorMessage [error_msg SYM_NOT_EXIST]
		editorMessage INFO $errorMessage
		return ""
	    }
	} else {
            if {$Transform == 2} {
                 set server_type [dis_command puts "\$tcl_platform(platform)"]
		if {$server_type == "windows"} {
                    set pos [expr $offset+$line-1]
                } else {
                   set pos [expr $offset-1]
		}
            } else {
	        set pos [expr $offset-1]
            }
	}
    } else {
	if {$Transform<2 && $modified($n)} {
	    set model_line [exec $Psethome/bin/mrg_update -mapL_to_L $line $local_fname $model_localcopy($n).diff2 $model_localcopy($n)]
	    if {$model_line==-1} {
		set errorMessage [error_msg SYM_NOT_EXIST]
		editorMessage INFO $errorMessage
		return ""
	    }
	} else {
	    set model_line $line
	}
	set pos $model_line,$col
    }
    return $pos
}

proc get_local_line {local_fname line n} {
    global model_localcopy modified Psethome Transform
    if {$line>0 && $Transform<2 && $modified($n)} {
	set local_line [exec $Psethome/bin/mrg_update -mapL_to_L $line $model_localcopy($n) $model_localcopy($n).diff1 $local_fname]
	if {$local_line==-1} {set local_line 0}
    } else {
	set local_line $line
    }
    return $local_line
}

proc update_filecache {local_fname n} {
    global Psethome model_localcopy modified
    exec $Psethome/bin/mrg_diff $model_localcopy($n) $local_fname $model_localcopy($n).diff1
    exec $Psethome/bin/mrg_diff $local_fname $model_localcopy($n) $model_localcopy($n).diff2
    set modified($n) 1
}
