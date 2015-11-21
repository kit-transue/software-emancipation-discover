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
#ifndef _xrefMapper_h
#define _xrefMapper_h

// xrefMapper.h
//------------------------------------------
// synopsis:
// 
// The xrefMapper class provides a way of associating an arbitrary object with
// each xrefSymbol; just as each xrefSymbol has an associated definition ddElement
// xrefMapper can provide the same capability for other objects (e.g., ParaANALYZER
// Entities).  In order to provide for typesafe use, all the xrefMap member functions
// are protected; the typical use will be to derive an application-specific class
// for the objects being associated and provide the casts in that class.  E.g.
//
//	class foo { ... };
//
//	class foo_xrefMapper: public xrefMapper {
//	public:
//	   void add_item(foo* fp, const symbolPtr& sym) {
//	      xrefMapper::add_item(fp, sym);
//	   }
//	   foo* item(const symbolPtr& sym) {
//	      return (foo*) xrefMapper::item(sym);
//	   }
//	};
//
// (Too bad we don\'t have templates yet, eh?)
//------------------------------------------

#ifndef _symbolPtr_h_
#include <symbolPtr.h>
#endif

class xrefMapper_rep;	// for internal use

class xrefMapper {
protected:
   xrefMapper();
   ~xrefMapper();

   void add_item(void* item, const symbolPtr&);
   void* item(const symbolPtr&) const;

private:
   xrefMapper_rep* rep_head;
};


/*
   START-LOG-------------------------------------------

   $Log: xrefMapper.h  $
   Revision 1.2 1995/07/27 20:18:07EDT rajan 
   Port
 * Revision 1.1  1993/05/10  11:25:10  wmm
 * Initial revision
 *
   END-LOG---------------------------------------------
*/

#endif // _xrefMapper_h
