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
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include "TclList.h"

class TclListTest : public CppUnit::TestFixture  {
    CPPUNIT_TEST_SUITE( TclListTest );
    CPPUNIT_TEST( testCreateDestroy );
    CPPUNIT_TEST( testAssign );
    CPPUNIT_TEST( testAppend );
    CPPUNIT_TEST_SUITE_END();

    void testCreateDestroy() {
        TclList list1;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("empty list should be zero length",
                                     0, list1.Length());
        char const *str = "hello";
        TclList list2(str);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("list should report length of string",
                                     (int)strlen(str), list2.Length());
        
    }

    void testAssign() {
        TclList list1;
        list1 = "hello";
        char const *s = list1[0];
	CPPUNIT_ASSERT(!strcmp(s, "hello"));
        list1 = "world";
    }

    void testAppend() {
        TclList list1;
        list1 = "hello";
        list1.Append(", world!");

        string ref2("world!");
        string var2(list1[1]);
	CPPUNIT_ASSERT_EQUAL(ref2, var2);

        string ref1("hello,");
        string var1(list1[0]);
	CPPUNIT_ASSERT_EQUAL(ref1, var1);

        string ref("hello, world!");
        string var(list1.Value());
	CPPUNIT_ASSERT_EQUAL(ref, var);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION ( TclListTest );
