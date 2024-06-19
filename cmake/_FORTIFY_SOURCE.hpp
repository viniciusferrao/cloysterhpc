// If in DEBUG mode, remove _FORTIFY_SOURCE
#ifndef NDEBUG
    #ifdef _FORTIFY_SOURCE
        #undef _FORTIFY_SOURCE
    #endif
#else
    #if defined(__GNUC__) && (__GNUC__ < 12)
        #ifdef _FORTIFY_SOURCE
            #if _FORTIFY_SOURCE < 2
                #undef _FORTIFY_SOURCE
                #define _FORTIFY_SOURCE 2
            #endif
        #else
            #define _FORTIFY_SOURCE 2
        #endif
    #else
        #ifdef _FORTIFY_SOURCE
            #if _FORTIFY_SOURCE < 3
                #undef _FORTIFY_SOURCE
                #define _FORTIFY_SOURCE 3
            #endif
        #else
            #define _FORTIFY_SOURCE 3
        #endif
    #endif
#endif
