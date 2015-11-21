service DISgpi

#include "include/gpi.h"

!ExtApp ExternalApp;

@@ dis_gpi_startup_dialog
{
    ExternalApp.StartupDialog();
    return TCL_OK;
}


@@ dis_gpi_shutdown_dialog
{
    ExternalApp.ShutdownDialog();
    return TCL_OK;
}


@@ dis_gpi_connect
{
    int result = ExternalApp.Connect (atoi (argv[1]));
    sprintf (interp->result, "%d", result);
    return TCL_OK;
}


@@ dis_gpi_disconnect
{
    int result = ExternalApp.Disconnect (atoi (argv[1]));
    sprintf (interp->result, "%d", result);
    return TCL_OK;
}


@@ dis_gpi_status
{
    ExternalApp.AppStatus (atoi (argv[1]));
    return TCL_OK;
}


@@ dis_gpi_getMessages
{
    const int TEMPINCR = 50;
    char *temp;
    int tempSize = TEMPINCR;
    temp = (char*)malloc (sizeof(char) * tempSize);
    temp[0] = '\0';

    for (int i = 0; i < ExternalApp.GetNumMessages(); i++) {
        char *newMessage = ExternalApp.GetMessage (i);
        if (strlen (newMessage) + strlen (temp) + 2 >= tempSize) {
            tempSize += TEMPINCR;
            temp = (char*)realloc (temp, tempSize);
        }
        if (i) strcat (temp, "\n");
        strcat (temp, newMessage);
    }

    Tcl_SetResult (interp, temp, TCL_VOLATILE);
    free (temp);
    return TCL_OK;
}


@@ dis_gpi_clearMessages
{
    ExternalApp.ClearMessages();
    return TCL_OK;
}


@@ dis_gpi_countClients
{
    sprintf (interp->result, "%d", ExternalApp.CountClients());
    return TCL_OK;
}


@@ dis_gpi_isActive
{
    sprintf (interp->result, "%d", ExternalApp.IsActive(atoi (argv[1])));
    return TCL_OK;
}


@@ dis_gpi_getClientStr
{
    Tcl_SetResult (interp, ExternalApp.GetClientStr (atoi(argv[1])), TCL_DYNAMIC);
    return TCL_OK;
}
