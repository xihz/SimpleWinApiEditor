// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the WINHOOKKEYLOGGER_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// WINHOOKKEYLOGGER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef WINHOOKKEYLOGGER_EXPORTS
#define WINHOOKKEYLOGGER_API __declspec(dllexport)
#else
#define WINHOOKKEYLOGGER_API __declspec(dllimport)
#endif

void deInitLogger(void);

extern "C"
WINHOOKKEYLOGGER_API LRESULT KeyBoardHookProc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam
);
