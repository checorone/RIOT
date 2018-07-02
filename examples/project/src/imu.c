#include <stdio.h>
#include <string.h>

#include "mutex.h"

#include "typedefs.h"

#define CTRL_REG1  (0x20)
#define CTRL_REG2  (0x21)
#define CTRL_REG3  (0x22)
#define CTRL_REG4  (0x23)
#define CTRL_REG5  (0x24)

#define ID_REG     (0x0F)

#define OUT_X      (0x28)
#define OUT_Y      (0x2A)
#define OUT_Z      (0x2C)
//=====================================
//=====================================
#define HYROSENSOR_ADR     (0b01101000)

#define HYROSENSOR_ID      (0xD3) 

#define HYRO_RANGE_250DPS  (0)
#define HYRO_RANGE_500DPS  (1)
#define HYRO_RANGE_2000DPS (2)

#define HYRO_SENS_FS_250   (0.00875)
#define HYRO_SENS_FS_500   (0.0175)
#define HYRO_SENS_FS_2000  (0.07)

#define HYRO_ADR_FS_250    (0x00)
#define HYRO_ADR_FS_500    (0x10)
#define HYRO_ADR_FS_2000   (0x20)

#define RAD_IN_DEG         (0.1745)
//==============================
//==============================
#define GSENSOR_ADR  (0b0011000)

#define GSENSOR_ID   (0x32)       

#define G_RANGE_2G   (2)
#define G_RANGE_4G   (4)
#define G_RANGE_8G   (8)

#define G_ADR_FS_2   (0x00)
#define G_ADR_FS_4   (0x10)
#define G_ADR_FS_8   (0x30)

#define G            (9.8)
//========================
//========================

typedef enum {
    hyro_sensor,
    g_sensor
} sensor_t;

extern int      i2c_setup(uint8_t dev_num);
extern int      i2c_check_dev(uint8_t dev_num, uint8_t addr, uint8_t reg_id, uint8_t chip_id);
extern int      i2c_write_one_reg(uint8_t dev_num, uint8_t addr, uint8_t reg, uint8_t byte);
extern uint8_t  i2c_read_one_reg(uint8_t dev_num, uint8_t addr, uint8_t reg);
extern uint16_t i2c_read_two_regs(uint8_t dev_num, uint8_t addr, uint8_t first);

extern int trace_imu_on;

imu_data_t imu_data;

static float gs_mult;
static float hs_mult;

static mutex_t mutex = MUTEX_INIT;

static int init_sensor(sensor_t sensor);
static int set_base_cfg(sensor_t sensor);
static int set_filter_cfg(sensor_t sensor);
static int set_range_cfg(sensor_t sensor, int range);

float read_GX(void);
float read_GY(void);
float read_GZ(void);
float read_deg_X(void);
float read_deg_Y(void);
float read_deg_Z(void);

void set_data_imu(float gx, float gy, float gz, float degx, float degy, float degz);
void get_data_imu(float *gx, float *gy, float *gz, float *degx, float *degy, float *degz);
void clear_data_imu(void);

void init_imu(void) {
    puts("\n[IMU] [INFO]: Starting initialization...");
    int res = i2c_setup(IMU_I2C_DEV);
    if (res == -1) {
        printf("[IMU] [ERROR]: i2c device (%d) creation failed. Aborting...\n", IMU_I2C_DEV);
        return;
    }
    else {
        printf("[IMU] [INFO]: Successfully created i2c device (%d).\n", IMU_I2C_DEV);
    }

    uint8_t status = 0;
    if(init_sensor(g_sensor) == -1)
        status += 1;
    if(init_sensor(hyro_sensor) == -1)
        status += 2;
    switch(status) {
        case 0: printf("[IMU] [INFO]: Successfully initiated IMU device on I2C_DEV(%d).\n", IMU_I2C_DEV); break;
        case 1: printf("[IMU] [ERROR]: GSensor initialization failed (IMU device on I2C_DEV(%d)).\n", IMU_I2C_DEV); break;
        case 2: printf("[IMU] [ERROR]: HyroSensor initialization failed (IMU device on I2C_DEV(%d)).\n", IMU_I2C_DEV); break;
        case 3: printf("[IMU] [ERROR]: All sensors failed on IMU device (I2C_DEV(%d)).\n", IMU_I2C_DEV); break;
    }
}

void process_imu(void) {
    float gx = read_GX();
    float gy = read_GY();
    float gz = read_GZ();
    float degx = read_deg_X();
    float degy = read_deg_Y();
    float degz = read_deg_Z();

    if(trace_imu_on) {
        puts("======================================================");
        printf("[IMU] [GSensor] [DATA]: X=%fg  Y=%fg  Z=%fg.\n", gx, gy, gz);
        printf("[IMU] [HyroSensor] [DATA]: X=%fdeg.  Y=%fdeg.  Z=%fdeg.\n", degx, degy, degz);
    }

    set_data_imu(gx,gy,gz,degx,degy,degz);
}

void set_data_imu(float gx, float gy, float gz, float degx, float degy, float degz) {
    mutex_lock(&mutex);
    if(imu_data.gx < gx)
        imu_data.gx = gx;
    if(imu_data.gy < gy)
        imu_data.gy = gy;
    if(imu_data.gz < gz)
        imu_data.gz = gz;
    if(imu_data.degx < degx)
        imu_data.degx = degx;
    if(imu_data.degy < degy)
        imu_data.degy = degy;
    if(imu_data.degz < degz)
        imu_data.degz = degz;
    mutex_unlock(&mutex);
}

void get_data_imu(float *gx, float *gy, float *gz, float *degx, float *degy, float *degz) {
    mutex_lock(&mutex);
    *gx = imu_data.gx;
    *gy = imu_data.gy;
    *gz = imu_data.gz;
    *degx = imu_data.degx;
    *degy = imu_data.degy;
    *degz = imu_data.degz;
    mutex_unlock(&mutex);
}

static int init_sensor(sensor_t sensor) {
    char sensor_name[11] = "empty";
    uint8_t addr;
    uint8_t id;
    int range;

    switch (sensor) {
    case g_sensor:
        strcpy(sensor_name, "GSensor");
        addr = GSENSOR_ADR;
        id = GSENSOR_ID;
        range = G_RANGE_2G;
        break;
    case hyro_sensor:
        strcpy(sensor_name, "HyroSensor");
        addr = HYROSENSOR_ADR;
        id = HYROSENSOR_ID;
        range = HYRO_RANGE_250DPS;
        break;
    default:
        puts("[IMU] [ERROR] : Invalid sensor given.");
        break;
    }
    
    int res;
    printf("[IMU] [%s] [INFO]: Checking sensor presence...\n", sensor_name);
    res = i2c_check_dev(IMU_I2C_DEV, addr, ID_REG, id);
    if (res == -1)
        printf("[IMU] [%s] [ERROR]: Failed to access sensor module. Skipping configuration step...\n", sensor_name);
    else if (res == -2)
        printf("[IMU] [%s] [ERROR]: Sensor module id mismatch. Skipping configuration step...\n", sensor_name);
    else if (res == 0) {
        printf("[IMU] [%s] [INFO]: Sensor successfully found. Starting configuration step...\n", sensor_name);
        res = 0;
        res += set_base_cfg(sensor);
        res += set_filter_cfg(sensor);
        res += set_range_cfg(sensor, range);
        if (res == 0)
            return 0;
        else
            return -1;
    }
    return -1;
}

static int set_base_cfg(sensor_t sensor) {
    uint8_t cfg = 0;
    uint8_t addr;
    char sensor_name[11];
    switch (sensor) {
        case g_sensor: {
            addr = GSENSOR_ADR;
            strcpy(sensor_name, "GSensor");
            cfg |= (1 << 0); // Enable X axis
            cfg |= (1 << 1); // Enable Y axis
            cfg |= (1 << 2); // Enable Z axis
            cfg |= (1 << 5); // Enter normal mode
            break;
        }
        case hyro_sensor: {
            addr = HYROSENSOR_ADR;
            strcpy(sensor_name, "HyroSensor");
            cfg |= (1 << 0); // Enable X axis
            cfg |= (1 << 1); // Enable Y axis
            cfg |= (1 << 2); // Enable Z axis
            cfg |= (1 << 3); // Enter normal mode (200 Hz)
            break;
        }
    }
    if(i2c_write_one_reg(IMU_I2C_DEV, addr, CTRL_REG1, cfg) == 1) {
        printf("[IMU] [%s] [INFO]: Successfully set working mode.\n", sensor_name);
        return 0;
    }
    else {
        printf("[IMU] [%s] [ERROR]: Failed to set working mode.\n", sensor_name);
        return -1;
    }
}

static int set_filter_cfg(sensor_t sensor) {
    uint8_t cfg2 = 0;
    uint8_t cfg5 = 0;
    uint8_t addr;
    char sensor_name[11];
    switch (sensor) {
        case g_sensor: {
            addr = GSENSOR_ADR;
            strcpy(sensor_name, "GSensor");
            //cfg2 |= (1 << 2);
            //cfg2 |= (1 << 3);
            //cfg2 |= (1 << 4);
            break;
        }
        case hyro_sensor: {
            addr = HYROSENSOR_ADR;
            strcpy(sensor_name, "HyroSensor");
            cfg2 |= (1 << 4);
            cfg5 |= (1 << 0);
            cfg5 |= (1 << 1);
            cfg5 |= (1 << 4);
            break;
        }
    }
    if(i2c_write_one_reg(IMU_I2C_DEV, addr, CTRL_REG2, cfg2) == 1 && i2c_write_one_reg(IMU_I2C_DEV, addr, CTRL_REG5, cfg5) == 1) {
        printf("[IMU] [%s] [INFO]: Successfully set filtering mode.\n", sensor_name);
        return 0;
    }
    else {
        printf("[IMU] [%s] [ERROR]: Failed to set filtering mode.\n", sensor_name);
        return -1;
    }
}

static int set_range_cfg(sensor_t sensor, int range) {
    uint8_t cfg = 0;
    uint8_t addr;
    char sensor_name[11];
    if(sensor == g_sensor) {
        strcpy(sensor_name, "GSensor");
        addr = GSENSOR_ADR;
        switch (range) {
            case G_RANGE_2G: {
                cfg = G_ADR_FS_2;
                gs_mult = G_RANGE_2G / 32767.0;
                break;
            }
            case G_RANGE_4G: {
                cfg = G_ADR_FS_4;
                gs_mult = G_RANGE_4G / 32767.0;
                break;
            }
            case G_RANGE_8G: {
                cfg = G_ADR_FS_8;
                gs_mult = G_RANGE_8G / 32767.0;
                break;
            }
            default: {
                gs_mult = G_RANGE_2G / 32767.0;    
            }
            break;
        }
    }
    else if (sensor == hyro_sensor) {
        strcpy(sensor_name, "HyroSensor");
        addr = HYROSENSOR_ADR;
        switch (range) {
            case HYRO_RANGE_250DPS: {
                cfg = HYRO_ADR_FS_250;
                hs_mult = HYRO_SENS_FS_250;
                break;
            }
            case HYRO_RANGE_500DPS: {
                cfg = HYRO_ADR_FS_500;
                hs_mult = HYRO_SENS_FS_500;
                break;
            }
            case HYRO_RANGE_2000DPS: {
                cfg = HYRO_ADR_FS_2000;
                hs_mult = HYRO_SENS_FS_2000;
                break;
            }
            default: {
                hs_mult = HYRO_ADR_FS_250;    
            }
            break;
        }
    }
    if(i2c_write_one_reg(IMU_I2C_DEV, addr, CTRL_REG4, cfg) == 1) {
        printf("[IMU] [%s] [INFO]: Successfully set range config.\n", sensor_name);
        return 0;
    }
    else {
        printf("[IMU] [%s] [ERROR]: Failed to set range config.\n", sensor_name);
        return -1;
    }
}


int16_t read_axis(uint8_t reg, uint8_t addr) {
    int16_t data = i2c_read_two_regs(IMU_I2C_DEV, addr, reg);
    return data;
}

int16_t read_X(uint8_t addr) {
    return read_axis(OUT_X, addr);
}

int16_t read_Y(uint8_t addr) {
    return read_axis(OUT_Y, addr);
}

int16_t read_Z(uint8_t addr) {
    return read_axis(OUT_Z, addr);
}

float read_GX(void) {
    return read_X(GSENSOR_ADR) * gs_mult;
}

float read_GY(void) {
    return read_Y(GSENSOR_ADR) * gs_mult;
}

float read_GZ(void) {
    return read_Z(GSENSOR_ADR) * gs_mult;
}

float read_AX(void) {
    return read_GX() * G;
}

float read_AY(void) {
    return read_GY() * G;
}

float read_AZ(void) {
    return read_GZ() * G;
}

float read_deg_X(void) {
    return read_X(HYROSENSOR_ADR) * hs_mult;
}

float read_deg_Y(void) {
    return read_Y(HYROSENSOR_ADR) * hs_mult;
}

float read_deg_Z(void) {
    return read_Z(HYROSENSOR_ADR) * hs_mult;
}

float read_rad_X(void) {
    return read_deg_X() * RAD_IN_DEG;
}

float read_rad_Y(void) {
    return read_deg_Y() * RAD_IN_DEG;
}

float read_rad_Z(void) {
    return read_deg_Z() * RAD_IN_DEG;
}

void clear_data_imu(void) {
    imu_data.gx = 0;
    imu_data.gy = 0;
    imu_data.gz = 0;
    imu_data.degx = 0;
    imu_data.degy = 0;
    imu_data.degz = 0;
}
