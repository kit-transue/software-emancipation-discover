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
// pathcvt.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream> // for i/o functions
#include <memory>
#include <algorithm>

using namespace std;


DWORD ShortToLongPathName(
    LPCTSTR lpszShortPath,
    LPTSTR lpszLongPath, 
    DWORD cchBuffer)
{
    // Catch null pointers.
    if (!lpszShortPath || !lpszLongPath)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    // Check whether the input path is valid.
    if (0xffffffff == GetFileAttributes(lpszShortPath))
        return 0;

    // Special characters.
    TCHAR const sep = TCHAR('\\');
    TCHAR const colon = TCHAR(':');

    // Make some short aliases for basic_strings of TCHAR.
    typedef std::basic_string<TCHAR> tstring;
    typedef tstring::traits_type traits;
    typedef tstring::size_type size;
    size const npos = tstring::npos;

    // Copy the short path into the work buffer and convert forward 
    // slashes to backslashes.
    tstring path = lpszShortPath;
    std::replace(path.begin(), path.end(), TCHAR('/'), sep);

    // We need a couple of markers for stepping through the path.
    size left = 0;
    size right = 0;

    // Parse the first bit of the path.
    if (path.length() >= 2 && isalpha(path[0]) && colon == path[1]) // Drive letter?
    {
        if (2 == path.length()) // 'bare' drive letter
        {
            right = npos; // skip main block
        }
        else if (sep == path[2]) // drive letter + backslash
        {
            // FindFirstFile doesn't like "X:\"
            if (3 == path.length())
            {
                right = npos; // skip main block
            }
            else
            {
                left = right = 3;
            }
        }
        else return 0; // parsing failure
    }
    else if (path.length() >= 1 && sep == path[0])
    {
        if (1 == path.length()) // 'bare' backslash
        {
            right = npos;  // skip main block
        }
        else 
        {
            if (sep == path[1]) // is it UNC?
            {
                // Find end of machine name
                right = path.find_first_of(sep, 2);
                if (npos == right)
                    return 0;

                // Find end of share name
                right = path.find_first_of(sep, right + 1);
                if (npos == right)
                    return 0;
            }
            ++right;
        }
    }
    // else FindFirstFile will handle relative paths

    // The data block for FindFirstFile.
    WIN32_FIND_DATA fd;

    // Main parse block - step through path.
    while (npos != right)
    {
        left = right; // catch up

        // Find next separator.
        right = path.find_first_of(sep, right);

        // Temporarily replace the separator with a null character so that
        // the path so far can be passed to FindFirstFile.
        if (npos != right)
            path[right] = 0;

        // See what FindFirstFile makes of the path so far.
        HANDLE hf = FindFirstFile(path.c_str(), &fd);
        if (INVALID_HANDLE_VALUE == hf)
            return 0;
        FindClose(hf);

        // Put back the separator.
        if (npos != right)
            path[right] = sep;

        // The file was found - replace the short name with the long.
        size old_len = (npos == right) ? path.length() - left : right - left;
        size new_len = traits::length(fd.cFileName);
        path.replace(left, old_len, fd.cFileName, new_len);

        // More to do?
        if (npos != right)
        {
            // Yes - move past separator .
            right = left + new_len + 1;

            // Did we overshoot the end? (i.e. path ends with a separator).
            if (right >= path.length())
                right = npos;
        }
    }

    // If buffer is too small then return the required size.
    if (cchBuffer <= path.length())
        return path.length() + 1;

    // Copy the buffer and return the number of characters copied.
    traits::copy(lpszLongPath, path.c_str(), path.length() + 1);
    return path.length();
}

int main(int argc, char* argv[])
{
	int nRetCode = 0;
	bool bShort = true;
	string param;
	string filename;
	for(int i=1;i<argc;i++) {
		param = argv[i];
		if(param.compare("-short")==0) 
			bShort = true;
		else if(param.compare("-long")==0)
			bShort = false;
		else 
			filename = param;
	}
	
	if(filename.empty()) {
		cout << "Error: " << "Empty path." << endl;
		nRetCode = 2;
	} else {
		if(bShort) {
			DWORD nSize = 1024;
			DWORD nNeedSize = 0;
			TCHAR* shortPath = new TCHAR[nSize];
			
			while((nNeedSize = GetShortPathName(filename.c_str(),shortPath,nSize))>nSize) {
				delete shortPath;
				nSize = nNeedSize;
				shortPath = new TCHAR[nSize];
			}
			
			if(nNeedSize!=0)
				cout << shortPath << endl;
			else {
				LPVOID lpMsgBuf;
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
							  FORMAT_MESSAGE_FROM_SYSTEM | 
							  FORMAT_MESSAGE_IGNORE_INSERTS,
							  NULL,
							  GetLastError(),
							  0, // Default language
							  (LPTSTR) &lpMsgBuf,
							  0,
							  NULL);
				// the new line character is already a part of the message
				cout  << "(" << filename << ")" << "Error: " << (LPCTSTR)lpMsgBuf; 
				// Free the buffer.
				LocalFree( lpMsgBuf );
				nRetCode = 1;
			}
			delete shortPath;
		} else {
			DWORD nSize = 1024;
			DWORD nNeedSize = 0;
			TCHAR* longPath = new TCHAR[nSize];
			
			while((nNeedSize = ShortToLongPathName(filename.c_str(),longPath,nSize))>nSize) {
				delete longPath;
				nSize = nNeedSize;
				longPath = new TCHAR[nSize];
			}
			
			cout << longPath << endl;
			delete longPath;
		}
	}
	return nRetCode;
}

