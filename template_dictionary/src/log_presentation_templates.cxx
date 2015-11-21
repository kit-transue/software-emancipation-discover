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
#include "filtersev_internal.h"
#include "logpresent.h"

#include <set>

INSTANTIATE_RB_TREE_PAIR(string, GroupData);

template class ::std::list < ::MBDriver::Message * > ;
template class ::std::list < ElementHandler * > ;
template class ::std::vector < ::MBDriver::Message * > ;
template class ::std::list < Subtask * > ;

template class __rwstd::__rb_tree <
	long, long, __rwstd::__ident < long, long >,
	::std::less < long >, ::std::allocator < long >
>;

template bool
::nTransport::listen < MBDriver::Message > (
	const ::nTransport::ServiceName & ,
	::nTransport::Communicator < MBDriver::Message > & ,
	string** ) ;

typedef ::std::list < Condition * > ::const_iterator clist_it;

template bool
std::lexicographical_compare < clist_it, clist_it > (
	clist_it, clist_it, clist_it, clist_it ) ;

template ::std::pair < clist_it, clist_it >
std::mismatch < clist_it, clist_it > ( clist_it, clist_it, clist_it ) ;

#endif // sun5
