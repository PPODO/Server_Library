#include <windows.h>

#define IDM_WINDOWCHILD 41000


#include "resource.h"
LRESULT CALLBACK MDIWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MDIChildProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
HWND hWndMain;
HWND g_hFrameWnd;
HWND g_hMDIClient;
LPCTSTR lpszClass = TEXT("MDI");
int ChildNum = 1;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) {
    HWND hWnd;
    MSG Message;
    WNDCLASS WndClass;
    g_hInst = hInstance;

    //Frame 윈도우 등록
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndClass.hInstance = hInstance;
    WndClass.lpfnWndProc = MDIWndProc;
    WndClass.lpszClassName = lpszClass;
    WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
    WndClass.style = CS_HREDRAW | CS_VREDRAW;
    RegisterClass(&WndClass);

    hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, (HMENU)NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow);
    g_hFrameWnd = hWnd;
    //차일드 윈도우 등록
    WndClass.lpszClassName = TEXT("MDIExamChild");
    WndClass.lpfnWndProc = MDIChildProc;
    WndClass.hIcon = LoadIcon(NULL, IDC_ARROW);
    WndClass.lpszMenuName = NULL;
    WndClass.cbWndExtra = sizeof(DWORD); //여분메모리
    RegisterClass(&WndClass);
    while (GetMessage(&Message, NULL, 0, 0)) {
        if (TranslateMDISysAccel(g_hMDIClient, &Message))
            TranslateMessage(&Message);
        DispatchMessage(&Message);
    }
    return (int)Message.wParam;
}

HWND hChildWnd[2];
LRESULT CALLBACK MDIWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
    CLIENTCREATESTRUCT ccs;
    MDICREATESTRUCT mcs;
    RECT rectView;
    int a;

    switch (iMessage) {
    case WM_CREATE:
        GetClientRect(hWnd, &rectView);
        hChildWnd[0] = CreateWindowEx(WS_EX_CLIENTEDGE, L"MDIExamChild", NULL, WS_CHILD | WS_VISIBLE, 0, 0, rectView.right, rectView.bottom / 2 - 1, hWnd, NULL, g_hInst, NULL);

        hChildWnd[1] = CreateWindowEx(WS_EX_CLIENTEDGE, L"MDIExamChild", NULL, WS_CHILD | WS_VISIBLE, 0, rectView.bottom / 2 + 1, rectView.right, rectView.bottom / 2 - 1, hWnd, NULL, g_hInst, NULL);

        a = GetLastError();
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_FILE_NEW: //새로운 윈도우 차일드만든다.
            SendMessage(hWnd, WM_CREATE, 0, 0);
            break;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return(DefFrameProc(hWnd, g_hMDIClient, iMessage, wParam, lParam));
}
LRESULT CALLBACK MDIChildProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
    PAINTSTRUCT ps;
    HDC hdc;
    TCHAR str[128];
    switch (iMessage) {
    case WM_CREATE:
        wsprintf(str, TEXT("Child %d"), ChildNum);
        SetWindowLong(hWnd, 0, ChildNum);
        ChildNum++;
        SetWindowText(hWnd, str);
        return 0;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        wsprintf(str, TEXT("This is a MDI %dth Child Window"), GetWindowLong(hWnd, 0));
        TextOut(hdc, 0, 0, str, lstrlen(str));
        EndPaint(hWnd, &ps);
        return 0;
    }
    return (DefMDIChildProc(hWnd, iMessage, wParam, lParam));
}