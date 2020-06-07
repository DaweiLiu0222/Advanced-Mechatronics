#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include <stdio.h>
#include "ssd1306.h"
#include "i2c_master_noint.h"
#include "imu.h"


// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
#pragma config WDTPS = PS1048576 // use largest wdt
#pragma config WINDIS = OFF // use non-window mode wdt
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations

void drawMessage(int x, int y, char* m);
void bar_x(signed short x, int z);
void bar_y(signed short y, int z);

int main() {

    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 0;
    
    i2c_master_setup();
    ssd1306_setup();
    imu_setup();

    __builtin_enable_interrupts();
    
    signed short imu[7];
    char message[50];

    while (1) {
        // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
        // remember the core timer runs at half the sysclk   24 megahertz
        
        _CP0_SET_COUNT(0);
        LATAbits.LATA4 = !LATAbits.LATA4;
        
        imu_read(IMU_OUT_TEMP_L, imu, 7);
        
        if (0) {
            sprintf(message, "g: %d %d %d  ", imu[1], imu[2], imu[3]);
            drawMessage(0, 0, message);
            sprintf(message, "a: %d %d %d  ", imu[4], imu[5], imu[6]);
            drawMessage(0, 8, message);
            sprintf(message, "t: %d  ", imu[0]);
            drawMessage(0, 16, message);
        } 
        else {
            bar_x(-imu[5], 1);
            bar_y(imu[4], 1);
        }
        
        ssd1306_update();
            
            
            
        while (_CP0_GET_COUNT() < 24000000/20) {
        }
        ssd1306_clear();
    }
    
}  

void setPin(unsigned char address, unsigned char Register, unsigned char value){
    i2c_master_start();
    i2c_master_send(address);
    i2c_master_send(Register);
    i2c_master_send(value);
    i2c_master_stop();
}

unsigned char GetPin(unsigned char address, unsigned char Register){
    unsigned char data;
    i2c_master_start();
    i2c_master_send(address);//write address
    i2c_master_send(Register);
    i2c_master_restart();
    i2c_master_send(address | 0b1);//read address
    data = i2c_master_recv();
    i2c_master_ack(1);
    i2c_master_stop();
    return data;
    
}

void i2c_master_read_multiple(unsigned char address, unsigned char Register, unsigned char *data, int length){
    
    i2c_master_start();
    i2c_master_send(address);//write address
    i2c_master_send(Register);
    i2c_master_restart();
    i2c_master_send(address | 0b1);//read address
    int i = 0;
    for (i=0; i<length; i++){
        *(data+i) = i2c_master_recv();
        if (i==length-1){
            i2c_master_ack(1); //end 
        } else {
            i2c_master_ack(0);//next one
        }
    }
    i2c_master_stop();
    
}

void drawletter(int x, int y, char letter){
    int i,j;
    for(i=0; i < 5; i++){
        for(j=0; j < 8; j++){
           if (((ASCII[letter-0x20][i] >> j) & 1 )== 1){
                ssd1306_drawPixel(i+x, j+y, 1);
           } else {
                ssd1306_drawPixel(i+x, j+y, 0); 
           }
        }
    }
}

void drawMessage(int x, int y, char* m){
    int s=0;
    while (m[s]!=0) {
        drawletter(x+5*s,y,m[s]);
        s++;
    }
}

void bar_x(signed short x, int z){
    int dir = 0.01*x;
    int i = 0;
    if (dir>=0){
        for(i=0; i<dir;i++){
            ssd1306_drawPixel(64 + i, 16, z);
        }
    } else {
        for(i=0; i>dir;i--){
            ssd1306_drawPixel(64 + i, 16, z);
        }
    }
}
void bar_y(signed short y, int z){
    int dir = 0.01*y;
    int i = 0;
    if (dir>=0){
        for(i=0; i<dir;i++){
            ssd1306_drawPixel(64, 16 + i, z);
        }
    } else {
        for(i=0; i>dir;i--){
            ssd1306_drawPixel(64, 16 + i, z);
        }
    }
}