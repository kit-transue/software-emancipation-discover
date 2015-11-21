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
#define	MUTEX_TIMEOUT	INFINITE
#define MUTEX_NAME_LEN	10
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "integrator.h"
#include "hooker.h"



void main( int argc, char *argv[] )
{

	HANDLE hEvent;
	HANDLE hMutex;
	DWORD nThreadId;
	HINSTANCE hDll;
	FARPROC pHookProc;
	HHOOK hHook;
	char pszDiscoverPath[ MAX_PATH ];
	char pszMutexName[ MUTEX_NAME_LEN ];
	

	HWND hDeveloperStudio = NULL;
	
	if( argc > 1 )
	{
		// get main Developer Studio window handle as a first parameter
		// in decimal format
		
		hDeveloperStudio = (HWND) atoi( argv[1] );
		
		// get a thread ID for a given window
		nThreadId = GetWindowThreadProcessId( hDeveloperStudio, NULL );

		if( nThreadId )
		{
			// If this event exists, SPYLoader is already running for this IDE
			if( !OpenEvent( EVENT_MODIFY_STATE, FALSE, argv[1] ) )
			{
				// Create event used by a dll to signal us that it should be unloaded
				// Use a string representing window handle in decimal format as a name
				// of the event
				hEvent = CreateEvent( NULL, FALSE, FALSE, argv[1] );
				
				if( hEvent )
				{
					// Create a mutex with 'M' added to the name of event descibed above
					// Mutex is used to prevent unloading a dll while dll functions didn't
					// return yet after above-mentioned event was signaled
					wsprintf( pszMutexName, "M%s", argv[1] );
					hMutex = CreateMutex( NULL, FALSE, pszMutexName );

					if( hMutex )
					{
						// Get a path to the directory containing integration modules executables
						GetDisRegistryStr(REG_DISCOVER_DIR, pszDiscoverPath, MAX_PATH, "");
						strcat( pszDiscoverPath, DLL_MODULENAME );

						//Load a dll
						hDll = LoadLibrary( pszDiscoverPath );

						if( hDll )
						{
							// Get an address of hook procedure. The one should be exported
							// from a dll as extern "C" __declspec(dllexport)
							pHookProc = GetProcAddress( hDll, "_GetMsgHookProc@12" );

							if( pHookProc )
							{
								// Establish WH_GETMESSAGE hook. This maps a dll into a process space
								// of Visual C++
								hHook = SetWindowsHookEx( WH_GETMESSAGE, (HOOKPROC)pHookProc, hDll, nThreadId );

								// Bring Visual C++ window to top. This also ensures that hook procedure
								// within a dll is called and window subclassing is done
								SetForegroundWindow( hDeveloperStudio ); 
								// Wait for event signaled from a dll, and then unload it
								WaitForSingleObject( hEvent, INFINITE );
								// If mutex is owned by a different thread (in a dll),
								// we have to wait till a function releases mutex and returns
								WaitForSingleObject( hMutex, MUTEX_TIMEOUT );
								// Now we can remove hook. This unloads a dll from a process
								// space of Visual C++
								UnhookWindowsHookEx( hHook );
								// Free dll
								FreeLibrary( hDll );

								CloseHandle( hEvent );
								CloseHandle( hMutex );

							}
							else
							{
								FreeLibrary( hDll );
								CloseHandle( hEvent );
								CloseHandle( hMutex );
							}
						}
						else
						{
							CloseHandle( hEvent );
							CloseHandle( hMutex );
						}
					}
					else
					CloseHandle( hEvent );
				}
			}
		}
	}
}


