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
#include <gtForm.h>
#include <gtPrim.h>

gtForm::gtForm() {}
gtForm::~gtForm() {}

void gtForm::vertStack(
	gtPrimitive* p0, gtPrimitive* p1, gtPrimitive* p2,
	gtPrimitive* p3, gtPrimitive* p4, gtPrimitive* p5,
	gtPrimitive* p6, gtPrimitive* p7)
{
    gtPrimitive* prims[8];
    prims[0] = p0;
    prims[1] = p1;
    prims[2] = p2;
    prims[3] = p3;
    prims[4] = p4;
    prims[5] = p5;
    prims[6] = p6;
    prims[7] = p7;

    gtPrimitive* prev = 0;

    for(int i = 0; i < 8; ++i)
    {
	if(prims[i])
	{
	    prims[i]->attach(gtTop, prev);
	    prims[i]->attach(gtLeft);
	    prims[i]->attach(gtRight);
	    prev = prims[i];
	}
    }
}

void gtForm::horzStack(
	gtPrimitive* p0, gtPrimitive* p1, gtPrimitive* p2,
	gtPrimitive* p3, gtPrimitive* p4, gtPrimitive* p5,
	gtPrimitive* p6, gtPrimitive* p7)
{
    gtPrimitive* prims[8];
    prims[0] = p0;
    prims[1] = p1;
    prims[2] = p2;
    prims[3] = p3;
    prims[4] = p4;
    prims[5] = p5;
    prims[6] = p6;
    prims[7] = p7;

    gtPrimitive* prev = 0;

    for(int i = 0; i < 8; ++i)
    {
	if(prims[i])
	{
	    prims[i]->attach(gtLeft, prev);
	    prims[i]->attach(gtTop);
	    prims[i]->attach(gtBottom);
	    prev = prims[i];
	}
    }
}

/*
   START-LOG-------------------------------------------

   $Log: gtForm.C  $
   Revision 1.1 1993/07/28 19:48:05EDT builder 
   made from unix file
Revision 1.1  1992/10/10  21:53:12  builder
Initial revision



   END-LOG---------------------------------------------

*/
