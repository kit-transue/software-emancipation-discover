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
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <x11_intrinsic.h>

#include <messages.h>

// VARIABLES FROM OUR PAST //

XtAppContext	UxAppContext;
Display*	UxDisplay;
Window		UxRootWindow;
Screen		UxScreen;
Widget		UxTopLevel;


// STUB CLASSES AND FUNCTIONS //

#if defined(sun5)
extern "C" char *gettext(char const * pBuff)
#else
extern "C" const char *gettext(const char* pBuff)
#endif
{
    char * ret_pBuff = (char *)pBuff;
    return ret_pBuff;
}

extern "C" void msg_diag(const char *, ...) {};
long  ste_get_pset_uxid() {return 0;}

void system_message_force_logger(int val) {}

int ste_get_gra_world_level () { return 1;}
void ste_interface_set_busy_cursor(int) {}

int popup_Question(const char *, const char *, const char *, const char *) { 
	return 1 ; }

extern "C" void msg_warn(const char* fmt...)
{
   return; 
}

void dis_message(const char*, systemMessageType, const char*, ...)
{
  return;
}

class time_log
{
  public:
    static int tail;
};

enum PropertyId {};

class Notifier
{
  public:

    void  add(void*);
    void  remove(void*);
    void  set_prop(PropertyId, void*, void*);
    void* get_prop(PropertyId, void*);
};

Notifier notifier;

void  Notifier::add(void *) {}
void* Notifier::get_prop(PropertyId, void *) { return 0; }
void  Notifier::remove(void *) {}
void  Notifier::set_prop(PropertyId, void *, void *) {}

void driver_client_hook (XEvent *) {};

void logger_end_transaction () {};
void logger_start_transaction () {};

extern "C" regex() { printf ("regex called\n"); return 0;}
extern "C" regcmp() { printf ("regcmp called\n"); return 0;}
extern "C" void ste_unfreeze(void) {};

class Object;
class ostream;
class Obj {
    void hash(void) const;
    void objname(void) const;
    ~Obj(void);
    void isEqual(const Object&) const;
    void size(void) const;
    void print(ostream&,int) const;
    void collectionp(void) const;
    void compare(const Object&) const;
    void relationalp(void) const;
    void includes(const Obj*) const;
    void put_signature(ostream&) const;
    void val_signature(void) const;
};

void Obj::hash(void) const {}
void Obj::objname(void) const {}
Obj::~Obj(void) {}
void Obj::isEqual(const Object&) const {}
void Obj::size(void) const {}
void Obj::print(ostream&,int) const {}
void Obj::collectionp(void) const {}
void Obj::compare(const Object&) const {}
void Obj::relationalp(void) const {}
void Obj::includes(const Obj*) const {}
void Obj::put_signature(ostream&) const {}
void Obj::val_signature(void) const {}
struct call_trace{
   static int logging;
};

#define FIX_FILE(ch,out) (ch==1?stdout:(ch==2?stderr:out))

