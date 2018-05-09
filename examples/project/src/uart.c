#include <string.h>

#include "board.h"
#include "periph/uart.h"
#include "uart_stdio.h"
#include "xtimer.h"

#include "typedefs.h"

#define POWEROFF_DELAY      (250U * US_PER_MS)      /* quarter of a second */

#ifndef UART_STDIO_DEV
#define UART_STDIO_DEV      (UART_UNDEF)
#endif

int uart_enable(int dev, uint32_t baud, rx_cb_t rx_cb) {
    /* initialize UART */
    int res = uart_init(UART_DEV(dev), baud, rx_cb, (void *)dev);
    if (res == UART_NOBAUD) 
        return 1;
    else if (res != UART_OK) 
        return 2;

    /* Test if poweron() and poweroff() work (or at least don't break
     * anything) */
    uart_poweroff(UART_DEV(dev));
    xtimer_usleep(POWEROFF_DELAY);
    uart_poweron(UART_DEV(dev));

    return 0;
}

int uart_disable(int dev) {
    uart_poweroff(UART_DEV(dev));
    return 0;
}

int uart_check_device(int dev) {
    if (dev < 0)
        return 1;
    if ((unsigned)dev >= UART_NUMOF)
        return 2;
    else if (UART_DEV(dev) == UART_STDIO_DEV)
        return 3;
    else 
        return 0;
}

int uart_dev_count(void) {
    return UART_NUMOF;
}

void uart_info(void) {
    puts("\nUART INFO:");
    printf("Available devices:               %i\n", UART_NUMOF);
    printf("UART used for STDIO (the shell): UART_DEV(%i)\n\n", UART_STDIO_DEV);
}

void uart_write_line(int dev, char * data) {
    uint8_t endline = (uint8_t)'\n';
    uart_write(UART_DEV(dev), (uint8_t *)data, strlen(data));
    uart_write(UART_DEV(dev), &endline, 1);
}
