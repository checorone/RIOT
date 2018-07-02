#include <stdio.h>

#include "mutex.h"

#include "minmea.h"

#include "typedefs.h"

extern void uart_write_line(int dev, char * data);
extern int  uart_setup(int dev, uint32_t baud, rx_cb_t rx_cb);

extern void rx_cb(void *arg, uint8_t data);

extern int trace_gps_on;

gps_data_t gps_data;

static mutex_t mutex = MUTEX_INIT;

void set_data_gps(float lat, float longt, float speed, float gn, float gp, float gl);
void get_data_gps(float *lat, float *longt, float *speed, float *gn, float *gp, float *gl);
void clear_data_gps(void);

void init_gps(void) {
    puts("\n[GPS] [INFO]: Starting initialization...");
    if( uart_setup(GPS_UART_DEV, 115200, rx_cb)  != 0) {
        puts("[GPS] [ERROR]: Failed to init UART port. Aborting...");
        return;
    }
    puts("[GPS] [INFO]: Successfully initiated UART port.");
    uart_write_line(GPS_UART_DEV, "$PMTK314,0,5,0,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0*2D\r\n");
    puts("[GPS] [INFO]: Successfully initiated GPS devide.");
    //uart_write_line(GPS_DEVICE, "$PMTK220,10000*2F\r\n");
}

void process_gps(char * str) {
    switch (minmea_sentence_id(str, false)) {
    case MINMEA_SENTENCE_RMC: {
        struct minmea_sentence_rmc frame;
        if (minmea_parse_rmc(&frame, str)) {
            float latitude  = minmea_tocoord(&frame.latitude);
            float longitude = minmea_tocoord(&frame.longitude);
            float speed     = minmea_tofloat(&frame.speed);
            if(trace_gps_on) {
                puts("==============================================================");
                printf("[GPS] [DATA]: GN: Latitude=%f;  Longitude=%f;  Speed=%f.\n", latitude, longitude, speed);
            }
            set_data_gps(latitude,longitude,speed,0,0,0);
        }
    } break;

    case MINMEA_SENTENCE_GGA: {
        struct minmea_sentence_gga frame;
        if (minmea_parse_gga(&frame, str)) {
            float gn_hdop = minmea_tofloat(&frame.hdop);
            if(trace_gps_on)
                printf("[GPS] [DATA]: GLONASS and GPS horizontal DOP = %f .\n", gn_hdop);
            set_data_gps(0,0,0,gn_hdop,0,0);
        }
    } break;

    case MINMEA_SENTENCE_GSA: {
        struct minmea_sentence_gsa frame;
        if (minmea_parse_gsa(&frame, str)) {
            if (str[2] == 'P') {
                float gp_hdop = minmea_tofloat(&frame.hdop);
                if(trace_gps_on)
                    printf("[GPS] [DATA]: GPS horizontal DOP = %f .\n", gp_hdop);
                set_data_gps(0,0,0,0,gp_hdop,0);
            }
            else if (str[2] == 'L') {
                float gl_hdop = minmea_tofloat(&frame.hdop);
                if(trace_gps_on)
                    printf("[GPS] [DATA]: GLONASS horizontal DOP = %f .\n", gl_hdop);
                set_data_gps(0,0,0,0,0,gl_hdop);
            }
        }
    } break;

    default: if(trace_gps_on) printf("[GPS] [RESPONSE]: Resieved response: \"%s\"\n", str); break;
    }
}

void set_data_gps(float lat, float longt, float speed, float gn, float gp, float gl) {
    mutex_lock(&mutex);
    if(lat != 0)
        gps_data.latitude = lat;
    if(longt != 0)
        gps_data.longitude = longt;
    if(speed != 0)
        gps_data.speed = speed;
    if(gn != 0)
        gps_data.gn_hdop = gn;
    if(gp != 0)
        gps_data.gp_hdop = gp;
    if(gl != 0)
        gps_data.gl_hdop = gl;
    mutex_unlock(&mutex);
}

void get_data_gps(float *lat, float *longt, float *speed, float *gn, float *gp, float *gl) {
    mutex_lock(&mutex);
    *lat = gps_data.latitude;
    *longt = gps_data.longitude;
    *speed = gps_data.speed;
    *gn = gps_data.gn_hdop;
    *gp = gps_data.gp_hdop;
    *gl = gps_data.gl_hdop;
    mutex_unlock(&mutex);
}

void clear_data_gps(void) {
    gps_data.latitude = 0;
    gps_data.longitude = 0;
    gps_data.speed = 0;
    gps_data.gn_hdop = 0;
    gps_data.gp_hdop = 0;
    gps_data.gl_hdop = 0;
}
