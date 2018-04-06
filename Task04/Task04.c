#include <stdio.h>
#include <conio.h>
#include <windows.h>

#define BUFF_SIZE           64000
#define THREAD_COUNT        1

typedef struct COPY_FILE_
{
    WCHAR FileIn[MAX_PATH];
    WCHAR FileOut[MAX_PATH];
} COPY_FILE, *PCOPY_FILE;

DWORD WINAPI AsyncCopyThread(LPVOID FilesToCopy)
{
    PCOPY_FILE Files = (PCOPY_FILE)FilesToCopy;

    HANDLE hSource = CreateFile(Files->FileIn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, FILE_FLAG_OVERLAPPED, NULL);
    if (hSource == INVALID_HANDLE_VALUE) {
        printf("hSource: Invalid Handle Value, error %d\n", GetLastError());
        return -1;
    }
    HANDLE hTarget = CreateFile(Files->FileOut, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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

        wcscpy_s(Files->FileIn, inplen, wFileIn);
        wcscpy_s(Files->FileOut, outlen, wFileOut);

        printf("Copying \"%ls\" as \"%ls\"\n", Files->FileIn, Files->FileOut);

        HANDLE hThread = CreateThread(NULL, 0, AsyncCopyThread, &Files, 0, &hThreadId);
        if (hThread == NULL) {
            printf("CreateThread failed, %d\n", GetLastError());
            break;
        }

        DWORD wait = WaitForSingleObject(hThread, INFINITE);
        if (wait != WAIT_FAILED) {
            printf("WaitForSingleObject() fail error %d\n", GetLastError());
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
        printf("Write output file name different from input\n");
        return -1;
    }

    WCHAR* out = (WCHAR*)malloc(MAX_PATH * sizeof(WCHAR));
    if (out == NULL) {
        printf("malloc() failed\n");
        return -1;
    }
    wcscpy_s(out, MAX_PATH * sizeof(WCHAR), argv[1]);
    
    HANDLE hSource = CreateFile(out, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSource == INVALID_HANDLE_VALUE) {
        printf("OUT: Invalid Handle Value, error %d\n", GetLastError());
        return -1;
    }

    INT cpy = FileCopyEx(argv[2], argv[1]);
    printf("File \"%ls\" is copied to \"%ls\"\n", argv[1], argv[2]);

    free(out);

    _getch();
    return 0;
}