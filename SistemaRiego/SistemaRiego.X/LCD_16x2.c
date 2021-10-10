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

#include "LCD_16x2_8-bit_Header_File.h"

/****************************Functions********************************/
void LCD_Init()
{
   	LCD_Dir1 = 0x00;          /*Set PORTB as output PORT for LCD data(D0-D7) pins*/
    LCD_Dir2 = 0x00;          /*Set PORTD as output PORT LCD Control(RS,EN) Pins*/
    MSdelay(20);
	LCD_Command(0x38);     /*uses 2 line and initialize 5*7 matrix of LCD*/
    LCD_Command(0x0c);     /*display on cursor off*/
    LCD_Command(0x06);     /*increment cursor (shift cursor to right)*/
	LCD_Command(0x01);     /*clear display screen*/
	
}

void LCD_Custom_Char(unsigned char loc,unsigned char *msg)
{
    unsigned char i;
    if(loc<8)
    {
     LCD_Command(0x40+(loc*8));     /* Command 0x40 and onwards forces the device to point CGRAM address */
       for(i=0;i<8;i++)             /* Write 8 byte for generation of 1 character */
           LCD_Char(msg[i]);
        
    }   
}

void LCD_Clear()
{
    	LCD_Command(0x01);     /*clear display screen*/
}

void LCD_Command(char cmd )
{
	ldata= cmd;            /*Send data to PORT as a command for LCD*/   
	RS = 0;                /*Command Register is selected*/
	EN = 1;                /*High-to-Low pulse on Enable pin to latch data*/ 
	NOP();
	EN = 0;
    MSdelay(3);
    
}


void LCD_Char(char dat)
{
	ldata= dat;            /*Send data to LCD*/  
	RS = 1;                /*Data Register is selected*/
	EN=1;                  /*High-to-Low pulse on Enable pin to latch data*/   
	NOP();
	EN=0;
    MSdelay(1);
}


void LCD_String(const char *msg)
{
	while((*msg)!=0)
	{		
	  LCD_Char(*msg);
	  msg++;
    }
		
}

void LCD_String_xy(char row,char pos,const char *msg)
{
    char location=0;
    if(row<1)
    {
        location=(0x80) | ((pos) & 0x0f);      /*Print message on 1st row and desired location*/
        LCD_Command(location);
    }
    else
    {
        location=(0xC0) | ((pos) & 0x0f);      /*Print message on 2nd row and desired location*/
        LCD_Command(location);    
    }  
    

    LCD_String(msg);

}
/*********************************Delay Function********************************/
void MSdelay(unsigned int val)
{
     unsigned int i,j;
        for(i=0;i<=val;i++)
            for(j=0;j<81;j++);      /*This count Provide delay of 1 ms for 8MHz Frequency */
 }


