#include <stdio.h>
#include "xtimer.h"
extern void init_uart_handler(void);
extern void init_nonuart_handler(void);
extern void init_network_handler(void);
extern void init_gps(void);
extern void init_gsm(void);
extern void init_imu(void);
extern void init_vbr(void);
extern void init_shell(void);

int main(void)
{
    puts("\n==========================");
    puts("=======Hello world!=======");
    puts("==========================");
    xtimer_init();
    init_uart_handler();
    init_gps();
    init_vbr();
    init_imu();
    init_nonuart_handler();
    init_gsm();
    init_network_handler();
    init_shell();
    return 0;
}
