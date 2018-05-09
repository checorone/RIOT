#include <stdio.h>
#include <stdlib.h>

#include "shell.h"

#include "typedefs.h"

static rx_cb_t rx_cb;

extern int  uart_check_device(int dev);
extern int  uart_enable(int dev, uint32_t baud, rx_cb_t rx_cb);
extern int  uart_disable(int dev);
extern void uart_info(void);
extern void uart_write_line(int dev, char * data);

static int parse_dev(char *arg)
{
    int dev = atoi(arg);
    int res = uart_check_device(dev);
    switch(res) {
        case 0: 
                return dev;
                break;
        case 1: 
                printf("Error: device can not be negative (%u).\n", dev);
                break;
        case 2: 
                printf("Error: Invalid UART_DEV device specified (%u).\n", dev);
                break;
        case 3: 
                printf("Error: The selected UART_DEV(%u) is used for the shell!\n", dev);
                break;
    }
    return -1;
}

static int cmd_start(int argc, char **argv)
{
    int dev, res;
    uint32_t baud;

    if (argc < 3) {
        printf("usage: %s <dev> <baudrate>\n", argv[0]);
        return 1;
    }
    /* parse parameters */
    dev = parse_dev(argv[1]);
    if (dev < 0) {
        return 1;
    }
    baud = atoi(argv[2]);

    res = uart_enable(dev, baud, rx_cb);
    if (res == 1) {
        printf("Error: Given baudrate (%u) not possible\n", (unsigned int)baud);
        return 1;
    }
    else if (res == 2) {
        puts("Error: Unable to initialize UART device\n");
        return 1;
    }
    else printf("Successfully initialized UART_DEV(%i)\n", dev);

    return 0;
}

static int cmd_stop(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s <dev> \n", argv[0]);
        return 1;
    }
    
    int dev = parse_dev(argv[1]);
    if (dev < 0) {
        return 1;
    }

    uart_disable(dev);
    printf("Successfully disabled UART_DEV(%i)\n", dev);

    return 0;
}


static int cmd_send(int argc, char **argv)
{
    int dev;
    
    if (argc < 3) {
        printf("usage: %s <dev> <data (string)>\n", argv[0]);
        return 1;
    }

    /* parse parameters */
    dev = parse_dev(argv[1]);
    if (dev < 0) {
        return 1;
    }

    printf("Sending \"%s\" to UART_DEV(%i).\n", argv[2], dev);
    uart_write_line(dev, argv[2]);
    return 0;
}

static const shell_command_t shell_commands[] = {
    { "start", "Start listening a UART device with a given baudrate", cmd_start },
    { "stop", "Stop listening a UART device", cmd_stop },
    { "send", "Send a string through given UART device", cmd_send },
    { NULL, NULL, NULL }
};

void init_shell(rx_cb_t cb) {
    rx_cb = cb;
    uart_info();
    /* run the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
}

