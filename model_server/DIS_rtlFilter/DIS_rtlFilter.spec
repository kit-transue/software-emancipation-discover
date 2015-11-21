service DISrtlFilter

#ifdef SERVER
#include "../../../clients/DIS_ui/interface.h"
#include "../../../clients/gala/include/gRTListServer.h"
#include "include/rtlFilter.h"
#include "filterButtonTable.h"

#include <string.h>

!extern Application* DISui_app;
!rtlFilter *myRtlFilter;

@@ dis_filterCreate
{

    char *newName = argv[1];
    char *dialogID = argv[2];
    struct rtlFilterSortInfo *s_info;
    struct rtlFilterFormatInfo *f_info;
    char *formatExpression;
    char *filter;
    char *hide;
    char *CLIExpression;

    if (!myRtlFilter) myRtlFilter = new rtlFilter;

    myRtlFilter->genFilterInfo (&argv[3], 
				&s_info, 
				&f_info, 
				&formatExpression,
				&filter, 
				&hide,
				&CLIExpression);
    myRtlFilter->Create (newName, 
                       rtlFilter::compute_sort_spec (s_info), 
                       rtlFilter::compute_format_spec (f_info, formatExpression), 
                       rtlFilter::compute_filter_spec (filter, CLIExpression), 
                       hide);

    genString command;
    command.printf ("dis_gdDialogIDEval {%s} {setFilterList}", dialogID);
    rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());

    delete s_info;
    delete f_info;

    return TCL_OK;
}

@@ dis_filterGetShow
{
    genString sort;
    genString format;
    genString show;
    genString hide;

    genString show_filt, cli_filt, fexpr;

    rtlFilterFormatInfo *f_info = NULL;
    rtlFilterSortInfo *s_info = NULL;

    char *dialogID = argv[1];
    int id = atoi (argv[2]);

    RTListServer* list = RTListServer::find (id);
    if (!list) return TCL_OK;
    
    list->getFilter (sort, format, show, hide);
    rtlFilter::RTL_parse_specs (&f_info, &s_info, format.str(), sort.str(), show.str(), show_filt, cli_filt, fexpr);

    Tcl_SetResult( interp, (char *)(show_filt.str() ? show_filt.str() : ""), TCL_VOLATILE );
    return TCL_OK;
    
}

@@ dis_filterGetHide
{
    genString sort;
    genString format;
    genString show;
    genString hide;

    genString show_filt, cli_filt, fexpr;

    rtlFilterFormatInfo *f_info = NULL;
    rtlFilterSortInfo *s_info = NULL;

    char *dialogID = argv[1];
    int id = atoi (argv[2]);

    RTListServer* list = RTListServer::find (id);
    if (!list) return TCL_OK;
    
    list->getFilter (sort, format, show, hide);
    rtlFilter::RTL_parse_specs (&f_info, &s_info, format.str(), sort.str(), show.str(), show_filt, cli_filt, fexpr);


    Tcl_SetResult (interp, (char *)(hide.str() ? hide.str() : ""), TCL_VOLATILE);
    return TCL_OK;
    
}

@@ dis_filterGetCLI
{
    genString sort;
    genString format;
    genString show;
    genString hide;

    genString show_filt, cli_filt, fexpr;

    rtlFilterFormatInfo *f_info = NULL;
    rtlFilterSortInfo *s_info = NULL;

    char *dialogID = argv[1];
    int id = atoi (argv[2]);

    RTListServer* list = RTListServer::find (id);
    if (!list) return TCL_OK;
    
    list->getFilter (sort, format, show, hide);
    rtlFilter::RTL_parse_specs (&f_info, &s_info, format.str(), sort.str(), show.str(), show_filt, cli_filt, fexpr);


    Tcl_SetResult(interp, (char *)(cli_filt.str() ? cli_filt.str() : ""), TCL_VOLATILE);
    return TCL_OK;
    
}

@@ dis_filterGetFormatSpec
{
    genString sort;
    genString format;
    genString show;
    genString hide;

    genString show_filt, cli_filt, fexpr;

    rtlFilterFormatInfo *f_info = NULL;
    rtlFilterSortInfo *s_info = NULL;

    char *dialogID = argv[1];
    int id = atoi (argv[2]);

    RTListServer* list = RTListServer::find (id);
    if (!list) return TCL_OK;
    
    list->getFilter (sort, format, show, hide);
    rtlFilter::RTL_parse_specs (&f_info, &s_info, format.str(), sort.str(), show.str(), show_filt, cli_filt, fexpr);


    Tcl_SetResult(interp, (char *)(fexpr.str() ? fexpr.str() : ""), TCL_VOLATILE);
    return TCL_OK;
    
}

@@ dis_filterGetSort
{

    genString sort;
    genString format;
    genString show;
    genString hide;
 
    genString show_filt, cli_filt, fexpr;

    rtlFilterFormatInfo *f_info = NULL;
    rtlFilterSortInfo *s_info = NULL;

    char *dialogID = argv[1];
    int id = atoi (argv[2]);

    RTListServer* list = RTListServer::find (id);
    if (!list) return TCL_OK;
    
    list->getFilter (sort, format, show, hide);
    rtlFilter::RTL_parse_specs (&f_info, &s_info, format.str(), sort.str(), show.str(), show_filt, cli_filt, fexpr);

    Tcl_SetResult (interp, (char *)(sort.str() ? rtlFilter::compute_sort_titles (s_info) : ""), TCL_VOLATILE);
    return TCL_OK;
}

@@ dis_filterGetFormat
{
    genString sort;
    genString format;
    genString show;
    genString hide;
 
    genString show_filt, cli_filt, fexpr;

    rtlFilterFormatInfo *f_info = NULL;
    rtlFilterSortInfo *s_info = NULL;

    char *dialogID = argv[1];
    int id = atoi (argv[2]);

    RTListServer* list = RTListServer::find (id);
    if (!list) return TCL_OK;
    
    list->getFilter (sort, format, show, hide);
    rtlFilter::RTL_parse_specs (&f_info, &s_info, format.str(), sort.str(), show.str(), show_filt, cli_filt, fexpr);

    Tcl_SetResult( interp, (char *)(format.str() ? rtlFilter::compute_format_titles (f_info) : ""), TCL_VOLATILE);
    return TCL_OK;
}

@@ dis_filterReset
{
    int id = atoi (argv[1]);

    RTListServer* list = RTListServer::find (id);
    if (!list) return TCL_OK;

    list->resetFilter();
    list->_propagate();
    return TCL_OK;
}

@@ dis_filterApply 
{
    int id = atoi (argv[1]);
    struct rtlFilterSortInfo *s_info;
    struct rtlFilterFormatInfo *f_info;
    char *filter;
    char *hide;
    char *formatExpression;
    char *CLIExpression;

    RTListServer* list = RTListServer::find (id);
    if (!list) return TCL_OK;

    myRtlFilter->genFilterInfo (&argv[2], 
				&s_info, 
				&f_info, 
				&formatExpression, 
				&filter, 
				&hide, 
				&CLIExpression);
    list->setFilter (rtlFilter::compute_sort_spec (s_info), 
                     rtlFilter::compute_format_spec (f_info, formatExpression), 
                     rtlFilter::compute_filter_spec (filter, CLIExpression), 
                     hide);
    list->_propagate();

    delete s_info;
    delete f_info;

    return TCL_OK;
}

@@ dis_filterDelete
{
    int index = atoi (argv[1]);
    if (!myRtlFilter) myRtlFilter = new rtlFilter;
    myRtlFilter->Delete (index);
    return TCL_OK;
}

@@ dis_filterRename
{
    int index = atoi (argv[1]);
    char *newName = argv[2];
    if (!myRtlFilter) myRtlFilter = new rtlFilter;
    myRtlFilter->Rename (index, newName);
    return TCL_OK;
}

@@ dis_filterSave
{
    int index = atoi (argv[1]);
    if (!myRtlFilter) myRtlFilter = new rtlFilter;
    myRtlFilter->Save (index);
    return TCL_OK;
}

@@ dis_filterGetCount
{
    if (!myRtlFilter) myRtlFilter = new rtlFilter;
    sprintf (interp->result, "%d", myRtlFilter->getFilterCount());
    return TCL_OK;
}

@@ dis_filterGetName
{
    int index = atoi (argv[1]);
    if (!myRtlFilter) myRtlFilter = new rtlFilter;
    Tcl_SetResult (interp, (char *)myRtlFilter->getFilterName (index), TCL_VOLATILE);
    return TCL_OK;
}

@@ dis_filterGetCategories
{
    genString categories;
    get_title_list(categories);
    Tcl_SetResult (interp, (char*)categories.str(), TCL_VOLATILE);
    return TCL_OK;
}

#endif

