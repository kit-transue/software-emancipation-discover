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
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "genError.h"
#ifndef ISO_CPP_HEADERS
#include <limits.h>
#else /* ISO_CPP_HEADERS */
#include <limits>
#endif /* ISO_CPP_HEADERS */

extern "C"
{
  int abs(int);
}

// these macros define the limits of the integer values
// and the type of integer that these routines will work with
// NOTE: if you change these then go change them in the lexer str_to_int.l
#define MAX_VAL INT_MAX
#define MIN_VAL INT_MIN
#define VAL_TYPE int
  


// strings that serve to store the text equivalents of the digits
static char const * ones[10] = {
"zero",
"one",
"two",
"three",
"four",
"five",
"six",
"seven",
"eight",
"nine"
};

static char const * teens[10]={
"ten",
"eleven",
"twelve",
"thirteen",
"fourteen",
"fifteen",
"sixteen",
"seventeen",
"eighteen",
"nineteen"
};

static char const * tens[10] = {
"zzz",
"ten",
"twenty",
"thirty",
"forty",
"fifty",
"sixty",
"seventy",
"eighty",
"ninety"
};


// This subroutine converts a number in the range -99 to 99 to
// its text equivalent
// the word "negative" is not prefixed
void tens_to_string( VAL_TYPE number, ostream& os)
{
    Initialize(tens_to_string);

    IF(number < -99 || number > 99) Error(ERR_INPUT);

    // to maintain proper indexing we need to use the absolute value
    number = abs(number);

    if(number < 10)      os << ones[number];
    else if(number < 20) os << teens[number - 10];
    else
    {
        os << tens[number/10];
        VAL_TYPE dd = number % 10;
        if(dd) os << ' ' << ones[dd];
    }

    Return
}

// This subroutine converts a number in the range of -999 to 999
// to its text equivalent.
// the word "negative" is not prefixed.
void hundreds_to_string(VAL_TYPE number, ostream& os)
{
    Initialize(hundreds_to_string);
    

    // range check
    IF(number < -999 || number > 999) Error(ERR_INPUT);
    
    // the number of 100s in the number
    VAL_TYPE hundreds = number / 100;

    // the left over digits
    VAL_TYPE rem =      number % 100;

    // if number > 100 create a string for it
    if(hundreds)
    {
        tens_to_string(hundreds, os);
        os << " hundred ";
    }

    // create a string for digits unaccounted for by hundreds
    if (rem)
      tens_to_string(rem, os);
    
}





void int_to_string(VAL_TYPE number, ostream& os)
{
    Initialize(int_to_string);

    // the trivial case
    if(number == 0)
    {
        os << "zero";
        return;
    }

    // support negative numbers
    if (number < 0)
    {
        os << "negative ";
    }
    
    // create a working copy
    VAL_TYPE rem = number;

#if MAX_VAL > 32768

    // capture the number of whole billions
    VAL_TYPE billions = rem / 1000000000;
    rem =               rem % 1000000000;
    if ( billions )    
    {
      hundreds_to_string(billions, os);
      os << " billion ";
    }

    // capture the number of whole millions
    VAL_TYPE millions = rem / 1000000;
    rem =               rem % 1000000;
    if ( millions )    
    {
      hundreds_to_string(millions, os);
      os << " million ";
    }
    
#endif
    
    // capture the number of whole thousands
    VAL_TYPE thousands = rem / 1000;
    rem =                rem % 1000;
    if ( thousands )    
    {
      hundreds_to_string(thousands, os);
      os << " thousand ";
    }
    
    // capture the number of whole hundreds and tens
    hundreds_to_string( rem, os );        
    
    return;
}

/*
   START-LOG-------------------------------------------

   $Log: int_to_string.cxx  $
   Revision 1.2 2000/07/10 23:07:08EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
Revision 1.2.1.3  1993/02/18  23:10:47  sharris
support the full range of 32 bit integers

Revision 1.2.1.2  1992/10/09  18:55:13  boris
Fix comments


   END-LOG---------------------------------------------

*/
