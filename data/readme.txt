##########################################################################
# Copyright (c) 2015, Synopsys, Inc.                                     #
# All rights reserved.                                                   #
#                                                                        #
# Redistribution and use in source and binary forms, with or without     #
# modification, are permitted provided that the following conditions are #
# met:                                                                   #
#                                                                        #
# 1. Redistributions of source code must retain the above copyright      #
# notice, this list of conditions and the following disclaimer.          #
#                                                                        #
# 2. Redistributions in binary form must reproduce the above copyright   #
# notice, this list of conditions and the following disclaimer in the    #
# documentation and/or other materials provided with the distribution.   #
#                                                                        #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    #
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      #
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  #
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   #
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, #
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       #
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  #
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  #
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    #
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  #
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   #
##########################################################################

This file contains important information about DISCOVER Release 7.0. Please
read and understand it before proceeding with your installation.


---------------------------------------------------------------
CONTENTS OF THIS README FILE

	Files contained on the installation CD
	UNIX Installation (How to untar the DISCOVER installation files)
	Windows NT Installation
	Notes for Windows NT Installations
	How to view the DISCOVER 7.0 on-line documentation

---------------------------------------------------------------
FILES CONTAINED ON THE INSTALLATION CD

	AUTORUN.INF	Autostart script that runs the NT 
			installation wizard
	docs		Directory containing on-line documentation library
	hp10.tz		Installation files for HP-UX 10.x
	hp.tz		Installation files for HP-UX 9.x
	irix6.tz	Installation files for IRIX 6.x
	master.exe	Installation wizard for NT
	readme.txt	This README file
	sun4.tz		Installation files for Sun4 (SunOS 4.1.x) systems
	sun5.tz		Installation files for Sun5 (Solaris 2.x) systems
	winnt		Directory containing installation files for NT


Note: On HP-UX, all file/directory names are in upper case and 
      end with ";1".

Note: You must make separate installations of DISCOVER for EACH platform
      that you will support at your site. Be sure to untar all the 
      installations that you will need. 



---------------------------------------------------------------
UNIX INSTALLATION--HOW TO UNTAR THE DISCOVER INSTALLATION FILES

1. Load the DISCOVER CD into the CD-ROM drive.

   Sun5 (Solaris 2.x) systems automatically mount the CD. The tar files are
   visible under /cdrom/discover. Sun5 users should proceed to Step 6. Sun4
   (SunOS 4.1.x) and HP-UX systems require you to mount the CD ROM
   drive from the command line, as described in Steps 2-5.

2. Become superuser (root) by entering "su" at the prompt.

3. Enter your superuser password.

4. Use the appropriate command, as listed below, to mount the CD ROM at the
   directory "cdrom". Create this directory if it does not already exist.

   SunOS 4.1.x:	/etc/mount -rt hsfs <filesystem> /cdrom
   HP-UX:	/etc/mount -F  cdfs <filesystem> /cdrom

   <filesystem> is the file system device name, such as /dev/sr0 for
   SunOS 4.1.x or /dev/dsk/c0t6d0 for HP-UX.

5. Log out from superuser status.

6. Execute "ls" on the directory where the CD is mounted. On Sun5 
   (Solaris 2.x) systems, this is /cdrom/discover. On Sun4 (SunOS
   4.1.x) and HP-UX, it is /cdrom. You should see the following
   files/directories on Sun4 (SunOS 4.1.x) and Sun5 (Solaris 2.x):

	docs
	hp10.tz
	hp.tz
	irix6.tz
	readme.txt
	sun4.tz
	sun5.tz

    On HP-UX, you should see the following listing:

	docs;1
	HP10.TZ;1
	HP.TZ;1
	IRIX6.TZ;1
	README.TXT;1
	SUN4.TZ;1
	SUN5.TZ;1

7. Use the appropriate command to extract the DISCOVER executable files for
   your system:

   SunOS 4.1.x:  	zcat < /cdrom/sun4.tz | tar xvf -
   Solaris 2.x: 	zcat < /cdrom/sun5.tz | tar xvf -
   HP-UX 9.x:		zcat < '/cdrom/HP.TZ;1' | tar xvf _
   HP-UX 10.x:		zcat < '/cdrom/HP10.TZ;1' | tar xvf -
   IRIX 6.x:		zcat < /cdrom/irix6.tz | tar xvf -

8. Read the section entitled HOW TO VIEW THE DISCOVER ON-LINE
   DOCUMENTATION, which explains how to view the on-line documentation
   library, including the complete DISCOVER installation instructions.



---------------------------------------------------------------
WINDOWS NT INSTALLATION

1. Load the DISCOVER CD into the CD-ROM drive. The Autorun file
   (the filename is Master) will execute, displaying a DISCOVER window
   that enables you to run the installation wizard.

2. Click on "Install DISCOVER." If you do not want to start the 
   installation, click on "EXIT" instead.

3. Follow the instructions provided by the installation wizard.





---------------------------------------------------------------
HOW TO VIEW THE DISCOVER 7.0 ON-LINE DOCUMENTATION

The entire DISCOVER 7.0 documentation library is included on your 
installation disk, in a format that enables you to view it on
line. The following list describes the DISCOVER 7.0 documentation
library. These files are stored on the installation CD in the
directory called DOCS.


docs/unix:

   aguide.pdf       	Description of the DISCOVER installation 
			procedure, procedures for integrating DISCOVER 
			into your development environment, including 
			defining projects, Information Model building and 
			maintenance, tool integration, and user setup

   aref.pdf		Deployment scripts and batch procedures for building
			Information Models

   parser.pdf		Description of the DISCOVER 7.0 parser (which
			runs during the Model-building procedure)

   parseref.pdf		A quick deployment guide for the DISCOVER 7.0 parser

   quickref.pdf		A reference card describing DISCOVER procedures

   relnotes.pdf		DISCOVER 7.0 Release Notes for UNIX

   uguide.pdf        	The DISCOVER user's guide for UNIX

   yguide.pdf		Procedure for performing a Year-2000 analysis on 
			your source code



docs/nt:


   parser.pdf		Description of the DISCOVER 7.0 parser (which
			runs during the Model-building procedure)

   parseref.pdf		A quick deployment guide for the DISCOVER 7.0 parser

   relnotes.pdf         DISCOVER 7.0 Release Notes for Windows NT

   uguide.pdf		The DISCOVER user's guide for Windows NT

   yguide.pdf		Procedure for performing a Year-2000 analysis on 
			your source code 
   
The DISCOVER documentation library consists of eight files stored in Adobe 
portable document format (.pdf). You can read these files using the Adobe 
Acrobat Reader, which Adobe distributes free of charge. If you do not have 
the Acrobat Reader, you can download it from Adobe's Web site. Adobe's URL 
is http://www.adobe.com. On their home page, you will find a link called 
"Get Acrobat Reader." This link enables you to download the latest version 
of the Adobe Acrobat Reader. The set of files that you download includes 
instructions for installing and starting the Acrobat Reader.

After starting the Acrobat Reader, select "Open" from the "File" menu. In 
the dialog box that appears, select the .pdf file that you want to view. 
Use the icons at the left side of the Acrobat Reader toolbar to view 
expandable bookmarks or a set of thumbnails (miniatures) of each page in 
the document.





This file was last updated on December 8, 1998.
