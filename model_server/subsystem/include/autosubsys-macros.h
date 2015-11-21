/*************************************************************************
* Copyright (c) 2015, Synopsys, Inc.                                     *
* All rights reserved.                                                   *
*                                                                        *
* Redistribution and use in source and binary forms, with or without     *
* modification, are permitted provided that the following conditions are *
* met:                                                                   *
*                                                                        *
* 1. Redistributions of source code must retain the above copyright      *
* notice, this list of conditions and the following disclaimer.          *
*                                                                        *
* 2. Redistributions in binary form must reproduce the above copyright   *
* notice, this list of conditions and the following disclaimer in the    *
* documentation and/or other materials provided with the distribution.   *
*                                                                        *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    *
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      *
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  *
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   *
* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  *
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  *
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  *
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   *
*************************************************************************/
#ifndef _autosubsys_macros_h
#define _autosubsys_macros_h

/*
 * These represent which weights there are. These ought to be enumerations,
 * but this file was originally referenced from a UIM/X interface, so it
 * had to be very vanilla C. At this point there's no reason not to change
 * it, except inertia.
 */

#define NWEIGHTS	22

#define AUS_FCALL		0
#define AUS_FCALL_INV		(AUS_FCALL+NWEIGHTS/2)
#define AUS_DATAREF		1
#define AUS_DATAREF_INV		(AUS_DATAREF+NWEIGHTS/2)
#define AUS_INSTANCE		2
#define AUS_INSTANCE_INV	(AUS_INSTANCE+NWEIGHTS/2)
#define AUS_ARGTYPE		3
#define AUS_ARGTYPE_INV		(AUS_ARGTYPE+NWEIGHTS/2)
#define AUS_RETTYPE		4
#define AUS_RETTYPE_INV		(AUS_RETTYPE+NWEIGHTS/2)
#define AUS_ELEMENT     	5
#define AUS_ELEMENT_INV		(AUS_ELEMENT+NWEIGHTS/2)
#define AUS_FRIEND		6
#define AUS_FRIEND_INV		(AUS_FRIEND+NWEIGHTS/2)
#define AUS_SUBCLASS		7
#define AUS_SUBCLASS_INV	(AUS_SUBCLASS+NWEIGHTS/2)
#define AUS_MEMBER      	8
#define AUS_MEMBER_INV		(AUS_MEMBER+NWEIGHTS/2)
#define AUS_FILENAME    9
#define AUS_LOGICNAME  10

#define AUS_THRESHOLD   11
#define AUS_FILE_PREFIX 12
#define AUS_LOGNAM_CNT  13
#define AUS_NUM_SUBS    14
#define AUS_EXIST_SUBS  15

// if any weight is added or deleted, WEIGHT_COUNT needs to be modified.
#define WEIGHT_COUNT 16

/*
 * These are the possible methods of dealing with existing subsystems.
 * Ignore existing does exactly that; we pretend they don't exist, and
 * feel free to incorporate their contents into other things.
 * Leave alone means we don't add their contents to any of our own 
 * subsystem, and append means we use the existing subsystems as nuclei
 * to put other things into.
 *
 * References to a "disjoint flag" refer to this state; before 
 * APPEND_TO_EXISTING was implemented, there was simply a "disjoint" 
 * setting which built subsystems that were either guaranteed to be
 * disjoint sets or were not.
 */

#define AUSM_IGNORE_EXISTING 0
#define AUSM_LEAVE_EXISTING_ALONE 1
#define AUSM_APPEND_TO_EXISTING 2

/*
   START-LOG-------------------------------------------

   $Log: autosubsys-macros.h  $
   Revision 1.4 1995/09/23 15:49:09EDT so 
   Bug track: 10677
   Subsystem Extraction and Domant Code analysis must be batch
 * Revision 1.2.1.2  1992/10/09  20:22:20  swu
 * *** empty log message ***
 *


   END-LOG---------------------------------------------

*/

#endif
