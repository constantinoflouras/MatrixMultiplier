/*
    File:   debug.h
    Description:
        Contains debug headers, print statements, etc.
*/
#ifndef __DEBUG_H

#define __DEBUG_H

#ifdef DEBUG
#define debug_printf(...)           \
    printf("[%s] ", __func__);      \
    printf(__VA_ARGS__)
#else
#define debug_printf(...)   /*  Compile statement out of the code   */
#endif
#endif
