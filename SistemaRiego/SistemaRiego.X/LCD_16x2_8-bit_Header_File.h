/*!
\file   Sistema.c
\date   2021-10-07
\author Fabian Marin- Fernando Dorado - Jeferson LLantén < fabianxd0496@unicauca.edu.co hfdorado@unicauca.edu.co jdllanten@unicauca.edu.co>
\brief  configuration bits pic18f4550.
\par Copyright
Information contained herein is proprietary to and constitutes valuable
confidential trade secrets of unicauca, and
is subject to restrictions on use and disclosure.
\par
Copyright (c) unicauca 2020. All rights reserved.
\par
The copyright notices above do not evidence any actual or
intended publication of this material.
******************************************************************************
*/

#ifndef XC_HEADER_TEMPLATE_H
#define	XC_HEADER_TEMPLATE_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <pic18f4550.h>

#define RS LATC0                    /*PORT 0 pin is used for Register Select*/
#define EN LATC1                    /*PORT 1 pin is used for Enable*/
#define ldata LATD                  /*PORT is used for transmitting data to LCD*/
#define LCD_Dir1 TRISD
#define LCD_Dir2 TRISC

void LCD_Init();
void LCD_Command(char );
void LCD_Char(char x);
void LCD_String(const char *);
void MSdelay(unsigned int );
void LCD_String_xy(char ,char ,const char*);
void LCD_Custom_Char(unsigned char loc,unsigned char *msg);

#endif	/* XC_HEADER_TEMPLATE_H */

