#ifndef _aseticons_h
#define _aseticons_h

// Character indices into X11 font "aseticons-20".
//
// This file should be updated whenever a character is added to the
// icon font.  Please keep characters in ASCII order to make it easier
// to find them.

#define ICONFONTNAME "aseticons-20"

#define ICON_IMPORTED_F       1
#define ICON_WRITABLE_F       2
#define ICON_MODIFIED_F       4
#define ICON_LOADED_F         8
	
enum iconType
{
    icon_NULL		= '\000',
    icon_ARROW_DOWN	= '\001',
    icon_ARROW_RIGHT	= '\002',

    icon_OTHER		= ' ',
    icon_KEYWORD	= '!',
    icon_INDEXHIT	= '"',

    icon_SHADOW		= '#',
    icon_WRITABLE_PROJECT = '$',

    icon_BLANK		= '0',

    icon_UNLOCKED	= (char)0x89,
    icon_LOCKED		= (char)0x8a,
    icon_UNLOCKED_CHANGED = (char)0x8b,
    icon_LOCKED_CHANGED	= (char)0x8c,

    icon_XREF_TYPEDEF	= (char) 0x7f,

    icon_FROB		= '@',
    icon_STE		= 'A',
    icon_RTL		= 'B',
    icon_SMT		= 'C',
    icon_DD		= 'D',
    icon_SUBSYS		= 'E',
    icon_SCRAPBOOK	= 'F',
    icon_PROJECT	= 'G',
    icon_UFILE		= 'H',
    icon_BREAKPOINT	= 'I',

    icon_MODULE_FLAGS    = (char)0x10,   /*LWMI*/

    icon_XREF_STE	= 'a',
    icon_XREF_C		= 'c',
    icon_XREF_CPP	= 'e',
    icon_XREF_UNKNOWN	= 'g',
    icon_XREF_FUNC	= 'i',
    icon_XREF_CLASS	= 'k',
    icon_XREF_VAR	= 'm',
    icon_XREF_ENUM	= 'o',
    icon_XREF_DD	= 'q',
    icon_XREF_SUBSYSTEM	= 's',
    icon_XREF_MAKEFILE	= 'u',
    icon_XREF_PROJECT	= 'w',
    icon_XREF_MACRO	= 'y',
    icon_XREF_UNION	= '{',
    icon_XREF_ENUM_VAL	= (char)0x7d,
    icon_XREF_FIELD	= (char)0x83,
    icon_XREF_SEMTYPE	= (char)0x85,

    icon_XREF_XDOC      = (char)0x87,
    icon_XREF_ESQL_CPP  = 0x9a,
    icon_XREF_ESQL_C    = 0x9c
};

#endif

/*
    START-LOG-------------------------------

    $Log: aseticons.h  $
    Revision 1.3 1997/05/06 16:49:11EDT wmm 
    Bug track: N/A
    Implement embedded SQL.  This involved adding new language types (ESQL C and ESQL C++) with associated icons and a new preprocessor
    that is invoked during normal (C) preprocessing, along with controlling preferences and logic.
 * Revision 1.2.1.11  1993/12/10  15:20:13  kws
 * Bug track: 4948
 * Give ability to see how files were modified in browser
 *
 * Revision 1.2.1.10  1993/12/03  15:27:59  kws
 * Bug track: N/A
 * Fix up some more icons (semtype and enum_val)
 *
 * Revision 1.2.1.9  1993/12/01  17:52:52  kws
 * Fix constant
 *
 * Revision 1.2.1.8  1993/12/01  02:30:52  kws
 * Fix constant
 *
 * Revision 1.2.1.7  1993/11/30  23:34:14  kws
 * Bug track: 4592
 * Part of bug fix : show member variables with a new icon
 *
 * Revision 1.2.1.6  1993/08/19  19:29:29  andrea
 * added icons for writable dd_modules
 *
 * Revision 1.2.1.5  1993/01/16  22:01:24  glenn
 * Add XREF_UNION
 *
 * Revision 1.2.1.4  1993/01/16  21:48:58  glenn
 * Add WRITABLE_PROJECT, XREF_MACRO
 *
 * Revision 1.2.1.3  1993/01/10  04:29:20  glenn
 * Add XREF_PROJECT.
 *
 * Revision 1.2.1.2  1992/10/09  19:56:33  kws
 * Fix comments
 *

    END-LOG---------------------------------
*/

