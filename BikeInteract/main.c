/* 
 * File:   main.c
 * Author: maxime
 *
 * Created on 13 juin 2015, 14:06
 */

#include <msp430g2553.h>
#include <legacymsp430.h>
#include <string.h>

const char NEWLINE[] = "\r\n";
unsigned int buffer[5];
unsigned int i = 0; 
/*
 * 
 */
void main(void) {
    /*** Watchdog timer and clock Set-Up ***/
    WDTCTL = WDTPW + WDTHOLD;		// Stop watchdog timer
    DCOCTL = 0;             		// Select lowest DCOx and MODx
    BCSCTL1 = CALBC1_1MHZ;  		// Set range
    DCOCTL = CALDCO_1MHZ;   		// Set DCO step + modulation
    
    //P1.5 TA0.0
    P1DIR |= BIT5 + BIT6;
    P1SEL   |=   BIT1 + BIT2 + BIT5 + BIT6;
    P1SEL2  |=   BIT1 + BIT2; 
     
    TA0CCTL1= CM_1 + CCIS_1 + CAP + SCS + CCIE;
    TA0CCTL1 &=~COV;
    TA0CTL = TASSEL_1 + MC_2 + TACLR;
    
    /* Place   UCA0    in  Reset   to  be  configured  */
    UCA0CTL1   =   UCSWRST;

    /* Configure   */
    UCA0CTL1   |=  UCSSEL_2;   //  SMCLK
    UCA0BR0    =   104;    //  1MHz    9600
    UCA0BR1    =   0;  //  1MHz    9600
    UCA0MCTL   =   UCBRS0; //  Modulation  UCBRSx  =   1

    /* Take    UCA0    out of  reset   */
    UCA0CTL1   &=  ~UCSWRST;

    /* Enable  USCI_A0 RX interrupt   */
    IE2    |=  UCA0RXIE;
    
    __bis_SR_register(LPM0_bits | GIE);
    while(1);
}

void write_uart(char * data)
{
    int i = 0;
    for(i=0;i < 6;i++)
    {
        while  (!(IFG2 & UCA0TXIFG));    //  USCI_A0 TX  buffer  ready?
            UCA0TXBUF  =   data[i];
    }
    for(i=0;NEWLINE[i];i++)
    {
        while  (!(IFG2 & UCA0TXIFG));    //  USCI_A0 TX  buffer  ready?
            UCA0TXBUF  =   NEWLINE[i];
    }
}

char * itoa (int value, char *result, int base, char reverse)
{
    // check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;
    
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    
    if (!reverse) {
        while (ptr1 < ptr) {
            tmp_char = *ptr;
            *ptr--= *ptr1;
            *ptr1++ = tmp_char;
        }
    }
    return result;
}

// Interruption du TimerA
interrupt(TIMER0_A0_VECTOR) TIMER0_A1_ISR(void) {  
    if(TA0IV == 2) {
        if(i==5) {
            i = 0;
        }
        else {
            buffer[i] = TA0CCR1;
            i++;
        }
    }
    else {
        i = 0;
        buffer[0] = 0;
    }
/*
 switch(TA0IV)
  {
    case 0: break;                          // No interrupts
    case 4: break;                          // TA0CCR2
    case 2:                                 // TA0CCR1
    {
        if(i==5)
        {
            i = 0;
        }
        else
        {
            buffer[i] = TA0CCR1;
            i++;
        }
        break;
    }
    case 6: break;                          // TA0CCR3
    default: break;
  }
*/
}

interrupt(USCIAB0RX_VECTOR) uart_rx(void) {
    buffer[0] = 17456;
    buffer[1] = 15464;
    buffer[2] = 13574;
    buffer[3] = 6549;
    buffer[4] = 46544;

    unsigned int bBuffer[5];
    memcpy(bBuffer, buffer, 5 * sizeof(int));

    int c = 0;
    int total;
    int mean;
    int frequency = 0;
    char freq[5];
    freq[0] = '\0';
    for(c = 0; c < 5; c++) {
        if(buffer[c] != 0) {
            total += buffer[c];
        }
        else
        {
            break;
        }
    }

    mean = total / (c + 1);

    long period = mean / 32768;
    if(period > 0) {
        frequency = 1 / period;
    }
    itoa(frequency, freq, 10, 0);
    write_uart(freq);
    while  (!(IFG2 & UCA0TXIFG));    //  USCI_A0 TX  buffer  ready?
            UCA0TXBUF  =  UCA0RXBUF;
}