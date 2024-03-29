#include "pch.h"
#include "framework.h"
#include "detours.h"
#include "stdio.h"
#include "stdarg.h"
#include "windows.h"
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <processthreadsapi.h>
#define MESSAGEBOXA 1
#define MESSAGEBOXW 2
#define CREATEFILE 3
#define WRITEFILE 4
#define READFILE 5
#define HEAPCREATE 6
#define HEAPDESTORY 7
#define HEAPFREE 8
#define REGCREATEKEYEX 9
#define REGSETVALUEEX 10
#define REGCLOSEKEY 11
#define REGOPENKEYEX 12
#define REGDELETEVALUE 13
#define WSASOCKET 14
#define WSASEND 15
#define WSACONNECT 16
#define WSARECV 17
#define CREATEPROCESSW 18
#define CREATEPROCESSA 19
#define CREATETHREAD 20
#define COPYFILE 21
#define OPENFILE 22
#define SEND 23
#define CONNECT 24
#define RECV 25
#define MYSOCKET 26
#define DELETEFILEW 27
#define DELETEFILEA 28
#pragma comment(lib,"detours.lib")
#pragma comment(lib,"ws2_32.lib")

struct info {
    int type, argNum;
    SYSTEMTIME st;
    char argName[10][30] = { 0 };
    char argValue[10][120] = { 0 };
};

SYSTEMTIME st;

info sendInfo;
HANDLE hSemaphore = OpenSemaphore(EVENT_ALL_ACCESS, FALSE, L"mySemaphore");
HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, L"ShareMemory");
LPVOID lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(info));

// 定义和引入需要HOOK的函数和替换的函数
static int (WINAPI* OldMessageBoxW)(_In_opt_ HWND hWnd, _In_opt_ LPCWSTR lpText, _In_opt_ LPCWSTR lpCaption, _In_ UINT uType) = MessageBoxW;
static int (WINAPI* OldMessageBoxA)(_In_opt_ HWND hWnd, _In_opt_ LPCSTR lpText, _In_opt_ LPCSTR lpCaption, _In_ UINT uType) = MessageBoxA;

extern "C" __declspec(dllexport) int WINAPI NewMessageBoxA(_In_opt_ HWND hWnd, _In_opt_ LPCSTR lpText, _In_opt_ LPCSTR lpCaption, _In_ UINT uType)
{
    info sendInfo;
    HANDLE hSemaphore = OpenSemaphore(EVENT_ALL_ACCESS, FALSE, L"mySemaphore");
    HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, L"ShareMemory");
    LPVOID lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(info));
    // 向sendInfo中写入数据
    sendInfo.type = 1;
    GetLocalTime(&(sendInfo.st));
    sendInfo.argNum = 4;
    // 参数名
    sprintf(sendInfo.argName[0], "hWnd");
    sprintf(sendInfo.argName[1], "lpText");
    sprintf(sendInfo.argName[2], "lpCaption");
    sprintf(sendInfo.argName[3], "uType");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", hWnd);
    sprintf(sendInfo.argValue[1], "%s", lpText);
    sprintf(sendInfo.argValue[2], "%s", lpCaption);
    sprintf(sendInfo.argValue[3], "%08X", uType);
    if (hSemaphore == NULL) {
        MessageBoxW(NULL, L"Failed to open semaphore.", L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }
    if (hMapFile == NULL) {
        DWORD dwError = GetLastError();
        wchar_t errorMessage[256];
        swprintf(errorMessage, 256, L"Failed to open file mapping object. Error code: %lu", dwError);
        MessageBoxW(NULL, errorMessage, L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }
    if (lpBase == NULL) {
        MessageBoxW(NULL, L"Failed to open lpBase.", L"Error", MB_OK | MB_ICONERROR);
        CloseHandle(hMapFile);
        return -1;
    }
    // 将sendinfo赋值到共享内存
    memcpy(lpBase, &sendInfo, sizeof(info));
    // 进行V操作，使得信号量+1
    ReleaseSemaphore(hSemaphore, 1, NULL);
    sendInfo.argNum = 0;
    // 返回原始接口
    return OldMessageBoxA(hWnd, lpText, lpCaption, uType);
}
extern "C" __declspec(dllexport) int WINAPI NewMessageBoxW(_In_opt_ HWND hWnd, _In_opt_ LPCWSTR lpText, _In_opt_ LPCWSTR lpCaption, _In_ UINT uType)
{
    char temp[70];
    sendInfo.type = MESSAGEBOXW;
    GetLocalTime(&(sendInfo.st));

    sendInfo.argNum = 4;
    // 参数名
    sprintf(sendInfo.argName[0], "hWnd");
    sprintf(sendInfo.argName[1], "lpText");
    sprintf(sendInfo.argName[2], "lpCaption");
    sprintf(sendInfo.argName[3], "uType");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", hWnd);
    // 宽字节转char
    memset(temp, 0, sizeof(temp));
    WideCharToMultiByte(CP_ACP, 0, lpText, wcslen(lpText), temp, sizeof(temp), NULL, NULL);
    strcpy(sendInfo.argValue[1], temp);
    // 宽字节转char
    memset(temp, 0, sizeof(temp));
    WideCharToMultiByte(CP_ACP, 0, lpCaption, wcslen(lpCaption), temp, sizeof(temp), NULL, NULL);
    strcpy(sendInfo.argValue[2], temp);

    sprintf(sendInfo.argValue[3], "%08X", uType);

    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);

    sendInfo.argNum = 0;
    return OldMessageBoxW(hWnd, lpText, lpCaption, uType);
}
// 文件操作 OpenFile CreateFile

static	HFILE(WINAPI* OldOpenFile)(
    LPCSTR lpFileName,
    LPOFSTRUCT lpReOpenBuff,
    UINT	uStyle
    ) = OpenFile;

extern "C" __declspec(dllexport) HFILE WINAPI NewOpenFile(
    LPCSTR lpFileName,
    LPOFSTRUCT lpReOpenBuff,
    UINT uStyle
)
{
    HFILE hFile = OldOpenFile(lpFileName, lpReOpenBuff, uStyle);
        sendInfo.type = OPENFILE;
        GetLocalTime(&(sendInfo.st));

        sendInfo.argNum = 4;
        // 参数名
        sprintf(sendInfo.argName[0], "lpFileName");
        sprintf(sendInfo.argName[1], "lpReOpenBuff");
        sprintf(sendInfo.argName[2], "uStyle");
        sprintf(sendInfo.argName[3], "Path:");

        sprintf(sendInfo.argValue[0], "%08X", lpFileName);
        sprintf(sendInfo.argValue[1], "%08X", lpReOpenBuff);
        sprintf(sendInfo.argValue[2], "%08X", uStyle);
        char FilePath[MAX_PATH]; // 使用MAX_PATH来保证路径长度
        strncpy(FilePath, lpFileName, MAX_PATH); // 将文件路径复制到FilePath变量中
        FilePath[MAX_PATH - 1] = '\0'; // 确保字符串以null结尾
        sprintf(sendInfo.argValue[3], "%s", FilePath);

        memcpy(lpBase, &sendInfo, sizeof(info));
        ReleaseSemaphore(hSemaphore, 1, NULL);
    return hFile;
}

static HANDLE(WINAPI* OldCreateFile)(
    LPCTSTR lpFileName,							// 文件名
    DWORD dwDesireAccess,						// 访问模式
    DWORD dwShareMode,							// 共享模式
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,	// 安全模式(即销毁方式)
    DWORD dwCreationDisposition,				// how to create
    DWORD dwFlagAndAttributes,					// 文件属性
    HANDLE hTemplateFile						// 模块文件句柄
    ) = CreateFile;

extern "C" __declspec(dllexport)HANDLE WINAPI NewCreateFile(
    LPCTSTR lpFileName,							// 文件名
    DWORD dwDesireAccess,						// 访问模式
    DWORD dwShareMode,							// 共享模式
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,	// 安全模式(即销毁方式)
    DWORD dwCreationDisposition,				// how to create
    DWORD dwFlagAndAttributes,					// 文件属性
    HANDLE hTemplateFile						// 模块文件句柄
)
{
    HANDLE hFile = OldCreateFile(lpFileName, dwDesireAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagAndAttributes, hTemplateFile);
    if (GetFileType(hFile) == FILE_TYPE_DISK) {
        char temp[120];
        sendInfo.type = CREATEFILE;
        GetLocalTime(&(sendInfo.st));

        sendInfo.argNum = 8;
        // 参数名
        sprintf(sendInfo.argName[0], "lpFileName");
        sprintf(sendInfo.argName[1], "dwDesiredAccess");
        sprintf(sendInfo.argName[2], "dwShareMode");
        sprintf(sendInfo.argName[3], "lpSecurityAttributes");
        sprintf(sendInfo.argName[4], "dwCreationDisposition");
        sprintf(sendInfo.argName[5], "dwFlagsAndAttributes");
        sprintf(sendInfo.argName[6], "hTemplateFile");
        sprintf(sendInfo.argName[7], "Path:");
        // 参数值
        // 宽字节转char
        memset(temp, 0, sizeof(temp));

        WideCharToMultiByte(CP_ACP, 0, lpFileName, wcslen(lpFileName), temp, sizeof(temp), NULL, NULL);
        strcpy(sendInfo.argValue[0], temp);
        sprintf(sendInfo.argValue[1], "%08X", dwDesireAccess);
        sprintf(sendInfo.argValue[2], "%08X", dwShareMode);
        sprintf(sendInfo.argValue[3], "%08X", lpSecurityAttributes);
        sprintf(sendInfo.argValue[4], "%08X", dwCreationDisposition);
        sprintf(sendInfo.argValue[5], "%08X", dwFlagAndAttributes);
        sprintf(sendInfo.argValue[6], "%08X", hTemplateFile);
        sprintf(sendInfo.argValue[7], "%s", temp);

        memcpy(lpBase, &sendInfo, sizeof(info));
        ReleaseSemaphore(hSemaphore, 1, NULL);
    }
    return hFile;
}

static BOOL(WINAPI* OldWriteFile)(
    HANDLE       hFile,
    LPCVOID      lpBuffer,
    DWORD        nNumberOfBytesToWrite,
    LPDWORD      lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped
    ) = WriteFile;

extern "C" __declspec(dllexport)BOOL WINAPI NewWriteFile(
    HANDLE       hFile,
    LPCVOID      lpBuffer,
    DWORD        nNumberOfBytesToWrite,
    LPDWORD      lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped
)
{
    //MessageBoxW(NULL, L"writefile.", L"Debug", MB_OK | MB_ICONERROR);
    if (GetFileType(hFile) == FILE_TYPE_DISK) {
        //MessageBoxW(NULL, L"writefile!", L"Debug", MB_OK | MB_ICONERROR);
        sendInfo.argNum = 6;
        // 参数名
        sprintf(sendInfo.argName[0], "hFile");
        sprintf(sendInfo.argName[1], "lpBuffer");
        sprintf(sendInfo.argName[2], "nNumberOfBytesToWrite");
        sprintf(sendInfo.argName[3], "lpNumberOfBytesWritten");
        sprintf(sendInfo.argName[4], "lpOverlapped");
        sprintf(sendInfo.argName[5], "FilePath");
        // 参数值
        sprintf(sendInfo.argValue[0], "%08X", hFile);
        sprintf(sendInfo.argValue[1], "%08X", lpBuffer);
        sprintf(sendInfo.argValue[2], "%08X", nNumberOfBytesToWrite);
        sprintf(sendInfo.argValue[3], "%08X", lpNumberOfBytesWritten);
        sprintf(sendInfo.argValue[4], "%08X", lpOverlapped);

        // 获取文件路径
        char szFilePath[MAX_PATH];
        if (GetFinalPathNameByHandleA(hFile, szFilePath, MAX_PATH, 0) != 0) {
            strcpy_s(sendInfo.argValue[5], MAX_PATH, szFilePath);
        }
        else {
            strcpy_s(sendInfo.argValue[5], MAX_PATH, "Unknown");
        }

        sendInfo.type = WRITEFILE;
        GetLocalTime(&(sendInfo.st));
        memcpy(lpBase, &sendInfo, sizeof(info));
        ReleaseSemaphore(hSemaphore, 1, NULL);
    }
    return OldWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

BOOL(WINAPI* OldCopyFile)(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    BOOL    bFailIfExists
    ) = CopyFile;

// Function prototype for the new CopyFile function
BOOL WINAPI NewCopyFile(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    BOOL    bFailIfExists
)
{
    // Call the original CopyFile function and store its return value
    BOOL result = OldCopyFile(lpExistingFileName, lpNewFileName, bFailIfExists);
    //MessageBoxW(NULL, L"CopyFile.", L"Debug", MB_OK | MB_ICONERROR);
    sendInfo.argNum = 3;
    // 参数名
    sprintf(sendInfo.argName[0], "lpExistingFileName,");
    sprintf(sendInfo.argName[1], "lpNewFileName");
    sprintf(sendInfo.argName[2], "bFailIfExists");
    // 参数值
    wchar_t existingFileName[MAX_PATH];
    wchar_t newFileName[MAX_PATH];
    wcscpy_s(existingFileName, MAX_PATH, lpExistingFileName);
    wcscpy_s(newFileName, MAX_PATH, lpNewFileName);

    sprintf(sendInfo.argValue[0], "%ls", existingFileName);
    sprintf(sendInfo.argValue[1], "%ls", newFileName);
    sprintf(sendInfo.argValue[2], "%d", bFailIfExists);
    sendInfo.type = COPYFILE;
    GetLocalTime(&(sendInfo.st));
    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);
    return result;
}

static BOOL(WINAPI* OldReadFile)(
    HANDLE       hFile,
    LPVOID       lpBuffer,
    DWORD        nNumberOfBytesToRead,
    LPDWORD      lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
    ) = ReadFile;

extern "C" __declspec(dllexport)BOOL WINAPI NewReadFile(
    HANDLE       hFile,
    LPVOID       lpBuffer,
    DWORD        nNumberOfBytesToRead,
    LPDWORD      lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
)
{
    if (GetFileType(hFile) == FILE_TYPE_DISK) {
        sendInfo.argNum = 6;
        char FilePath[MAX_PATH];
        // 参数名
        sprintf(sendInfo.argName[0], "hFile");
        sprintf(sendInfo.argName[1], "lpBuffer");
        sprintf(sendInfo.argName[2], "nNumberOfBytesToRead");
        sprintf(sendInfo.argName[3], "lpNumberOfBytesRead");
        sprintf(sendInfo.argName[4], "lpOverlapped");
        sprintf(sendInfo.argName[5], "FilePath");
        // 参数值
        sprintf(sendInfo.argValue[0], "%08X", hFile);
        sprintf(sendInfo.argValue[1], "%08X", lpBuffer);
        sprintf(sendInfo.argValue[2], "%08X", nNumberOfBytesToRead);
        sprintf(sendInfo.argValue[3], "%08X", lpNumberOfBytesRead);
        sprintf(sendInfo.argValue[4], "%08X", lpOverlapped);
        // 获取文件路径
        DWORD dwSize = GetFinalPathNameByHandle(hFile, (LPWSTR)FilePath, MAX_PATH, FILE_NAME_NORMALIZED);
        if (dwSize == 0) {
            sprintf(FilePath, "Failed to get file path: %d", GetLastError());
        }
        else sprintf(sendInfo.argValue[5], "%s", FilePath);
        sendInfo.type = READFILE;
        GetLocalTime(&(sendInfo.st));
        memcpy(lpBase, &sendInfo, sizeof(info));
        ReleaseSemaphore(hSemaphore, 1, NULL);
    }
    return OldReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}

// 定义旧的函数指针类型
typedef BOOL(WINAPI* OLD_DELETE_FILEW)(
    LPCTSTR lpFileName
    );

// 保存旧的函数指针
static OLD_DELETE_FILEW OldDeleteFileW = DeleteFileW;

// 新的 DeleteFile 函数
extern "C" __declspec(dllexport) BOOL WINAPI NewDeleteFileW(
    LPCTSTR lpFileName
) {
    // 调用原始的 DeleteFile 函数
    BOOL result = OldDeleteFileW(lpFileName);

    // 将参数和值记录到 sendInfo 中
    sendInfo.type = DELETEFILEW;
    GetLocalTime(&(sendInfo.st));
    sendInfo.argNum = 1;
    //MessageBoxW(NULL, lpFileName, L"Debug", MB_OK | MB_ICONERROR);
    sprintf_s(sendInfo.argName[0], "lpFileName");
    WideCharToMultiByte(CP_UTF8, 0, lpFileName, -1, sendInfo.argValue[0], MAX_PATH, NULL, NULL);
    //MessageBoxW(NULL, (LPCWSTR)sendInfo.argValue[0], L"Debug", MB_OK | MB_ICONERROR);
    GetLocalTime(&(sendInfo.st));
    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);
    return result;
}

// 定义旧的函数指针类型
typedef BOOL(WINAPI* OLD_DELETE_FILEA)(
    LPCSTR lpFileName
    );

// 保存旧的函数指针
static OLD_DELETE_FILEA OldDeleteFileA = DeleteFileA;

// 新的 DeleteFile 函数
extern "C" __declspec(dllexport) BOOL WINAPI NewDeleteFileA(
    LPCSTR lpFileName
) {
    // 调用原始的 DeleteFile 函数
    BOOL result = OldDeleteFileA(lpFileName);

    // 获取当前工作目录
    CHAR szFullPath[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, szFullPath);

    // 构建完整路径
    CHAR szCompletePath[MAX_PATH];
    sprintf(szCompletePath, "%s\\%s", szFullPath, lpFileName);

    // 将参数和值记录到 sendInfo 中
    sendInfo.type = DELETEFILEA;
    GetLocalTime(&(sendInfo.st));
    sendInfo.argNum = 1;
    sprintf(sendInfo.argName[0], "lpFileName");
    sprintf(sendInfo.argValue[0], "%s", szCompletePath);

    // 拷贝信息到共享内存区域
    memcpy(lpBase, &sendInfo, sizeof(info));

    // 发送信号量
    ReleaseSemaphore(hSemaphore, 1, NULL);

    return result;
}

// 堆操作 HeapCreate HeaoDestory HeapAlloc HeapFree
static HANDLE(WINAPI* OldHeapCreate)(DWORD fIOoptions, SIZE_T dwInitialSize, SIZE_T dwMaximumSize) = HeapCreate;

extern "C" __declspec(dllexport)HANDLE WINAPI NewHeapCreate(DWORD fIOoptions, SIZE_T dwInitialSize, SIZE_T dwMaximumSize)
{
    HANDLE hFile = OldHeapCreate(fIOoptions, dwInitialSize, dwMaximumSize);
    sendInfo.argNum = 4;
    // 参数名
    sprintf(sendInfo.argName[0], "fIOoptions");
    sprintf(sendInfo.argName[1], "dwInitialSize");
    sprintf(sendInfo.argName[2], "dwMaximumSize");
    sprintf(sendInfo.argName[3], "HANDLE");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", fIOoptions);
    sprintf(sendInfo.argValue[1], "%IIX", dwInitialSize);
    sprintf(sendInfo.argValue[2], "%IIX", dwMaximumSize);
    sprintf(sendInfo.argValue[3], "%I64X", hFile);
    sendInfo.type = HEAPCREATE;
    GetLocalTime(&(sendInfo.st));
    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);
    //MessageBoxW(NULL, L"heapcreate.", L"Debug", MB_OK | MB_ICONERROR);
    return hFile;
}

static BOOL(WINAPI* OldHeapDestory)(HANDLE) = HeapDestroy;

extern "C" __declspec(dllexport) BOOL WINAPI NewHeapDestory(HANDLE hHeap)
{
    BOOL hFile = OldHeapDestory(hHeap);
    sendInfo.argNum = 1;
    // 参数名
    sprintf(sendInfo.argName[0], "hHeap");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", hHeap);
    sendInfo.type = HEAPDESTORY;
    GetLocalTime(&(sendInfo.st));
    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);
    //MessageBoxW(NULL, L"heapdestory.", L"Debug", MB_OK | MB_ICONERROR);
    return hFile;
}

static BOOL(WINAPI* OldHeapFree)(HANDLE hHeap, DWORD dwFlags, _Frees_ptr_opt_ LPVOID lpMem) = HeapFree;
extern "C" __declspec(dllexport) BOOL WINAPI NewHeapFree(HANDLE hHeap, DWORD dwFlags, _Frees_ptr_opt_ LPVOID lpMem) {
    BOOL ret = OldHeapFree(hHeap, dwFlags, lpMem);
    sendInfo.argNum = 3;
    // 参数名
    sprintf(sendInfo.argName[0], "hHeap");
    sprintf(sendInfo.argName[1], "dwFlags");
    sprintf(sendInfo.argName[2], "lpMem");
    // 参数值
    sprintf(sendInfo.argValue[0], "%I64X", hHeap);
    sprintf(sendInfo.argValue[1], "%08X", dwFlags);
    sprintf(sendInfo.argValue[2], "%08X", lpMem);
    sendInfo.type = HEAPFREE;
    GetLocalTime(&(sendInfo.st));
    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);
    //MessageBoxW(NULL, L"heapfree.", L"Debug", MB_OK | MB_ICONERROR);
    return ret;
}
/*
static LPVOID(WINAPI* OldHeapAlloc)(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes) = HeapAlloc;

extern "C" __declspec(dllexport) LPVOID WINAPI NewHeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes)
{
    LPVOID ret = OldHeapAlloc(hHeap, dwFlags, dwBytes);
    sendInfo.argNum = 3;
    // 参数名
    sprintf(sendInfo.argName[0], "hHeap");
    sprintf(sendInfo.argName[1], "dwFlags");
    sprintf(sendInfo.argName[2], "dwBytes");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", hHeap);
    sprintf(sendInfo.argValue[1], "%08X", dwFlags);
    sprintf(sendInfo.argValue[2], "%08X", dwBytes);
    sendInfo.type = HEAPALLOC;
    GetLocalTime(&(sendInfo.st));
    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);
    //MessageBoxW(NULL, L"heapfree.", L"Debug", MB_OK | MB_ICONERROR);
    return ret;
}
*/

static LSTATUS(WINAPI* OldRegCreateKeyEx)(
    HKEY                        hKey,
    LPCWSTR                     lpSubKey,
    DWORD                       Reserved,
    LPWSTR                      lpClass,
    DWORD                       dwOptions,
    REGSAM                      samDesired,
    const LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY                       phkResult,
    LPDWORD                     lpdwDisposition) = RegCreateKeyEx;

extern "C" __declspec(dllexport)LSTATUS WINAPI NewRegCreateKeyEx(
    HKEY                        hKey,
    LPCWSTR                     lpSubKey,
    DWORD                       Reserved,
    LPWSTR                      lpClass,
    DWORD                       dwOptions,
    REGSAM                      samDesired,
    const LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY                       phkResult,
    LPDWORD                     lpdwDisposition
) {
    char temp[70];
    sendInfo.argNum = 10;
    WCHAR subKeyBuffer[MAX_PATH];

    // 复制 lpSubKey 参数到 subKeyBuffer 缓冲区
    wcsncpy(subKeyBuffer, lpSubKey, MAX_PATH);
    subKeyBuffer[MAX_PATH - 1] = L'\0'; // 保证路径以 null 结尾

    // 将 WCHAR 格式的路径转换成多字节字符集格式（可选）
    char subKeyBufferMultibyte[MAX_PATH];
    WideCharToMultiByte(CP_ACP, 0, subKeyBuffer, -1, subKeyBufferMultibyte, MAX_PATH, NULL, NULL);

    // 参数名
    sprintf(sendInfo.argName[0], "hKey");
    sprintf(sendInfo.argName[1], "lpSubKey");
    sprintf(sendInfo.argName[2], "Reserved");
    sprintf(sendInfo.argName[3], "lpClass");
    sprintf(sendInfo.argName[4], "dwOptions");
    sprintf(sendInfo.argName[5], "samDesired");
    sprintf(sendInfo.argName[6], "lpSecurityAttributes");
    sprintf(sendInfo.argName[7], "phkResult");
    sprintf(sendInfo.argName[8], "lpdwDisposition");
    sprintf(sendInfo.argName[9], "path");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", hKey);
    // 宽字节转char
    memset(temp, 0, sizeof(temp));
    WideCharToMultiByte(CP_ACP, 0, lpSubKey, wcslen(lpSubKey), temp, sizeof(temp), NULL, NULL);
    strcpy(sendInfo.argValue[1], temp);
    sprintf(sendInfo.argValue[2], "%08X", Reserved);
    sprintf(sendInfo.argValue[3], "%08X", lpClass);
    sprintf(sendInfo.argValue[4], "%08X", dwOptions);
    sprintf(sendInfo.argValue[5], "%08X", samDesired);
    sprintf(sendInfo.argValue[6], "%08X", lpSecurityAttributes);
    sprintf(sendInfo.argValue[7], "%08X", phkResult);
    sprintf(sendInfo.argValue[8], "%08X", lpdwDisposition);
    sprintf(sendInfo.argValue[9], "%s", subKeyBufferMultibyte);

    sendInfo.type = REGCREATEKEYEX;
    GetLocalTime(&(sendInfo.st));
    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);
    return OldRegCreateKeyEx(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
}

static LSTATUS(WINAPI* OldRegSetValueEx)(
    HKEY       hKey,
    LPCWSTR    lpValueName,
    DWORD      Reserved,
    DWORD      dwType,
    const BYTE* lpData,
    DWORD      cbData
    ) = RegSetValueEx;

extern "C" __declspec(dllexport)LSTATUS WINAPI NewRegSetValueEx(
    HKEY       hKey,
    LPCWSTR    lpValueName,
    DWORD      Reserved,
    DWORD      dwType,
    const BYTE * lpData,
    DWORD      cbData)
{
    char temp[70];
    sendInfo.argNum = 6;
    // 参数名
    sprintf(sendInfo.argName[0], "hKey");
    sprintf(sendInfo.argName[1], "lpValueName");
    sprintf(sendInfo.argName[2], "Reserved");
    sprintf(sendInfo.argName[3], "dwType");
    sprintf(sendInfo.argName[4], "lpData");
    sprintf(sendInfo.argName[5], "cbData");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", hKey);
    // 宽字节转char
    memset(temp, 0, sizeof(temp));
    WideCharToMultiByte(CP_ACP, 0, lpValueName, wcslen(lpValueName), temp, sizeof(temp), NULL, NULL);
    strcpy(sendInfo.argValue[1], temp);
    sprintf(sendInfo.argValue[2], "%08X", Reserved);
    sprintf(sendInfo.argValue[3], "%08X", dwType);
    memset(temp, 0, sizeof(temp));
    WideCharToMultiByte(CP_ACP, 0, (LPCWCH)lpData, wcslen((LPCWCH)lpData), temp, sizeof(temp), NULL, NULL);
    strcpy(sendInfo.argValue[4], temp);
    sprintf(sendInfo.argValue[5], "%08X", cbData);


    sendInfo.type = REGSETVALUEEX;
    GetLocalTime(&(sendInfo.st));
    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);
    return OldRegSetValueEx(hKey, lpValueName, Reserved, dwType, lpData, cbData);
}

static LSTATUS(WINAPI* OldRegCloseKey)(HKEY hKey) = RegCloseKey;

extern "C" __declspec(dllexport)LSTATUS WINAPI NewRegCloseKey(HKEY hKey)
{
    sendInfo.argNum = 1;
    // 参数名
    sprintf(sendInfo.argName[0], "hKey");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", hKey);
    sendInfo.type = REGCLOSEKEY;
    GetLocalTime(&(sendInfo.st));
    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);
    return OldRegCloseKey(hKey);
}

static LSTATUS(WINAPI* OldRegOpenKeyEx)(
    HKEY    hKey,
    LPCWSTR lpSubKey,
    DWORD   ulOptions,
    REGSAM  samDesired,
    PHKEY   phkResult
    ) = RegOpenKeyEx;
extern "C" __declspec(dllexport)LSTATUS WINAPI NewRegOpenKeyEx(
    HKEY    hKey,
    LPCWSTR lpSubKey,
    DWORD   ulOptions,
    REGSAM  samDesired,
    PHKEY   phkResult)
{
    char temp[70];
    sendInfo.argNum = 5;
    // 参数名
    sprintf(sendInfo.argName[0], "hKey");
    sprintf(sendInfo.argName[1], "lpSubKey");
    sprintf(sendInfo.argName[2], "ulOptions");
    sprintf(sendInfo.argName[3], "samDesired");
    sprintf(sendInfo.argName[4], "phkResult");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", hKey);
    // 宽字节转char
    memset(temp, 0, sizeof(temp));
    WideCharToMultiByte(CP_ACP, 0, lpSubKey, wcslen(lpSubKey), temp, sizeof(temp), NULL, NULL);
    strcpy(sendInfo.argValue[1], temp);
    sprintf(sendInfo.argValue[2], "%08X", ulOptions);
    sprintf(sendInfo.argValue[3], "%08X", samDesired);
    sprintf(sendInfo.argValue[4], "%08X", phkResult);

    sendInfo.type = REGOPENKEYEX;
    GetLocalTime(&(sendInfo.st));
    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);
    return OldRegOpenKeyEx(hKey, lpSubKey, ulOptions, samDesired, phkResult);
}

static LSTATUS(WINAPI* OldRegDeleteValue)(
    HKEY    hKey,
    LPCWSTR lpValueName
    ) = RegDeleteValue;

extern "C" __declspec(dllexport)LSTATUS WINAPI NewRegDeleteValue(
    HKEY    hKey,
    LPCWSTR lpValueName)
{
    char temp[70];
    sendInfo.argNum = 2;
    // 参数名
    sprintf(sendInfo.argName[0], "hKey");
    sprintf(sendInfo.argName[1], "lpValueName");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", hKey);
    // 宽字节转char
    memset(temp, 0, sizeof(temp));
    WideCharToMultiByte(CP_ACP, 0, lpValueName, wcslen(lpValueName), temp, sizeof(temp), NULL, NULL);
    strcpy(sendInfo.argValue[1], temp);
    sendInfo.type = REGDELETEVALUE;
    GetLocalTime(&(sendInfo.st));
    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);
    return OldRegDeleteValue(hKey, lpValueName);
}

static SOCKET(WINAPI* OldWSASocket)(
    int af,
    int type,
    int protocol,
    LPWSAPROTOCOL_INFO lpProtocolInfo,
    GROUP              g,
    DWORD              dwFlags
    ) = WSASocket;
extern "C" __declspec(dllexport)SOCKET WINAPI NewWSASocket(
    int af,
    int type,
    int protocol,
    LPWSAPROTOCOL_INFO lpProtocolInfo,
    GROUP              g,
    DWORD              dwFlags
) {
    //printf("HUST CSE: WSASocket Hooked\n");
    sendInfo.argNum = 6;
    // 参数名
    sprintf(sendInfo.argName[0], "af");
    sprintf(sendInfo.argName[1], "type");
    sprintf(sendInfo.argName[2], "protocol");
    sprintf(sendInfo.argName[3], "lpProtocolInfo");
    sprintf(sendInfo.argName[4], "g");
    sprintf(sendInfo.argName[5], "dwFlags");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", af);
    sprintf(sendInfo.argValue[1], "%08X", type);
    sprintf(sendInfo.argValue[2], "%08X", protocol);
    sprintf(sendInfo.argValue[3], "%08X", lpProtocolInfo);
    sprintf(sendInfo.argValue[4], "%08X", g);
    sprintf(sendInfo.argValue[5], "%08X", dwFlags);
    sendInfo.type = WSASOCKET;
    GetLocalTime(&(sendInfo.st));
    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);
    return OldWSASocket(af, type, protocol, lpProtocolInfo, g, dwFlags);
}

static int (WINAPI* OldWSASend)(
    SOCKET                             s,
    LPWSABUF                           lpBuffers,
    DWORD                              dwBufferCount,
    LPDWORD                            lpNumberOfBytesSent,
    DWORD                              dwFlags,
    LPWSAOVERLAPPED                    lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    ) = WSASend;
extern "C" __declspec(dllexport)int WINAPI NewWSASend(
    SOCKET                             s,
    LPWSABUF                           lpBuffers,
    DWORD                              dwBufferCount,
    LPDWORD                            lpNumberOfBytesSent,
    DWORD                              dwFlags,
    LPWSAOVERLAPPED                    lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
) {
    //printf("HUST CSE: WSASend Hooked\n");
    // 获取对方IP和port
    struct sockaddr_in addr;
    int addr_len = sizeof(addr);
    getpeername(s, (struct sockaddr*)&addr, &addr_len);
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
    int port = ntohs(addr.sin_port);
    //内容
    for (DWORD i = 0; i < dwBufferCount; ++i) {
        LPWSABUF pBuffer = &lpBuffers[i];
        // Assuming pBuffer->buf points to the buffer containing the text
        // and pBuffer->len specifies the length of the buffer
        char* text = (char*)pBuffer->buf;
        DWORD textLength = pBuffer->len;
        sprintf(sendInfo.argValue[9], "%s", text);
    }
    sendInfo.argNum = 10;
    // 参数名
    sprintf(sendInfo.argName[0], "s");
    sprintf(sendInfo.argName[1], "lpBuffers");
    sprintf(sendInfo.argName[2], "dwBufferCount");
    sprintf(sendInfo.argName[3], "lpNumberOfBytesSent");
    sprintf(sendInfo.argName[4], "dwFlags");
    sprintf(sendInfo.argName[5], "lpOverlapped");
    sprintf(sendInfo.argName[6], "lpCompletionRoutine");
    sprintf(sendInfo.argName[7], "IP");
    sprintf(sendInfo.argName[8], "port");
    sprintf(sendInfo.argName[9], "SendText");
    // 参数值
    sprintf(sendInfo.argValue[0], "%IIX", s);
    sprintf(sendInfo.argValue[1], "%08X", lpBuffers);
    sprintf(sendInfo.argValue[2], "%08X", dwBufferCount);
    sprintf(sendInfo.argValue[3], "%08X", lpNumberOfBytesSent);
    sprintf(sendInfo.argValue[4], "%s", dwFlags);
    sprintf(sendInfo.argValue[5], "%s", lpOverlapped);
    sprintf(sendInfo.argValue[6], "%s", lpCompletionRoutine);
    sprintf(sendInfo.argValue[7], "%s", ip);
    sprintf(sendInfo.argValue[8], "%d", port);
    sendInfo.type = WSASEND;
    GetLocalTime(&(sendInfo.st));
    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);
    return OldWSASend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);
}

static int (WINAPI* OldWSAConnect)(
    SOCKET         s,
    const sockaddr* name,
    int            namelen,
    LPWSABUF       lpCallerData,
    LPWSABUF       lpCalleeData,
    LPQOS          lpSQOS,
    LPQOS          lpGQOS
    ) = WSAConnect;
extern "C" __declspec(dllexport)int WINAPI NewWSAConnect(
    SOCKET         s,
    const sockaddr * name,
    int            namelen,
    LPWSABUF       lpCallerData,
    LPWSABUF       lpCalleeData,
    LPQOS          lpSQOS,
    LPQOS          lpGQOS
) {
    //name->sa_data
    //printf("HUST CSE: WSAConnect Hooked\n");
    struct sockaddr_in* sock = (struct sockaddr_in*)name;
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(sock->sin_addr), ip, INET_ADDRSTRLEN);
    int port = ntohs(sock->sin_port);
    // 参数名
    sendInfo.argNum = 9;
    sprintf(sendInfo.argName[0], "s");
    sprintf(sendInfo.argName[1], "name");
    sprintf(sendInfo.argName[2], "namelen");
    sprintf(sendInfo.argName[3], "lpCallerData");
    sprintf(sendInfo.argName[4], "lpCalleeData");
    sprintf(sendInfo.argName[5], "lpSQOS");
    sprintf(sendInfo.argName[6], "lpGQOS");
    sprintf(sendInfo.argName[7], "IP");
    sprintf(sendInfo.argName[8], "port");
    // 参数值
    sprintf(sendInfo.argValue[0], "%IIX", s);
    sprintf(sendInfo.argValue[1], "%08X", name);
    sprintf(sendInfo.argValue[2], "%08X", namelen);
    sprintf(sendInfo.argValue[3], "%s", lpCallerData);
    sprintf(sendInfo.argValue[4], "%d", lpCalleeData);
    sprintf(sendInfo.argValue[5], "%p", lpSQOS);
    sprintf(sendInfo.argValue[6], "%p", lpGQOS);
    sprintf(sendInfo.argValue[7], "%s", ip);
    sprintf(sendInfo.argValue[8], "%d", port);
    sendInfo.type = WSACONNECT;
    GetLocalTime(&(sendInfo.st));
    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);
    return OldWSAConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS);
}

static int (WINAPI* OldWSARecv)(
    SOCKET                             s,
    LPWSABUF                           lpBuffers,
    DWORD                              dwBufferCount,
    LPDWORD                            lpNumberOfBytesRecvd,
    LPDWORD                            lpFlags,
    LPWSAOVERLAPPED                    lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    ) = WSARecv;
extern "C" __declspec(dllexport)int WINAPI NewWSARecv(
    SOCKET                             s,
    LPWSABUF                           lpBuffers,
    DWORD                              dwBufferCount,
    LPDWORD                            lpNumberOfBytesRecvd,
    LPDWORD                            lpFlags,
    LPWSAOVERLAPPED                    lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
) {
    // 获取对方IP和port
    struct sockaddr_in addr;
    int addr_len = sizeof(addr);
    getpeername(s, (struct sockaddr*)&addr, &addr_len);
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
    int port = ntohs(addr.sin_port);

    char szBuffer[MAXBYTE] = { 0 };
    // 将 szBuffer 转换为 wchar_t 类型的字符串
    int requiredSize = MultiByteToWideChar(CP_ACP, 0, szBuffer, -1, NULL, 0);
    wchar_t* wideBuffer = new wchar_t[requiredSize];
    MultiByteToWideChar(CP_ACP, 0, szBuffer, -1, wideBuffer, requiredSize);

    // 提取接收到的文本信息
    sprintf(sendInfo.argValue[9], "%Ls", wideBuffer);
    //MessageBoxW(NULL, wideBuffer, L"Debug", MB_OK | MB_ICONERROR);
    sendInfo.argNum = 10;
    // 参数名
    sprintf(sendInfo.argName[0], "s");
    sprintf(sendInfo.argName[1], "lpBuffers");
    sprintf(sendInfo.argName[2], "dwBufferCount");
    sprintf(sendInfo.argName[3], "lpNumberOfBytesRecvd");
    sprintf(sendInfo.argName[4], "lpFlags");
    sprintf(sendInfo.argName[5], "lpOverlapped");
    sprintf(sendInfo.argName[6], "lpCompletionRoutine");
    sprintf(sendInfo.argName[7], "IP");
    sprintf(sendInfo.argName[8], "port");
    sprintf(sendInfo.argName[9], "RecvText");
    // 参数值
    sprintf(sendInfo.argValue[0], "%IIX", s);
    sprintf(sendInfo.argValue[1], "%s", lpBuffers);
    sprintf(sendInfo.argValue[2], "%08X", dwBufferCount);
    sprintf(sendInfo.argValue[3], "%08X", lpNumberOfBytesRecvd);
    sprintf(sendInfo.argValue[4], "%08X", lpFlags);
    sprintf(sendInfo.argValue[5], "%08X", lpOverlapped);
    sprintf(sendInfo.argValue[6], "%08X", lpCompletionRoutine);
    sprintf(sendInfo.argValue[7], "%s", ip);
    sprintf(sendInfo.argValue[8], "%d", port);
    sendInfo.type = WSARECV;
    GetLocalTime(&(sendInfo.st));
    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);
    return OldWSARecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine);
}

typedef SOCKET(WINAPI* OLD_SOCKET)(
    int af,
    int type,
    int protocol
    );

// 保存旧的函数指针
static OLD_SOCKET Oldsocket = socket;

// 新的 socket 函数
extern "C" __declspec(dllexport) SOCKET WINAPI Newsocket(
    int af,
    int type,
    int protocol
) {
    // 调用原始的 socket 函数
    SOCKET result = Oldsocket(af, type, protocol);

    // 将参数和值记录到 sendInfo 中
    sendInfo.type = MYSOCKET;
    sendInfo.argNum = 3;
    sprintf(sendInfo.argName[0], "af");
    sprintf(sendInfo.argName[1], "type");
    sprintf(sendInfo.argName[2], "protocol");
    sprintf(sendInfo.argValue[0], "%d", af);
    sprintf(sendInfo.argValue[1], "%d", type);
    sprintf(sendInfo.argValue[2], "%d", protocol);
    // 获取本地套接字的IP和端口信息
    sockaddr_storage addr;
    int addrLen = sizeof(addr);
    if (getsockname(result, (sockaddr*)&addr, &addrLen) == 0) {
        if (addr.ss_family == AF_INET) {
            struct sockaddr_in* s = (struct sockaddr_in*)&addr;
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &s->sin_addr, ip, sizeof(ip));
            sprintf(sendInfo.argName[3], "local_ip");
            sprintf(sendInfo.argValue[3], "%s", ip);
            sprintf(sendInfo.argName[4], "local_port");
            sprintf(sendInfo.argValue[4], "%d", ntohs(s->sin_port));
            sendInfo.argNum += 2;
        }
        else if (addr.ss_family == AF_INET6) {
            struct sockaddr_in6* s = (struct sockaddr_in6*)&addr;
            char ip[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &s->sin6_addr, ip, sizeof(ip));
            sprintf(sendInfo.argName[3], "local_ip");
            sprintf(sendInfo.argValue[3], "%s", ip);
            sprintf(sendInfo.argName[4], "local_port");
            sprintf(sendInfo.argValue[4], "%d", ntohs(s->sin6_port));
            sendInfo.argNum += 2;
        }
    }
    GetLocalTime(&(sendInfo.st));
    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);
    return result;
}

// 保存旧的函数指针
static int (WINAPI* Oldsend)(
    SOCKET s,
    const char* buf,
    int len,
    int flags
    ) = send;

// 新的 send 函数
extern "C" __declspec(dllexport) int WINAPI Newsend(
    SOCKET s,
    const char* buf,
    int len,
    int flags
) {
    // 调用原始的 send 函数
    int result = Oldsend(s, buf, len, flags);

    // 将参数和值记录到 sendInfo 中
    sendInfo.type = SEND;
    GetLocalTime(&(sendInfo.st));
    sendInfo.argNum = 4;
    sprintf(sendInfo.argName[0], "s");
    sprintf(sendInfo.argName[1], "buf");
    sprintf(sendInfo.argName[2], "len");
    sprintf(sendInfo.argName[3], "flags");
    sprintf(sendInfo.argValue[0], "%IIX", s);
    sprintf(sendInfo.argValue[1], "%s", buf);
    sprintf(sendInfo.argValue[2], "%d", len);
    sprintf(sendInfo.argValue[3], "%d", flags);

    // 获取套接字的本地地址信息
    sockaddr_storage addr;
    int addrLen = sizeof(addr);
    if (getsockname(s, (sockaddr*)&addr, &addrLen) == 0) {
        if (addr.ss_family == AF_INET) {
            struct sockaddr_in* saddr = (struct sockaddr_in*)&addr;
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &saddr->sin_addr, ip, INET_ADDRSTRLEN);
            sprintf(sendInfo.argName[4], "local_ip");
            sprintf(sendInfo.argValue[4], "%s", ip);
            sprintf(sendInfo.argName[5], "local_port");
            sprintf(sendInfo.argValue[5], "%d", ntohs(saddr->sin_port));
            sendInfo.argNum += 2;
        }
        else if (addr.ss_family == AF_INET6) {
            struct sockaddr_in6* saddr = (struct sockaddr_in6*)&addr;
            char ip[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &saddr->sin6_addr, ip, INET6_ADDRSTRLEN);
            sprintf(sendInfo.argName[4], "local_ip");
            sprintf(sendInfo.argValue[4], "%s", ip);
            sprintf(sendInfo.argName[5], "local_port");
            sprintf(sendInfo.argValue[5], "%d", ntohs(saddr->sin6_port));
            sendInfo.argNum += 2;
        }
    }
    GetLocalTime(&(sendInfo.st));
    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);
    return result;
}

// 定义旧的函数指针类型
static int (WINAPI* Oldconnect)(
    SOCKET s,
    const struct sockaddr* name,
    int namelen
    ) = connect;

// 新的 connect 函数
extern "C" __declspec(dllexport) int WINAPI Newconnect(
    SOCKET s,
    const struct sockaddr* name,
    int namelen
) {
    // 调用原始的 connect 函数
    int result = Oldconnect(s, name, namelen);

    // 将参数和值记录到 sendInfo 中
    sendInfo.type = CONNECT;
    GetLocalTime(&(sendInfo.st));
    sendInfo.argNum = 2;
    sprintf(sendInfo.argName[0], "s");
    sprintf(sendInfo.argName[1], "name");
    sprintf(sendInfo.argValue[0], "%IIX", s);

    // 获取连接目标的IP和端口信息
    if (name->sa_family == AF_INET) {
        struct sockaddr_in* addr_in = (struct sockaddr_in*)name;
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(addr_in->sin_addr), ip, INET_ADDRSTRLEN);
        sprintf(sendInfo.argName[2], "remote_ip");
        sprintf(sendInfo.argValue[2], "%s", ip);
        sprintf(sendInfo.argName[3], "remote_port");
        sprintf(sendInfo.argValue[3], "%d", ntohs(addr_in->sin_port));
        sendInfo.argNum += 2;
    }
    else if (name->sa_family == AF_INET6) {
        struct sockaddr_in6* addr_in6 = (struct sockaddr_in6*)name;
        char ip[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &(addr_in6->sin6_addr), ip, INET6_ADDRSTRLEN);
        sprintf(sendInfo.argName[2], "remote_ip");
        sprintf(sendInfo.argValue[2], "%s", ip);
        sprintf(sendInfo.argName[3], "remote_port");
        sprintf(sendInfo.argValue[3], "%d", ntohs(addr_in6->sin6_port));
        sendInfo.argNum += 2;
    }
    GetLocalTime(&(sendInfo.st));
    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);
    return result;
}

// 定义旧的函数指针类型
static int (WINAPI* Oldrecv)(
    SOCKET s,
    char* buf,
    int len,
    int flags
    ) = recv;

// 新的 recv 函数
extern "C" __declspec(dllexport) int WINAPI Newrecv(
    SOCKET s,
    char* buf,
    int len,
    int flags
) {
    // 调用原始的 recv 函数
    int result = Oldrecv(s, buf, len, flags);

    // 将参数和值记录到 sendInfo 中
    sendInfo.type = RECV;
    GetLocalTime(&(sendInfo.st));
    sendInfo.argNum = 4;
    sprintf(sendInfo.argName[0], "s");
    sprintf(sendInfo.argName[1], "buf");
    sprintf(sendInfo.argName[2], "len");
    sprintf(sendInfo.argName[3], "flags");
    sprintf(sendInfo.argValue[0], "%IIX", s);
    sprintf(sendInfo.argValue[1], "%s", buf);
    sprintf(sendInfo.argValue[2], "%d", len);
    sprintf(sendInfo.argValue[3], "%d", flags);
    GetLocalTime(&(sendInfo.st));
    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);
    return result;
}

static BOOL(WINAPI* OldCreateProcessW)(
    LPCWSTR               lpApplicationName,
    LPWSTR                lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCWSTR               lpCurrentDirectory,
    LPSTARTUPINFOW        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    ) = CreateProcessW;
extern "C" __declspec(dllexport)int WINAPI NewCreateProcessW(
    LPCWSTR               lpApplicationName,
    LPWSTR                lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCWSTR               lpCurrentDirectory,
    LPSTARTUPINFOW        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
) {
    //MessageBoxW(NULL, L"PROCESSW.", L"Debug", MB_OK | MB_ICONERROR);
    char CommandLine[MAX_PATH]; // 定义一个足够大的缓冲区来保存应用程序名称的多字节字符串

    // 将 lpApplicationName 转换为多字节字符串
    if (CommandLine != NULL) {
        // 使用 wcstombs 将 lpApplicationName 转换为多字节字符串
        wcstombs(CommandLine, lpCommandLine, MAX_PATH);
    }
    else {
        // 如果 lpApplicationName 为空，则将字符串设置为 "(NULL)"
        strcpy(CommandLine, "(NULL)");
    }

    sendInfo.argNum = 10;
    // 参数名
    sprintf(sendInfo.argName[0], "lpApplicationName");
    sprintf(sendInfo.argName[1], "lpCommandLine");
    sprintf(sendInfo.argName[2], "lpProcessAttributes");
    sprintf(sendInfo.argName[3], "lpThreadAttributes");
    sprintf(sendInfo.argName[4], "bInheritHandles");
    sprintf(sendInfo.argName[5], "dwCreationFlags");
    sprintf(sendInfo.argName[6], "lpEnvironment");
    sprintf(sendInfo.argName[7], "lpCurrentDirectory");
    sprintf(sendInfo.argName[8], "lpStartupInfo");
    sprintf(sendInfo.argName[9], "lpProcessInformation");
    // 参数值
    sprintf(sendInfo.argValue[0], "%ls", lpApplicationName);
    sprintf(sendInfo.argValue[1], "%ls", lpCommandLine);
    sprintf(sendInfo.argValue[2], "%p", lpProcessAttributes);
    sprintf(sendInfo.argValue[3], "%p", lpThreadAttributes);
    sprintf(sendInfo.argValue[4], "%d", bInheritHandles);
    sprintf(sendInfo.argValue[5], "%lu", dwCreationFlags);
    sprintf(sendInfo.argValue[6], "%p", lpEnvironment);
    sprintf(sendInfo.argValue[7], "%ls", lpCurrentDirectory);
    sprintf(sendInfo.argValue[8], "%p", lpStartupInfo);
    sprintf(sendInfo.argValue[9], "%p", lpProcessInformation);
    sendInfo.type = CREATEPROCESSW;
    GetLocalTime(&(sendInfo.st));
    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);
    Sleep(20);
    TCHAR  text[MAX_PATH + 128] = { 0 };
    wsprintf(text, L"HOOKing！程序正在创建其他进程，是否启动: %s \n\n程序路径：%s", lpApplicationName, lpCommandLine);
    int choice = MessageBox(0, text, TEXT("提示"), MB_YESNO);
    if (choice == IDNO) {
        SetLastError(5);
        return false;
    }
    return OldCreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

static BOOL(WINAPI* OldCreateProcessA)(
    LPCSTR                lpApplicationName,
    LPSTR                 lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCSTR                lpCurrentDirectory,
    LPSTARTUPINFOA        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    ) = CreateProcessA;

extern "C" __declspec(dllexport) int WINAPI NewCreateProcessA(
    LPCSTR                lpApplicationName,
    LPSTR                 lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCSTR                lpCurrentDirectory,
    LPSTARTUPINFOA        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
) {
    //MessageBoxW(NULL, L"PROCESSA.", L"Debug", MB_OK | MB_ICONERROR);
    char CommandLine[MAX_PATH]; // Define a buffer large enough to hold the multi-byte string of the application name

    // Convert lpApplicationName to a multi-byte string
    if (lpCommandLine != NULL) {
        // Use strcpy to convert lpCommandLine to a multi-byte string
        strcpy(CommandLine, lpCommandLine);
    }
    else {
        // If lpApplicationName is empty, set the string to "(NULL)"
        strcpy(CommandLine, "(NULL)");
    }

    sendInfo.argNum = 10;
    // Argument names
    sprintf(sendInfo.argName[0], "lpApplicationName");
    sprintf(sendInfo.argName[1], "lpCommandLine");
    sprintf(sendInfo.argName[2], "lpProcessAttributes");
    sprintf(sendInfo.argName[3], "lpThreadAttributes");
    sprintf(sendInfo.argName[4], "bInheritHandles");
    sprintf(sendInfo.argName[5], "dwCreationFlags");
    sprintf(sendInfo.argName[6], "lpEnvironment");
    sprintf(sendInfo.argName[7], "lpCurrentDirectory");
    sprintf(sendInfo.argName[8], "lpStartupInfo");
    sprintf(sendInfo.argName[9], "lpProcessInformation");
    // Argument values
    sprintf(sendInfo.argValue[0], "%s", lpApplicationName);
    sprintf(sendInfo.argValue[1], "%s", lpCommandLine);
    sprintf(sendInfo.argValue[2], "%p", lpProcessAttributes);
    sprintf(sendInfo.argValue[3], "%p", lpThreadAttributes);
    sprintf(sendInfo.argValue[4], "%d", bInheritHandles);
    sprintf(sendInfo.argValue[5], "%lu", dwCreationFlags);
    sprintf(sendInfo.argValue[6], "%p", lpEnvironment);
    sprintf(sendInfo.argValue[7], "%s", lpCurrentDirectory);
    sprintf(sendInfo.argValue[8], "%p", lpStartupInfo);
    sprintf(sendInfo.argValue[9], "%p", lpProcessInformation);
    sendInfo.type = CREATEPROCESSA;
    GetLocalTime(&(sendInfo.st));
    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);
    Sleep(20);
    char text[MAX_PATH + 128] = { 0 };
    sprintf(text, "HOOKing！是否启动: %s \n\n程序路径：%s", lpApplicationName, lpCommandLine);
    int choice = MessageBoxA(0, text, "提示", MB_YESNO);
    if (choice == IDNO) {
        SetLastError(5);
        return false;
    }
    return OldCreateProcessA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

static HANDLE(WINAPI* OldCreateThread)(
    LPSECURITY_ATTRIBUTES   lpThreadAttributes,
    SIZE_T                  dwStackSize,
    LPTHREAD_START_ROUTINE  lpStartAddress,
    LPVOID                  lpParameter,
    DWORD                   dwCreationFlags,
    LPDWORD                 lpThreadId
    ) = CreateThread;

extern "C" __declspec(dllexport) HANDLE WINAPI NewCreateThread(
    LPSECURITY_ATTRIBUTES   lpThreadAttributes,
    SIZE_T                  dwStackSize,
    LPTHREAD_START_ROUTINE  lpStartAddress,
    LPVOID                  lpParameter,
    DWORD                   dwCreationFlags,
    LPDWORD                 lpThreadId
) {
    sendInfo.argNum = 6;
    // Argument names
    sprintf(sendInfo.argName[0], "lpThreadAttributes");
    sprintf(sendInfo.argName[1], "dwStackSize");
    sprintf(sendInfo.argName[2], "lpStartAddress");
    sprintf(sendInfo.argName[3], "lpParameter");
    sprintf(sendInfo.argName[4], "dwCreationFlags");
    sprintf(sendInfo.argName[5], "lpThreadId");
    // Argument values
    sprintf(sendInfo.argValue[0], "%p", lpThreadAttributes);
    sprintf(sendInfo.argValue[1], "%llu", dwStackSize);
    sprintf(sendInfo.argValue[2], "%p", lpStartAddress);
    sprintf(sendInfo.argValue[3], "%p", lpParameter);
    sprintf(sendInfo.argValue[4], "%lu", dwCreationFlags);
    sprintf(sendInfo.argValue[5], "%p", lpThreadId);
    sendInfo.type = CREATETHREAD;
    GetLocalTime(&(sendInfo.st));
    memcpy(lpBase, &sendInfo, sizeof(info));
    ReleaseSemaphore(hSemaphore, 1, NULL);
    /*
    Sleep(50);
    // Display a message box indicating the thread creation
    char text[128] = { 0 };
    sprintf(text, "HOOKing！是否创建线程？");
    int choice = MessageBoxA(0, text, "提示", MB_YESNO);
    if (choice == IDNO) {
        SetLastError(5);
        return NULL;
    }
    */
    // Call the original CreateThread function
    return OldCreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
}


BOOL WINAPI DllMain(HMODULE hModule,
    DWORD ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        DisableThreadLibraryCalls(hModule);
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)OldMessageBoxW, NewMessageBoxW);
        DetourAttach(&(PVOID&)OldMessageBoxA, NewMessageBoxA);
        DetourAttach(&(PVOID&)OldCreateFile, NewCreateFile);
        DetourAttach(&(PVOID&)OldOpenFile, NewOpenFile);
        DetourAttach(&(PVOID&)OldWriteFile, NewWriteFile);
        DetourAttach(&(PVOID&)OldReadFile, NewReadFile);
        DetourAttach(&(PVOID&)OldCopyFile, NewCopyFile);
        DetourAttach(&(PVOID&)OldDeleteFileW, NewDeleteFileW);
        DetourAttach(&(PVOID&)OldDeleteFileA, NewDeleteFileA);
        DetourAttach(&(PVOID&)OldHeapCreate, NewHeapCreate);
        DetourAttach(&(PVOID&)OldHeapDestory, NewHeapDestory);
        DetourAttach(&(PVOID&)OldHeapFree, NewHeapFree);
        //DetourAttach(&(PVOID&)OldHeapAlloc, NewHeapAlloc);
        DetourAttach(&(PVOID&)OldRegCreateKeyEx, NewRegCreateKeyEx);
        DetourAttach(&(PVOID&)OldRegSetValueEx, NewRegSetValueEx);
        DetourAttach(&(PVOID&)OldRegDeleteValue, NewRegDeleteValue);
        DetourAttach(&(PVOID&)OldRegCloseKey, NewRegCloseKey);
        DetourAttach(&(PVOID&)OldRegOpenKeyEx, NewRegOpenKeyEx);
        DetourAttach(&(PVOID&)OldWSASocket, NewWSASocket);
        DetourAttach(&(PVOID&)OldWSASend, NewWSASend);
        DetourAttach(&(PVOID&)OldWSAConnect, NewWSAConnect);
        DetourAttach(&(PVOID&)OldWSARecv, NewWSARecv);
        DetourAttach(&(PVOID&)Oldsocket, Newsocket);
        DetourAttach(&(PVOID&)Oldsend, Newsend);
        DetourAttach(&(PVOID&)Oldconnect, Newconnect);
        DetourAttach(&(PVOID&)Oldrecv, Newrecv);
        DetourAttach(&(PVOID&)OldCreateProcessW, NewCreateProcessW);
        DetourAttach(&(PVOID&)OldCreateProcessA, NewCreateProcessA);
        DetourAttach(&(PVOID&)OldCreateThread, NewCreateThread);
        DetourTransactionCommit();
    }
    break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)OldMessageBoxW, NewMessageBoxW);
        DetourDetach(&(PVOID&)OldMessageBoxA, NewMessageBoxA);
        DetourDetach(&(PVOID&)OldCreateFile, NewCreateFile);
        DetourDetach(&(PVOID&)OldOpenFile, NewOpenFile);
        DetourDetach(&(PVOID&)OldWriteFile, NewWriteFile);
        DetourDetach(&(PVOID&)OldReadFile, NewReadFile);
        DetourDetach(&(PVOID&)OldCopyFile, NewCopyFile);
        DetourDetach(&(PVOID&)OldDeleteFileW, NewDeleteFileW);
        DetourDetach(&(PVOID&)OldDeleteFileA, NewDeleteFileA);
        DetourDetach(&(PVOID&)OldHeapCreate, NewHeapCreate);
        DetourDetach(&(PVOID&)OldHeapDestory, NewHeapDestory);
        DetourDetach(&(PVOID&)OldHeapFree, NewHeapFree);
        //DetourDetach(&(PVOID&)OldHeapAlloc, NewHeapAlloc);
        DetourDetach(&(PVOID&)OldRegCreateKeyEx, NewRegCreateKeyEx);
        DetourDetach(&(PVOID&)OldRegSetValueEx, NewRegSetValueEx);
        DetourDetach(&(PVOID&)OldRegDeleteValue, NewRegDeleteValue);
        DetourDetach(&(PVOID&)OldRegCloseKey, NewRegCloseKey);
        DetourDetach(&(PVOID&)OldRegOpenKeyEx, NewRegOpenKeyEx);
        DetourDetach(&(PVOID&)OldWSASocket, NewWSASocket);
        DetourDetach(&(PVOID&)OldWSASend, NewWSASend);
        DetourDetach(&(PVOID&)OldWSAConnect, NewWSAConnect);
        DetourDetach(&(PVOID&)OldWSARecv, NewWSARecv);
        DetourDetach(&(PVOID&)Oldsocket, Newsocket);
        DetourDetach(&(PVOID&)Oldsend, Newsend);
        DetourDetach(&(PVOID&)Oldconnect, Newconnect);
        DetourDetach(&(PVOID&)Oldrecv, Newrecv);
        DetourDetach(&(PVOID&)OldCreateProcessW, NewCreateProcessW);
        DetourDetach(&(PVOID&)OldCreateProcessA, NewCreateProcessA);
        DetourDetach(&(PVOID&)OldCreateThread, NewCreateThread);
        DetourTransactionCommit();
        UnmapViewOfFile(lpBase);
        CloseHandle(hMapFile);
        break;
    }
    return TRUE;
}