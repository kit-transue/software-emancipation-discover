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
#ifndef AUTOSUBSYS_WEIGHTS_H
#define AUTOSUBSYS_WEIGHTS_H

#ifndef _autosubsys_macros_h
#include <autosubsys-macros.h>
#endif

/*
 * A weight consists of a relation and an integer weight that represents
 * how much it should factor in to the binding calculations. The decomposer
 * class gets passed an array of them when it starts up, and all calculations
 * are done using that array. This makes it theoretically possible to work
 * on any relations without recompiling. In practice, since the relations
 * are installed by name in Entity.h.C, that won't work very well.
 */

struct weight {
  bool outgoing;
  int m_weight;
  int count;
};

extern weight defaultweights[NWEIGHTS];
extern int threshold;
extern int num_subsys;

#endif

/*
   START-LOG-------------------------------------------

   $Log: autosubsys-weights.h  $
   Revision 1.5 1996/04/29 13:52:22EDT kws 
   Renaming 'weight' member of weight struct
 * Revision 1.2.1.2  1992/10/09  20:22:22  swu
 * *** empty log message ***
 *


   END-LOG---------------------------------------------

*/
