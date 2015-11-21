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
#ifdef _WIN32 
#include <windows.h>
#include <winmain.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

//------------------------------------------------
// Matt Pietrek 1996
// Microsoft Systems Journal, October 1996
//------------------------------------------------
char * _ppszArgv[_MAX_CMD_LINE_ARGS+1];

int __cdecl ConvertCommandLineToArgcArgv( void )
{
    int cbCmdLine;
    int argc;
    PSTR pszSysCmdLine, pszCmdLine;
    
    // Set to no argv elements, in case we have to bail out
    _ppszArgv[0] = 0;

    // First get a pointer to the system's version of the command line, and
    // figure out how long it is.
    pszSysCmdLine = GetCommandLine();
    cbCmdLine = lstrlen( pszSysCmdLine );

    // Allocate memory to store a copy of the command line.  We'll modify
    // this copy, rather than the original command line.  Yes, this memory
    // currently doesn't explicitly get freed, but it goes away when the
    // process terminates.
    pszCmdLine = (PSTR)HeapAlloc( GetProcessHeap(), 0, cbCmdLine+1 );
    if ( !pszCmdLine )
        return 0;

    // Copy the system version of the command line into our copy
    lstrcpy( pszCmdLine, pszSysCmdLine );

    if ( '"' == *pszCmdLine )   // If command line starts with a quote ("),
    {                           // it's a quoted filename.  Skip to next quote.
        pszCmdLine++;
    
        _ppszArgv[0] = pszCmdLine;  // argv[0] == executable name
    
        while ( *pszCmdLine && (*pszCmdLine != '"') )
            pszCmdLine++;

        if ( *pszCmdLine )      // Did we see a non-NULL ending?
            *pszCmdLine++ = 0;  // Null terminate and advance to next char
        else
            return 0;           // Oops!  We didn't see the end quote
    }
    else    // A regular (non-quoted) filename
    {
        _ppszArgv[0] = pszCmdLine;  // argv[0] == executable name

        while ( *pszCmdLine && (' ' != *pszCmdLine) && ('\t' != *pszCmdLine) )
            pszCmdLine++;

        if ( *pszCmdLine )
            *pszCmdLine++ = 0;  // Null terminate and advance to next char
    }

    // Done processing argv[0] (i.e., the executable name).  Now do th
    // actual arguments

    argc = 1;

    while ( 1 )
    {
        // Skip over any whitespace
        while ( *pszCmdLine && (' ' == *pszCmdLine) || ('\t' == *pszCmdLine) )
            pszCmdLine++;

        if ( 0 == *pszCmdLine ) // End of command line???
            return argc;

        if ( '"' == *pszCmdLine )   // Argument starting with a quote???
        {
            pszCmdLine++;   // Advance past quote character

            _ppszArgv[ argc++ ] = pszCmdLine;
            _ppszArgv[ argc ] = 0;

            // Scan to end quote, or NULL terminator
            while ( *pszCmdLine && (*pszCmdLine != '"') )
                pszCmdLine++;
                
            if ( 0 == *pszCmdLine )
                return argc;
            
            if ( *pszCmdLine )
                *pszCmdLine++ = 0;  // Null terminate and advance to next char
        }
        else                        // Non-quoted argument
        {
            _ppszArgv[ argc++ ] = pszCmdLine;
            _ppszArgv[ argc ] = 0;

            // Skip till whitespace or NULL terminator
            while ( *pszCmdLine && (' '!=*pszCmdLine) && ('\t'!=*pszCmdLine) )
                pszCmdLine++;
            
            if ( 0 == *pszCmdLine )
                return argc;
            
            if ( *pszCmdLine )
                *pszCmdLine++ = 0;  // Null terminate and advance to next char
        }

        if ( argc >= (_MAX_CMD_LINE_ARGS) )
            return argc;
    }
}
//------------------------------------------------

BOOL CreateConsole(void) {
  FreeConsole();        //to be safe :)
  if( AllocConsole()) {
	int hCrt = _open_osfhandle((long)
	GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
	*stdout = *(::_fdopen(hCrt, "w"));
	::setvbuf(stdout, NULL, _IONBF, 0);
	*stderr = *(::_fdopen(hCrt, "w"));
	::setvbuf(stderr, NULL, _IONBF, 0);
	return TRUE;
  }
	return FALSE;
}
#endif // _WIN32
