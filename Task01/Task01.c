#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <stdbool.h>
#include <windows.h>
#define BUFF_SIZE           512
#define OUTPUT_FORMAT_STR   "%-50s%-19s%-22s%s\n\n"

typedef struct FORMAT_
{
    FLOAT size;
    WCHAR * sFormat;
} FORMAT, *PFORMAT;

typedef struct FILE_DESC_
{
    WCHAR cFileName[MAX_PATH];
    LONGLONG fSize;
    FILETIME time;
    DWORD dwFileAttributes;
    WCHAR sFileAttributes[4];
    SYSTEMTIME systime;
    struct FILE_DESC_* next;
} FILE_DESC, *PFILE_DESC;

VOID CopyExFile(WCHAR* szNameOut, WCHAR* szNameIn)
{
    DWORD dwCountRead = 0, dwCountWrite = 0;

    HANDLE hSource = CreateFile(szNameIn, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE hTarget = CreateFile(szNameOut, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hSource == INVALID_HANDLE_VALUE) {
        if (hTarget != INVALID_HANDLE_VALUE)
            CloseHandle(hSource);
        return;
    }
    if (hTarget == INVALID_HANDLE_VALUE) {
        if (hSource != INVALID_HANDLE_VALUE)
            CloseHandle(hTarget);
        return;
    }

    UCHAR * buf = (UCHAR*)malloc(BUFF_SIZE);
    if (buf == NULL) {
        printf("Memory isn't allocated\n");
        return;
    }

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
}

FORMAT ConvertToFormat(FLOAT fileSize)
{
    FORMAT bytes;
    WCHAR * str[] = { L"bytes", L"Kbytes", L"Mbytes", L"Gbytes" };
    bytes.size = 0;
    bytes.sFormat = str[0];

    BYTE i = 0;
    while ((fileSize > 999) && (i < 4))
    {
        if (fileSize == 0)
            break;
        bytes.size = (FLOAT)fileSize / 1024;
        bytes.sFormat = str[i + 1];
        (FLOAT)fileSize = bytes.size;
        ++i;
    }
    return bytes;
}

WCHAR* ParseFileAttribute(DWORD fileAttributes, WCHAR* str, DWORD size)
{
    if (size < 4)
        return NULL;
    for (BYTE i = 0; i < 3; ++i) {
        str[i] = L'-';
    }
    str[3] = L'\0';

    if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        str[0] = L'd';
    if (fileAttributes & FILE_ATTRIBUTE_ARCHIVE)
        str[0] = L'a';
    if (fileAttributes & FILE_ATTRIBUTE_READONLY)
        str[1] = L'r';
    if (fileAttributes & FILE_ATTRIBUTE_HIDDEN)
        str[2] = L'h';

    return str;
}

FILE_DESC* SortFilesBySize(FILE_DESC* folderData, UINT count)
{
    FILE_DESC temp;
    for (UINT i = 0; i < count; ++i) {
        for (UINT j = 0; j < count; ++j) {
            if (folderData[i].fSize > folderData[j].fSize) {
                temp = folderData[i];
                folderData[i] = folderData[j];
                folderData[j] = temp;
            }
        }
    }
    return folderData;
}

VOID FormPath(WCHAR* res, size_t len, WCHAR* s1, WCHAR* s2)
{
    *res = 0;
    wcscpy_s(res, len, s1);
    if (res[wcslen(res) - 1] == '*')
        res[wcslen(res) - 1] = L'\0';
    wcscat_s(res, len, s2);
}

bool isValidDir(WIN32_FIND_DATA data)
{
    if (!wcscmp(data.cFileName, L"..") || !wcscmp(data.cFileName, L"."))
        return false;
    else
        return true;
}

VOID CheckValidDir(WCHAR* path, WCHAR* subfolder, size_t folderLen, WIN32_FIND_DATA data)
{
    FormPath(subfolder, folderLen, path, data.cFileName);
    wcscat(subfolder, L"\\*");
}

INT PrintFileInfo(FILE_DESC* folderData)
{
    FORMAT format;
    SYSTEMTIME systime;

    if (FileTimeToSystemTime(&folderData->time, &systime) == 0) {
        printf("FileTimeToSystemTime conversion failed\n");
        free(folderData);
        return -1;
    }
    format = ConvertToFormat((FLOAT)folderData->fSize);
    printf("%-50ls%-7.2lf %-10ls %02u-%02u-%02u  %02d:%02d:%02d %4ls\n",
        folderData->cFileName, format.size, format.sFormat, systime.wYear, systime.wMonth, systime.wDay,
        systime.wHour, systime.wMinute, systime.wSecond, folderData->sFileAttributes);
    return 0;
}

VOID WriteFileData(FILE_DESC* folderData, WIN32_FIND_DATA* data)
{
    LARGE_INTEGER fsize;
    wcscpy_s(folderData->cFileName, sizeof(folderData->cFileName) / sizeof(WCHAR), data->cFileName);

    fsize.LowPart = data->nFileSizeLow;
    fsize.HighPart = data->nFileSizeHigh;
    folderData->fSize = fsize.QuadPart;

    folderData->time = data->ftLastAccessTime;
    if (FileTimeToSystemTime(&folderData->time, &folderData->systime) == 0) {
        printf("FileTimeToSystemTime conversion failed\n");
        return;
    }

    ParseFileAttribute(data->dwFileAttributes, folderData->sFileAttributes, sizeof(folderData->sFileAttributes) / sizeof(WCHAR));
    folderData->dwFileAttributes = data->dwFileAttributes;
}

INT PrintFileInfoEx(FILE_DESC* folderData)
{
    FORMAT format;

    printf("\n"OUTPUT_FORMAT_STR, "NAME", "SIZE", "DATE AND TIME", "ATTRIBUTES");
    while (folderData != NULL) {
        format = ConvertToFormat((FLOAT)folderData->fSize);
        printf("%-50ls%-7.2lf %-10ls %02u-%02u-%02u  %02d:%02d:%02d %4ls\n", folderData->cFileName, format.size, format.sFormat,
            folderData->systime.wYear, folderData->systime.wMonth, folderData->systime.wDay,
            folderData->systime.wHour, folderData->systime.wMinute, folderData->systime.wSecond, folderData->sFileAttributes);

        folderData = folderData->next;
    }
    return 0;
}

VOID DeleteList(FILE_DESC* head)
{
    FILE_DESC* previous = NULL;
    while (head != NULL) {
        previous = head;
        head = head->next;
        free(previous);
    }
}

LONGLONG GetDirSize(WCHAR* path)
{
    WIN32_FIND_DATA data;
    LARGE_INTEGER fsize;
    LONGLONG tsum = 0, psum = 0;

    HANDLE hFind = FindFirstFile(path, &data);
    if (hFind == INVALID_HANDLE_VALUE)
        return 0;

    do {
        if (!isValidDir(data))
            continue;
        size_t folderLen = wcslen(path) + wcslen(data.cFileName) + wcslen(L"\\*") + 1;
        WCHAR* subfolder = (WCHAR*)malloc(folderLen * sizeof(WCHAR));
        if (subfolder == NULL) {
            printf("Memory isn't allocated in GetDirSize()\n");
            break;
        }

        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            CheckValidDir(path, subfolder, folderLen, data);
            psum = GetDirSize(subfolder);
        }
        fsize.LowPart = data.nFileSizeLow;
        fsize.HighPart = data.nFileSizeHigh;
        tsum += fsize.QuadPart;
        tsum += psum;
        psum = 0;
        free(subfolder);
    } while (FindNextFile(hFind, &data));

    FindClose(hFind);
    return tsum;
}

INT FindFilesInDir(WCHAR* path)
{
    WIN32_FIND_DATA data;
    HANDLE hFind = FindFirstFile(path, &data);
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("Invalid handle value, error %d", GetLastError());
        return -1;
    }

    FILE_DESC* head = NULL;
    FILE_DESC* last = NULL;

    do {
        FILE_DESC* folderData = (FILE_DESC*)malloc(sizeof(FILE_DESC));
        if (folderData == NULL) {
            printf("Memory isn't allocated\n");
            return -1;
        }

        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && (isValidDir(data) == true)) {
            size_t folderLen = wcslen(data.cFileName) + wcslen(path) + wcslen(L"\\*") + 1;
            WCHAR* subfolder = (WCHAR*)malloc(folderLen * sizeof(WCHAR));
            if (subfolder == NULL) {
                printf("Memory isn't allocated\n");
                free(folderData);
                break;
            }

            CheckValidDir(path, subfolder, folderLen, data);
            LARGE_INTEGER dsize;
            dsize.QuadPart = GetDirSize(subfolder);
            data.nFileSizeHigh = dsize.HighPart;
            data.nFileSizeLow = dsize.LowPart;
            free(subfolder);
        }

        WriteFileData(folderData, &data);

        folderData->next = NULL;
        if (head == NULL) {
            head = folderData;
            last = head;
        }
        else {
            last->next = folderData;
            last = folderData;
        }
    } while (FindNextFile(hFind, &data));

    PrintFileInfoEx(head);
    DeleteList(head);

    FindClose(hFind);
    return 0;
}

INT main(INT argc, CHAR *argv[])
{
    WCHAR* dirPath = L"E:\\*";

    INT findFiles = FindFilesInDir(dirPath);
    if (findFiles == -1)
        printf("FindFilesInDir() failed\n");

    _getch();
    return 0;
}