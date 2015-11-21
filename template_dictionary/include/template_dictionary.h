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
//
// This file provides convenient macros for instantiating common templates
// from the STL.  It should only be used within the template_dictionary
// directory, and not anywhere else.
//

#define RB_TREE_PAIR(key, value) __rwstd::__rb_tree < key, ::std::pair < key const, value >, __rwstd::__select1st < ::std::pair < key const, value >, key >, ::std::less < key >, ::std::allocator < ::std::pair < key const, value > > >

#define INSTANTIATE_RB_TREE_PAIR(key, value) template class __rwstd::__rb_tree < key, ::std::pair < key const, value >, __rwstd::__select1st < ::std::pair < key const, value >, key >, ::std::less < key >, ::std::allocator < ::std::pair < key const, value > > >

#define INSTANTIATE_COPY(input, output) template output std::copy < input, output > ( input, input, output )

#define INSTANTIATE_SET(type) template class __rwstd::__rb_tree < type, type, __rwstd::__ident < type, type>, ::std::less < type > , ::std::allocator < type > >
