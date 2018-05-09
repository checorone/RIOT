#include <stdio.h>

extern void init_handler(void);

int main(void)
{
    puts("\nHello world!");
    init_handler();
    return 0;
}
