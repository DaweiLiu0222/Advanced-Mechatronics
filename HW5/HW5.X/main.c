#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include "ws2812b.h"
#include "ssd1306.h"
#include <stdio.h>

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
    LATAbits.LATA4 = 1;
    i2c_master_setup();
    ssd1306_setup();
    
    ws2812b_setup();

    __builtin_enable_interrupts();
    
    wsColor color1[4];
    color1[0] = HSBtoRGB(72, 1, 1);
    color1[1] = HSBtoRGB(144, 1, 1);
    color1[2] = HSBtoRGB(216, 1, 1);
    color1[3] = HSBtoRGB(288, 1, 1);

    
    wsColor color2[4];
    color2[0] = HSBtoRGB(144, 1, 1);
    color2[1] = HSBtoRGB(216, 1, 1);
    color2[2] = HSBtoRGB(288, 1, 1);
    color2[3] = HSBtoRGB(360, 1, 1);
    
    wsColor color3[4];
    color3[0] = HSBtoRGB(216, 1, 1);
    color3[1] = HSBtoRGB(288, 1, 1);
    color3[2] = HSBtoRGB(360, 1, 1);
    color3[3] = HSBtoRGB(72, 1, 1);
    
    wsColor color4[4];
    color4[0] = HSBtoRGB(288, 1, 1);
    color4[1] = HSBtoRGB(360, 1, 1);
    color4[2] = HSBtoRGB(72, 1, 1);
    color4[3] = HSBtoRGB(144, 1, 1);
    
    wsColor color5[4];
    color5[0] = HSBtoRGB(360, 1, 1);
    color5[1] = HSBtoRGB(72, 1, 1);
    color5[2] = HSBtoRGB(144, 1, 1);
    color5[3] = HSBtoRGB(216, 1, 1);
    
    wsColor color6[4];
    color1[0] = HSBtoRGB(72, 1, 1);
    color1[1] = HSBtoRGB(144, 1, 1);
    color1[2] = HSBtoRGB(216, 1, 1);
    color1[3] = HSBtoRGB(288, 1, 1);
    
    char m[50];
    char n[50];
    char o[50];
    
    int a = 0; int b = 0; int c = 0;
    a = color1[0].r;
    b = color1[0].g;
    c = color1[0].b;
    sprintf(m,"R = %d",a);
    sprintf(n,"G = %d",b);
    sprintf(o,"B = %d",c);
    drawMessage(15,8,m);
    drawMessage(15,16,n);
    drawMessage(15,24,o);
    ssd1306_update();

    while (1) {

        ws2812b_setColor(color1,4);
        
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT() < 24000000/10) {}
        
        ws2812b_setColor(color2,4);
        
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT() < 24000000/10) {}
        
        ws2812b_setColor(color3,4);
        
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT() < 24000000/10) {}
        
        ws2812b_setColor(color4,4);
        
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT() < 24000000/10) {}
        
        ws2812b_setColor(color5,4);
        
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT() < 24000000/10) {}
        
        ws2812b_setColor(color6,4);
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
