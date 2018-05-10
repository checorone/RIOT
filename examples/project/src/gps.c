#include <stdio.h>

#include "minmea.h"

#include "typedefs.h"

#define GPS_DEVICE (2)

typedef struct {
    float lattitude;
    float longitude;
    float speed;
} gps_data;

extern void uart_write_line(int dev, char * data);
extern int  uart_enable(int dev, uint32_t baud, rx_cb_t rx_cb);

void init_gps(void) {

    uart_enable(GPS_DEVICE, 115200, NULL);
    uart_write_line(GPS_DEVICE, "$PMTK314,0,2,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0*2A\r\n");
    //uart_write_line(GPS_DEVICE, "$PMTK220,10000*2F\r\n");
}

void process_data(string data) {
//    switch (minmea_sentence_id(data.buf, false)) {
//        case MINMEA_SENTENCE_RMC: {
//            struct minmea_sentence_rmc frame;
//            if (minmea_parse_rmc(&frame, data.buf)) {
//                printf("$RMC: raw coordinates and speed: (%ld/%ld,%ld/%ld) %ld/%ld\n",
//                       frame.latitude.value, frame.latitude.scale,
//                       frame.longitude.value, frame.longitude.scale,
//                       frame.speed.value, frame.speed.scale);
//                printf("$RMC fixed-point coordinates and speed scaled to three decimal places: (%ld,%ld) %ld\n",
//                       minmea_rescale(&frame.latitude, 1000),
//                       minmea_rescale(&frame.longitude, 1000),
//                       minmea_rescale(&frame.speed, 1000));
//                printf("$RMC floating point degree coordinates and speed: (%f,%f) %f\n",
//                       minmea_tocoord(&frame.latitude),
//                       minmea_tocoord(&frame.longitude),
//                       minmea_tofloat(&frame.speed));
//            }
//        } break;

//        case MINMEA_SENTENCE_GGA: {
//            struct minmea_sentence_gga frame;
//            if (minmea_parse_gga(&frame, data.buf)) {
//                printf("$GGA: fix quality: %d\n", frame.fix_quality);
//            }
//        } break;

//        case MINMEA_SENTENCE_GSV: {
//            struct minmea_sentence_gsv frame;
//            if (minmea_parse_gsv(&frame, data.buf)) {
//                printf("$GSV: message %d of %d\n", frame.msg_nr, frame.total_msgs);
//                printf("$GSV: sattelites in view: %d\n", frame.total_sats);
//                for (int i = 0; i < 4; i++)
//                    printf("$GSV: sat nr %d, elevation: %d, azimuth: %d, snr: %d dbm\n",
//                           frame.sats[i].nr,
//                           frame.sats[i].elevation,
//                           frame.sats[i].azimuth,
//                           frame.sats[i].snr);
//            }
//        } break;

//        default: puts(""); break;
//    }
    printf("%s", data.buf);
}
