#include "r3dPCH.h"
#include "r3d.h"
#include "r3dDebug.h"

#include <windows.h>
#include <DbgHelp.h>
#include <stdio.h>
#include <time.h>

#pragma comment(lib, "Dbghelp.lib")

extern void r3dCloseLogFile();

bool CreateConfigPath(char* dest);
bool CreateWorkPath(char* dest);

static char g_LastCrashDumpPath[MAX_PATH * 2] = {0};

static void r3dSafeLog(const char* fmt, ...)
{
    char buffer[4096] = {0};

    va_list args;
    va_start(args, fmt);
    vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, fmt, args);
    va_end(args);

    r3dOutToLog("%s", buffer);
}

static void r3dMakeCrashFileName(char* outPath, size_t outPathSize)
{
    char crashDir[MAX_PATH * 2] = {0};

    if (!CreateConfigPath(crashDir))
    {
        GetCurrentDirectoryA(sizeof(crashDir), crashDir);
        size_t len = strlen(crashDir);

        if (len > 0 && crashDir[len - 1] != '\\' && crashDir[len - 1] != '/')
            strcat_s(crashDir, "\\");
    }

    strcat_s(crashDir, "Crashes\\");
    CreateDirectoryA(crashDir, NULL);

    SYSTEMTIME st;
    GetLocalTime(&st);

    sprintf_s(
        outPath,
        outPathSize,
        "%sWarInc_%04u-%02u-%02u_%02u-%02u-%02u_%u.dmp",
        crashDir,
        st.wYear,
        st.wMonth,
        st.wDay,
        st.wHour,
        st.wMinute,
        st.wSecond,
        GetCurrentProcessId()
    );
}

static BOOL CALLBACK r3dMiniDumpCallback(
    PVOID,
    const PMINIDUMP_CALLBACK_INPUT input,
    PMINIDUMP_CALLBACK_OUTPUT output
)
{
    if (!input || !output)
        return FALSE;

    switch (input->CallbackType)
    {
    case IncludeModuleCallback:
    case IncludeThreadCallback:
    case ThreadCallback:
    case ThreadExCallback:
        return TRUE;

    case ModuleCallback:
        if (!(output->ModuleWriteFlags & ModuleReferencedByMemory))
            output->ModuleWriteFlags &= ~ModuleWriteModule;

        return TRUE;

    case MemoryCallback:
        return FALSE;

    default:
        return TRUE;
    }
}

static bool r3dWriteMiniDump(EXCEPTION_POINTERS* exceptionPointers)
{
    r3dMakeCrashFileName(g_LastCrashDumpPath, sizeof(g_LastCrashDumpPath));

    HANDLE file = CreateFileA(
        g_LastCrashDumpPath,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (file == INVALID_HANDLE_VALUE)
    {
        r3dSafeLog("CreateFileA for crash dump failed. Error: %u\n", GetLastError());
        return false;
    }

    MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
    exceptionInfo.ThreadId = GetCurrentThreadId();
    exceptionInfo.ExceptionPointers = exceptionPointers;
    exceptionInfo.ClientPointers = FALSE;

    MINIDUMP_CALLBACK_INFORMATION callbackInfo;
    callbackInfo.CallbackRoutine = r3dMiniDumpCallback;
    callbackInfo.CallbackParam = NULL;

    MINIDUMP_TYPE dumpType = (MINIDUMP_TYPE)(
        MiniDumpNormal |
        MiniDumpWithDataSegs |
        MiniDumpWithHandleData |
        MiniDumpWithIndirectlyReferencedMemory |
        MiniDumpScanMemory
    );

    BOOL result = MiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        file,
        dumpType,
        exceptionPointers ? &exceptionInfo : NULL,
        NULL,
        &callbackInfo
    );

    DWORD errorCode = GetLastError();

    CloseHandle(file);

    if (!result)
    {
        r3dSafeLog("MiniDumpWriteDump failed. Error: %u\n", errorCode);
        return false;
    }

    r3dSafeLog("Crash dump created: %s\n", g_LastCrashDumpPath);
    return true;
}

static LONG WINAPI r3dUnhandledExceptionFilter(EXCEPTION_POINTERS* exceptionPointers)
{
    r3dSafeLog("\n");
    r3dSafeLog("============================================================\n");
    r3dSafeLog("WarInc crash detected.\n");
    r3dSafeLog("============================================================\n");

    bool dumpCreated = r3dWriteMiniDump(exceptionPointers);

    r3dCloseLogFile();

    char message[4096] = {0};

    if (dumpCreated)
    {
        sprintf_s(
            message,
            "Application crashed.\n\nCrash dump saved:\n%s\n\nAlso check r3dLog.txt.",
            g_LastCrashDumpPath
        );
    }
    else
    {
        sprintf_s(
            message,
            "Application crashed.\n\nCrash dump was not created.\nCheck r3dLog.txt."
        );
    }

    MessageBoxA(NULL, message, "WarInc Crash", MB_OK | MB_ICONERROR);

    return EXCEPTION_EXECUTE_HANDLER;
}

r3dThreadAutoInstallCrashHelper::r3dThreadAutoInstallCrashHelper(DWORD)
{
    m_nInstallStatus = 0;

    if (!IsDebuggerPresent())
    {
        SetUnhandledExceptionFilter(r3dUnhandledExceptionFilter);
        m_nInstallStatus = 1;
    }
}

r3dThreadAutoInstallCrashHelper::~r3dThreadAutoInstallCrashHelper()
{
}

void r3dThreadEntryHelper(threadEntry_fn fn, DWORD in)
{
    if (!fn)
        return;

    if (IsDebuggerPresent())
    {
        fn(in);
        return;
    }

    SetUnhandledExceptionFilter(r3dUnhandledExceptionFilter);

    __try
    {
        fn(in);
    }
    __except (r3dUnhandledExceptionFilter(GetExceptionInformation()))
    {
    }
}