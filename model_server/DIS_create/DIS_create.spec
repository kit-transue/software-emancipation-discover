service DIScreate

#ifdef SERVER
#include "include/create.h"
#include "../../../clients/DIS_ui/interface.h"
#include "galaxy_undefs.h"
#include "cLibraryFunctions.h"
#include "include/boilerplate.h"
#include "genString.h"

!extern "C" popup_define_class (char *, char *, int);
!extern "C" popup_define_struct (char *);
!extern "C" popup_define_function ();
!extern Application* DISui_app;

#endif

@@ dis_create_init 
{
    const char *dialog = argv[1];

    popup_define_class ("", "", 0);
    popup_define_struct ("");
    popup_define_function ();
    Boilerplate::initialize (dialog);
    return TCL_OK;
}

@@ dis_create_close
{
    Boilerplate::close ();
    return TCL_OK;
}

@@ dis_create_class
{
    createClass (argv[1], argv[2], argv[3], argv[4]);
    return TCL_OK;
}


@@ dis_create_function
{
    const char *result = createFunction (argv[1], argv[2], argv[3], argv[4], 
                                         argv[5], argv[6], argv[7], argv[8]);

    if (result) {
        genString command;
        command.printf ("dis_prompt {OKDialog} {Create/sw} {%s}", result);
        rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
    }
    
    return TCL_OK;
}


@@ dis_create_struct 
{
    createStruct (argv[1], argv[2], argv[3]);
    return TCL_OK;
}


@@ dis_bp_deleteBoilerplate 
{
    int item = atoi (argv[1]);

    Boilerplate::DeleteInstanceBoilerplate(item);
    return TCL_OK;
}


@@ dis_bp_newBoilerplate
{

    const char *bpName = argv[1];

    Boilerplate::NewInstanceBoilerplate(bpName);
    return TCL_OK;
}


@@ dis_bp_getBpCount
{
    char *filename = argv[1];

    if (!filename || filename[0] == '\0') {
        sprintf (interp->result, "%d", 0);
    }

    else {
        Boilerplate::load_bplist (filename);
        sprintf (interp->result, "%d", Boilerplate::GetInstanceBpCount());
    }

    return TCL_OK;
}


@@ dis_bp_getBoilerplate
{
    char *filename = argv[1];
    int item = atoi (argv[2]);

    if (strcmp (filename, Boilerplate::GetInstanceBoilerplateType()))
        Boilerplate::load_bplist (filename);

    const char *boilerplate = Boilerplate::GetInstanceBoilerplate (item);
    if (boilerplate)
        sprintf (interp->result, "%s", boilerplate);

    return TCL_OK;
}


@@ dis_bp_getDefaultBoilerplate
{

    char *bpName = argv[1];

    if (strcmp (bpName, Boilerplate::GetInstanceBoilerplateType()))
        Boilerplate::load_bplist (bpName);

    const char *defaultBp = Boilerplate::GetInstanceDefault ();
    if (defaultBp)
        sprintf (interp->result, "%s", defaultBp);

    return TCL_OK;
}


@@ dis_bp_setDefaultBoilerplate
{

    int index = atoi (argv[1]);
    int global = atoi (argv[2]);

    Boilerplate::SetInstanceDefault (index, global);
    return TCL_OK;
}


@@ dis_bp_saveDefaults
{

    Boilerplate::SaveInstanceDefaults ();
    return TCL_OK;
}


@@ dis_bp_restoreDefaults
{

    Boilerplate::RestoreInstanceDefaults ();
    return TCL_OK;
}


@@ dis_bp_removeDefault
{

    Boilerplate::RemoveInstanceDefault ();
    return TCL_OK;
}
