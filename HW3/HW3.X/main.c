#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include"i2c_master_noint.h"

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

void initExpander();
void setExpander(unsigned char address, unsigned char Register, unsigned char value);
unsigned char readExpander(unsigned char address, unsigned char Register);

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
    
    // Initializing I2C2 with initExpander function created
    initExpander();
    
    __builtin_enable_interrupts();
    
    
    
    while(1) { 
        
        _CP0_SET_COUNT(0);      // Setting Core Timer count to 0
        while(_CP0_GET_COUNT() < 24000000/10) {;}
        LATAbits.LATA4 = !LATAbits.LATA4;

        unsigned char e = readExpander(0b01000000,0b10011)%2;
        
        if (e){
            setExpander(0b01000000,0b10100,0b00000000);
        }
        else{
            setExpander(0b01000000,0b10100,0b10000000);
        }
    }
}

void initExpander() {
    i2c_master_setup();     
    
    // A pins as output
    i2c_master_start();     
    i2c_master_send(0b01000000);       
    i2c_master_send(0x00);      
    i2c_master_send(0x00);     
    i2c_master_stop();   
    
    // B pins as input
    i2c_master_start();     
    i2c_master_send(0b01000000);       
    i2c_master_send(0x01);      
    i2c_master_send(0xFF);     
    i2c_master_stop(); 
}

void setExpander(unsigned char address, unsigned char Register, unsigned char value) {
    i2c_master_start();     
    i2c_master_send(address);    
    i2c_master_send(Register);     
    i2c_master_send(value);     
    i2c_master_stop();     
}

unsigned char readExpander(unsigned char address, unsigned char Register) {
    i2c_master_start();     // make the start bit
    i2c_master_send(address);      
    i2c_master_send(Register);     
    i2c_master_restart();       
    i2c_master_send(address|0x41);    
    unsigned char r = i2c_master_recv();    
    i2c_master_ack(1);      // make the ack bit
    i2c_master_stop();      // make the stop bit
    return r;
}