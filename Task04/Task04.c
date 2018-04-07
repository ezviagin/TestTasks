#include <stdio.h>
#include <conio.h>
#include <windows.h>

#define BUFF_SIZE           64000
#define THREAD_COUNT        1
#define FILE_COUNT          2

HANDLE hEventHold;

typedef struct COPY_FILE_
{
    WCHAR FileIn[MAX_PATH];
    WCHAR FileOut[MAX_PATH];
} COPY_FILE, *PCOPY_FILE;

DWORD WINAPI AsyncCopyThread(LPVOID FilesToCopy)
{
    PCOPY_FILE Files = (PCOPY_FILE)FilesToCopy;

    printf("%ls and %ls\n", Files->FileIn, Files->FileOut);

    /* Test names as string (for a while) */
    HANDLE hSource = CreateFile(L"D:\\HLKStudio.iso", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, FILE_FLAG_OVERLAPPED);
    if (hSource == INVALID_HANDLE_VALUE) {
        printf("hSource: Invalid Handle Value, error %d\n", GetLastError());
        return -1;
    }
    HANDLE hTarget = CreateFile(L"D:\\test.iso", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hTarget == INVALID_HANDLE_VALUE) {
        printf("hTarget: Invalid Handle Value, error %d\n", GetLastError());
        CloseHandle(hSource);
        return -1;
    }

    UCHAR * buf = (UCHAR*)malloc(BUFF_SIZE);
    if (buf == NULL) {
        printf("Memory isn't allocated\n");
        CloseHandle(hSource);
        CloseHandle(hTarget);
        return -1;
    }
    ZeroMemory(buf, BUFF_SIZE);

    DWORD dwCountRead = 0;
    DWORD dwCountWrite = 0;

    //DWORD WaitEvent = WaitForSingleObject(hEventHold, INFINITE);
    /*if (WaitEvent == WAIT_FAILED) {
        printf("AsyncCopyThread(): WaitForSingleObject() failed, error %d\n", GetLastError());
        return -1;
    }*/

    while (TRUE) {
        if (!ReadFile(hSource, buf, BUFF_SIZE, &dwCountRead, NULL)) {
            printf("Can't read file, error %d\n", GetLastError());
            break;
        }
        if (dwCountRead == 0)
            break;
        if (!WriteFile(hTarget, buf, BUFF_SIZE, &dwCountWrite, NULL)) {
            printf("Can't write file, error %d\n", GetLastError());
            break;
        }
    }
    //SetEvent(hEventHold);

    free(buf);
    CloseHandle(hSource);
    CloseHandle(hTarget);
    
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

        hEventHold = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (hEventHold == NULL) {
            printf("CreateEvent() failed. Code %d\n", GetLastError());
            break;
        }

        HANDLE hThread = CreateThread(NULL, 0, AsyncCopyThread, &Files, 0, &hThreadId);
        if (hThread == NULL) {
            printf("CreateThread failed, %d\n", GetLastError());
            break;
        }

        //SetEvent(hEventHold);

        DWORD WaitThread = WaitForSingleObject(hThread, INFINITE);
        if (WaitThread != WAIT_FAILED) {
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

    /* Test for Valid Handle */
    HANDLE hSource = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSource == INVALID_HANDLE_VALUE) {
        printf("OUT: Invalid Handle Value, error %d\n", GetLastError());
        return -1;
    }

    INT ReturnValue = FileCopyEx(argv[2], argv[1]);
    /* Future return checks... */

    printf("File \"%ls\" is copied to \"%ls\"\n", argv[1], argv[2]);

    _getch();
    return 0;
}