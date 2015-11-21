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
#include "xmlstrs.h"
#include "minidom.h"

using namespace MiniXML;

const XMLCh argXMLStr[] = 
{
	chLatin_a, chLatin_r, chLatin_g, chNull
};
const XMLCh buildXMLStr[] = 
{
	chLatin_b, chLatin_u, chLatin_i, chLatin_l, chLatin_d, chNull
};
const XMLCh cancelXMLStr[] = 
{
	chLatin_c, chLatin_a, chLatin_n, chLatin_c, chLatin_e, chLatin_l, chNull
};
const XMLCh clearmakeXMLStr[] = 
{
	chLatin_c, chLatin_l, chLatin_e, chLatin_a, chLatin_r, chLatin_m, chLatin_a, chLatin_k, chLatin_e, chNull
};
const XMLCh codeRoverStartedXMLStr[] =
{
	chLatin_c, chLatin_o, chLatin_d, chLatin_e, chUnderscore, chLatin_r, chLatin_o, chLatin_v, chLatin_e, chLatin_r, chUnderscore, chLatin_s, chLatin_t, chLatin_a, chLatin_r, chLatin_t, chLatin_e, chLatin_d, chNull
};
const XMLCh codeRoverFailedXMLStr[] =
{
	chLatin_c, chLatin_o, chLatin_d, chLatin_e, chUnderscore, chLatin_r, chLatin_o, chLatin_v, chLatin_e, chLatin_r, chUnderscore, chLatin_f, chLatin_a, chLatin_i, chLatin_l, chLatin_e, chLatin_d, chNull
};
const XMLCh commandXMLStr[] =
{
	chLatin_c, chLatin_o, chLatin_m, chLatin_m, chLatin_a, chLatin_n, chLatin_d, chNull
};
const XMLCh doneXMLStr[] = 
{
	MiniXML::chLatin_d, MiniXML::chLatin_o, MiniXML::chLatin_n, MiniXML::chLatin_e, MiniXML::chNull
};
const XMLCh dspXMLStr[] = 
{
	chLatin_d, chLatin_s, chLatin_p, chNull
};
const XMLCh errorXMLStr[] = 
{
	chLatin_e, chLatin_r, chLatin_r, chLatin_o, chLatin_r, chNull
};
const XMLCh failedXMLStr[] =
{
	chLatin_f, chLatin_a, chLatin_i, chLatin_l, chLatin_e, chLatin_d, chNull
};
const XMLCh helpXMLStr[] = 
{
	MiniXML::chLatin_h, MiniXML::chLatin_e, MiniXML::chLatin_l, MiniXML::chLatin_p, MiniXML::chNull
};
const XMLCh invokeModelBuildXMLStr[] =
{
	chLatin_i, chLatin_n, chLatin_v, chLatin_o, chLatin_k, chLatin_e, chSpace, chLatin_m, chLatin_o, chLatin_d, chLatin_e, chLatin_l, chSpace, chLatin_b, chLatin_u, chLatin_i, chLatin_l, chLatin_d, chNull
};
const XMLCh messageXMLStr[] = 
{
	chLatin_m, chLatin_e, chLatin_s, chLatin_s, chLatin_a, chLatin_g, chLatin_e, chNull
};
const XMLCh msgXMLStr[] = 
{
	chLatin_m, chLatin_s, chLatin_g, chNull
};
const XMLCh nodeXMLStr[] = 
{
	MiniXML::chLatin_n, MiniXML::chLatin_o, MiniXML::chLatin_d, MiniXML::chLatin_e, MiniXML::chNull
};
const XMLCh normalXMLStr[] = 
{
	MiniXML::chLatin_n, MiniXML::chLatin_o, MiniXML::chLatin_r, MiniXML::chLatin_m, MiniXML::chLatin_a, MiniXML::chLatin_l, MiniXML::chNull
};
const XMLCh outputXMLStr[] = 
{
	chLatin_o, chLatin_u, chLatin_t, chLatin_p, chLatin_u, chLatin_t, chNull
};
const XMLCh parentXMLStr[] = 
{
	MiniXML::chLatin_p, MiniXML::chLatin_a, MiniXML::chLatin_r, MiniXML::chLatin_e, MiniXML::chLatin_n, MiniXML::chLatin_t, MiniXML::chNull
};
const XMLCh progressXMLStr[] = 
{
	chLatin_p, chLatin_r, chLatin_o, chLatin_g, chLatin_r, chLatin_e, chLatin_s, chLatin_s, chNull
};
const XMLCh qataskXMLStr[] = 
{
	chLatin_q, chLatin_a, chLatin_t, chLatin_a, chLatin_s, chLatin_k, chNull
};
const XMLCh sevXMLStr[] = 
{
	MiniXML::chLatin_s, MiniXML::chLatin_e, MiniXML::chLatin_v, MiniXML::chNull
};
const XMLCh severityXMLStr[] = 
{
	MiniXML::chLatin_s, MiniXML::chLatin_e, MiniXML::chLatin_v, MiniXML::chLatin_e, MiniXML::chLatin_r, MiniXML::chLatin_i, MiniXML::chLatin_t, MiniXML::chLatin_y, MiniXML::chNull
};
const XMLCh startXMLStr[] = 
{
	chLatin_s, chLatin_t, chLatin_a, chLatin_r, chLatin_t, chNull
};
const XMLCh startPmodServerXMLStr[] = 
{
	chLatin_s, chLatin_t, chLatin_a, chLatin_r, chLatin_t, chUnderscore, chLatin_p, chLatin_m, chLatin_o, chLatin_d, chUnderscore, chLatin_s, chLatin_e, chLatin_r, chLatin_v, chLatin_e, chLatin_r, chNull
};
const XMLCh warningXMLStr[] = 
{
	chLatin_w, chLatin_a, chLatin_r, chLatin_n, chLatin_i, chLatin_n, chLatin_g, chNull
};
const XMLCh zeroXMLStr[] = 
{
    MiniXML::chDigit_0, MiniXML::chNull
};
const XMLCh autoshutdownXMLStr[] = 
{
    chLatin_a, chLatin_u, chLatin_t, chLatin_o, chLatin_s, chLatin_h, chLatin_u, chLatin_t, chLatin_d, chLatin_o, chLatin_w, chLatin_n, chNull
};
