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
# popnode is viewClassNode * here
# Because this procedure is alway called with NULL popnode, it doesn't make any diff
#

eval {
    proc InitList {} {
	global tagSMSortPriority priority_list sel
		set tagSMSortPriority { 
			{llength $priority_list} 
			{lindex $priority_list $Item} 
			{
				if { [llength $Selection] == 0 } {
					dis_ItemEnable Up 0
					dis_ItemEnable Down 0
				} else {
					set sel [lindex $Selection 0]
					if { $sel > 0 } {
						dis_ItemEnable Up 1 
					} else {
						dis_ItemEnable Up 0
					}
					set len [llength $priority_list]
					incr len -1
					if { $sel < $len } {
						dis_ItemEnable  Down 1 
					} else {
						dis_ItemEnable Down 0
					}
				}			
			}
			{ }
		}

    }

    proc GetResult {} {
	global tagSMSortType
	set ret 1
	switch -exact -- $tagSMSortType {
		EntityType {  set ret 0 }
		ASCIIOrder {  set ret 1 }
		DeclOrder {  set ret 2 }
	}
	return $ret
    }


    proc OpenDialog {args} {
	
	global tagSMSortType tagSMSortPriority
	global priority_list sel
	global entity_type_list
	global popnode
	global order_list
	
	global OK 
	global  gd
	set OK 0
	
	set popnode 0
	catch { set popnode [lindex $args 0] }

	set priority_list {}
	set sel 0
	dis_ItemEnable Up 0
	dis_ItemEnable Down 0

	set tagSMSortType ASCIIOrder		
	set order_list {0 1 2}
	
	set entity_type_list {
	    {Functions} {Types} {Variables}
	} 
	

	InitList
	
	dis_gdInitExclusiveGroup $gd  tagSMSortType {
	    global priority_list order_list entity_type_list tagSMSortType
	    switch -exact -- $tagSMSortType {
		EntityType {  set priority_list $entity_type_list }
		ASCIIOrder {  set priority_list {} }
		DeclOrder {  set priority_list {} }
	    }
	    InitList
	    dis_ItemEnable Up 0
	    dis_ItemEnable Down 0
	    set order_list {0 1 2}
	}

	dis_gdInitButton $gd  OK {
	    set ret [GetResult]
	    leval DISview "dis_subsys_sort_members  \"$popnode\" \"$ret\" \"$order_list\""
	    dis_close_dialog $gd	
	}
	
	dis_gdInitButton $gd  Apply {
	    set ret [GetResult]
	    leval DISview "dis_subsys_sort_members  \"$popnode\" \"$ret\" \"$order_list\""

	}

	dis_gdInitButton $gd  Reset {
	    leval DISview "dis_sort_members_reset  \"$popnode\""
	}
	
	dis_gdInitButton $gd  Cancel {
	    dis_close_dialog $gd
	}

	dis_gdInitButton $gd  Up {
	    set new_index $sel
	    incr new_index -1
	    set order_list [linsert $order_list $new_index [lindex $order_list $sel]]
	    set priority_list [linsert $priority_list $new_index [lindex $priority_list $sel]]
	    set old_index $sel
	    incr old_index 
	    set order_list [lreplace $order_list $old_index $old_index]
	    set priority_list [lreplace $priority_list $old_index $old_index]
	    InitList
	    dis_gdListItemSelect tagSMSortPriority $new_index
	}

	dis_gdInitButton $gd  Down {
	    set new_index $sel
	    incr new_index 2
	    set order_list [linsert $order_list $new_index [lindex $order_list $sel]]
	    set priority_list [linsert $priority_list $new_index [lindex $priority_list $sel]]
	    set order_list [lreplace $order_list $sel $sel]
	    set priority_list [lreplace $priority_list $sel $sel]
	    InitList
	    incr new_index -1
	    dis_gdListItemSelect tagSMSortPriority $new_index
	    
	}

	return  OK

    }

    set status [ OpenDialog $Args ]
    return $status
}
