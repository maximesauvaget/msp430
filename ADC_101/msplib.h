/* 
 * File:   msplib.h
 * Author: maxime
 *
 * Created on 14 mai 2015, 19:42
 */

#ifndef MSPLIB_H
#define	MSPLIB_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <math.h>

char * itoa (int value, char *result, int base, char reverse);
int atoi(char * value, int base);

#ifdef	__cplusplus
}
#endif

#endif	/* MSPLIB_H */

