#include <stdlib.h>
#include <stdio.h>
//=================
//=================
#include "thread.h"
#include "msg.h"
#include "ringbuffer.h"
//=================
//=================
#include "xtimer.h"
#include "timex.h"
//===================
//===================
#include "typedefs.h"

#define HANDLER_PRIO (THREAD_PRIORITY_MAIN - 1)

//================================================================
//==================NETWORK AND NON UART SECTION==================
//================================================================
#define NETWORK_DELAY           (30000LU * US_PER_MS) /* 5000 ms */
#define MEASURE_DELAY           (500LU * US_PER_MS) /* 500 ms */

static kernel_pid_t nonuart_handler_pid;
static char nonuart_handler_stack[THREAD_STACKSIZE_MAIN];

static kernel_pid_t network_handler_pid;
static char network_handler_stack[THREAD_STACKSIZE_MAIN];


extern void   process_imu(void);
extern void   process_vbr(void);

extern void json_convert(char * str);
extern void gsm_send_msg(char * msg);
extern void clear_data_imu(void);
extern void clear_data_vbr(void);

 static void * network_handler(void *arg)
 {
    (void)arg;
    xtimer_sleep(10);
   
    while (1) {
        xtimer_ticks32_t last = xtimer_now();
        puts("[CONTROLLER] [NET] [INFO]: Starting transmission...");
        char * buf = (char *) malloc(600);
        if (buf == NULL)  {
            puts("[CONTROLLER] [ERROR]: Can not allocate memory for json string. Can not send data to server.");
            return NULL;
        }

        json_convert(buf);
        clear_data_imu();
        clear_data_vbr();
        printf("[CONTROLLER] [NET] [DATA]: %s", buf);
        gsm_send_msg(buf);

        free(buf);


        puts("[CONTROLLER] [NET] [INFO]: Stoping transmission...");
        xtimer_periodic_wakeup(&last, NETWORK_DELAY);
    }

    /* this should never be reached */
    return NULL;
 }

 static void * nonuart_handler(void *arg)
 {
    (void)arg;
    xtimer_ticks32_t last = xtimer_now();
   
    while (1) {
         process_imu();
         process_vbr();
         xtimer_periodic_wakeup(&last, MEASURE_DELAY);
    }

    /* this should never be reached */
    return NULL;
 }

//================================================================
//=====================UART IO HANDLING SECTION===================
//================================================================
#define STRING_SIZE       (155U)
#define BUFFER_SIZE       (155U)
#define QUEUE_SIZE        (8U)

extern void process_gsm(char * str);
extern void process_gps(char * str);

static kernel_pid_t uart_handler_pid;
static char uart_handler_stack[THREAD_STACKSIZE_MAIN];

typedef struct {
   char rx_mem[BUFFER_SIZE];
   ringbuffer_t rx_buf;
} ctx_t;

static ctx_t * ctx;

void rx_cb(void *arg, uint8_t data)
{
   int dev = (int)arg;
   ringbuffer_add_one(&(ctx[dev].rx_buf), data);
   if (data == '\n') {
       msg_t msg;
       msg.content.value = dev;
       msg_send(&msg, uart_handler_pid);
   }
}

static void string_append(char * str, unsigned int lenght, char ch) {
    if(lenght >= BUFFER_SIZE - 1) {
        puts("[CONTROLLER] [UART] [ERROR]: Line buffer is full.");
        return;
    }
    str[lenght] = ch;
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
       char str[STRING_SIZE] = "";
       unsigned int lenght = 0;

       do {
           c = (int)ringbuffer_get_one(&(ctx[dev].rx_buf));
           string_append(str, lenght, c);
           lenght++;
       } while (c != '\n');
       str[lenght] = '\0';
       switch (dev) {
       case GSM_UART_DEV:
           process_gsm(str);
           break;
       case GPS_UART_DEV:
           process_gps(str);
           break;
       default:
           printf("[CONTROLLER] [UART] [ERROR]: Data received from unknown device. UART_DEV(%i) : %s\n", dev, str);
           break;
       }
   }

   /* this should never be reached */
   return NULL;
}



//================================================================
//======================INIT HANDLING SECTION=====================
//================================================================
void init_uart_handler(void) {
   puts("\n[CONTROLLER] [INFO]: Starting initialization of handlers.");

   //=====================================================
   //=====================================================
   int count = UART_DEV_COUNT;
   ctx = (ctx_t *) malloc(count * sizeof(ctx_t));
   if (ctx == NULL)  {
       puts("[CONTROLLER] [ERROR]: Can not allocate memory for ring buffers. Aborting...");
       return;
   }
   for (int i = 0; i < count; i++)
       ringbuffer_init(&(ctx[i].rx_buf), ctx[i].rx_mem, BUFFER_SIZE);
   puts("[CONTROLLER] [INFO]: Successfully initiated ringbuffers.");

   puts("[CONTROLLER] [INFO]: Starting UART handler thread...");
   uart_handler_pid = thread_create(uart_handler_stack, sizeof(uart_handler_stack),
                               HANDLER_PRIO, 0, uart_handler, NULL, "UART handler");
   if (uart_handler_pid < 0) {
      puts("[CONTROLLER] [ERROR]: Can not start UART handler thread. Aborting...");
      return;
   }
   else puts("[CONTROLLER] [INFO]: Successfully started UART handler thread.");

}



void init_nonuart_handler(void) {
    puts("\n[CONTROLLER] [INFO]: Starting NON UART handler thread...");
    nonuart_handler_pid = thread_create(nonuart_handler_stack, sizeof(nonuart_handler_stack),
                                HANDLER_PRIO, 0, nonuart_handler, NULL, "NON UART handler");
    if (nonuart_handler_pid < 0) {
       puts("[CONTROLLER] [ERROR]: Can not start NON UART handler thread. Aborting...");
       return;
    }
    else puts("[CONTROLLER] [INFO]: Successfully started NON UART handler thread.");
}

void init_network_handler(void) {
    puts("[CONTROLLER] [INFO]: Starting NETWORK handler thread...");
    network_handler_pid = thread_create(network_handler_stack, sizeof(network_handler_stack),
                                HANDLER_PRIO, 0, network_handler, NULL, "Network handler");
    if (network_handler_pid < 0) {
       puts("[CONTROLLER] [ERROR]: Can not start NETWORK handler thread. Aborting...");
       return;
    }
    else puts("[CONTROLLER] [INFO]: Successfully started NETWORK handler thread.");
}
