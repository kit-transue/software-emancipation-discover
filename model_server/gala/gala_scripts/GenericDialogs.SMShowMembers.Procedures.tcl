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
    proc InitializeDialog { popnode } {
	global SMFunctions SMVariables SMClasses SMUnions SMEnums SMTypedefs SMMacros SMFiles

	set SMFunctions [leval DISview "dis_view_SMGetShowValue \"$popnode\" Functions"]
	set SMVariables [leval DISview "dis_view_SMGetShowValue \"$popnode\" Variables"]
	set SMClasses  [leval DISview "dis_view_SMGetShowValue \"$popnode\" Classes"]
	set SMUnions [leval DISview "dis_view_SMGetShowValue \"$popnode\" Unions"]
	set SMEnums [leval DISview "dis_view_SMGetShowValue \"$popnode\" Enums"]
	set SMTypedefs [leval DISview "dis_view_SMGetShowValue \"$popnode\" Typedefs"]
	set SMMacros [leval DISview "dis_view_SMGetShowValue \"$popnode\" Macros"]
	set SMFiles [leval DISview "dis_view_SMGetShowValue \"$popnode\" Files"]
    }

    proc OpenDialog { args } {
	global gd
	global SMOK SMApply SMCancel
	global SMFunctions SMVariables SMClasses SMUnions SMEnums SMTypedefs SMMacros SMFiles
	global popnode
	
	set popnode 0
	catch { set popnode [lindex $args 0] }
	InitializeDialog $popnode

	dis_gdInitButton $gd SMOK {
	    leval DISview "dis_view_SMSetShowValues \"$popnode\" $SMFunctions $SMVariables $SMClasses $SMUnions $SMEnums $SMTypedefs $SMMacros $SMFiles"
	    dis_close_dialog
	}

	dis_gdInitButton $gd SMApply {
	    leval DISview "dis_view_SMSetShowValues \"$popnode\" $SMFunctions $SMVariables $SMClasses $SMUnions $SMEnums $SMTypedefs $SMMacros $SMFiles"
	}

	dis_gdInitButton $gd SMCancel {
	    dis_close_dialog
	}

	return  OK
    }

    set status [ OpenDialog $Args ]
    return $status
}
