/* 
 * File:   main.c
 * Author: maxime
 *
 * Created on 13 mai 2015, 22:09
 * 
 * send string "Hello UART" via UART when pressing P1.3
 * echo back rx buffer
 */

#include <msp430g2553.h>
#include <legacymsp430.h>
 
#define TXLED BIT0
#define RXLED BIT6
#define TXD BIT2
#define RXD BIT1

const char string[] = "Hello UART\r\n";

interrupt(PORT1_VECTOR) port1_isr(void) {
    if(P1IFG & BIT3) {
        char i = 0;
        
        for(i=0; string[i]; i++)
        {
            while  (!(IFG2 & UCA0TXIFG));    //  USCI_A0 TX  buffer  ready?
            UCA0TXBUF = string[i];
        }
        
        do {
            P1IFG &= ~BIT3;
        } while (P1IFG & BIT3);
    }
}

interrupt(USCIAB0RX_VECTOR) uart_rx(void) {
   P1OUT |= RXLED; 
   while  (!(IFG2 & UCA0TXIFG));    //  USCI_A0 TX  buffer  ready?
	UCA0TXBUF  =   UCA0RXBUF;  //  TX  -&amp;gt;   RXed    character
   P1OUT &= ~RXLED; 
}

int main(void)
{
    WDTCTL =   WDTPW   +   WDTHOLD;    //  Stop    WDT

    /* Use Calibration values  for 1MHz    Clock   DCO*/
    DCOCTL =   0;
    BCSCTL1    =   CALBC1_1MHZ;
    DCOCTL =   CALDCO_1MHZ;

    /* Configure   Pin Muxing  P1.1    RXD and P1.2    TXD */
    P1SEL  =   BIT1    |   BIT2    ;
    P1SEL2 =   BIT1    |   BIT2;
    
    P1DIR |= TXLED + RXLED;
    
    /* Configure P1.3 as input w/ pull-up res*/
    P1DIR &= ~BIT3;
    P1REN |= BIT3;
    P1OUT |= BIT3;

    /* Place   UCA0    in  Reset   to  be  configured  */
    UCA0CTL1   =   UCSWRST;

    /* Configure   */
    UCA0CTL1   |=  UCSSEL_2;   //  SMCLK
    UCA0BR0    =   104;    //  1MHz    9600
    UCA0BR1    =   0;  //  1MHz    9600
    UCA0MCTL   =   UCBRS0; //  Modulation  UCBRSx  =   1

    /* Take    UCA0    out of  reset   */
    UCA0CTL1   &=  ~UCSWRST;

    /* Enable  USCI_A0 RX TX interrupt   */
    IE2    |=  UCA0RXIE;
    
    /* Enable interrupt on P1.3*/
    P1IE |= BIT3;
    P1IES &= BIT3;

    __bis_SR_register(LPM0_bits    +   GIE);   //  Enter   LPM0,   interrupts  enabled
}