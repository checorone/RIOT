#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xtimer.h"
#include "periph/gpio.h"

#include "typedefs.h"

#define SDID        ("3dcc6661784f4bd2b21dc0a8ccf47084")
#define TOKEN       ("c8f17e3149174eb18959dce27b6381ff")
#define ENDPOINT    ("https://api.artik.cloud/v1.1")
#define CMDDELAY    (1)
#define REQDELAY    (10)

gpio_t PK_PIN = GPIO_PIN(PORT_C, 1);
gpio_t ST_PIN = GPIO_PIN(PORT_C, 0);

extern void uart_write_line(int dev, char * data);
extern int  uart_setup(int dev, uint32_t baud, rx_cb_t rx_cb);

extern int trace_gsm_on;

extern void rx_cb(void *arg, uint8_t data);

static void send_cmd(char * cmd);
static void send_data(char * data);

void init_gsm(void) {
    puts("\n[GSM] [INFO]: Starting initialization...");

    if( uart_setup(GSM_UART_DEV, 115200, rx_cb)  != 0) {
        puts("[GSM] [ERROR]: Failed to init UART port. Aborting...");
        return;
    }
    puts("[GSM] [INFO]: Successfully initiated UART port.");

    gpio_init(PK_PIN, GPIO_OUT);
    gpio_init(ST_PIN, GPIO_IN);
    puts("[GSM] [INFO]: Successfully enabled control pins.");

    if(gpio_read(ST_PIN) != 0) {
        puts("[GSM] [INFO]: Rebooting network device...");
        gpio_set(PK_PIN);
        xtimer_sleep(3);
        gpio_clear(PK_PIN);
        xtimer_sleep(3);
    }

    gpio_set(PK_PIN);
    xtimer_sleep(5);
    gpio_clear(PK_PIN);
    xtimer_sleep(5);

    puts("[GSM] [INFO]: Testing network device...");
    if(gpio_read(ST_PIN) == 0) {
        puts("[GSM] [ERROR]: Failed to turn on network device. Aborting...");
        return;
    }
    puts("[GSM] [INFO]: Successfully turned on network device.");

    puts("[GSM] [INFO]: Starting configuration of GSM device:");
    send_cmd("AT\r\n");
    send_cmd("AT+CPIN?\r\n");
    send_cmd("AT+CREG?\r\n");
    send_cmd("AT+CGATT?\r\n");
    send_cmd("AT+CIPSHUT\r\n");
    send_cmd("AT+CIPSTATUS\r\n");
    send_cmd("AT+CIPMUX=0\r\n");
    send_cmd("AT+CSTT=\"internet.beeline.ru\",\"beeline\",\"beeline\"\r\n");
    send_cmd("AT+CIICR\r\n");
    send_cmd("AT+CIFSR\r\n");
    puts("[GSM] [INFO]: Successfully initiated GSM device.");
}

void gsm_send_msg(char * msg) {
    (void)msg;
    if(trace_gsm_on) puts("\n[GSM] [INFO]: Initializing GPRS connection:");
    send_cmd("ATE0\r\n");
    send_cmd("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n");
    send_cmd("AT+SAPBR=3,1,\"APN\",\"internet\"\r\n");
    send_cmd("AT+SAPBR=1,1\r\n");
    send_cmd("AT+SAPBR=2,1\r\n");

    if(trace_gsm_on) puts("\n[GSM] [INFO]: Trying HTTP request:");
    send_cmd("AT+HTTPINIT\r\n");
    send_cmd("AT+HTTPPARA=\"CID\",1\r\n");
    send_cmd("AT+HTTPPARA=\"URL\",\"https://api.artik.cloud/v1.1/messages\"\r\n");
    send_cmd("AT+HTTPPARA=\"CONTENT\",\"application/json\"\r\n");
    send_cmd("AT+HTTPPARA=\"USERDATA\",\"Authorization:Bearer 84f792c411614df49f45806d0017bd04\"\r\n");
    send_cmd("AT+HTTPSSL=1\r\n");
    send_cmd("AT+HTTPSSL?\r\n");
    send_data(msg);
    send_cmd("AT+HTTPACTION=1\r\n");
    xtimer_sleep(REQDELAY);
    send_cmd("AT+HTTPREAD\r\n");

    if(trace_gsm_on) puts("\n[GSM] [INFO]: Terminate GPRS connection:");
    send_cmd("AT+HTTPTERM\r\n");
    send_cmd("AT+SAPBR=0,1\r\n");
}

void process_gsm(char * str) {
    if(trace_gsm_on && strlen(str) > 2)
        printf("[GSM] [ANSWER]: %s", str);
}

static void send_cmd(char * cmd) {

    if (trace_gsm_on) {
        puts("--------------------------");
        printf("[GSM] [COMMAND]: %s", cmd);
    }
    uart_write_line(GSM_UART_DEV, cmd);
    xtimer_sleep(CMDDELAY);
}

static void send_data(char * data) {
    char * cmd = "AT+HTTPDATA=";

    char lenght[4];
    __itoa(strlen(data), lenght,10);
    char latency[6];
    __itoa(10000, latency,10);

    char * str = (char *) calloc(strlen(cmd) + strlen(lenght) + strlen(",") + strlen(latency) + strlen("\r\n") + 1, sizeof(char));
    strcat(str, cmd);
    strcat(str, lenght);
    strcat(str, ",");
    strcat(str, latency);
    strcat(str, "\r\n");
    send_cmd(str);
    free(str);

    xtimer_sleep(CMDDELAY);
    uart_write_line(GSM_UART_DEV, data);
    xtimer_sleep(CMDDELAY);
    xtimer_sleep(CMDDELAY);
}

const char * get_dev_id(void) {
    return SDID;
}
