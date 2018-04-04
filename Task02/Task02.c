#include <stdio.h>
#include <conio.h>
#include <windows.h>

#define THREAD_NUM 2
#define BASE_THREAD_NUM 1
#define COUNT				10

typedef struct DATA_ {
    INT threadNum;
    INT data[COUNT];
} DATA, *PDATA;

DWORD WINAPI PrintValsThread(LPVOID lpParam)
{
    DATA* pVal = (DATA*)lpParam;
    for (BYTE i = 0; i < COUNT; ++i) {
        pVal->data[i] = i;
        printf("Thread %d, i = %d\n", pVal->threadNum, pVal->data[i]);
    }
    return 0;
}

INT main(INT argc, WCHAR **argv)
{
    DATA* pVal[THREAD_NUM + 1];
    DWORD dwthreadNumArray[THREAD_NUM];
    HANDLE hThreadArray[THREAD_NUM];

    for (INT i = 0; i < THREAD_NUM; ++i) {
        pVal[i] = (DATA*)malloc(sizeof(DATA));
        if (pVal[i] == NULL) {
            printf("pVal[%d] == NULL\n", i);
            break;
        }
        pVal[i]->threadNum = BASE_THREAD_NUM + i;

        hThreadArray[i] = CreateThread(NULL, 0, PrintValsThread, pVal[i], 0, &dwthreadNumArray[i]);
        if (hThreadArray[i] == NULL) {
            printf("hThreadArray[%d] is NULL\n", i);
            break;
        }
    }

    DATA mainThreadData;
    mainThreadData.threadNum = 0;
    for (INT i = 0; i < COUNT; ++i) {
        mainThreadData.data[i] = i;
        printf("Thread %d, i = %d\n", mainThreadData.threadNum, mainThreadData.data[i]);
    }

    INT wait = WaitForMultipleObjects(THREAD_NUM, hThreadArray, TRUE, INFINITE);
    if (wait == WAIT_FAILED)
        printf("The function WaitForMultipleObjects() has been failed\n");

    for (INT i = 0; i < THREAD_NUM; ++i) {
        if (pVal[i] != NULL)
            free(pVal[i]);
        CloseHandle(hThreadArray[i]);
    }

    printf("It's over\n");
    _getch();
    return 0;
}