#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <stdbool.h>

#define THREAD_INCREMENT            2
#define THREAD_DECREMENT            1
#define THREAD_TOTAL (THREAD_INCREMENT + THREAD_DECREMENT)
#define COUNT                       10
#define EVENT_COUNT                 2

HANDLE hEventArr[EVENT_COUNT];

typedef struct VALUE_
{
    INT value;
    INT vmax;
    INT vmin;
} VALUE;

DWORD WINAPI IncrementValue(LPVOID lpParameter)
{
    VALUE* v = (VALUE*)lpParameter;

    while (TRUE) {
        DWORD dwWaitStop = WaitForMultipleObjects(EVENT_COUNT, hEventArr, FALSE, INFINITE);
        if (dwWaitStop != WAIT_OBJECT_0) {
            printf("Couldnt wait for WAIT_OBJECT_0\n");
            return -1;
        }
        printf("Enter increment: %#x\n", GetCurrentThreadId());

        ++(v->value);
       
        if (v->value > v->vmax)
            v->vmax = v->value;
        if (v->value < v->vmin)
            v->vmin = v->value;
        printf("+ IncrementValue: actual value = %2d\n", v->value);
        
        printf("Exit  increment: %#x\n", GetCurrentThreadId());
        Sleep(250);
        SetEvent(hEventArr[0]);
    }
    printf("+ + + IncrementValue final result: %2d\n", v->value);

    return 0;
}

DWORD WINAPI DecrementValue(LPVOID lpParameter)
{
    VALUE* v = (VALUE*)lpParameter;

    while (TRUE) {
        DWORD dwWaitStop = WaitForMultipleObjects(EVENT_COUNT, hEventArr, FALSE, INFINITE);
        if (dwWaitStop != WAIT_OBJECT_0) {
            printf("Couldnt wait for WAIT_OBJECT_0\n");
            return -1;
        }

        printf("Enter decrement: %#x\n", GetCurrentThreadId());
        
        v->value -= 2;

        if (v->value > v->vmax)
            v->vmax = v->value;
        if (v->value < v->vmin)
            v->vmin = v->value;
        printf("- DecrementValue: actual value = %2d\n", v->value);
     
        printf("Exit  decrement: %#x\n", GetCurrentThreadId());
        Sleep(250);
        SetEvent(hEventArr[0]);
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
        hEventArr[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (hEventArr[0] == NULL) {
            printf("CreateEvent() failed. Code %d\n", GetLastError());
            break;
        }
        hEventArr[1] = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (hEventArr[1] == NULL) {
            printf("CreateEvent() failed. Code %d\n", GetLastError());
            break;
        }

        for (INT i = 0; i < THREAD_INCREMENT; ++i) {
            hThreadArr[i] = CreateThread(NULL, 0, IncrementValue, &v->value, 0, &hThreadArrId[i]);
            if (hThreadArr[i] == NULL) {
                printf("hThreadArrArray[%d] failed, error %d\n", i, GetLastError());
                break;
            }
        }
        hThreadArr[2] = CreateThread(NULL, 0, DecrementValue, &v->value, 0, &hThreadArrId[2]);
        if (hThreadArr[2] == NULL) {
            printf("hThreadArrArray[2] failed, error %d\n", GetLastError());
            break;
        }

        SetEvent(hEventArr[0]);
        
        _getch();
        printf("Button is pressed. Stop all threads\n");
        SetEvent(hEventArr[1]);
        break;

        INT wait = WaitForMultipleObjects(THREAD_TOTAL, hThreadArr, TRUE, INFINITE);
        if (wait == WAIT_FAILED) {
            printf("The function WaitForMultipleObjects() has been failed\n");
            break;
        }

        CloseHandle(hEventArr);

    } while (FALSE);
    
    printf("MIN: %d, MAX: %d\n", v->vmin, v->vmax);
    free(v);

    _getch();
    return 0;
}
