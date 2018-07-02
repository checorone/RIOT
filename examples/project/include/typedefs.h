#pragma once

#define GPS_UART_DEV (2)
#define GSM_UART_DEV (1)
#define IMU_I2C_DEV  (0)
#define VBR_ADC_DEV  (0)

#define UART_DEV_COUNT (3)

#define PACKET_MAX_LEN (256)

typedef struct {
    float latitude;
    float longitude;

    float speed;

    float gn_hdop;
    float gp_hdop;
    float gl_hdop;
} gps_data_t;

typedef struct {
   	float gx;
   	float gy;
   	float gz;

   	float degx;
   	float degy;
   	float degz;
} imu_data_t;

typedef struct 
{
	float vibration;
} vbr_data_t;


typedef void(* rx_cb_t) (void *arg, uint8_t data);
