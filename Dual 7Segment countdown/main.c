/* 
 * File:   main.c
 * Author: maxime
 *
 * Created on 25 avril 2015, 21:48
 */
/*
Dual 7 Segment (Common Cathode) FJ5101AH Display Map:

    G   F  GND  A   B
 ___|___|___|___|___|____
|                        |
|        A               |
|    F       B           |
|        G               |
|    E       C           |
|        D       H(Dot)  |
|________________________|
    |   |   |   |   |
    E   D  GND  C   H

74HC595 Map:
     _______
Q1  |1 *  16|  Vcc                  PINS 1-7, 15   Q0 - Q7   Output Pins
Q2  |2    15|  Q0                   PIN 8	   GND	     Ground, Vss
Q3  |3    14|  DS                   PIN 9	   Q7"	     Serial Out
Q4  |4    13|  OE                   PIN 10	   MR	     Master Reclear, active low
Q5  |5    12|  ST_CP                PIN 11	   SH_CP     Shift register clock pin
Q6  |6    11|  SH_CP                PIN 12	   ST_CP     Storage register clock pin (latch pin)
Q7  |7    10|  MR                   PIN 13	   OE	     Output enable, active low
GND |8_____9|  Q7"                  PIN 14	   DS	     Serial data input
                                    PIN 16	   Vcc	     Positive supply voltage
           _______
   F   Q1-|1 *  16|-5V
   E   Q2-|2    15|-Q0   G
   D   Q3-|3    14|-P1.0
   C   Q4-|4    13|-GND
   B   Q5-|5    12|-P1.5
   A   Q6-|6    11|-P1.4
   DOT Q7-|7    10|-5V
      GND-|8_____9|_____________________
                                        |
           _______                      |
   F   Q1-|1 *  16|-5V                  |
   E   Q2-|2    15|-Q0   G              |
   D   Q3-|3    14|-____________________|
   C   Q4-|4    13|-GND
   B   Q5-|5    12|-P1.5
   A   Q6-|6    11|-P1.4
   DOT Q7-|7    10|-5V
      GND-|8_____9|-NILL 
 
 
 */

#include <msp430g2553.h>
#include <legacymsp430.h>

//Define our pins
#define TXD BIT2
#define RXD BIT1
#define DATA BIT0 // DS -> 1.0
#define CLOCK BIT4 // SH_CP -> 1.4
#define LATCH BIT5 // ST_CP -> 1.5
#define ENABLE BIT6 // OE -> 1.6
// The OE pin can be tied directly to ground, but controlling
// it from the MCU lets you turn off the entire array without
// zeroing the register

// Setup the combination to display each number on the display
#define ZERO 126
#define ONE 12
#define TWO 182
#define THREE 158
#define FOUR 204
#define FIVE 218
#define SIX 250
#define SEVEN 14
#define EIGHT 254
#define NINE 222
#define MAX 223

#define COUNTER_MAX 99

//byte type
typedef unsigned char byte;

// Declare functions
void delay ( unsigned int );
void pulseClock ( void );
void shiftOut ( byte );
void enable ( void );
void disable ( void );
void init ( void );
void pinWrite ( unsigned int, unsigned char );

byte NUMBERS[10] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE};

volatile unsigned int counter = 0;

// Interruption du TimerA
interrupt(TIMER0_A0_VECTOR) tick(void) {   
    UC0IE |= UCA0TXIE; // Enable USCI_A0 TX interrupt
    UCA0TXBUF = 't';
    
    //Set latch to low (should be already)
    P1OUT &= ~LATCH;
 
    int c = counter;
    do
    {
        int digit = c%10;
        c /= 10;
        
        shiftOut(NUMBERS[digit]);
    }while (c > 0);
    
    if(counter < 10) {
        shiftOut(ZERO);
    }
    
    counter--;    
    if(counter == 0) counter = COUNTER_MAX;
    

    // Pulse the latch pin to write the values into the storage register
    P1OUT |= LATCH;
    P1OUT &= ~LATCH;
} 

interrupt(USCIAB0RX_VECTOR) uart_rx(void) {
    counter = COUNTER_MAX;
   if (UCA0RXBUF == 's') // 'u' received?
   {
      UC0IE |= UCA0TXIE; // Enable USCI_A0 TX interrupt
   }
}

interrupt(USCIAB0TX_VECTOR) uart_tx(void){
   UCA0TXBUF = 'z'; // TX next character
   UC0IE &= ~UCA0TXIE; // Disable USCI_A0 TX interrupt
}

void main( void )
{
    // Stop watchdog timer to prevent time out reset
    WDTCTL = WDTPW + WDTHOLD;
    P1SEL |= RXD + TXD ; // P1.1 = RXD, P1.2=TXD
    P1SEL2 |= RXD + TXD ; // P1.1 = RXD, P1.2=TXD
    P1DIR |= (DATA + CLOCK + LATCH + ENABLE);  // Setup pins as outputs
    
    BCSCTL1 |= DIVA_3;        // Horloge = ACLK / 8
    BCSCTL3 |= XCAP_3;        // Condensateur interne de 12.5pF pour le quartz 32KHz
    DCOCTL = CALDCO_1MHZ;
    
    CCTL0 = CCIE;             // Interruption CCR0 activé
    CCR0 = 512;               // 512 tick du quartz 32KHz = 1 seconde
    TACTL = TASSEL_1 | ID_3 | MC_1;  // Horloge du TIMER 1 = ACLK, division par 8, front montant

    UCA0CTL1 |= UCSSEL_3; // SMCLK
    UCA0BR0 = 0x08; // 1MHz 115200
    UCA0BR1 = 0x00; // 1MHz 115200
    UCA0MCTL = UCBRS2 + UCBRS0; // Modulation UCBRSx = 5
    UCA0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**
    UC0IE |= UCA0RXIE; // Enable USCI_A0 RX interrupt
    
    enable(); // Enable output (pull OE low)

    counter = COUNTER_MAX;
    _BIS_SR(LPM3_bits | GIE); // Mise en sommeil avec interruptions
}
 
// Writes a value to the specified bitmask/pin. Use built in defines
// when calling this, as the shiftOut() function does.
// All nonzero values are treated as "high" and zero is "low"
void pinWrite( unsigned int bit, unsigned char val )
{
  if (val){
    P1OUT |= bit;
  } else {
    P1OUT &= ~bit;
  }
}
 
// Pulse the clock pin
void pulseClock( void )
{
  P1OUT |= CLOCK;
  P1OUT ^= CLOCK;
 
}
 
// Take the given 8-bit value and shift it out, LSB to MSB
void shiftOut(byte val)
{
    char i;

    // Iterate over each bit, set data pin, and pulse the clock to send it
    // to the shift register

    for (i = 0; i < 8; i++)  {
        pinWrite(DATA, (val & (1 << i)));
        pulseClock();
    }
}
 
// These functions are just a shortcut to turn on and off the array of
// LED's when you have the enable pin tied to the MCU. Entirely optional.
void enable( void )
{
  P1OUT &= ~ENABLE;
}
 
void disable( void )
{
  P1OUT |= ENABLE;
}