// strings.cpp

#include "strings.h"

#include <stdarg.h>
#include <stdio.h>

//  Temporary varargs CONS_Printf
//
char* va(char* format, ...)
{
    va_list      argptr;
    static char  string[1024];

    va_start (argptr, format);
    vsprintf (string, format, argptr);
    va_end (argptr);

    return string;
}
