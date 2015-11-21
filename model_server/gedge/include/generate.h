/* generate.h  (C) Stefan Manke 1990 */


#ifndef _GENERATE_H
#define _GENERATE_H

#define TYPESNUM(v)   (sizeof v / sizeof (struct t_Types))
#define INPUTSNUM(v)  (sizeof v / sizeof (struct t_Inputs))

enum possible_c_type {CHARTYPE, INTTYPE, FLOATTYPE, STRINGTYPE, ENUMTYPE,
                      STRUCTTYPE, POINTERTYPE, ARRAYTYPE, LABELTYPE};


struct t_Types;
typedef struct t_Types Types;

struct t_Inputs;
typedef struct t_Inputs Inputs;

struct t_EnumData {
            char *id;
            long /* Widget */ w;
            int num;
	};

typedef struct t_EnumData EnumData;

struct t_Inputs {
         char *label;
	 char *widgetlabel;
         char *addstring;
         char *inputstring;
         char *defaultvalue;
         int indent;
	 long /* Widget */ w;
	 long /* Widget */ labelw;
	 enum possible_c_type c_type;
	 EnumData *enumdata;
	 int enumlength;
       };


struct t_Types {
         char *name;
	 long /* Widget */ boxw;
	 long /* Widget */ w;
	 int id;
	 int length;
	 Inputs *inputs;
       };
 

#endif
