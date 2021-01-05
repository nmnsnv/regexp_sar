// IMPORTENT: This page is rather nonesence! do not look for much logic

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *cloneDoubleStr(char *fromStr, int size);
int *getSubArr(int *arr, int start, int end);
int factorial(int i);
int fibonaci(int i);

int main()
{
    char *doubleStr = cloneDoubleStr("hello", strlen("hello"));
    printf("New = %s\n", doubleStr);
    free(doubleStr);
    printf("fact 4: %d\n", factorial(4));
    int a[] = {1, 2, 3, 4, 5};
    getSubArr(a, 1, 3);
}

int factorial(int i)
{
    if (i == 1)
    {
        return 1;
    }
    else
    {
        return factorial(i - 1) * i;
    }
}

char *cloneDoubleStr(char *fromStr, int size)
{
    char *newChar = malloc(sizeof(char) * size * 2 + 1);
    memcpy(newChar, fromStr, size);
    memcpy(&newChar[size], fromStr, size);
    newChar[size * 2] = '\0';
    return newChar;
}

int *getSubArr(int *arr, int start, int end)
{
    int subSize = end - start;
    int *subArr = malloc(sizeof(int) * subSize);

    int i;
    for (i = start; i < end; i++)
    {
        subArr[i - start] = arr[i];
    }
    return subArr;
}

void someNestedMalloc(int bla, char boo)
{
    while (1)
    {
        fibonaci(4);
        while (1)
        {
            int *a = malloc(sizeof(int));
            free(a);
            break;
        }
        if (1)
        {
            int c = 1;
        }
        break;
    }
}

int someNestedWithoutMalloc()
{
    if (1)
    {
        if (0)
        {
            return 1;
        }
        else
        {
            return 2;
        }
        return 3;
    }
    return 4;
}

void mmalloc()
{
    if (0)
    {
        mmalloc();
    }
}

void mallocc()
{
    if (0)
    {
        mallocc();
    }
}

int fibonaci(int i)
{
    if (i <= 1)
    {
        return 1;
    }
    else
    {
        return fibonaci(i - 1) + fibonaci(i - 2);
    }
}
