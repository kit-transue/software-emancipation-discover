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
#ifdef sun5

#include "template_dictionary.h"

#include <map>
#include <string>
#include <list>
#include <vector>

class CImpactAction;
class CImpactActionsSet;
class CArgument;
class CCallbackInfo;
class CIMAttachmentCallbackHandler;

// Report templates
class CReportEntry;
template class ::std::list< CReportEntry* >;

// ArgumentsTable templates
class CArgumentFields;
template class ::std::vector< CArgumentFields* >;

// QueryResults tepmplates  
typedef ::std::string TField;
typedef ::std::vector < TField* > TRecord;
template class ::std::vector< TField* >;
template class ::std::list< TRecord* >;

template class ::std::list< CImpactAction* > ;
template class ::std::list< CArgument* > ;
template class ::std::list< CCallbackInfo* > ; 
template class ::std::list< CIMAttachmentCallbackHandler* > ; 

template class ::std::vector< CImpactActionsSet* > ;

#endif /* sun5 */
