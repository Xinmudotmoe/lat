#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "library_private.h"

#ifdef ANDROID
    const char* libxssName = "libXss.so";
#else
    const char* libxssName = "libXss.so.1";
#endif

#define LIBNAME libxss

#ifdef ANDROID
    #define CUSTOM_INIT \
        setNeededLibs(lib, 2, "libX11.so", "libXext.so");
#else
    #define CUSTOM_INIT \
        setNeededLibs(lib, 2, "libX11.so.6", "libXext.so.6");
#endif

#include "wrappedlib_init.h"
