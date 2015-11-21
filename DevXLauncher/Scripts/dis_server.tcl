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

set CurSet [new_set]
change_prompt {}

set selections [new_set]
set group(Clipboard) [new_set]
set Tempdir [ getvar disTempdir]

source_dis dislite.dis
source_dis list_ent.dis

set langs [languages]

if { [lsearch $langs CPP] >= 0 } {
    source_dis inheritance.dis
}

proc prepare_names {args} {
    set obj [set_copy $args]
    foreach el $obj {
	puts [list [pname $el] [fname $el] [lname $el]]
    }
}

proc browser_query { ind cmd } {
    if [catch {uplevel #0 $cmd} result] {
	    set result
    }
    print $result
}
proc dis_query { cmd {attr {}} } {
    if [catch {uplevel #0 $cmd} result] {
	return $result
    }
    if [llength $attr] {
	print_attribute $attr $result
    } else {
	print $result
    }
}

proc dis_print { set {attr {}} } {
    set result [uplevel #0 "set_copy -i $set"]

    if [llength $attr] {
	print_attribute $attr $result
    } else {
	print $result
    }
}

proc dis_modules {} {
    global all_modules
    set all_modules [modules /]
    print_attribute fname $all_modules
}

# proj/scope return -1, otherwise return number of definitions 
proc dis_check_defs { obj kind } {
    global cur_def
    switch $kind {
	scope -
	proj   {
	    set cur_def [contents $obj]
	    return -1
	}
	module -
	instance  {
	    set cur_def $obj
	    return 1 
	}
	string { 
	    set cur_def [instances $obj]
	    return [size $cur_def]
	}
	local { 
	    set cur_def [get_declarations $obj]
	    return [size $cur_def]
	}
	macro - 
	typedef - 
	struct {
	    set cur_def [def_symbols $obj]
	    set no_defs [size  $cur_def]
	    if {$no_defs > 1} {
		set cur_def [filter included $cur_def]
		set no_defs [size $cur_def]
	    }
	    return $no_defs
	}
	default { 
	    set cur_def [def_symbols $obj]
	    return [size $cur_def]
	}
    }
}

proc describe_symbol { sym } {
    set kind [kind $sym]
    set name [name $sym]

    switch $kind {
	proj    {return [list $kind $name]}
	scope   {return [list $kind $name]}
	module  {return [list $kind $name]}
    }
    set no_defs [dis_check_defs $sym $kind]
    global cur_def
    
    if {$no_defs == 1} {
	set file [lname $cur_def]
    } elseif {$no_defs > 1} {
	set file [list {} $no_defs]
    } elseif [generated $sym] {
	set file generated
    } else {
	set file [rscope $sym]
    }    

    switch $kind {
	funct {
	    if [cfunction $sym] {
		append name [args $sym]
		set kind cfun
	    } 
	    if [member $sym] {
		set kind method
		lappend kind [ppp $sym]

		if [abstract $sym] {
		    lappend kind abstract
		} elseif [virtual $sym] {
		    lappend kind virtual
		}
	    }
	    if [inline $sym] {
		lappend kind inline
	    }
	}

	var { 
	    if [member $sym] {
		set kind field
		lappend kind [ppp $sym]
	    }
	}
	field {
	    lappend kind [ppp $sym]
	}
	macro {
	    if {$no_defs == 1} {
		set def [uses -version $cur_def]
		if {[size $def]==1} {
		    if [const $cur_def] {
			lappend name [name $def]
		    } else {
			append name [name $def]
		    }
		}
	    }
	    return [list $kind $name $file]
	}
	default {return [list $kind $name $file]}
    }
    if [static $sym] {lappend kind static}
    lappend kind [type $sym]
#    lappend name [attributes $sym]
    list $kind $name $file
}
