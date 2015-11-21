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

#include "message_templates.cxx"
#include <list>
#include <utility>

#include "configpar_internal.h"

INSTANTIATE_RB_TREE_PAIR(string, bool ConfigDocumentHandler::*);
// INSTANTIATE_RB_TREE_PAIR(string, OptionInterpreter);
INSTANTIATE_RB_TREE_PAIR(string, OptionInterpreter*);
INSTANTIATE_RB_TREE_PAIR(string, SuffixInterpreter);
INSTANTIATE_RB_TREE_PAIR(string, SuffixInterpreter*);

typedef ::std::deque < OptionInterpreter const * > oip_deque_type;
template class oip_deque_type;
INSTANTIATE_COPY(oip_deque_type::const_iterator, ::std::back_insert_iterator < oip_deque_type > );
INSTANTIATE_COPY(oip_deque_type::const_iterator, ::std::insert_iterator < oip_deque_type > );

template class ::std::vector < ::std::string > ;
template class ::std::vector < OutputBlock * > ;
template class ::std::vector < NameSelector * > ;
template class ::std::vector < NameSelectorEquivalent * > ;
template class ::std::vector < const char * > ;
template class ::std::vector < OptionInterpreter * > ;
template class ::std::list< ::std::string>;
typedef ::std::pair< ::std::string,bool> switch_setting;
template class ::std::list<switch_setting>;

#endif // sun5
