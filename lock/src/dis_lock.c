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
#else
#include <sys/types.h>
#include <unistd.h>
#endif /* _WIN32 */

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NUMBER_LENGTH 16

static char bufin [MAX_NUMBER_LENGTH + 1];
static char bufout[MAX_NUMBER_LENGTH + 1];

main(argc, argv)
  int argc; char** argv;
{
  long size = 4;
  int sz, ret_ind = 0;
  int ii, len = 0;
  int fd;

#ifdef _WIN32
  HANDLE     hFile;
  DWORD	     dwNumberOfBytesTransferred;
  OVERLAPPED Overlapped;

#else
  int cmd = F_LOCK;
#endif

  bufin[0] = 0; bufout[0] = 0;
  for (ii = 0 ; ii <= MAX_NUMBER_LENGTH ; ++ii) { bufin[ii] = 0; }

#ifdef _WIN32

  hFile= CreateFile( 
		    argv[1],		    // file name
		    GENERIC_READ | GENERIC_WRITE ,	// access mode
		    FILE_SHARE_READ | FILE_SHARE_WRITE, // share mode
		    0,			    // LPSECURITY_ATTRIBUTES 
		    OPEN_ALWAYS,	    // how to create
		    FILE_ATTRIBUTE_NORMAL,  // file attributes
		    NULL );                 // handle to template file
  if ( hFile == INVALID_HANDLE_VALUE )
      return 1;

  ZeroMemory( &Overlapped, sizeof(OVERLAPPED) );

  /* Hang untill the file is unlocked */
  LockFileEx( hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, size, 0,  &Overlapped );
  GetOverlappedResult( hFile, &Overlapped, &dwNumberOfBytesTransferred, TRUE );

  /* read ascii number */
  ReadFile(hFile, (char*)bufin, MAX_NUMBER_LENGTH, &dwNumberOfBytesTransferred, NULL);
  sz = dwNumberOfBytesTransferred;

#else
  fd = open(argv[1], O_RDWR | O_CREAT, 0666);

  /* If open failed sleep for 1 second and try again. */
  if (fd <= 0) {
    sleep(1);
    fd = open(argv[1], O_RDWR | O_CREAT, 0666);
  }

  if(fd <= 0) {
    perror(0);
    return 1;
  }

  /* Hang untill the file is unlocked */

  lockf(fd, cmd, size);
  
  /* read ascii number */
  sz = read(fd, bufin, MAX_NUMBER_LENGTH);

#endif

  if(sz == 0) {
    /* empty file. Initialize with '1' */
    ret_ind  =  1;
    bufin[0] = '1';
  } else
    ret_ind = atoi(bufin);
  
  /* increment the number to print it on standard out*/
  ret_ind ++;


  for (ii = 0 ; ii <= MAX_NUMBER_LENGTH ; ++ii) { bufout[ii] = 0; }
  sprintf (bufout,"%d",ret_ind);
  sz = strlen(bufout);

#ifdef _WIN32

  SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
  WriteFile( hFile, (char*)bufout, MAX_NUMBER_LENGTH, &dwNumberOfBytesTransferred, NULL);

#else
  lseek(fd, 0, SEEK_SET);
  sz = write(fd,bufout,MAX_NUMBER_LENGTH);
#endif

  if(argc>2){
    char* cmd = argv[2];
    printf("start %d %s\n", ret_ind, cmd);
    system(cmd); 
    printf("end %d %s\n", ret_ind, cmd);
  } else
    printf ("%s", bufin);

#ifdef _WIN32

  UnlockFile( hFile, 0, 0, size, 0 );
  _lclose( (HFILE)hFile );

#else
  lockf(fd, F_ULOCK, 0);
  close (fd);
#endif

  return 0;
}
