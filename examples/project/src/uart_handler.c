#include <stdlib.h>
#include <stdio.h>

#include "thread.h"
#include "msg.h"
#include "ringbuffer.h"

#include "typedefs.h"

#define BUFFER_SIZE (512U)
#define QUEUE_SIZE (16U)

#define HANDLER_PRIO        (THREAD_PRIORITY_MAIN - 1)

static kernel_pid_t handler_pid;
static char handler_stack[THREAD_STACKSIZE_MAIN];

typedef struct {
    char rx_mem[BUFFER_SIZE];
    ringbuffer_t rx_buf;
} ctx_t;

static ctx_t * ctx;

extern void init_shell(rx_cb_t cb);
extern int  uart_dev_count(void);
extern void process_data(string str);

void rx_cb(void *arg, uint8_t data)
{
    int dev = (int)arg;

    ringbuffer_add_one(&(ctx[dev].rx_buf), data);
    if (data == '\n') {
        msg_t msg;
        msg.content.value = dev;
        msg_send(&msg, handler_pid);
    }
}



static void string_append(string * str, char ch) {
    if(str->lenght == BUFFER_SIZE - 1) {
        puts("Error: line buf is full.");
        return;
    }
    str->buf[str->lenght] = ch;
    str->lenght++;
}

static void * uart_handler(void *arg)
{
    (void)arg;
    msg_t msg;
    msg_t msg_queue[QUEUE_SIZE];
    msg_init_queue(msg_queue, QUEUE_SIZE);

    while (1) {
        msg_receive(&msg);
        int dev = msg.content.value;
        char c;
        string str;
        str.lenght = 0;

        printf("UART_DEV(%i) RX: ", dev);
        do {
            c = (int)ringbuffer_get_one(&(ctx[dev].rx_buf));
            string_append(&str, c);
        } while (c != '\n');
        str.buf[str.lenght] = '\0';
        process_data(str);
    }

    /* this should never be reached */
    return NULL;
}

void init_handler(void) {
    /* start the handler thread */
    int size = uart_dev_count();
    ctx = (ctx_t *) malloc(size * sizeof(ctx_t));          
    if (ctx == NULL)  {
        puts("Error: Can not allocate memory for ring buffers.");
        return;
    }
    for (int i = 0; i < size; i++) {
        ringbuffer_init(&(ctx[i].rx_buf), ctx[i].rx_mem, BUFFER_SIZE);
    }
    puts("Successfully initialized ring buffers.");

    handler_pid = thread_create(handler_stack, sizeof(handler_stack),
                                HANDLER_PRIO, 0, uart_handler, NULL, "Print arrived messages");

    init_shell(rx_cb);
}
