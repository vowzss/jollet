#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #ifdef SERRIS_EXPORTS
        #define SERRIS_API __declspec(dllexport)
    #else
        #define SERRIS_API __declspec(dllimport)
    #endif
#else
    #if __GNUC__ >= 4
        #define SERRIS_API __attribute__((visibility("default")))
    #else
        #define SERRIS_API
    #endif
#endif