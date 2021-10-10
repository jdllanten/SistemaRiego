/*
 Controlador para los dispositivos:
 *      Sensor de Temperatura y Humedad
 *      Reloj tiempo real
 *      Leds de Advertencia
 *		Bombillas
 *		LCD 16x2
 *		I2C
 *		Virtual terminal
 Se utiliza el Microcontrolador PIC 18F4550
 */

/*!
\file   Controlador.c
\date   2020-12-14
\author Carlos Hoyos <joiroce@unicauca.edu.co> 104617020768
		Eliana Camayo <eacamayo@unicauca.edu.co> 104616021725
\brief  Example Controler .

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

// This is a guard condition so that contents of this file are not included
// more than once.  
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

