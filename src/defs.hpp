#ifdef GEODE_IS_WINDOWS
    #ifdef CR_EXPORTING
        #define CR_DLL __declspec(dllexport)
    #else
        #define CR_DLL __declspec(dllimport)
    #endif
#else
    #define CR_DLL
#endif