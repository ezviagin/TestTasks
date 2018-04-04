#include <stdio.h>
#include <conio.h>
#include <windows.h>

#define THREAD_INCREMENT            2
#define THREAD_DECREMENT            1
#define THREAD_TOTAL (THREAD_INCREMENT + THREAD_DECREMENT)
#define COUNT                       10

HANDLE hEvent;

typedef struct VALUE_
{
    INT value;
    INT vmax;
    INT vmin;
} VALUE;

DWORD WINAPI IncrementValue(LPVOID lpParameter)
{
    VALUE* v = (VALUE*)lpParameter;

    for (USHORT i = 0; i < COUNT; ++i) {
        DWORD dwWaitResult = WaitForSingleObject(hEvent, INFINITE);;
        if (dwWaitResult != WAIT_OBJECT_0) {
            printf("Couldnt wait for WAIT_OBJECT_0\n");
            return -1;
        }
        ResetEvent(hEvent);
        printf("Enter increment: %#x\n", GetCurrentThreadId());

        ++(v->value);
       
        if (v->value > v->vmax)
            v->vmax = v->value;
        if (v->value < v->vmin)
            v->vmin = v->value;
        printf("+ IncrementValue: actual value = %2d\n", v->value);
        
        printf("Exit increment: %#x\n", GetCurrentThreadId());
        SetEvent(hEvent);
    }
    printf("+ + + IncrementValue final result: %2d\n", v->value);

    return 0;
}

DWORD WINAPI DecrementValue(LPVOID lpParameter)
{
    VALUE* v = (VALUE*)lpParameter;

    for (USHORT i = 0; i < COUNT; ++i) {
        DWORD dwWaitResult = WaitForSingleObject(hEvent, INFINITE);
        if (dwWaitResult != WAIT_OBJECT_0) {
            printf("Couldnt wait for WAIT_OBJECT_0\n");
            return -1;
        }
        ResetEvent(hEvent);
        printf("Enter decrement: %#x\n", GetCurrentThreadId());
        v->value -= 2;

        if (v->value > v->vmax)
            v->vmax = v->value;
        if (v->value < v->vmin)
            v->vmin = v->value;
        printf("- DecrementValue: actual value = %2d\n", v->value);
     
        printf("Exit decrement: %#x\n", GetCurrentThreadId());
        SetEvent(hEvent);
        
    }
    printf("- - - DecrementValue final result: %2d\n", v->value);
    
    return 0;
}

int main(int argc, char**argv)
{
    INT hThreadArrId[THREAD_TOTAL];
    HANDLE hThreadArr[THREAD_TOTAL];
    
    VALUE* v = (VALUE*)malloc(sizeof(VALUE));
    if (v == NULL) {
        printf("Malloc failed\n");
        return -1;
    }
    memset(v, 0, sizeof(VALUE));
    
    do {
        hEvent = CreateEvent(NULL, FALSE, FALSE, "Task03");
        if (hEvent == NULL) {
            printf("CreateEvent() failed. Code %d\n", GetLastError());
            break;
        }

        for (INT i = 0; i < THREAD_INCREMENT; ++i) {
            hThreadArr[i] = CreateThread(NULL, 0, IncrementValue, &v->value, 0, (LPDWORD)&hThreadArrId[i]);
            if (hThreadArr[i] == NULL) {
                printf("hThreadArrArray[%d] failed, error %d\n", i, GetLastError());
                break;
            }
        }

        hThreadArr[2] = CreateThread(NULL, 0, DecrementValue, &v->value, 0, (LPDWORD)&hThreadArrId[2]);
        if (hThreadArr[2] == NULL) {
            printf("hThreadArrArray[2] failed, error %d\n", GetLastError());
            break;
        }

        SetEvent(hEvent);

        INT wait = WaitForMultipleObjects(THREAD_TOTAL, hThreadArr, TRUE, INFINITE);
        if (wait == WAIT_FAILED) {
            printf("The function WaitForMultipleObjects() has been failed\n");
            break;
        }

        CloseHandle(hEvent);

    } while (FALSE);
    
    printf("MIN: %d, MAX: %d\n", v->vmin, v->vmax);

    free(v);

    _getch();
    return 0;
}
