#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include "adc.h"
#include "ssd1306.h"
#include <stdio.h>
#include "ws2812b.h"


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
void drawletter(int x, int y, char letter);
void drawMessage(int x, int y, char* m);

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
    adc_setup();
    ctmu_setup();
    i2c_master_setup();
    ssd1306_setup();
    ws2812b_setup();
    
    
    __builtin_enable_interrupts();
   

    wsColor color1[5];
    color1[0] = HSBtoRGB(60, 1, 1);
    color1[1] = HSBtoRGB(120, 1, 1);
    color1[2] = HSBtoRGB(180, 1, 1);
    color1[3] = HSBtoRGB(240, 1, 1);
    color1[4] = HSBtoRGB(300, 1, 1);
    
    wsColor color2[5];
    color2[0] = HSBtoRGB(120, 1, 1);
    color2[1] = HSBtoRGB(180, 1, 1);
    color2[2] = HSBtoRGB(240, 1, 1);
    color2[3] = HSBtoRGB(300, 1, 1);
    color2[4] = HSBtoRGB(360, 1, 1);
    
    wsColor color3[5];
    color3[0] = HSBtoRGB(180, 1, 1);
    color3[1] = HSBtoRGB(240, 1, 1);
    color3[2] = HSBtoRGB(300, 1, 1);
    color3[3] = HSBtoRGB(360, 1, 1);
    color3[4] = HSBtoRGB(60, 1, 1);
    
    
    int ctmuread1[10];
    int read1=0;
    int ctmuread2[10];
    int read2=0;
    char message[50];
    int i=0;
    
    while (1) {
        // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
        // remember the core timer runs at half the sysclk   24 megahertz
        
        _CP0_SET_COUNT(0);
        for(i=0;i<10;i++){ctmuread1[i]=ctmu_read(0,24);}
        for(i=0;i<10;i++){ctmuread2[i]=ctmu_read(1,40);}
        read1=ctmuread1[0]+ctmuread1[1]+ctmuread1[2]+ctmuread1[3]+ctmuread1[4]+ctmuread1[5]+ctmuread1[6]+ctmuread1[7]+ctmuread1[8]+ctmuread1[9];
        read2=ctmuread2[0]+ctmuread2[1]+ctmuread2[2]+ctmuread2[3]+ctmuread2[4]+ctmuread2[5]+ctmuread2[6]+ctmuread2[7]+ctmuread2[8]+ctmuread2[9];
        
        
        sprintf(message, "%d", read1);
        drawMessage(0, 8, message);
        sprintf(message, "%d", read2);
        drawMessage(0, 16 , message);
        ssd1306_update();
        
        if (read1<5000){
            ws2812b_setColor(color1,5);
        } else if (read2<5000){
            ws2812b_setColor(color2,5);
        } else {
            ws2812b_setColor(color3,5);
        }

        while (_CP0_GET_COUNT() < 24000000/10) {}
    }
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