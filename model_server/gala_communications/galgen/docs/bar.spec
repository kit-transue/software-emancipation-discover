service bar

#include <stdio.h>

!int bar_global_var;

@ extern "C" int eval (vstr* str)
{
    return bar_app->eval (str);
}

@ int message ()
{
    printf ("here here here\n");
    return 55;
}

@ async computation (int arg)
{
    printf ("The arg is %d\n", arg);
    sleep (5);
    printf ("server computation done\n");
}

@ int print (int arg)
{
    printf ("client - %d\n",arg);

    return 0;
}
