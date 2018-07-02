#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shell.h"

#include "typedefs.h"

extern void uart_info(void);
extern void i2c_info(void);
extern void adc_info(void);

extern void get_data_vbr(float * vibration);
extern void get_data_imu(float *gx, float *gy, float *gz, float *degx, float *degy, float *degz);
extern void get_data_gps(float *lat, float *longt, float *speed, float *gn, float *gp, float *gl);

extern void uart_write_line(int dev, char * data);
extern void json_convert(char * str);

extern void gsm_send_msg(char * msg);

int trace_gps_on = 0;
int trace_imu_on = 0;
int trace_vbr_on = 0;
int trace_gsm_on = 1;

static int cmd_get(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    puts("\n[SHELL] [INFO]: Carrying info from modules...");
    
    float latitude;
    float longitude;
    float speed;
    float gn_hdop;
    float gp_hdop;
    float gl_hdop;
    float gx;
    float gy;
    float gz;
    float degx;
    float degy;
    float degz;
    float vibration;

    get_data_vbr(&vibration);
    get_data_imu(&gx, &gy, &gz, &degx, &degy, &degz);
    get_data_gps(&latitude, &longitude, &speed, &gn_hdop, &gp_hdop, &gl_hdop);

    printf("[SHELL] [DATA]: latitude=%f longitude=%f speed=%f GP and GL HDOP=%f GP HDOP=%f GL HDOP=%f\n",
        latitude, longitude, speed, gn_hdop, gp_hdop, gl_hdop);
    printf("[SHELL] [DATA]: Acceleration: X=%fg Y=%fg Z=%fg Rotattion: X=%fdeg Y=%fdeg Z=%fdeg\n",
        gx, gy, gz, degx, degy, degz);
    printf("[SHELL] [DATA]: Vibration=%f\n",
        vibration);

    return 0;
}

static int cmd_info(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    uart_info();
    i2c_info();
    adc_info();

    return 0;
}


static int cmd_send(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    char * buf = (char *) malloc(600);
    if (buf == NULL)  {
        puts("[SHELL] [ERROR]: Can not allocate memory for json string. Aborting...");
        return 1;
    }

    json_convert(buf);
    gsm_send_msg(buf);

    free(buf);
    return 0;
}

static int cmd_trace(int argc, char **argv)
{   
    if (argc < 3) {
        printf("usage: %s <dev> <mode> \n", argv[0]);
        return 1;
    }

    /* parse device */
    char dev[4];
    int dev_num = -1;
    if (strcmp(argv[1], "gps") == 0) {
        strcpy(dev, "GPS");
        dev_num = 0;
    }
    else if (strcmp(argv[1], "imu") == 0) {
        strcpy(dev, "IMU");
        dev_num = 1;
    }
    else if (strcmp(argv[1], "vbr") == 0) {
        strcpy(dev, "VBR");
        dev_num = 2;
    }
    else if (strcmp(argv[1], "gsm") == 0) {
        strcpy(dev, "GSM");
        dev_num = 3;
    }
    else {
        printf("usage: %s <dev> <mode> \n", argv[0]);
        return 1;
    }


    /* parse mode */
    char mode[10];
    int trace_on = 0;
    if (strcmp(argv[2], "on") == 0) {
        strcpy(mode, "enabled");
        trace_on = 1;
    }
    else if (strcmp(argv[2], "off") == 0) {
        strcpy(mode, "disabled");
        trace_on = 0;
    }
    else {
        printf("usage: %s <dev> <mode> \n", argv[0]);
        return 1;
    }

    switch (dev_num) {
    case 0:
        trace_gps_on = trace_on;
        break;
    case 1:
        trace_imu_on = trace_on;
        break;
    case 2:
        trace_vbr_on = trace_on;
        break;
    case 3:
        trace_gsm_on = trace_on;
        break;
    default:
        break;
    }

    printf("[SHELL] [INFO]: Tracing mode for device \"%s\" %s\n", dev, mode);

    return 0;
}

static const shell_command_t shell_commands[] = {
    { "info", "Print info about ports.", cmd_info },
    { "get",  "Get current data.", cmd_get },
    { "send", "Send data to server.", cmd_send },
    { "trace", "Enable/disable tracing data.", cmd_trace },
    { NULL, NULL, NULL }
};

void init_shell(void) {
    uart_info();
    i2c_info();
    adc_info();
    /* run the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
}

