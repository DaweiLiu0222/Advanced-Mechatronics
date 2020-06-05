#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include"SSD1306.h"
#include"font.h"

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
void drawMessage(int x, int y, char* message);

int main() {

    __builtin_disable_interrupts();

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
    TRISBbits.TRISB4 = 1;
    LATAbits.LATA4 = 0;
      
    i2c_master_setup();
    ssd1306_setup();
    
    __builtin_enable_interrupts();
    
    char message[50];
    char number[50];
    sprintf(message,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    sprintf(number,"01234567890123456789012345");
    
    drawMessage(0,0,message);
    ssd1306_update();
    drawMessage(0,8,number);
    ssd1306_update();
    
    char Hello[50];
    char FPS[50];
    int i = 0;
    float f;
    
    while (1) {
        // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
        // remember the core timer runs at half the sysclk   24 megahertz
        _CP0_SET_COUNT(0);      // Setting Core Timer count to 0
        while(_CP0_GET_COUNT() < 24000000/10) {;}
        LATAbits.LATA4 = !LATAbits.LATA4;
        
        sprintf(Hello,"Hello World! i = %d", i);
        drawMessage(5,16,Hello);
        ssd1306_update();
        i++;
        
        _CP0_SET_COUNT(0);
        ssd1306_update();
        f = _CP0_GET_COUNT();
        f = 24000000/f;
        sprintf(FPS,"FPS = %.2f", f);
        drawMessage(70,24,FPS);
        ssd1306_update();
        
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

void drawMessage(int x, int y, char* message){
    int s=0;
    while (message[s]!=0) {
        drawletter(x+5*s,y,message[s]);
        s++;
    }
}
