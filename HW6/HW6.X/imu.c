#include "imu.h"

void imu_setup(){
    unsigned char who = 0;

    // read from IMU_WHOAMI
    who = GetPin(IMU_ADDR, IMU_WHOAMI);
    
    if (who != 0b01101001){
        while(1){
        _CP0_SET_COUNT(0);
        ssd1306_drawPixel(0, 0, 0);
        ssd1306_update();
        while (_CP0_GET_COUNT() < 24000000) {}  
        }
    }

    // init IMU_CTRL1_XL
    setPin(IMU_ADDR,IMU_CTRL1_XL,0b10000010);
    
    // init IMU_CTRL2_G
    setPin(IMU_ADDR,IMU_CTRL2_G,0b10001000);

    // init IMU_CTRL3_C
    unsigned char ctrl3 = GetPin(IMU_ADDR, IMU_CTRL3_C);
    ctrl3 = ctrl3 | 0b100;
    setPin(IMU_ADDR,IMU_CTRL3_C,ctrl3);

}

void imu_read(unsigned char Register, signed short * data, int len){
    unsigned char array[2*len];
    // read multiple from the imu, each data takes 2 reads so you need len*2 chars
    i2c_master_read_multiple(IMU_ADDR, Register, array, len*2);
    // turn the chars into the shorts
    int i = 0;
    for (i = 0; i<len; i++){
        *(data + i)= array[2*i] | (array[2*i+1] << 8);
    }
}

