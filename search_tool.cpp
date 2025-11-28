#include <windows.h>
#include <shellapi.h>
#include <dwmapi.h>
#include <imm.h>
#include <string>
#include <iostream>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "imm32.lib")

HWND g_hwnd = NULL;
HINSTANCE g_hInstance = NULL;
bool g_isVisible = false;
const int ID_EDIT = 101;
const int WM_HOTKEY_ID = 1;

const COLORREF BG_COLOR = RGB(45, 45, 48);
const COLORREF TEXT_COLOR = RGB(255, 255, 255);
const COLORREF BORDER_COLOR = RGB(100, 100, 100);
const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 80;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void RegisterHotKey();
void UnregisterHotKey();
void CenterWindow(HWND hwnd);
void EnableModernUI(HWND hwnd);
std::wstring SearchQuery(const std::wstring& query);

const wchar_t* CLASS_NAME = L"SearchToolWindow";

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    g_hInstance = hInstance;
    
    HANDLE hMutex = CreateMutexW(NULL, TRUE, L"Global\\SearchToolMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hMutex);
        return 0;
    }
    
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(BG_COLOR);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    if (!RegisterClassW(&wc)) {
        MessageBoxW(NULL, L"窗口类注册失败！", L"错误", MB_ICONERROR);
        CloseHandle(hMutex);
        return 1;
    }
    
    g_hwnd = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        CLASS_NAME,
        L"搜索工具",
        WS_POPUP,
        0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, NULL
    );
    
    if (!g_hwnd) {
        MessageBoxW(NULL, L"窗口创建失败！", L"错误", MB_ICONERROR);
        return 1;
    }
    
    EnableModernUI(g_hwnd);
    RegisterHotKey();
    
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    UnregisterHotKey();
    CloseHandle(hMutex);
    return (int)msg.wParam;
}

LRESULT CALLBACK EditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    switch (uMsg) {
        case WM_KEYDOWN: {
            if (wParam == VK_RETURN) {
                wchar_t buffer[1024];
                GetWindowTextW(hwnd, buffer, 1024);
                std::wstring query(buffer);
                
                if (!query.empty()) {
                    SearchQuery(query);
                    ShowWindow(GetParent(hwnd), SW_HIDE);
                    g_isVisible = false;
                    SetWindowTextW(hwnd, L"");
                }
                return 0;
            } else if (wParam == VK_ESCAPE) {
                ShowWindow(GetParent(hwnd), SW_HIDE);
                g_isVisible = false;
                SetWindowTextW(hwnd, L"");
                return 0;
            }
            break;
        }
    }
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hEdit = NULL;
    
    switch (uMsg) {
        case WM_CREATE: {
            hEdit = CreateWindowExW(
                WS_EX_CLIENTEDGE,
                L"EDIT",
                L"",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                10, 10, WINDOW_WIDTH - 20, WINDOW_HEIGHT - 20,
                hwnd, (HMENU)ID_EDIT, g_hInstance, NULL
            );
            
            HFONT hFont = CreateFontW(
                32,
                0,
                0,
                0,
                FW_NORMAL,
                FALSE,
                FALSE,
                FALSE,
                DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY,
                DEFAULT_PITCH | FF_DONTCARE,
                L"Microsoft YaHei"
            );
            SendMessageW(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            SetWindowSubclass(hEdit, EditProc, 0, 0);
            
            SetFocus(hEdit);
            break;
        }
        
        case WM_HOTKEY: {
            if (wParam == WM_HOTKEY_ID) {
                if (!g_isVisible) {
                    CenterWindow(hwnd);
                    ShowWindow(hwnd, SW_SHOW);
                    g_isVisible = true;
                    SetWindowTextW(hEdit, L"");
                    SetForegroundWindow(hwnd);
                    SetTimer(hwnd, 2, 50, NULL);
                } else {
                    ShowWindow(hwnd, SW_HIDE);
                    g_isVisible = false;
                }
            }
            break;
        }
        
        case WM_KILLFOCUS: {
            static bool firstFocusLoss = true;
            if (firstFocusLoss) {
                firstFocusLoss = false;
                break;
            }
            SetTimer(hwnd, 1, 100, NULL);
            break;
        }
        
        case WM_TIMER: {
            if (wParam == 1) {
                HWND focused = GetForegroundWindow();
                if (focused != hwnd && focused != hEdit) {
                    ShowWindow(hwnd, SW_HIDE);
                    g_isVisible = false;
                }
                KillTimer(hwnd, 1);
            } else if (wParam == 2) {
                SetFocus(hEdit);
                SendMessageW(hEdit, WM_IME_SETCONTEXT, TRUE, ISC_SHOWUICOMPOSITIONWINDOW | ISC_SHOWUICANDIDATEWINDOW);
                HIMC hIMC = ImmGetContext(hEdit);
                if (hIMC) {
                    ImmSetOpenStatus(hIMC, TRUE);
                    ImmReleaseContext(hEdit, hIMC);
                }
                KillTimer(hwnd, 2);
            }
            break;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HPEN hPen = CreatePen(PS_SOLID, 2, BORDER_COLOR);
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
            
            RECT rect;
            GetClientRect(hwnd, &rect);
            Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
            
            SelectObject(hdc, hOldPen);
            DeleteObject(hPen);
            
            EndPaint(hwnd, &ps);
            break;
        }
        
        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }
        
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    return 0;
}

void RegisterHotKey() {
    if (!g_hwnd) {
        return;
    }
    
    if (RegisterHotKey(g_hwnd, WM_HOTKEY_ID, MOD_ALT, VK_SPACE)) {
        return;
    }
    
    if (RegisterHotKey(g_hwnd, WM_HOTKEY_ID, MOD_ALT, 0x51)) {
        return;
    }
    
    if (RegisterHotKey(g_hwnd, WM_HOTKEY_ID, MOD_CONTROL, VK_SPACE)) {
        return;
    }
    
    RegisterHotKey(g_hwnd, WM_HOTKEY_ID, 0, VK_F8);
}

void UnregisterHotKey() {
    if (g_hwnd) {
        UnregisterHotKey(g_hwnd, WM_HOTKEY_ID);
    }
}

void CenterWindow(HWND hwnd) {
    RECT rect;
    GetWindowRect(hwnd, &rect);
    
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    
    int x = (screenWidth - width) / 2;
    int y = (screenHeight - height) / 2;
    
    SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void EnableModernUI(HWND hwnd) {
    DWMWINDOWATTRIBUTE dwmAttribute = DWMWA_USE_IMMERSIVE_DARK_MODE;
    BOOL value = TRUE;
    DwmSetWindowAttribute(hwnd, dwmAttribute, &value, sizeof(value));
    
    HRGN hRgn = CreateRoundRectRgn(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 12, 12);
    SetWindowRgn(hwnd, hRgn, TRUE);
}

std::wstring UrlEncode(const std::wstring& query) {
    std::wstring encodedQuery;
    for (size_t i = 0; i < query.length(); i++) {
        wchar_t c = query[i];
        if (c == L' ') {
            encodedQuery += L"+";
        } else if ((c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z') || 
                   (c >= L'0' && c <= L'9') || c == L'-' || c == L'_' || 
                   c == L'.' || c == L'~') {
            encodedQuery += c;
        } else {
            char utf8Bytes[4] = {0};
            int byteCount = 0;
            
            if (c <= 0x7F) {
                utf8Bytes[0] = (char)c;
                byteCount = 1;
            } else if (c <= 0x7FF) {
                utf8Bytes[0] = (char)(0xC0 | ((c >> 6) & 0x1F));
                utf8Bytes[1] = (char)(0x80 | (c & 0x3F));
                byteCount = 2;
            } else {
                utf8Bytes[0] = (char)(0xE0 | ((c >> 12) & 0x0F));
                utf8Bytes[1] = (char)(0x80 | ((c >> 6) & 0x3F));
                utf8Bytes[2] = (char)(0x80 | (c & 0x3F));
                byteCount = 3;
            }
            
            for (int j = 0; j < byteCount; j++) {
                wchar_t hexStr[5];
                swprintf_s(hexStr, 5, L"%%%02X", (unsigned char)utf8Bytes[j]);
                encodedQuery += hexStr;
            }
        }
    }
    return encodedQuery;
}

std::wstring SearchQuery(const std::wstring& query) {
    std::wstring encodedQuery = UrlEncode(query);
    std::wstring searchUrl = L"https://www.google.com/search?q=" + encodedQuery;
    
    ShellExecuteW(NULL, L"open", searchUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
    
    return searchUrl;
}