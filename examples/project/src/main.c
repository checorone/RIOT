#include <stdio.h>

extern void init_handler(void);
extern void init_gps(void);

int main(void)
{
    puts("\nHello world!");
    init_gps();
    init_handler();
    return 0;
}
