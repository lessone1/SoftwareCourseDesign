#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <winsock2.h>
#include <filesystem>
#include <iostream>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")
#define PAGE_SIZE	4096

// 函数声明
void headCreateAndDestory();
void writeFileString();
void readFileString();
void regCreateAndSetValue();
void regOpenAndDelValue();
void recvData();
void sendData();
void headRepeatedRelease();
void selfReplication();
void selfReplication2();
void RunTestExample();
void modifyStartupRegistry();

// 窗体全局变量
HWND hwndButton1, hwndButton2, hwndButton3, hwndButton4, hwndButton5, hwndButton6,
hwndButton7, hwndButton8, hwndButton9, hwndButton10, hwndButton11, hwndButton12,
hwndButton13, hwndButton14, hwndButton15, title1, title2;

// 在窗体全局变量中添加静态文本框句柄
HWND hwndStaticText;

// 窗体过程函数
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

void AddFormattedText(const wchar_t* format, ...) {
    // 定义可变参数列表
    va_list args;
    va_start(args, format);

    // 使用 vsnprintf 获取格式化后的字符串
    int length = _vscwprintf(format, args) + 1; // 获取格式化后的字符串长度
    wchar_t* buffer = new wchar_t[length];
    vswprintf(buffer, length, format, args); // 将格式化后的字符串写入缓冲区

    // 关闭可变参数列表
    va_end(args);

    // 获取当前文本框中的文本
    int currentLength = GetWindowTextLength(hwndStaticText);
    wchar_t* currentText = new wchar_t[currentLength + 1];
    GetWindowText(hwndStaticText, currentText, currentLength + 1);

    // 合并当前文本和新的格式化文本
    int totalLength = currentLength + length;
    wchar_t* updatedText = new wchar_t[totalLength + 1];
    wcscpy_s(updatedText, totalLength + 1, currentText);
    wcscat_s(updatedText, totalLength + 1, buffer);

    // 将更新后的文本重新设置到文本框中
    SetWindowText(hwndStaticText, updatedText);

    // 释放内存
    delete[] buffer;
    delete[] currentText;
    delete[] updatedText;
}

void ClearText() {
    SetWindowText(hwndStaticText, L"");
}

// 入口函数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    // 窗体注册
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = L"WindowClass";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, L"Window Registration Failed!", L"Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // 窗体创建
    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, L"WindowClass", L"virus",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 830, 440, NULL, NULL,
        hInstance, NULL);

    if (hwnd == NULL) {
        MessageBox(NULL, L"Window Creation Failed!", L"Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // 显示窗体
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 消息循环
    while (GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}

// 窗体过程函数
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
    {
        // 创建按钮
        hwndButton1 = CreateWindowEx(0, L"BUTTON", L"MessageBoxA弹窗", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            15, 38, 220, 25, hwnd, (HMENU)1, NULL, NULL);
        // 创建其他按钮，位置可根据需要调整
        // 请按照需要添加更多按钮
        hwndButton2 = CreateWindowEx(0, L"BUTTON", L"MessageBoxW弹窗", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            15, 78, 220, 25, hwnd, (HMENU)2, NULL, NULL);
        hwndButton3 = CreateWindowEx(0, L"BUTTON", L"堆：创建 + 销毁", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            15, 118, 220, 25, hwnd, (HMENU)3, NULL, NULL);
        hwndButton4 = CreateWindowEx(0, L"BUTTON", L"文件：写文件", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            15, 158, 220, 25, hwnd, (HMENU)4, NULL, NULL);
        hwndButton5 = CreateWindowEx(0, L"BUTTON", L"文件：读文件", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            15, 198, 220, 25, hwnd, (HMENU)5, NULL, NULL);
        hwndButton6 = CreateWindowEx(0, L"BUTTON", L"注册表：创建 + 设置", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            15, 238, 220, 25, hwnd, (HMENU)6, NULL, NULL);
        hwndButton7 = CreateWindowEx(0, L"BUTTON", L"注册表：打开 + 删除", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            15, 278, 220, 25, hwnd, (HMENU)7, NULL, NULL);
        hwndButton8 = CreateWindowEx(0, L"BUTTON", L"Socket：接受", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            15, 318, 220, 25, hwnd, (HMENU)8, NULL, NULL);
        hwndButton9 = CreateWindowEx(0, L"BUTTON", L"Socket：发送", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            15, 358, 220, 25, hwnd, (HMENU)9, NULL, NULL);
        hwndButton10 = CreateWindowEx(0, L"BUTTON", L"堆二次释放（会闪退）", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            255, 158, 220, 25, hwnd, (HMENU)10, NULL, NULL);
        //hwndButton11 = CreateWindowEx(0, L"BUTTON", L"Modify EXE Program", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        //	10, 410, 220, 25, hwnd, (HMENU)11, NULL, NULL);
        hwndButton12 = CreateWindowEx(0, L"BUTTON", L"WriteFile自我复制", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            255, 38, 220, 25, hwnd, (HMENU)12, NULL, NULL);
        hwndButton13 = CreateWindowEx(0, L"BUTTON", L"CopyFile自我复制", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            255, 78, 220, 25, hwnd, (HMENU)13, NULL, NULL);
        hwndButton14 = CreateWindowEx(0, L"BUTTON", L"开机自启动", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        	255, 198, 220, 25, hwnd, (HMENU)14, NULL, NULL);
        hwndButton15 = CreateWindowEx(0, L"BUTTON", L"运行exe程序", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            255, 118, 220, 25, hwnd, (HMENU)15, NULL, NULL);
        hwndStaticText = CreateWindowEx(0, L"STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_LEFT,
            490, 37, 300, 350, hwnd, NULL, NULL, NULL);
        title1 = CreateWindowEx(0, L"STATIC", L"详情", WS_CHILD | WS_VISIBLE | SS_LEFT,
            490, 10, 100, 20, hwnd, nullptr, NULL, nullptr);
        title2 = CreateWindowEx(0, L"STATIC", L"功能列表", WS_CHILD | WS_VISIBLE | SS_LEFT,
            15, 10, 100, 20, hwnd, nullptr, NULL, nullptr);

        // 创建宋体字体
        HFONT hFont = CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_SWISS, L"宋体-方正超大字符集");

        // 将宋体字体应用到按钮和文本框上
        SendMessage(hwndButton1, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndButton2, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndButton3, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndButton4, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndButton5, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndButton6, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndButton7, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndButton8, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndButton9, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndButton10, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndButton12, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndButton13, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndButton14, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndButton15, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndStaticText, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(title1, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(title2, WM_SETFONT, (WPARAM)hFont, TRUE);
    }
    break;
    case WM_COMMAND:
        ClearText();
        // 按钮单击事件处理
        switch (LOWORD(wParam)) {
        case 1:
            AddFormattedText(L"运行MessageBoxA:\n");
            MessageBoxA(NULL, "I'm MessageBoxA", "I'm MessageBoxA's title", MB_OK);
            break;
        case 2:
            AddFormattedText(L"运行MessageBoxW:\n");
            MessageBoxW(NULL, L"I'm MessageBoxW", L"I'm MessageBoxW's title", MB_OK);
            break;
        case 3:
            AddFormattedText(L"堆创建 + 销毁：\n");
            headCreateAndDestory();
            break;
        case 4:
            AddFormattedText(L"写文件：\n");
            writeFileString();
            break;
        case 5:
            AddFormattedText(L"读文件：\n");
            readFileString();
            break;
        case 6:
            AddFormattedText(L"创建注册表并赋值：\n");
            regCreateAndSetValue();
            break;
        case 7:
            AddFormattedText(L"打开注册表并删除值：\n");
            regOpenAndDelValue();
            break;
        case 8:
            AddFormattedText(L"接收socket：\n");
            recvData();
            break;
        case 9:
            AddFormattedText(L"发送socket：\n");
            sendData();
            break;
        case 10:
            AddFormattedText(L"堆二次释放：\n");
            headRepeatedRelease();
            break;
            //case 11:
            //	modifyExProgram();
            //	break;
        case 12:
            AddFormattedText(L"通过WriteFile自我复制：\n");
            selfReplication();
            break;
        case 13:
            AddFormattedText(L"通过CopyFile自我复制：\n");
            selfReplication2();
            break;
        case 14:
            AddFormattedText(L"修改注册表自启动\n");
            modifyStartupRegistry();
            break;
        case 15:
            AddFormattedText(L"运行EXE程序：\n");
            RunTestExample();
            break;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
        break;
    }
    return 0;
}
void headCreateAndDestory() {
    AddFormattedText(L"Successfully created!\n");
    //SetWindowText(hwndStaticText, L"Press any key to start HeapCreate!\r\n");
    //getchar();
    HANDLE hHeap = HeapCreate(HEAP_NO_SERIALIZE, PAGE_SIZE * 10, PAGE_SIZE * 100);

    int* pArr = (int*)HeapAlloc(hHeap, 0, sizeof(int) * 30);
    for (int i = 0; i < 30; ++i)
    {
        pArr[i] = i + 1;
    }

    AddFormattedText(L"Successfully created!\n");
    Sleep(10);
    AddFormattedText(L"Heap information:");
    for (int i = 0; i < 30; ++i)
    {
        if (i % 5 == 0) AddFormattedText(L"\n ");
        AddFormattedText(L"%3d ", pArr[i]);
        //printf("%3d ", pArr[i]);
    }
    AddFormattedText(L"\n\n");
    AddFormattedText(L"Start HeapFree!\n");
    AddFormattedText(L"Successfully free!\n\n");
    //getchar();
    HeapFree(hHeap, 0, pArr);
    Sleep(10);
    AddFormattedText(L"Start HeapDestory!\n");
    //getchar();
    HeapDestroy(hHeap);

    AddFormattedText(L"Successfully destory!\n");
}
void writeFileString()
{
    CHAR* pBuffer;
    int fileSize = 0;
    char writeString[20] = "HUST CSE!!!";
    bool flag;
    HANDLE hOpenFile = (HANDLE)CreateFile(L"a.txt", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, NULL, NULL);
    if (hOpenFile == INVALID_HANDLE_VALUE)
    {
        hOpenFile = NULL;
        AddFormattedText(L"Can not open the file\n");
        return;
        //MessageBoxA(NULL, "Can not open the file", "Playwav", MB_OK);
    }
    AddFormattedText(L"successfully open the file: a.txt\n");
    Sleep(100);
    //printf("input a string:");
    //scanf("%s", writeString);
    flag = WriteFile(hOpenFile, writeString, strlen(writeString), NULL, NULL);
    if (flag) {
        AddFormattedText(L"successful writed!\n");
    }
    FlushFileBuffers(hOpenFile);
    CloseHandle(hOpenFile);
}
void readFileString() {
    CHAR* pBuffer;
    int fileSize = 0;
    bool flag;
    HANDLE hOpenFile = (HANDLE)CreateFile(L"a.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL);
    if (hOpenFile == INVALID_HANDLE_VALUE)
    {
        hOpenFile = NULL;
        AddFormattedText(L"Can not open the file\n");
        return;
    }
    Sleep(50);
    AddFormattedText(L"successfully open a file\n");
    fileSize = GetFileSize(hOpenFile, NULL);
    pBuffer = (char*)malloc((fileSize + 1) * sizeof(char));
    flag = ReadFile(hOpenFile, pBuffer, fileSize, NULL, NULL);
    Sleep(10);
    pBuffer[fileSize] = 0;
    if (flag) {
        // 将 szBuffer 转换为 wchar_t 类型的字符串
        int requiredSize = MultiByteToWideChar(CP_ACP, 0, pBuffer, -1, NULL, 0);
        wchar_t* wideBuffer = new wchar_t[requiredSize];
        MultiByteToWideChar(CP_ACP, 0, pBuffer, -1, wideBuffer, requiredSize);
        AddFormattedText(L"successfully read a string:%s!\n", wideBuffer);
    }
    free(pBuffer);
    CloseHandle(hOpenFile);
}
void regCreateAndSetValue() {
    // 创建注册表并设置键值
    HKEY hKey = NULL;
    TCHAR Data[254];
    memset(Data, 0, sizeof(Data));
    wcsncpy_s(Data, TEXT("test example!!"), 254);

    size_t lRet = RegCreateKeyEx(HKEY_CURRENT_USER, (LPWSTR)L"aaaMykey", 0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL, &hKey, NULL);
    if (lRet == ERROR_SUCCESS) {
        AddFormattedText(L"create successfully!\n");
    }
    else {
        AddFormattedText(L"failed to create!\n");
    }
    Sleep(10);
    // 修改注册表键值，没有则创建它
    size_t iLen = wcslen(Data);
    // 设置键值
    lRet = RegSetValueEx(hKey, L"panfeng", 0, REG_SZ, (CONST BYTE*)Data, sizeof(TCHAR) * iLen);
    if (lRet == ERROR_SUCCESS)
    {
        AddFormattedText(L"set value successfully!\n");
        return;
    }
    else {
        AddFormattedText(L"failed to set value!\n");
    }
    Sleep(10);
    RegCloseKey(hKey);
}
void regOpenAndDelValue() {
    HKEY hKey = NULL;
    size_t lRet = RegOpenKeyEx(HKEY_CURRENT_USER, (LPWSTR)L"aaaMykey", 0, KEY_ALL_ACCESS, &hKey);
    if (lRet == ERROR_SUCCESS) {
        AddFormattedText(L"open successfully!\n");
    }
    else {
        AddFormattedText(L"open failed\n");
    }
    Sleep(10);
    lRet = RegDeleteValue(hKey, L"panfeng");
    if (lRet == ERROR_SUCCESS) {
        AddFormattedText(L"delete success!\n");
    }
    else {
        AddFormattedText(L"delete fail!\n");
    }
    Sleep(10);
    RegCloseKey(hKey);
}
void headRepeatedRelease() {
    AddFormattedText(L"Start HeapCreate!\n");
    //getchar();
    HANDLE hHeap = HeapCreate(HEAP_NO_SERIALIZE, PAGE_SIZE * 10, PAGE_SIZE * 100);

    int* pArr = (int*)HeapAlloc(hHeap, 0, sizeof(int) * 30);
    for (int i = 0; i < 30; ++i)
    {
        pArr[i] = i + 1;
    }
    AddFormattedText(L"Successfully created!\n");
    Sleep(10);
    for (int i = 0; i < 30; ++i)
    {
        if (i % 5 == 0)
            printf_s("\n");
        AddFormattedText(L"%3d ", pArr[i]);
    }
    AddFormattedText(L"\n\n");
    AddFormattedText(L"Start the first HeapFree!\n");
    //getchar();
    HeapFree(hHeap, 0, pArr);
    Sleep(10);
    AddFormattedText(L"Start the second HeapFree!\n");
    //getchar();
    HeapFree(hHeap, 0, pArr);
    Sleep(10);
    AddFormattedText(L"Destroy the heap!\n");
    //getchar();
    HeapDestroy(hHeap);
}
/*
void modifyExProgram() {
    HANDLE hOpenFile = (HANDLE)CreateFile(L"a.exe", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, NULL, NULL);
    CloseHandle(hOpenFile);
}
*/
void selfReplication() {

    // 获取当前可执行文件的路径
    wchar_t szFilePath[MAX_PATH];
    GetModuleFileName(NULL, szFilePath, MAX_PATH);
    Sleep(10);
    // 拼接目标文件路径，假设目标路径为 D 盘根目录下的 "app.exe"
    wchar_t szDestPath[MAX_PATH];
    swprintf(szDestPath, MAX_PATH, L"D:\\app.exe");

    // 打开当前可执行文件，并读取其内容
    HANDLE hSrcFile = CreateFile(szFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSrcFile == INVALID_HANDLE_VALUE) {
        AddFormattedText(L"Failed to open source file.\n");
        return;
    }
    Sleep(30);
    // 创建目标文件，以进行复制
    HANDLE hDestFile = CreateFile(szDestPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDestFile == INVALID_HANDLE_VALUE) {
        AddFormattedText(L"Failed to create destination file.\n");
        CloseHandle(hSrcFile);
        return;
    }
    Sleep(100);
    // 读取并写入源文件内容
    BYTE buffer[4096 * 4];
    DWORD dwBytesRead = 0, dwBytesWritten = 0;
    while (ReadFile(hSrcFile, buffer, sizeof(buffer), &dwBytesRead, NULL) && dwBytesRead > 0) {
        WriteFile(hDestFile, buffer, dwBytesRead, &dwBytesWritten, NULL);
    }
    Sleep(10);
    // 关闭文件句柄
    CloseHandle(hSrcFile);
    CloseHandle(hDestFile);

    AddFormattedText(L"WriteFile: 自我复制成功到D盘根目录成功！\n");
}

void selfReplication2() {
    //自我复制
    wchar_t src[MAX_PATH];	//缓冲区
    wchar_t dest[MAX_PATH] = L"D://app.exe";
    //获取程序本身的路径
    GetModuleFileName(NULL, src, MAX_PATH);
    wchar_t tmp[MAX_PATH];
    int id;
    //提取后缀名
    for (id = wcslen(src) - 1; id >= 0 && src[id] != L'.'; --id);
    wcsncpy(tmp, src + id, MAX_PATH);
    //提取文件名
    Sleep(30);
    CopyFileW(src, dest, FALSE);		// 复制文件
    AddFormattedText(L"CopyFile: 自我复制成功到D盘根目录成功！\n");
}


void modifyStartupRegistry() {
    HKEY hKey = NULL;
    TCHAR Data[MAX_PATH];
    GetModuleFileName(NULL, Data, MAX_PATH); // 获取程序的完整路径

    // 创建并打开注册表项
    size_t lRet = RegCreateKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL, &hKey, NULL);
    if (lRet == ERROR_SUCCESS) {
        AddFormattedText(L"创建注册表项成功!\n");
    }
    else {
        AddFormattedText(L"创建注册表项失败!\n");
    }
    Sleep(10);

    // 设置键值为程序路径
    lRet = RegSetValueEx(hKey, L"MyProgram", 0, REG_SZ, (CONST BYTE*)Data, sizeof(TCHAR) * wcslen(Data));
    if (lRet == ERROR_SUCCESS) {
        AddFormattedText(L"设置键值成功!\n");
        return;
    }
    else {
        AddFormattedText(L"设置键值失败!\n");
    }
    Sleep(10);
    RegCloseKey(hKey); // 关闭注册表项句柄
}
/*
void recvData() {
    //初始化DLL
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    //创建套接字
    SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    //向服务器发起请求
    sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));  //每个字节都用0填充
    sockAddr.sin_family = PF_INET;
    sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    sockAddr.sin_port = htons(1234);
    connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
    Sleep(500);
    //接收服务器传回的数据
    char szBuffer[MAXBYTE] = { 0 };
    recv(sock, szBuffer, MAXBYTE, NULL);
    //输出接收到的数据
    printf("Message form server: %s\n", szBuffer);
    //关闭套接字
    closesocket(sock);
    //终止使用 DLL
    WSACleanup();
}
void sendData() {
    //初始化 DLL
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    //创建套接字
    SOCKET servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    //绑定套接字
    sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));  //每个字节都用0填充
    sockAddr.sin_family = PF_INET;  //使用IPv4地址
    sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  //具体的IP地址
    sockAddr.sin_port = htons(1234);  //端口
    bind(servSock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
    //进入监听状态
    listen(servSock, 20);
    //接收客户端请求
    SOCKADDR clntAddr;
    int nSize = sizeof(SOCKADDR);
    SOCKET clntSock = accept(servSock, (SOCKADDR*)&clntAddr, &nSize);
    //向客户端发送数据
    char str[32] = "Hello World!";
    send(clntSock, str, strlen(str) + sizeof(char), NULL);
    //关闭套接字
    closesocket(clntSock);
    closesocket(servSock);
    //终止 DLL 的使用
    WSACleanup();
}
*/

DWORD WINAPI recvDataThread(LPVOID lpParameter) {
    // 初始化 DLL
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        AddFormattedText(L"WSAStartup failed\n");
        return 1; // 返回值为非零表示错误
    }
    Sleep(10);
    // 创建套接字
    SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    if (sock == INVALID_SOCKET) {
        AddFormattedText(L"Failed to create socket\n");
        WSACleanup();
        return 1;
    }
    Sleep(10);
    // 向服务器发起请求
    sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    sockAddr.sin_port = htons(1234);
    if (WSAConnect(sock, (SOCKADDR*)&sockAddr, sizeof(sockAddr), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
        Sleep(10);
        AddFormattedText(L"Failed to connect to server\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    Sleep(500); // 暂停 500 毫秒

    // 接收服务器传回的数据
    char szBuffer[MAXBYTE] = { 0 };
    DWORD dwBytesReceived;
    WSABUF DataBuf;
    DataBuf.buf = szBuffer;
    DataBuf.len = MAXBYTE;
    DWORD dwFlags = 0;
    WSAOVERLAPPED Overlapped;
    memset(&Overlapped, 0, sizeof(Overlapped));
    if (WSARecv(sock, &DataBuf, 1, &dwBytesReceived, &dwFlags, &Overlapped, NULL) == SOCKET_ERROR) {
        AddFormattedText(L"Failed to receive data from server\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    Sleep(10);
    // 输出接收到的数据
    // 将 szBuffer 转换为 wchar_t 类型的字符串
    int requiredSize = MultiByteToWideChar(CP_ACP, 0, szBuffer, -1, NULL, 0);
    wchar_t* wideBuffer = new wchar_t[requiredSize];
    MultiByteToWideChar(CP_ACP, 0, szBuffer, -1, wideBuffer, requiredSize);

    // 使用转换后的字符串输出
    AddFormattedText(L"Message from server: %s\n", wideBuffer);

    // 关闭套接字
    closesocket(sock);
    Sleep(10);
    // 终止使用 DLL
    WSACleanup();
    return 0;
}

void recvData() {
    // 创建新线程
    HANDLE hThread = CreateThread(NULL, 0, recvDataThread, NULL, 0, NULL);
    if (hThread == NULL) {
        AddFormattedText(L"Failed to create thread\n");
        return;
    }

    // 关闭线程句柄
    CloseHandle(hThread);
}


DWORD WINAPI sendDataThread(LPVOID lpParameter) {
    // 与原来的 sendData 函数相同的内容
    // 初始化 DLL
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        AddFormattedText(L"WSAStartup failed\n");
        return 0;
    }
    Sleep(10);
    // 创建套接字
    SOCKET servSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    if (servSock == INVALID_SOCKET) {
        AddFormattedText(L"Failed to create socket\n");
        WSACleanup();
        return 0;
    }
    Sleep(10);
    // 绑定套接字
    sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    sockAddr.sin_port = htons(1234);
    if (bind(servSock, (SOCKADDR*)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR) {
        AddFormattedText(L"Failed to bind socket\n");
        closesocket(servSock);
        WSACleanup();
        return 0;
    }
    Sleep(10);
    // 进入监听状态
    if (listen(servSock, 20) == SOCKET_ERROR) {
        AddFormattedText(L"listen failed\n");
        closesocket(servSock);
        WSACleanup();
        return 0;
    }
    Sleep(10);
    // 接收客户端请求
    SOCKADDR clntAddr;
    int nSize = sizeof(SOCKADDR);
    SOCKET clntSock = accept(servSock, (SOCKADDR*)&clntAddr, &nSize);
    if (clntSock == INVALID_SOCKET) {
        AddFormattedText(L"accept failed\n");
        closesocket(servSock);
        WSACleanup();
        return 0;
    }
    Sleep(10);
    // 向客户端发送数据
    char str[32] = "Hello HUST CSE";
    DWORD dwBytesSent;
    WSABUF DataBuf;
    DataBuf.buf = str;
    DataBuf.len = strlen(str) + sizeof(char);
    DWORD dwFlags = 0;
    WSAOVERLAPPED Overlapped;
    memset(&Overlapped, 0, sizeof(Overlapped));
    if (WSASend(clntSock, &DataBuf, 1, &dwBytesSent, dwFlags, &Overlapped, NULL) == SOCKET_ERROR) {
        AddFormattedText(L"send failed\n");
        closesocket(clntSock);
        closesocket(servSock);
        WSACleanup();
        return 0;
    }
    Sleep(10);
    AddFormattedText(L"SendData: Hello HUST CSE\n");
    AddFormattedText(L"Data sent successfully\n");

    // 关闭套接字
    closesocket(clntSock);
    closesocket(servSock);
    WSACleanup();
    return 0;
}

void sendData() {
    // 创建新线程
    HANDLE hThread = CreateThread(NULL, 0, sendDataThread, NULL, 0, NULL);
    if (hThread == NULL) {
        AddFormattedText(L"Failed to create thread\n");
        return;
    }
    // 关闭线程句柄
    CloseHandle(hThread);
}

/*
void memoryOperation() {
    //getchar();
    char temp[100] = "";
    AddFormattedText(L"Copy memory start!\n");
    //getchar();
    //memccpy(temp, "hello\n", 6);
    memcpy(temp, "hello\n", 6);
    AddFormattedText(L"%s", temp);
    AddFormattedText(L"Move memory start!\n");
    //getchar();
    memmove(temp, "world\n", 6);
    AddFormattedText(L"%s", temp);
}
*/


void RunTestExample() {
    // 创建进程信息结构
    PROCESS_INFORMATION pi;
    // 创建启动信息结构
    STARTUPINFO si;
    // 初始化启动信息结构
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);

    // 设置 D 盘目录下的 app.exe 的路径
    wchar_t szPath[MAX_PATH];
    swprintf(szPath, MAX_PATH, L"D:\\app.exe");
    AddFormattedText(L"运行路径：D:\\app.exe\n");
    // 创建进程
    if (!CreateProcessW(NULL, szPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        // 创建进程失败
        AddFormattedText(L"Failed to create process: %d\n", GetLastError());
        return;
    }

    // 关闭进程和线程句柄，防止资源泄漏
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    AddFormattedText(L"Process created successfully.\n");
}