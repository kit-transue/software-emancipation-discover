October 30, 2000

This file describes the various formats in which the splash screen is 
stored, and the methods used to update and convert it.  It supersedes
the document in /paraset/src/clients/splash about updating splash
screens.

Currently, editing of splash screens is done in Adobe Photoshop.
Splash screens, icons, and other images are used in GIF format by
CodeRover; the GIF versions of them are stored in 
/paraset/src/DeveloperXPress/Dislite/Images/CodeRover.  Discover uses
xbm and xpm versions of the same, or similar, images.  Notably, these
images include the icon used by Discover when minimized, logo50.xbm, 
the image used for the splash screen, splash.xpm, and the image used 
for the company name and logo on the about screen, about_logo.xbm.
Currently, about_logo.xbm contains a slightly ugly version of the 
Upspring logo, and is not used in the about screen.

To export from Photoshop, select Export GIF89a.  Select non-interlaced.
If Export->GIF is not available, the file may be in CMYK mode.  Change
mode to RGB and export should go OK.

The GIF file may be imported by gimp and written to "XPm".  The filename
that you save the file as is included (with a _xpm suffix) in the
declaration of the array, so naming may be important.  For splash.xpm,
you need to rename the array to "splash" anyway.

The last time that images were updated, the .xbm files were converted
from GIF using a program available on SunOS 4 called xv.  splash.xpm
was converted by loading splash.bmp into GIMP and saving as Xpm.  GIMP
is currently installed in /users/sschmidt/gimp.  After saving as Xpm, 
some massaging of the .xpm file may be necessary to allow its inclusion
in ProgressBar.C;  in particular, the name of the array defined in the
.xpm file must be changed to splash[].

It appears that GIMP is not capable of reading .xbm files, but that it
is capable of reading .psd (Photoshop) files.  In the future, perhaps
we will be able to do all of our image editing in GIMP, but this
requires more skill than we currently have.

Whatever method is used to produce splash.xpm, make sure that it does
not require too many colors.  16 should be more than enough.  If the
splash screen uses too many colors, there will be none left for the
browser and viewer.  (Actually, I found that with the MKS splash screens
containing faded source code, 20 colors work dramatically better than
16.)  To set the color depth, save the file as a .xpm to define the
file format, then select image->Indexed, then in the "Indexed Color
Conversion" dialog, select "Generate optimal palette" with somewhere
between 16 and 20 colors.

Some notes on where these things are included, sure to become out of date
soon:
	splash.xpm: ui/misc/progress_bar
