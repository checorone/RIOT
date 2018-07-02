#include "board.h"
#include "periph/i2c.h"


#include "typedefs.h"

int i2c_setup(int dev_num) {
     i2c_t dev = I2C_DEV(dev_num);
     i2c_acquire(dev);
     if (i2c_init_master(dev, I2C_SPEED_NORMAL) < 0) {
         i2c_release(dev);
         return -1;
     }
     i2c_release(dev);
     return 0;
}

int i2c_check_dev(int dev_num, uint8_t addr, uint8_t reg_id, uint8_t chip_id) {
    i2c_t dev = I2C_DEV(dev_num);
    uint8_t id;
    i2c_acquire(dev);
    if (i2c_read_reg(dev, addr, reg_id, (char *)&id) != 1) {
        i2c_release(dev);
        return -1;
    }

    if (id != chip_id) {
        i2c_release(dev);
        return -2;
    }
    i2c_release(dev);
    return 0;
}

int16_t i2c_read_two_regs(int dev_num, uint8_t addr, uint8_t first) {
         int16_t data;
         int8_t low;
         int8_t high;
	 i2c_t dev = I2C_DEV(dev_num);
	 i2c_acquire(dev);
         i2c_read_reg(dev, addr, first, (char *)&low);
         i2c_read_reg(dev, addr, first + 1, (char *)&high);
	 i2c_release(dev);
         data = (high << 8) | low;
	 return data;
}

int8_t i2c_read_one_reg(int dev_num, uint8_t addr, uint8_t reg) {
	 int8_t data;
	 i2c_t dev = I2C_DEV(dev_num);
	 i2c_acquire(dev);
	 i2c_read_reg(dev, addr, reg, (char *)&data);
	 i2c_release(dev);
	 return data;
}

int i2c_write_one_reg(int dev_num, uint8_t addr, uint8_t reg, uint8_t byte) {
	 i2c_t dev = I2C_DEV(dev_num);
	 i2c_acquire(dev);
	 int status = i2c_write_reg(dev, addr, reg, byte);
	 i2c_release(dev);
	 return status;
}

void i2c_info(void) {
    puts("\n[I2C INFO]:");
    printf("Available I2C devices:               %i\n", I2C_NUMOF);
    printf("I2C used for IMU: I2C_DEV(%i)\n", IMU_I2C_DEV);
}
