service DISdormant


!#ifdef SERVER
!#include <gRTListServer.h>
!#include "include/DORMANTswApp.h"

!static DORMANTswApp* App = NULL;
!#endif


@@ dis_dormant_startup
{
    if (DORMANTswApp::isRunning() ) {
//        printf("DORMANTsw SERVER:DISdormant already started!!!.\n");
    }else {
	App = new DORMANTswApp;
	App->Startup();
    }

    return TCL_OK;
}



@@ dis_dormant_shutdown
{
    if (DORMANTswApp::isRunning() ) {
	App->Shutdown();
	delete App;
	App = NULL;
    }else {
        printf("DORMANTsw SERVER:DISdormant already shut down!!!.\n");
    }

    return TCL_OK;
}


@@ dis_dormant_update_roots
{
   if(argc != 3) {
       printf("Invalid parameter!!\n");
   } else {
       int rootRTL = atoi(argv[1]);
       int selectionRTL = atoi(argv[2]);
       App->UpdateRoots(rootRTL, selectionRTL);
   }

   return TCL_OK;
}


@@ dis_dormant_extract
{
   if(argc != 2) {
       printf("Invalid parameter!!\n");
   } else {
       int rootRTL = atoi(argv[1]);
       App->Extract(rootRTL);
   }

   return TCL_OK;
}





