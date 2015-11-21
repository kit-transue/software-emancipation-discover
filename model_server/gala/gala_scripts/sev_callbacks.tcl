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
#this is the communications abstraction /utility file used for abstracting
# the sev interfaces away from gala so that we could develop the two simultaneously



proc is_ms_windows {} {
	set ret 0
	global tcl_platform
	if { [info exists tcl_platform(platform)]  && $tcl_platform(platform) == "windows" } {
		set ret 1
	}
	return $ret
}



global CLIENT_EVAL

if {[is_ms_windows] ==1 } {
	set CLIENT_EVAL "dis_client_access_eval"
} else {set CLIENT_EVAL "dis_aset_eval_cmd"}





# returns a space separated list of the children
proc lsev_get_children {path} {
	global CLIENT_EVAL
	set cmd "sev_get \"$path\" children"
	set ret [$CLIENT_EVAL $cmd]
	#dis_confirm1 "OK" OK "ret=$ret path =$path"
	return $ret
	#if { [expr [string length $path] %2] == 1&& [string length $path]>2} {return ""}
	#return {bob frank julie joe nara pinar sughosh}
}

proc lsev_get_type {path} {	
	global CLIENT_EVAL
	#set x [$CLIENT_EVAL "sev_get_item \"$path\""]
	#set x [lindex $x 0]
        set x [$CLIENT_EVAL "lindex \[sev_get_item \"$path\"\] 0"]
	return $x
}

#I don't check the return value...
proc lsev_set_weight {path newval} {
	global CLIENT_EVAL
	set cmd "sev_set \"$path\" weight $newval"
	return  [$CLIENT_EVAL $cmd]
}

#returns an integer between 0 and 100
proc lsev_get_weight {path} {
	global CLIENT_EVAL
	set cmd "sev_get \"$path\" weight"
	return [$CLIENT_EVAL $cmd]
}

proc lsev_get_threshold {path} {
	global CLIENT_EVAL
	set ret ""
	catch {
		set cmd "sev_get \"$path\" threshold"
		set ret [$CLIENT_EVAL $cmd]
		}
	return $ret
}

proc lsev_set_threshold {path val} {
	global CLIENT_EVAL
	set ret ""
	catch {
		set cmd "sev_set \"$path\" threshold $val"
		set ret [$CLIENT_EVAL $cmd]
		}
	return $ret
}



#you should have all of the data to run this.
proc lsev_run_queries {} {
	global CLIENT_EVAL
	dis_confirm1 "Running Queries..." OK "Not really doing anything"
	return 0
}

#returns 1 if it is valid, 0 otherwise.
proc lsev_path_valid {path} {
	global CLIENT_EVAL
	return 1
}
#sets the input selection (current project, selected groups, etc.)
#we'll need to specify the format of this.
proc lsev_set_input {input} {
	global CLIENT_EVAL
	return 1
}

#returns an int 0-100
proc  lsev_get_score {path} {
	global CLIENT_EVAL
	#dis_confirm1 asdf OK "path=$path"
	set cmd "sev_get \"$path\" score"
	set x [$CLIENT_EVAL $cmd]
	return $x
}
#same as above.
proc lsev_get_num_inst {path} {
	global CLIENT_EVAL
	set cmd "sev_get \"$path\" hitno"
	set x [$CLIENT_EVAL $cmd]
	return $x
	#return 3
	}
#hook to tell you that the whole thing is starting
proc lsev_startup {} {
	global CLIENT_EVAL
	global env
	#first have to source the proper file
	#set x [array names env]
	#dis_confirm1 "env" OK "env vars are $x"

	set f [dis_getenv "PSETHOME"]
	set f [split $f "\\"]
	#dis_confirm1 "PSETHOME" OK "f=$f in sevc"
	set f [join $f "/"]
	#dis_confirm1 "PSETHOME" OK "f=$f in sevc2"
	set f "$f/lib/sev.dis"
	set cmd "source $f"
	$CLIENT_EVAL $cmd
        global SEV_RESET_STATE
        if { ! ([ info exists SEV_RESET_STATE ] && $SEV_RESET_STATE == "1") } {
	  set cmd "sev_load_mru"
	  $CLIENT_EVAL $cmd
          set SEV_RESET_STATE 0
	}
	#dis_confirm1 "doneinit " OK "Doneinit"
}	

#shutdown hook.
proc lsev_shutdown {} {
	global CLIENT_EVAL
	return 1
}

#requires you to reset the state of the tree
proc lsev_reset {} {
	global CLIENT_EVAL
	set cmd "sev_init"
	$CLIENT_EVAL $cmd
        global SEV_RESET_STATE
        set SEV_RESET_STATE 1
	return 1
}

proc lsev_report {type filename bverbose} {
	global CLIENT_EVAL

 	if { [string compare "$type" "HTML"] == 0} {
	  set new_filename $filename
	  set extn [ file extension $new_filename]
	  if { [regexp -nocase {.html} "$extn"] == 0 } {
	      set new_filename "$new_filename.html"
      }
	  set cmd "sev_create_report \"$type\" \"$new_filename\" $bverbose"
    } else {
      set cmd "sev_create_report \"$type\" \"$filename\" $bverbose"
    }

	return [$CLIENT_EVAL $cmd]
}

#not used yet.
proc lsev_progress {} {
	global CLIENT_EVAL
	return 1
}
proc lsev_get_enabled {path} {
	global CLIENT_EVAL
	set cmd "sev_get \"$path\" on"
	return [$CLIENT_EVAL $cmd]
}
proc lsev_set_enabled {path val} {
	global CLIENT_EVAL
	set cmd "sev_set \"$path\" on $val"
	return [$CLIENT_EVAL $cmd]
}

proc lsev_get_input_token {name} {
	global CLIENT_EVAL
	set cmd "sev_get_input_token \"$name\""
	return [$CLIENT_EVAL $cmd]
}

proc lsev_get_input_types {} {
	global CLIENT_EVAL
	set cmd "sev_get_input_types"
	return [$CLIENT_EVAL $cmd]
}
proc lsev_recalculate {path} {
	global CLIENT_EVAL
	set cmd "sev_calc_score \"$path\""
	return [$CLIENT_EVAL "$cmd"]
}

proc lsev_get_report_types {} {
	global CLIENT_EVAL
	set cmd "sev_get_report_formats"
	set ret [$CLIENT_EVAL "$cmd"]
	return $ret
}


proc lsev_browse_results {path} {
	global CLIENT_EVAL
	set cmd "sev_browse_results \"$path\""
	$CLIENT_EVAL "$cmd"	
}


proc lsev_run_query {path input} {
	global CLIENT_EVAL
	set cmd "sev_run \"$path\" \"$input\""
	#dis_confirm1 Running OK "$cmd"
	catch {$CLIENT_EVAL "$cmd"} x
	#dis_confirm1 asdf OK "$x"
	return "$x"
}


#not used yet.
proc lsev_cancel {} {
	global CLIENT_EVAL
	return 1
}
proc lsev_open_session {sessionname} {
	global CLIENT_EVAL
	set cmd "sev_load \"$sessionname\""
	$CLIENT_EVAL $cmd
}
proc lsev_remove_session {sessionname} {
	global CLIENT_EVAL
	set cmd "sev_delete \"$sessionname\""
	$CLIENT_EVAL $cmd
}

proc lsev_save_session {nm} {
	global CLIENT_EVAL
	set cmd "sev_save \"$nm\""
	$CLIENT_EVAL $cmd
}

proc lsev_list_sessions {} {
	global CLIENT_EVAL
	set x "[$CLIENT_EVAL "sev_list"]"
	return "$x"
}
proc lsev_get_current_session {} {
	global CLIENT_EVAL
	return [$CLIENT_EVAL "sev_current_session"]
}

proc lsev_license {x y} {
	global CLIENT_EVAL
	return [$CLIENT_EVAL "license $x $y"]
}
