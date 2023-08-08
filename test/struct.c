#include "stdio.h"

typedef struct
{
    const char *name[2];
    int a[2];
} A;
typedef struct
{
    A a;
}B;

B b[10] = {
    {{"hello","111"}, {1,2}},
    {{"world","222"}, {3,4}},
    };
int main()
{
    printf("%s\n", b[0].a.name[0]);
    printf("%d\n",b[1].a.a[0]);
    return 0;
}