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

#include <msg.h>
#include <msg_format.h>

#include "service.h"
#include "mbuildmsg.h"
#include "mbmsgser.h"

#include <dis_iosfwd.h>
#include <string>
#include <map>
#include <vector>
#include <utility>
#include <stack>

template class ::std::vector < ostream * > ;
template class ::std::vector < string > ;
template class ::std::map < ostream * , msg_class * > ;
template class ::std::stack < unsigned long > ;

INSTANTIATE_RB_TREE_PAIR(ostream *, msg_class *);

template bool
::nTransport::connect < MBDriver::Message > (
	const ::nTransport::ServicePort & ,
	::nTransport::Communicator < MBDriver::Message > & ,
	string** ,
	::nTransport::Connection< MBDriver::Message > ** ) ;

typedef ::std::deque < unsigned long > atype ;
INSTANTIATE_COPY(atype::const_iterator, ::std::back_insert_iterator < atype >);

typedef RB_TREE_PAIR(string, string *)::const_iterator btype ;
template void std::__advance < btype , unsigned > 
	( btype & , unsigned , std::bidirectional_iterator_tag ) ;

INSTANTIATE_RB_TREE_PAIR(string, string *);

template class ::std::deque < MBDriver::Message * > ;

INSTANTIATE_SET (string);

INSTANTIATE_RB_TREE_PAIR (string, msg_traits);

#endif /* sun5 */
