
#include <windows.h>
#include <process.h> // For _beginthreadex
#include <string>
#include "framework.h"
#include "injector.h"
#include <fstream>
#include <iostream> // For debugging purposes
#include <thread> // For std::thread
#include <unordered_map>
#include <unordered_set>
#include <detours.h>
#pragma comment(lib, "detours.lib")
#include <commctrl.h>
#pragma comment(lib, "Comctl32.lib")

struct info {
    int type, argNum;
    SYSTEMTIME st;
    char argName[10][30] = { 0 };
    char argValue[10][120] = { 0 };
};

// 全局变量:
HWND hWnd;
info recvInfo;
extern info recvInfo;
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HANDLE hSemaphore;
LPVOID lpBase;
HANDLE hMapFile;
HWND hListBox1, hListBox2; // 列表框句柄
BOOL bNewInfo = false; // 新消息标志
SYSTEMTIME lastMessageTime;
int lastType = -1;
int lastSelect = 9999;
BOOL ShowOutput = TRUE;
HWND hOutputText;
BOOL status = FALSE;
wchar_t szFile[MAX_PATH] = L"";
std::unordered_set<int> heapSet;
BOOL MessageHOOK = TRUE, FileHOOK = TRUE, HeapHOOK = FALSE, RegHOOK = TRUE, SocketHOOK = TRUE, ThreadHOOK = TRUE, ProcessHOOK = TRUE;
std::unordered_map<int, const char*> umap = {   {1,"MessageBoxA"},{2,"MessageBoxW"},{3,"CreateFile"},{4,"WriteFile"},{5,"ReadFile"},
                                                {6,"HeapCreate"},{7,"HeapDestory"},{8,"HeapFree"},{9,"RegCreateKeyEX"} ,
                                                {10,"RegSetValueEX"},{11,"RegCloseKey"},{12,"RegOpenKeyEX"},{13,"RegDeleteValue"},
                                                {14,"Socket"},{15,"WSASend"},{16,"WSAConnect"},{17,"WSARecv"},{18,"CreateProcessW"},
                                                {19,"CreateProcessA"},{20,"CreateThread"}, {21,"CopyFile"}, {22,"OpenFile"},{23,"send"},
                                                {24,"connect"},{25,"recv"},{26,"socket"},{27,"DeleteFileW"},{28,"DeleteFileA"},{29,"HeapAlloc"}};

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK CodeRepoDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK FAQDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
unsigned int __stdcall RunExecutable(void* lpParam);
DWORD WINAPI InjectThread(LPVOID lpParam);
void AddrecvInfoToListBox(const info& recvInfo);
void SecurityCheck();
bool ShouldShowReminder();
void UpdateReminderPreference(bool showReminder);
int ShowReminderMessageBox();
LPCWSTR string2LPCWSTR(std::string str);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_INJECTOR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_INJECTOR));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_INJECTOR));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_INJECTOR);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 将实例句柄存储在全局变量中

    hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 920, 550, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    // 创建信号量
    hSemaphore = CreateSemaphore(NULL, 0, 1, L"mySemaphore");
    if (hSemaphore == NULL) {
        // 处理错误
        MessageBoxA(NULL, "信号量初始化错误", "error", MB_OK);
    }
    hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(info), L"ShareMemory");
    lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);

    // 创建标题
    CreateWindowEx(0, L"STATIC", L"Functions", WS_CHILD | WS_VISIBLE | SS_LEFT,
        10, 10, 100, 20, hWnd, nullptr, hInstance, nullptr);

    CreateWindowEx(0, L"STATIC", L"Details", WS_CHILD | WS_VISIBLE | SS_LEFT,
        355, 10, 100, 20, hWnd, nullptr, hInstance, nullptr);

    CreateWindowEx(0, L"STATIC", L"Warnings", WS_CHILD | WS_VISIBLE | SS_LEFT,
        355, 258, 100, 20, hWnd, nullptr, hInstance, nullptr);

    CreateWindowEx(0, L"STATIC", L"Options", WS_CHILD | WS_VISIBLE | SS_LEFT,
        745, 10, 100, 20, hWnd, nullptr, hInstance, nullptr);

    // 创建HOOK列表框控件
    hListBox1 = CreateWindowEx(WS_EX_CLIENTEDGE, L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL,
        10, 35, 320, 400, hWnd, (HMENU)IDC_LISTHOOKBOX, hInstance, NULL);

    // 创建WARNINGS列表框控件
    hListBox2 = CreateWindowEx(WS_EX_CLIENTEDGE, L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
        355, 280, 530, 149, hWnd, (HMENU)IDC_LISTWARNBOX, hInstance, NULL);



    if (!hListBox1)
    {
        return FALSE;
    }

    // 创建输出框
    hOutputText = CreateWindowEx(0, L"STATIC", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_LEFT,
        355, 35, 370, 210, hWnd, (HMENU)IDC_INFOTEXT, hInstance, NULL);

    // 创建输出框2
    hOutputText = CreateWindowEx(0, L"STATIC", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_LEFT,
        745, 35, 140, 210, hWnd, (HMENU)IDC_INFOTEXT, hInstance, NULL);

    // 创建清空HOOK列表框按钮
    CreateWindowEx(0, L"BUTTON", L"Clear", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        112, 10, 50, 20, hWnd, (HMENU)IDC_CLEAR_LIST1_BUTTON, hInstance, NULL);
    // 创建清空WARNINGS列表框按钮
    CreateWindowEx(0, L"BUTTON", L"Clear", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        460, 258, 50, 20, hWnd, (HMENU)IDC_CLEAR_LIST2_BUTTON, hInstance, NULL);

    // 创建复选框
    HWND hCheckBox1 = CreateWindowEx(0, L"BUTTON", L"弹窗截获", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
        750, 40, 100, 20, hWnd, (HMENU)IDC_CHECKBOX1, hInstance, NULL);
    SendMessage(hCheckBox1, BM_SETCHECK, BST_CHECKED, 0); // 设置为选中状态
    SendMessage(hCheckBox1, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

    HWND hCheckBox2 = CreateWindowEx(0, L"BUTTON", L"文件操作截获", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
        750, 78, 120, 20, hWnd, (HMENU)IDC_CHECKBOX2, hInstance, NULL);
    SendMessage(hCheckBox2, BM_SETCHECK, BST_CHECKED, 0); // 设置为选中状态
    SendMessage(hCheckBox2, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

    HWND hCheckBox3 = CreateWindowEx(0, L"BUTTON", L"堆操作截获", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
        750, 116, 100, 20, hWnd, (HMENU)IDC_CHECKBOX3, hInstance, NULL);
    //SendMessage(hCheckBox3, BM_SETCHECK, BST_CHECKED, 0); // 设置为选中状态
    //SendMessage(hCheckBox3, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

    HWND hCheckBox4 = CreateWindowEx(0, L"BUTTON", L"注册表操作截获", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
        750, 148, 120, 20, hWnd, (HMENU)IDC_CHECKBOX4, hInstance, NULL);
    SendMessage(hCheckBox4, BM_SETCHECK, BST_CHECKED, 0); // 设置为选中状态
    SendMessage(hCheckBox4, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

    HWND hCheckBox5 = CreateWindowEx(0, L"BUTTON", L"socket截获", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
        750, 184, 100, 20, hWnd, (HMENU)IDC_CHECKBOX5, hInstance, NULL);
    SendMessage(hCheckBox5, BM_SETCHECK, BST_CHECKED, 0); // 设置为选中状态
    SendMessage(hCheckBox5, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

    HWND hCheckBox6 = CreateWindowEx(0, L"BUTTON", L"线程截获", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
        750, 220, 120, 20, hWnd, (HMENU)IDC_CHECKBOX6, hInstance, NULL);
    SendMessage(hCheckBox6, BM_SETCHECK, BST_CHECKED, 0); // 设置为选中状态
    SendMessage(hCheckBox6, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

    //HWND hCheckBox7 = CreateWindowEx(0, L"BUTTON", L"进程截获", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
    //    780, 330, 120, 20, hWnd, (HMENU)IDC_CHECKBOX7, hInstance, NULL);
    //SendMessage(hCheckBox7, BM_SETCHECK, BST_CHECKED, 0); // 设置为选中状态

    // 获取系统默认字体
    LOGFONT lf;
    GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);

    // 调整字体大小
    lf.lfHeight = -MulDiv(11, GetDeviceCaps(GetDC(hWnd), LOGPIXELSY), 72); // 将 12 修改为所需的字体大小

    // 创建新的字体
    HFONT hFont = CreateFontIndirect(&lf);

    // 设置窗体标题的字体
    SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    // 设置窗体中的所有控件的字体为新的字体
    EnumChildWindows(hWnd, [](HWND hwnd, LPARAM lParam) -> BOOL {
        // 设置子窗口的字体
        SendMessage(hwnd, WM_SETFONT, (WPARAM)lParam, MAKELPARAM(TRUE, 0));
        // 继续枚举下一个子窗口
        return TRUE;
        }, (LPARAM)hFont);

    // 启动定时器
    SetTimer(hWnd, 1, 1, NULL); // 每0.001秒触发一次定时器

    SetWindowText(hWnd, L"Injector - 当前未HOOK");

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    if (ShouldShowReminder()) {
        // 显示提醒消息框
        int result = ShowReminderMessageBox();
        if (result == IDYES) {
            // 用户选择了“不再提醒”，更新提醒首选项
            UpdateReminderPreference(false);
        }
    }
    return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_TIMER:
        // 每0.001秒检查一次信号量状态
        if (WaitForSingleObject(hSemaphore, 10) == WAIT_OBJECT_0) {
            memcpy(&recvInfo, lpBase, sizeof(info));
            // Check if recvInfo.type corresponds to a HOOK operation, if so, skip
            if (MessageHOOK==FALSE && (recvInfo.type == 1 || recvInfo.type == 2)||
                FileHOOK == FALSE && (recvInfo.type == 3 || recvInfo.type == 4 || recvInfo.type == 5 || recvInfo.type == 21 || recvInfo.type == 22 || recvInfo.type == 27 || recvInfo.type == 28) ||
                HeapHOOK == FALSE && (recvInfo.type == 6 || recvInfo.type == 7 || recvInfo.type == 8) ||
                RegHOOK == FALSE && (recvInfo.type == 9 || recvInfo.type == 10 || recvInfo.type == 11 || recvInfo.type == 12 || recvInfo.type == 13) ||
                SocketHOOK == FALSE && (recvInfo.type == 14 || recvInfo.type == 15 || recvInfo.type == 16 || recvInfo.type == 17 || recvInfo.type == 23 || recvInfo.type == 24 || 
                    recvInfo.type == 25 || recvInfo.type == 26) ||
                ThreadHOOK == FALSE && (recvInfo.type == 20) ||
                ProcessHOOK == FALSE && (recvInfo.type == 18 || recvInfo.type == 19)) {
                break;
            }
            // 信号量已被释放，调用 AddrecvInfoToListBox 函数
            AddrecvInfoToListBox(recvInfo);
            // 释放信号量
            ReleaseSemaphore(hSemaphore, 1, NULL);

            // 设置新消息标志
            bNewInfo = true;
        }
        {
            int selectedIndex = SendMessage(hListBox1, LB_GETCURSEL, 0, 0); // Get the index of the selected item
            if (selectedIndex == lastSelect) break;
            if (selectedIndex != LB_ERR)
            {
                lastSelect = selectedIndex;
                // Get the text of the selected item
                char infoString[512];
                SendMessageA(hListBox1, LB_GETTEXT, selectedIndex, (LPARAM)infoString);

                // Set the text of the right text box
                HWND hTextBox = GetDlgItem(hWnd, IDC_INFOTEXT); // Assuming IDC_INFOTEXT is the ID of the right text box
                // Clear the existing text
                SetWindowTextA(hTextBox, "");
                // Set the new text
                SetWindowTextA(hTextBox, infoString);
            }
        }
        break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);
        // 处理复选框消息
        if (wmEvent == BN_CLICKED) {
            // 检查复选框状态并弹出相应提示框
            switch (wmId) {
            case IDC_CHECKBOX1:
                if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX1)) {
                    //MessageBox(hWnd, L"弹窗截获已开启", L"提示", MB_OK | MB_ICONINFORMATION);
                    MessageHOOK = TRUE;
                }
                else {
                    //MessageBox(hWnd, L"弹窗截获已关闭", L"提示", MB_OK | MB_ICONINFORMATION);
                    MessageHOOK = FALSE;
                }
                break;
            case IDC_CHECKBOX2:
                if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX2)) {
                    //MessageBox(hWnd, L"文件操作截获已开启", L"提示", MB_OK | MB_ICONINFORMATION);
                    FileHOOK = TRUE;
                }
                else {
                    //MessageBox(hWnd, L"文件操作截获已关闭", L"提示", MB_OK | MB_ICONINFORMATION);
                    FileHOOK = FALSE;
                }
                break;
            case IDC_CHECKBOX3:
                if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX3)) {
                    //MessageBox(hWnd, L"堆操作截获已开启", L"提示", MB_OK | MB_ICONINFORMATION);
                    HeapHOOK = TRUE;
                }
                else {
                    //MessageBox(hWnd, L"堆操作截获已关闭", L"提示", MB_OK | MB_ICONINFORMATION);
                    HeapHOOK = FALSE;
                }
                break;
            case IDC_CHECKBOX4:
                if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX4)) {
                    //MessageBox(hWnd, L"注册表操作截获已开启", L"提示", MB_OK | MB_ICONINFORMATION);
                    RegHOOK = TRUE;
                }
                else {
                    //MessageBox(hWnd, L"注册表操作截获已关闭", L"提示", MB_OK | MB_ICONINFORMATION);
                    RegHOOK = FALSE;
                }
                break;
            case IDC_CHECKBOX5:
                if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX5)) {
                    //MessageBox(hWnd, L"socket截获已开启", L"提示", MB_OK | MB_ICONINFORMATION);
                    SocketHOOK = TRUE;
                }
                else {
                    //MessageBox(hWnd, L"socket截获已关闭", L"提示", MB_OK | MB_ICONINFORMATION);
                    SocketHOOK = FALSE;
                }
                break;
            case IDC_CHECKBOX6:
                if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX6)) {
                    //MessageBox(hWnd, L"线程截获已开启", L"提示", MB_OK | MB_ICONINFORMATION);
                    ThreadHOOK = TRUE;
                }
                else {
                    //MessageBox(hWnd, L"线程截获已关闭", L"提示", MB_OK | MB_ICONINFORMATION);
                    ThreadHOOK = FALSE;
                }
                break;
            /*case IDC_CHECKBOX7:
                if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX7)) {
                    //MessageBox(hWnd, L"进程截获已开启", L"提示", MB_OK | MB_ICONINFORMATION);
                    ProcessHOOK = TRUE;
                }
                else {
                    //MessageBox(hWnd, L"进程截获已关闭", L"提示", MB_OK | MB_ICONINFORMATION);
                    ProcessHOOK = FALSE;
                }
                break;
                */
            }
        }
        // 分析菜单选择:
        switch (wmId)
        {
        case IDC_CLEAR_LIST1_BUTTON:
        {
            // 清空列表框
            SendMessage(hListBox1, LB_RESETCONTENT, 0, 0);
            HWND hTextBox = GetDlgItem(hWnd, IDC_INFOTEXT);
            SetWindowTextA(hTextBox, "");
            break;
        }
        case IDC_CLEAR_LIST2_BUTTON:
        {
            // 清空列表框
            SendMessage(hListBox2, LB_RESETCONTENT, 0, 0);
            break;
        }
        //case IDC_CLEAR_OUTPUT_BUTTON:
        //{
        //    HWND hTextBox = GetDlgItem(hWnd, IDC_INFOTEXT);
        //    SetWindowTextA(hTextBox, "");
        //    break;
        //}
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case ID_CODEREPO:
        {
            DialogBox(hInst, MAKEINTRESOURCE(IDD_CODE_REPO), hWnd, CodeRepoDlgProc);
            break;
        }
        case ID_FAQ:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_FAQ), hWnd, FAQDlgProc);
            break;
        case IDM_open:
            if (HIWORD(wParam) == BN_CLICKED) {
                // Open file dialog to select an executable
                OPENFILENAME ofn;
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hWnd;
                ofn.lpstrFilter = L"Executable Files (*.exe)\0*.exe\0All Files (*.*)\0*.*\0";
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = MAX_PATH;
                ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
                if (GetOpenFileName(&ofn)) {
                    // Create a new thread to run the selected executable and inject
                    std::wstring* exePath = new std::wstring(szFile);
                    std::thread injectThread(InjectThread, exePath);
                    injectThread.detach(); // Detach the thread to run independently
                }
            }
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: 在此处添加使用 hdc 的任何绘图代码...
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
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

//开源仓库
INT_PTR CALLBACK CodeRepoDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_OPEN_REPO && HIWORD(wParam) == BN_CLICKED)
        {
            // Open the code repository URL
            ShellExecute(NULL, L"open", L"https://gitee.com/less-one/hust-detours", NULL, NULL, SW_SHOWNORMAL);
        }
        break;
    case WM_CLOSE:
        EndDialog(hDlg, 0);
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

//FAQ框
INT_PTR CALLBACK FAQDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_OPEN_FAQ && HIWORD(wParam) == BN_CLICKED)
        {
            // Open the FAQ URL
            ShellExecute(NULL, L"open", L"https://gitee.com/less-one/hust-detours/issues", NULL, NULL, SW_SHOWNORMAL);
        }
        break;
    case WM_CLOSE:
        EndDialog(hDlg, 0);
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

// Function to run the selected executable in a separate thread
unsigned int __stdcall RunExecutable(void* lpParam) {
    std::wstring* exePath = reinterpret_cast<std::wstring*>(lpParam);
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (CreateProcess(NULL, const_cast<LPWSTR>(exePath->c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        // Optional: Wait for the process to finish
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    delete exePath; // Clean up allocated memory
    return 0;
}

DWORD WINAPI InjectThread(LPVOID lpParam) {
    std::wstring* exePath = reinterpret_cast<std::wstring*>(lpParam);
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    si.cb = sizeof(STARTUPINFO);
    WCHAR DirPath[MAX_PATH + 1];
    //wcscpy_s(DirPath, MAX_PATH, L"D:\\SoftwareSecurityCourseDesign\\PFSafetyGuard\\PFDLL\\x64\\Debug");
    //char DLLPath[MAX_PATH + 1] = "D:\\SoftwareSecurityCourseDesign\\PFSafetyGuard\\PFDLL\\x64\\Debug\\PFDLL.dll";
    wcscpy_s(DirPath, MAX_PATH, L"E:\\Desktop\\课程\\软件安全课程设计\\hust-detours\\OurWork\\Dll1\\x64\\Debug");
    char DLLPath[MAX_PATH + 1] = "E:\\Desktop\\课程\\软件安全课程设计\\hust-detours\\OurWork\\Dll1\\x64\\Debug\\Dll1.dll";
    //wcscpy_s(DirPath, MAX_PATH, L"E:\\Desktop\\课程\\软件安全课程设计\\reference\\PFSafetyGuard\\PFDLL\\x64\\Debug");
    //char DLLPath[MAX_PATH + 1] = "E:\\Desktop\\课程\\软件安全课程设计\\reference\\PFSafetyGuard\\PFDLL\\x64\\Debug\\PFDLL.dll";
    //wcscpy_s(DirPath, MAX_PATH, L"D:\\SoftwareSecurityCourseDesign\\PFSafetyGuard\\PFDLL\\x64\\Debug");
    //char DLLPath[MAX_PATH + 1] = "D:\\SoftwareSecurityCourseDesign\\PFSafetyGuard\\PFDLL\\x64\\Debug\\PFDLL.dll";
    LPCWSTR temp = exePath->c_str();

    if (DetourCreateProcessWithDllEx(temp, NULL, NULL, NULL, TRUE, CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED, NULL, DirPath, &si, &pi, DLLPath, NULL))
    {
        // 修改窗口标题以显示连接到的程序名称
        std::wstring windowTitle = L"Injector - 正在HOOK: ";
        windowTitle += exePath->substr(exePath->find_last_of(L"\\") + 1); // 获取程序名称部分
        SetWindowText(hWnd, windowTitle.c_str());
        //MessageBoxA(NULL, "INJECT", "INJECT", NULL);
        if (ResumeThread(pi.hThread) == -1) {
            DWORD dwError = GetLastError();
            LPVOID lpMsgBuf;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, dwError,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&lpMsgBuf, 0, NULL);

            MessageBox(NULL, (LPCTSTR)lpMsgBuf, L"Error", MB_OK | MB_ICONERROR);

            LocalFree(lpMsgBuf);
        }
        WaitForSingleObject(pi.hProcess, INFINITE);
        //清空set
        heapSet.clear();
        SetWindowText(hWnd, L"Injector - 当前未HOOK");
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else
    {
        DWORD dwError = GetLastError();
        LPVOID lpMsgBuf;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, dwError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&lpMsgBuf, 0, NULL);

        MessageBox(NULL, (LPCTSTR)lpMsgBuf, L"Error", MB_OK | MB_ICONERROR);

        LocalFree(lpMsgBuf);
    }
    delete exePath; // Clean up allocated memory
    return 0;
}

// 在接收到信号量后将 recvInfo 的内容添加到列表框中
void AddrecvInfoToListBox(const info& recvInfo)
{
    // 将 recvInfo 的内容格式化为字符串
    char infoString[512];
    sprintf_s(infoString, "Type: %-15s\nTime: %02d:%02d:%02d      \nArgs:\n", umap[recvInfo.type] ,recvInfo.st.wHour, recvInfo.st.wMinute, recvInfo.st.wSecond);
    for (int i = 0; i < recvInfo.argNum; ++i)
    {
        char argInfo[256];
        sprintf_s(argInfo, "%s = %s\n", recvInfo.argName[i], recvInfo.argValue[i]);
        strcat_s(infoString, argInfo);
    }
    if (lastType == recvInfo.type && lastMessageTime.wMinute == recvInfo.st.wMinute && lastMessageTime.wSecond == recvInfo.st.wSecond && lastMessageTime.wMilliseconds == recvInfo.st.wMilliseconds) return;

        lastType = recvInfo.type;
        lastMessageTime = recvInfo.st;
        // 添加到列表框中
        SendMessageA(hListBox1, LB_ADDSTRING, 0, (LPARAM)infoString);
        // 滚动到列表框底部
        SendMessageA(hListBox1, LB_SETTOPINDEX, SendMessageA(hListBox1, LB_GETCOUNT, 0, 0) - 1, 0);
        SecurityCheck();
}

void SecurityCheck() {
    unsigned  temp;
    switch (recvInfo.type)
    {
    case 3: { // createfile
        // 获取写入的文件路径
        std::string filePath = recvInfo.argValue[0];
        // 将路径转换为宽字符
        LPCWSTR wFilePath = string2LPCWSTR(filePath);

        // 获取文件的大小
        HANDLE hFile = CreateFile(wFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD fileSizeHigh;
            DWORD fileSizeLow = GetFileSize(hFile, &fileSizeHigh);
            CloseHandle(hFile);

            // 比较文件大小
            if (fileSizeLow == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) {
                DWORD dwError = GetLastError();
                LPVOID lpMsgBuf;
                FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL, dwError,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR)&lpMsgBuf, 0, NULL);

                MessageBox(NULL, (LPCTSTR)lpMsgBuf, L"Error", MB_OK | MB_ICONERROR);
                break;
            }
            else {
                // 文件大小有效，与当前程序大小进行比较
                WIN32_FILE_ATTRIBUTE_DATA fileAttributes;
                if (GetFileAttributesEx(wFilePath, GetFileExInfoStandard, &fileAttributes)) {
                    ULONGLONG fileSize = (static_cast<ULONGLONG>(fileAttributes.nFileSizeHigh) << 32) | fileAttributes.nFileSizeLow;
                    ULONGLONG currentProgramSize;

                    // 获取当前程序的大小
                    HANDLE hCurrentFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (hCurrentFile != INVALID_HANDLE_VALUE) {
                        DWORD currentFileSizeHigh;
                        DWORD currentFileSizeLow = GetFileSize(hCurrentFile, &currentFileSizeHigh);
                        CloseHandle(hCurrentFile);

                        if (currentFileSizeLow == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) {
                            // 获取当前程序大小失败
                            break;
                        }
                        else {
                            currentProgramSize = (static_cast<ULONGLONG>(currentFileSizeHigh) << 32) | currentFileSizeLow;
                        }
                    }

                    // 比较文件大小
                    if (fileSize == currentProgramSize) {
                        // 文件大小相同，猜测程序在进行自我复制！
                        SendMessageA(hListBox2, LB_ADDSTRING, 0, (LPARAM)"[CreateFile] Program is self - replicating! ");
                    }
                }
            }
        }
        break;
    }
    case 6: {//heapcreate
        temp = strtoul(recvInfo.argValue[3], NULL, 16);//handle
        heapSet.insert(temp);
        break;
    }
    case 8: {//heapfree
        temp = strtoul(recvInfo.argValue[0], NULL, 16);
        if (heapSet.find(temp) == heapSet.end()) {
            // 添加到列表框中
            SendMessageA(hListBox2, LB_ADDSTRING, 0, (LPARAM)"[Free] The heap repeatedly releases or releases a non-existent heap!");
            // 滚动到列表框底部
            SendMessageA(hListBox2, LB_SETTOPINDEX, SendMessageA(hListBox2, LB_GETCOUNT, 0, 0) - 1, 0);
        }
        else {
            heapSet.erase(temp);
        }
        break;
    }
    case 9: {//regcreateex
        if (strstr(recvInfo.argValue[1], "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")) {
            // 添加到列表框中
            SendMessageA(hListBox2, LB_ADDSTRING, 0, (LPARAM)"[EditReg] The program is modifying the startup key in the registry!");
            // 滚动到列表框底部
            SendMessageA(hListBox2, LB_SETTOPINDEX, SendMessageA(hListBox2, LB_GETCOUNT, 0, 0) - 1, 0);
        }
        break;
    }
    case 18: {
        SendMessageA(hListBox2, LB_ADDSTRING, 0, (LPARAM)"[CreateProcess] The program is trying to create process!");
        // 滚动到列表框底部
        SendMessageA(hListBox2, LB_SETTOPINDEX, SendMessageA(hListBox2, LB_GETCOUNT, 0, 0) - 1, 0);
        break;
    }
    case 19: {
        SendMessageA(hListBox2, LB_ADDSTRING, 0, (LPARAM)"[CreateProcess] The program is trying to create process!");
        // 滚动到列表框底部
        SendMessageA(hListBox2, LB_SETTOPINDEX, SendMessageA(hListBox2, LB_GETCOUNT, 0, 0) - 1, 0);
        break;
    }
    case 20: {
        SendMessageA(hListBox2, LB_ADDSTRING, 0, (LPARAM)"[CreateThread] The program is trying to create thread!");
        // 滚动到列表框底部
        SendMessageA(hListBox2, LB_SETTOPINDEX, SendMessageA(hListBox2, LB_GETCOUNT, 0, 0) - 1, 0);
        break;
    }
    case 21:
    {
        SendMessageA(hListBox2, LB_ADDSTRING, 0, (LPARAM)"[CopyFile] Program is self - replicating! ");
        SendMessageA(hListBox2, LB_SETTOPINDEX, SendMessageA(hListBox2, LB_GETCOUNT, 0, 0) - 1, 0);
        break;
    }
    case 27: {
        char mssge[50];
        sprintf_s(mssge, "[DeleteFileW] Program is trying to delete %s !",recvInfo.argValue[0]);
        SendMessageA(hListBox2, LB_ADDSTRING, 0, (LPARAM)mssge);
        SendMessageA(hListBox2, LB_SETTOPINDEX, SendMessageA(hListBox2, LB_GETCOUNT, 0, 0) - 1, 0);
        break;
    }
    case 28: {
        char mssge[50];
        sprintf_s(mssge, "[DeleteFileA] Program is trying to delete %s !", recvInfo.argValue[0]);
        SendMessageA(hListBox2, LB_ADDSTRING, 0, (LPARAM)mssge);
        SendMessageA(hListBox2, LB_SETTOPINDEX, SendMessageA(hListBox2, LB_GETCOUNT, 0, 0) - 1, 0);
        break;
    }
    }
}

bool ShouldShowReminder() {
    std::ifstream configFile("config.ini");
    if (configFile.is_open()) {
        std::string line;
        while (std::getline(configFile, line)) {
            if (line.find("ShowReminder") != std::string::npos) {
                int value = std::stoi(line.substr(line.find("=") + 1));
                return (value == 1); // 如果值为1，则显示提醒，否则不显示
            }
        }
        configFile.close();
    }
    // 默认显示提醒
    return true;
}

void UpdateReminderPreference(bool showReminder) {
    std::ofstream configFile("config.ini", std::ios::app);
    if (configFile.is_open()) {
        configFile << "ShowReminder=" << (showReminder ? "1" : "0") << std::endl;
        configFile.close();
    }
}

int ShowReminderMessageBox() {
    int choice = MessageBoxA(NULL, "建议在使用时关闭: [搜狗拼音]\n是否不再提醒？", "使用建议", MB_YESNO | MB_ICONINFORMATION);
    return choice;
}

LPCWSTR string2LPCWSTR(std::string str)
{
    size_t size = str.length();
    int wLen = ::MultiByteToWideChar(CP_UTF8,
        0,
        str.c_str(),
        -1,
        NULL,
        0);
    wchar_t* buffer = new wchar_t[wLen + 1];
    memset(buffer, 0, (wLen + 1) * sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), size, (LPWSTR)buffer, wLen);
    return buffer;
}