/* 
 * File:   msplib.c
 * Author: maxime
 *
 * Created on 14 mai 2015, 19:42
 */

#include "msplib.h"
#include <math.h>

/*
 http://www.strudel.org.uk/itoa/
 */
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

int atoi(char * value, int base) {
    // check that the base if valid
    if (base < 2 || base > 36) { return -1; }

    char* ptr = value;
    char* ref = "0123456789abcdefghijklmnopqrstuvwxyz";
    int c;
    int power = 0;
    double resultat;
    
    while(*ptr) {
        for(c=0;ref[c]!=*ptr;c++);
        
        resultat += c * powf(base, power);
        
        power++;
        *ptr++;
    }
    
    return (int)resultat;
}