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
#include <cppunit/extensions/HelperMacros.h>

#include "XrefTable.h"
#include "_Xref.h"

class XrefTableTest : public CppUnit::TestFixture  {
    CPPUNIT_TEST_SUITE( XrefTableTest );
    CPPUNIT_TEST( testCreate );
    CPPUNIT_TEST_SUITE_END();

    static char const * const XrefTableTest::sym_file;
    static char const * const XrefTableTest::ind_file;
    static char const * const XrefTableTest::link_file;
    
    void testCreate() {
        Xref *xref = NULL;
        XrefTable t(xref, sym_file, ind_file, link_file);
    }

#if 0 // cannot test standalone: Xref builds these, so test that there.
    // XXX: but we should untangle this.
    void testInsert() {
        Xref xref;
        XrefTable t(&xref, sym_file, ind_file, link_file);
        t.insert_symbol(DD_MACRO, 42, "the_answer");
    }
#endif
};

char const * const XrefTableTest::sym_file  = "xrefTableTest.pmod.sym";
char const * const XrefTableTest::ind_file  = "xrefTableTest.pmod.ind";
char const * const XrefTableTest::link_file = "xrefTableTest.pmod.lin";

CPPUNIT_TEST_SUITE_REGISTRATION ( XrefTableTest );
