#include "chrono.h"

#ifdef _WIN32
#include <windows.h>

double chrono_maintenant(void) {
    static LARGE_INTEGER frequence = {0};
    LARGE_INTEGER maintenant;
    if (frequence.QuadPart == 0) {
        QueryPerformanceFrequency(&frequence);
    }
    QueryPerformanceCounter(&maintenant);
    return (double) maintenant.QuadPart / (double) frequence.QuadPart;
}

#else
#include <time.h>

double chrono_maintenant(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double) t.tv_sec + (double) t.tv_nsec * 1e-9;
}

#endif
