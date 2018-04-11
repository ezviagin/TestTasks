#include <stdio.h>
#include <conio.h>
#include <windows.h>

#define BUFF_SIZE           64000
#define THREAD_COUNT        1
#define FILE_COUNT          2
#define THREAD_DELAY        100

typedef struct COPY_FILE_
{
    WCHAR FileIn[MAX_PATH];
    WCHAR FileOut[MAX_PATH];
} COPY_FILE, *PCOPY_FILE;

DWORD WINAPI AsyncCopyThread(LPVOID FilesToCopy)
{
    PCOPY_FILE Files = (PCOPY_FILE)FilesToCopy;
    
    /* Test names as string (for a while) */
    HANDLE hSource = CreateFile(L"D:\\HLKStudio.iso", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
    if (hSource == INVALID_HANDLE_VALUE) {
        printf("hSource: Invalid Handle Value, error %d\n", GetLastError());
        return -1;
    }
    HANDLE hTarget = CreateFile(L"D:\\test.iso", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
    if (hTarget == INVALID_HANDLE_VALUE) {
        printf("hTarget: Invalid Handle Value, error %d\n", GetLastError());
        CloseHandle(hSource);
        return -1;
    }

    HANDLE hEventRead = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (hEventRead == NULL) {
        printf("CreateEvent() failed. Code %d\n", GetLastError());
        CloseHandle(hSource);
        CloseHandle(hTarget);
        return -1;
    }
    HANDLE hEventWrite = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (hEventWrite == NULL) {
        printf("CreateEvent() failed. Code %d\n", GetLastError());
        CloseHandle(hSource);
        CloseHandle(hTarget);
        CloseHandle(hEventRead);
        return -1;
    }

    UCHAR * buf = (UCHAR*)malloc(BUFF_SIZE);
    if (buf == NULL) {
        printf("Memory isn't allocated\n");
        CloseHandle(hSource);
        CloseHandle(hTarget);
        CloseHandle(hEventRead);
        CloseHandle(hEventWrite);
        return -1;
    }
    ZeroMemory(buf, BUFF_SIZE);

    OVERLAPPED OverlappedRead;
    OVERLAPPED OverlappedWrite;
   
    ZeroMemory(&OverlappedRead, sizeof(OVERLAPPED));
    ZeroMemory(&OverlappedWrite, sizeof(OVERLAPPED));
   
    OverlappedRead.hEvent  = hEventRead;
    OverlappedWrite.hEvent = hEventWrite;

    LARGE_INTEGER BytesRead;
    LARGE_INTEGER BytesWritten;

    INT RetValWrite = 0;
    INT RetValRead = 0;
    INT RetValOverlapped = 0;

    UINT i = 0;
    while (TRUE)
    {
        RetValRead = ReadFile(hSource, buf, BUFF_SIZE, NULL, &OverlappedRead);
        if (!RetValRead && (GetLastError() == ERROR_IO_PENDING)) {
            RetValOverlapped = GetOverlappedResult(&OverlappedRead.hEvent, &OverlappedRead, &BytesRead.u, TRUE);
            if (GetLastError() == ERROR_HANDLE_EOF) {
                printf("Copying is completed, error %d\n", GetLastError());
                break;
            }
        }
        
        RetValWrite = WriteFile(hTarget, buf, BUFF_SIZE, NULL, &OverlappedWrite);
        if (!RetValWrite && (GetLastError() == ERROR_IO_PENDING)) {
            RetValOverlapped = GetOverlappedResult(&OverlappedWrite.hEvent, &OverlappedWrite, &BytesWritten.u, TRUE);
            if (GetLastError() == FALSE) {
                printf("Copying is completed, error %d\n", GetLastError());
                break;
            }
        }
        
        if (BytesRead.QuadPart == 4194313)
            printf("Stop here, BytesRead.QuadPart == %lld, error %d\n", BytesRead.QuadPart, GetLastError());

        OverlappedWrite.Offset += BytesRead.LowPart;
        OverlappedWrite.OffsetHigh += BytesRead.HighPart;
        
        OverlappedRead.Offset += BytesWritten.LowPart;
        OverlappedRead.OffsetHigh += BytesWritten.HighPart;

        ++i;
    }

    free(buf);

    CloseHandle(hSource);
    CloseHandle(hTarget);
    CloseHandle(hEventRead);
    CloseHandle(hEventWrite);
    
    return 0;
}

INT FileCopyEx(WCHAR* wFileOut, WCHAR* wFileIn)
{
    DWORD hThreadId;
    PCOPY_FILE Files = (PCOPY_FILE)malloc(sizeof(COPY_FILE));
    if (Files == NULL) {
        printf("Malloc() failed\n");
        return -1;
    }

    do {
        size_t inplen = wcslen(wFileIn) + 1;
        size_t outlen = wcslen(wFileOut) + 1;
        if (inplen > MAX_PATH || outlen > MAX_PATH) {
            printf("Path to file is too long\n");
            break;
        }

        wcscpy_s(Files->FileIn, sizeof(Files->FileIn) / sizeof(WCHAR), wFileIn);
        wcscpy_s(Files->FileOut, sizeof(Files->FileIn) / sizeof(WCHAR), wFileOut);

        printf("Copying \"%ls\" as \"%ls\"\n", Files->FileIn, Files->FileOut);

        HANDLE hThread = CreateThread(NULL, 0, AsyncCopyThread, &Files, 0, &hThreadId);
        if (hThread == NULL) {
            printf("CreateThread failed, %d\n", GetLastError());
            break;
        }

        DWORD WaitForThread = WaitForSingleObject(hThread, INFINITE);
        if (WaitForThread == WAIT_FAILED) {
            printf("FileCopyEx(): WaitForSingleObject() fail error %d\n", GetLastError());
            break;
        }
    } while (FALSE);
 
    free(Files);
    return 0;
}

INT wmain(INT argc, WCHAR** argv)
{
    if (argc < 2) {
        printf("Write input and output file\n");
        return -1;
    }
    if (argc < 3) {
        printf("Write output file\n");
        return -1;
    }
    
    if (!wcscmp(argv[1], argv[2])) {
        printf("File already exists\n");
        return -1;
    }

    HANDLE hEventMain = CreateEvent(NULL, FALSE, FALSE, NULL);
    DWORD WaitMain = WaitForSingleObject(hEventMain, 0);
    if (WaitMain == WAIT_FAILED) {
        printf("Main(): WaitForSingleObject() failed, error %d\n", GetLastError());
    }


    INT ReturnValue = FileCopyEx(argv[2], argv[1]);
    if (ReturnValue == -1)
        printf("File \"%ls\" is copied to \"%ls\"\n", argv[1], argv[2]);

    SetEvent(hEventMain);

    _getch();
    return 0;
}