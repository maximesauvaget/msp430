/* 
 * File:   main.c
 * Author: maxime
 *
 * Created on 14 mai 2015, 11:40
 */

#include <msp430g2553.h>
#include <legacymsp430.h>
#include "msplib.h"

#define DATA BIT0 // DS -> 1.0
#define CLOCK BIT4 // SH_CP -> 1.4
#define LATCH BIT5 // ST_CP -> 1.5
#define ENABLE BIT6 // OE -> 1.6
// The OE pin can be tied directly to ground, but controlling
// it from the MCU lets you turn off the entire array without
// zeroing the register

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
void showTemp(char *);

struct CharMap
{
  char c;
  byte v;
};
 
const int cmap_len = 41;
const struct CharMap cmap[] = {
{' ', 0},
{'1', 12},
{'2', 182},
{'3', 158},
{'4', 204},
{'5', 218},
{'6', 250},
{'7', 14},
{'8', 254},
{'9', 222},
{'0', 126},
{'A', 0},
{'b', 0},
{'c', 0},
{'C', 0},
{'d', 0},
{'e', 0},
{'E', 0},
{'F', 0},
{'g', 0},
{'H', 0},
{'h', 0},
{'I', 0},
{'J', 0},
{'L', 0},
{'n', 0},
{'o', 0},
{'P', 0},
{'q', 0},
{'r', 0},
{'S', 0},
{'t', 0},
{'u', 0},
{'U', 0},
{'y', 0},
{'-', 0},
{'~', 0},
{'_', 0},
{'.', 1},
{'|', 0},
{'=', 0}
};
 
byte getCode(char c) 
{
  byte r = 2;
  int i;
  
  for (i = 0 ; i < cmap_len ; i++) {
    if (c == cmap[i].c) {
      r = cmap[i].v;
      break;
    }
  }
   
  return r;
}

int temp;
int tempC;
char sTempC[4];

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;
    
    /* Clock settings */
    DCOCTL = 0;
    BCSCTL1 |= DIVA_3+CALBC1_1MHZ;        
    BCSCTL3 |= XCAP_3;        
    DCOCTL = CALDCO_1MHZ;
    
    P1DIR |= (DATA + CLOCK + LATCH + ENABLE);  // Setup pins as outputs
    
    /* Configure   Pin Muxing  P1.1    RXD and P1.2    TXD */
    P1SEL  =   BIT1    |   BIT2;
    P1SEL2 =   BIT1    |   BIT2;

    /* Place   UCA0    in  Reset   to  be  configured  */
    UCA0CTL1   =   UCSWRST;

    /* Configure   */
    UCA0CTL1   |=  UCSSEL_2;   //  SMCLK
    UCA0BR0    =   104;    //  1MHz    9600
    UCA0BR1    =   0;  //  1MHz    9600
    UCA0MCTL   =   UCBRS0; //  Modulation  UCBRSx  =   1

    /* Take    UCA0    out of  reset   */
    UCA0CTL1   &=  ~UCSWRST;
    
    /* ADC Configuration */
    ADC10CTL0 = SREF_1|ADC10SHT_3|REFON|ADC10ON|ADC10IE;
    ADC10CTL1 = ADC10SSEL_1|INCH_10|ADC10DIV_1;
    
    enable(); // Enable output (pull OE low)
          
    while(1) {
        ADC10CTL0 |= ENC + ADC10SC;
        
        __bis_status_register(LPM0_bits + GIE);
        
        //ADC reading
        temp = ADC10MEM;
        //Convert reading to Celsius
        tempC = ((temp - 673) * 423) / 1024;
        itoa(tempC, sTempC, 10, 1);

           while  (!(IFG2 & UCA0TXIFG));    //  USCI_A0 TX  buffer  ready?
            UCA0TXBUF = 'a';
        
        showTemp(sTempC);

        __delay_cycles(500000);
    }
}

interrupt(ADC10_VECTOR) ADC_isr(void) {
    LPM0_EXIT;
}

void showTemp(char * data) {
    //Set latch to low (should be already)
    P1OUT &= ~LATCH;
    while(*data) {
        shiftOut(getCode(*data));
        *data++;
    }
    // Pulse the latch pin to write the values into the storage register
    P1OUT |= LATCH;
    P1OUT &= ~LATCH;
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