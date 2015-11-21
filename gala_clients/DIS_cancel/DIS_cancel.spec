service DIScancel

#include <tcl.h>
#include <signal.h>

@@ dis_cancel
{
    if (argc != 2)
	return TCL_ERROR;

    int sid = atoi(argv[1]);

    kill (SIGUSR1, sid);

    return TCL_OK;
}
