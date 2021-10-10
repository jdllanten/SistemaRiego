
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


#include <pic18f4550.h>
#include <xc.h>
#include <stdio.h>
#include "Configuration_Header_File.h"
#include "LCD_16x2_8-bit_Header_File.h"
#include "I2C_Master_File.h"



#define _XTAL_FREQ  8000000L
#define DATA_OUTPUT LATC2              /* assign Port pin for data*/
#define DATA_INPUT PORTCbits.RC2       /* read data from Port pin*/
#define DATA_DIR TRISCbits.RC2      /* Port direction */

#define LED_GREEN PORTAbits.RA3         // Definimos el puerto del led verde
#define LED_YELLOW PORTAbits.RA4         // Definimos el puerto del led Amarillo
#define LED_RED PORTAbits.RA5         // Definimos el puerto del led Rojo

#define MOTOR PORTBbits.RB6

#define DEVICE_ID_WRITE 0xD0
#define DEVICE_ID_READ 0xD1

//Procedimietos del sensor de temperatura y humedad
void DHT11_Start();
void DHT11_CheckResponse();
char DHT11_ReadData();
void printTempHumedad();


//Procedimietos Utilizados por el PIC o el Main
void ON_LEDS(char);
void OFF_LEDS();
void OFF_LAMPARAS();
void ON_LAMPARAS(); 


// Procedimientos del RTC
void Mostrar_Fecha();
void Verificar_Hora(); 

//LED (Bombilla)
unsigned char bombillaCar0[8] = {0x0E, 0x11, 0x11, 0x11, 0x0E, 0x0E, 0x00, 0x00};
unsigned char good[8] = {0x00, 0x1B, 0x1B, 0x00, 0x11, 0x11, 0x0E, 0x00};
unsigned char alert[8] = {0x00, 0x1B, 0x1B, 0x00, 0x0E, 0x11, 0x11, 0x00};

//Variables utilizadas por el sistema.
char var_RH_Decimal, var_RH_Integral, var_T_Decimal, var_T_Integral;
char vec_value[10];
int banAgregar = 0; //Bandera
int acumBom = 0; //Acumulador para comparar bombillas

//Variables del RTC
int sec, min, hour;
int Day, Date, Month, Year;
char secs[10], mins[10], hours[10];
char date[10], month[10], year[10];
char Clock_type = 0x06;
char AM_PM = 0x05;
char days[7] = {'S', 'M', 'T', 'W', 't', 'F', 's'};
//Fin variables RTC


/*
 * El proyecto contiene:
 *      Sensor de Temperatura y Humedad
 *      Reloj tiempo real
 *      Leds de Advertencia
 *		Bombillas
 *		LCD 16x2
 *		I2C
 *		Virtual terminal
 */
//main
void main() {
    TRISA = 0;

    //Inicializar las bombillas como apagadas
    PORTAbits.RA0 = 0;
    PORTAbits.RA1 = 0;
    PORTAbits.RA2 = 0;
    
    PORTBbits.RB6 = 0;

    //Configura UART a 9600 baudios
    TRISCbits.RC6 = 0; //  Pin RC6 como salida digital para TX.
    TXSTAbits.TX9 = 0; //  Modo-8bits.
    TXSTAbits.TXEN = 1; //  Habilita Transmisión.
    TXSTAbits.SYNC = 0; //  Modo-Asíncrono.
    TXSTAbits.BRGH = 0; //  Modo-Baja Velocidad de Baudios.
    BAUDCONbits.BRG16 = 0; //  Baudios modo-8bits.
    RCSTAbits.SPEN = 1; //  Hbilita el Módulo SSP como UART.
    SPBRG = (unsigned char) (((_XTAL_FREQ / 9600) / 64) - 1); //baudios  = 9600

    OSCCON = 0x72; /* set internal oscillator with frequency 8 MHz*/
    //3TRISAbits.TRI3SA2 = 0;

    ADCON1 = 0x0F; /* this makes all pins as a digital I/O pins */

    while (1) {
        LCD_Init(); /* initialize LCD16x2 */
        I2C_Init(); /*initialize I2C protocol*/
        MSdelay(10);
        
            DHT11_Start(); /* send start pulse to DHT11 module */
            DHT11_CheckResponse(); /* wait for response from DHT11 module */

            /* read 40-bit data from DHT11 module */
            var_RH_Integral = DHT11_ReadData(); /* read Relative Humidity's integral value */
            var_RH_Decimal = DHT11_ReadData(); /* read Relative Humidity's decimal value */
            var_T_Integral = DHT11_ReadData(); /* read Temperature's integral value */
            var_T_Decimal = DHT11_ReadData(); /* read Relative Temperature's decimal value */

            printTempHumedad(); // Imprime la temperatura leida en el LCD

            char buffer_TX[] = "No se ha prendido ninguna bombilla\r";
            for (int i = 0; i < 35; i++) {
                //  espera a que el registro de transmisión este disponible o vacio.
                while (!TXSTAbits.TRMT) {
                }
                // escribe el dato que se enviará a través de TX.
                TXREG = buffer_TX[i];
            }
            MSdelay(1000); // 1 seg de espera para aparecer la tem leida

            while (banAgregar == 0) {

                ON_LAMPARAS();
                printTempHumedad();

                MSdelay(1000);
            }
            Mostrar_Fecha();
            MSdelay(1000);
            banAgregar = 0;
            acumBom = 0;
            OFF_LAMPARAS();
            OFF_LEDS();
            MSdelay(2000);

    }
}

// funciones del sensor de temperatura y humedad

char DHT11_ReadData() {
    char i, data = 0;
    for (i = 0; i < 8; i++) {
        while (!(DATA_INPUT & 1)); /* wait till 0 pulse, this is start of data pulse */
        __delay_us(30);
        if (DATA_INPUT & 1) /* check whether data is 1 or 0 */
            data = ((data << 1) | 1);
        else
            data = (data << 1);
        while (DATA_INPUT & 1);

    }
    return data;
}

void DHT11_Start() {
    DATA_DIR = 0; /* set as output port */
    DATA_OUTPUT = 0; /* send low pulse of min. 18 ms width */
    __delay_ms(18);
    DATA_OUTPUT = 1; /* pull data bus high */
    __delay_us(20);
    DATA_DIR = 1; /* set as input port */

}

void DHT11_CheckResponse() {
    while (DATA_INPUT & 1); /* wait till bus is High */
    while (!(DATA_INPUT & 1)); /* wait till bus is Low */
    while (DATA_INPUT & 1); /* wait till bus is High */
}
//fin funciones de sensor de temperatura


// Método para apagar LEDS
void OFF_LEDS() {
    LED_YELLOW = 0;
    LED_GREEN = 0;
    LED_RED = 0;

    char buffer_TX[] = "Apagando LEDS\r";
    for (int i = 0; i < 15; i++) {
        //  espera a que el registro de transmisión este disponible o vacio.
        while (!TXSTAbits.TRMT) {
        }
        //  escribe el dato que se enviará a través de TX.
        TXREG = buffer_TX[i];
    }
}
// Método para encender LEDS
void ON_LEDS(char T_Integral) {
    LCD_Custom_Char(5, good);

    int valor = T_Integral;
    if (valor < 30) {
        LED_YELLOW = 1;
        LED_GREEN = 0;
        LED_RED = 0;
    } else if (valor >= 30 && valor <= 35) { //Temperatura ideal. Entre los 30 y 35 grados
        LED_YELLOW = 0;
        LED_GREEN = 1;
        LED_RED = 0;

        LCD_Command(0xc0 | (15)); /*Display characters from c0(2nd row) location */
        LCD_Char(5); /*To display custom character send address as data to point stored character */

        char buffer_TX[] = "Temperatura Ideal\r";
        for (int i = 0; i < 19; i++) {
            //  espera a que el registro de transmisión este disponible o vacio.
            while (!TXSTAbits.TRMT) {
            }
            //  escribe el dato que se enviará a través de TX.
            TXREG = buffer_TX[i];
        }
    }
   else {
        LED_YELLOW = 0;
        LED_GREEN = 0;
        LED_RED = 1;
    }
}
//Apagar lampara
void OFF_LAMPARAS() {
    PORTAbits.RA0 = 0;
    PORTAbits.RA1 = 0;
    PORTAbits.RA2 = 0;

    char buffer_TX[] = "Bombillas Apagadas \r";
    for (int i = 0; i < 20; i++) {
        //  espera a que el registro de transmisión este disponible o vacio.
        while (!TXSTAbits.TRMT) {
        }
        //  escribe el dato que se enviará a través de TX.
        TXREG = buffer_TX[i];
    }
}

//Imrimir en el LCD los datos de Temperatura y Humedad
void printTempHumedad() {
    /* convert humidity value to ascii and send it to display*/
    sprintf(vec_value, "%d", var_RH_Integral);
    LCD_String_xy(0, 9, vec_value);
    sprintf(vec_value, ".%d ", var_RH_Decimal);
    LCD_String(vec_value);
    LCD_Char('%');
    if(var_RH_Integral<80){
        MOTOR = 1;
        __delay_ms(100);
        MOTOR = 0;
        __delay_ms(100);
     /*char buffer_TX[] = "Motor On \n";
        for (int i = 0; i < 19; i++) {
            //  espera a que el registro de transmisión este disponible o vacio.
            while (!TXSTAbits.TRMT) {
            }
            //  escribe el dato que se enviará a través de TX.
            TXREG = buffer_TX[i];
        }*/
    }else if(var_RH_Integral>80){
    PORTBbits.RB6 = 0;
    /*char buffer_TX[] = "Motor off \n";
        for (int i = 0; i < 19; i++) {
            //  espera a que el registro de transmisión este disponible o vacio.
            while (!TXSTAbits.TRMT) {
            }
            //  escribe el dato que se enviará a través de TX.
            TXREG = buffer_TX[i];
        }*/
    
    }else 
        PORTBbits.RB6 = 0;

    /* convert temperature value to ascii and send it to display*/
    sprintf(vec_value, "%d", var_T_Integral);
    LCD_String_xy(0, 0, vec_value);
    sprintf(vec_value, ".%d", var_T_Decimal);
    LCD_String(vec_value);
    LCD_Char(0xdf);
    LCD_Char('C');

    LCD_String_xy(0, 6, "T");
    LCD_String_xy(0, 15, "H");
}

//Procedimiento de enceder las bombillas y de mostrarlas tanto en la consola como en el LCD
void ON_LAMPARAS() {
    LCD_Custom_Char(0, bombillaCar0);
    LCD_Custom_Char(1, bombillaCar0);
    LCD_Custom_Char(2, bombillaCar0);
    LCD_Custom_Char(3, alert);

    ON_LEDS(var_T_Integral); //Encender los leds de advertencia
    if (var_T_Integral < 30 && acumBom == 0) {
        PORTAbits.RA0 = 1;
        char buffer_TX[] = "Se encendio la ;bombilla 1\r";
        for (int i = 0; i < 27; i++) {
            //  espera a que el registro de transmisión este disponible o vacio.
            while (!TXSTAbits.TRMT) {
            }
            //  escribe el dato que se enviará a través de TX.
            TXREG = buffer_TX[i];
        }
        //Parte para Imprimir el Caracter
        LCD_Command(0xc0 | (0)); /*Display characters from c0(2nd row) location */
        LCD_Char(0); /*To display custom character send address as data to point stored 
                                       character */

        var_T_Integral = var_T_Integral + 6;
        acumBom = 6;

    } else if (var_T_Integral < 30 && acumBom == 6) {
        PORTAbits.RA1 = 1;

        char buffer_TX[] = "Se encendio la bombilla 2\r";
        for (int i = 0; i < 27; i++) {
            //  espera a que el registro de transmisión este disponible o vacio.
            while (!TXSTAbits.TRMT) {
            }
            //  escribe el dato que se enviará a través de TX.
            TXREG = buffer_TX[i];
        }
        //Parte para Imprimir el Caracter
        LCD_Command(0xc0 | (3)); /*Display characters from c0(2nd row) location */
        LCD_Char(1); /*To display custom character send address as data to point stored character */

        var_T_Integral = var_T_Integral + 6;
        acumBom = 12;
    } else if (var_T_Integral < 30 && acumBom == 12) {
        PORTAbits.RA2 = 1;

        char buffer_TX[] = "Se encendio la bombilla 3\r";
        for (int i = 0; i < 27; i++) {
            //  espera a que el registro de transmisión este disponible o vacio.
            while (!TXSTAbits.TRMT) {
            }
            //  escribe el dato que se enviará a través de TX.
            TXREG = buffer_TX[i];
        }

        //Parte para Imprimir el Caracter
        LCD_Command(0xc0 | (6)); /*Display characters from c0(2nd row) location */
        LCD_Char(2); /*To display custom character send address as data to point stored character */

var_T_Integral = var_T_Integral + 6;
    } else if (var_T_Integral > 35) {
        banAgregar = 1;

        char buffer_TX[] = "Temperatura Alta \r";
        for (int i = 0; i < 19; i++) {
            //  espera a que el registro de transmisión este disponible o vacio.
            while (!TXSTAbits.TRMT) {
            }
            //  escribe el dato que se enviará a través de TX.
            TXREG = buffer_TX[i];
        }
        //Parte para Imprimir el Caracter
        for (char i = 0; i <= 5; i++) {
            LCD_Command(0xc0 | (i*3)); /*Display characters from c0(2nd row) location */
            LCD_Char(6); /*To display custom character send address as data to point stored character */
        
        }
        MSdelay(100);
    } else
        banAgregar = 1;
    MSdelay(1000);

}

//Procedimeintos utilizados en RTC
void RTC_Read_Clock(char read_clock_address) {
    I2C_Start(DEVICE_ID_WRITE);
    I2C_Write(read_clock_address); /* address from where time needs to be read*/
    I2C_Repeated_Start(DEVICE_ID_READ);
    sec = I2C_Read(0); /*read data and send ack for continuous reading*/
    min = I2C_Read(0); /*read data and send ack for continuous reading*/
    hour = I2C_Read(1); /*read data and send nack for indicating stop reading*/

}

void RTC_Read_Calendar(char read_calendar_address) {
    I2C_Start(DEVICE_ID_WRITE);
    I2C_Write(read_calendar_address); /* address from where time needs to be read*/
    I2C_Repeated_Start(DEVICE_ID_READ);
    Day = I2C_Read(0); /*read data and send ack for continuous reading*/
    Date = I2C_Read(0); /*read data and send ack for continuous reading*/
    Month = I2C_Read(0); /*read data and send ack for continuous reading*/
    Year = I2C_Read(1); /*read data and send nack for indicating stop reading*/
    I2C_Stop();
}

//Procedimiento para leer y mostrar la hora en el LCD
void Mostrar_Fecha() {
    LCD_Init();
    RTC_Read_Clock(0); /*second,minute and hour*/
    I2C_Stop();
    MSdelay(1000);
    if (hour & (1 << Clock_type)) { /* check clock is 12hr or 24hr */

        if (hour & (1 << AM_PM)) { /* check AM or PM */
            LCD_String_xy(1, 14, "PM");
        } else {
            LCD_String_xy(1, 14, "AM");
        }

        hour = hour & (0x1f);
        sprintf(secs, "%x ", sec); /*%x for reading BCD format from RTC DS1307*/
        sprintf(mins, "%x:", min);
        sprintf(hours, "Tim:%x:", hour);
        LCD_String_xy(0, 0, hours);
        LCD_String(mins);
        LCD_String(secs);
    } else {

        hour = hour & (0x3f);
        sprintf(secs, "%x ", sec); /*%x for reading BCD format from RTC DS1307*/
        sprintf(mins, "%x:", min);
        sprintf(hours, "Tim:%x:", hour);
        LCD_String_xy(0, 0, hours);
        LCD_String(mins);
        LCD_String(secs);
    }

    RTC_Read_Calendar(3); /*day, month, year*/
    I2C_Stop();
    sprintf(date, " Dia %x-", Date);
    sprintf(month, "%x-", Month);
    sprintf(year, "%x ", Year);
    LCD_String_xy(2, 0, date);
    LCD_String(month);
    LCD_String(year);

    /* find day */
    switch (days[Day]) {
        case 'S':
            LCD_String("Sun");
            break;
        case 'M':
            LCD_String("Mon");
            break;
        case 'T':
            LCD_String("Tue");
            break;
        case 'W':
            LCD_String("Wed");
            break;
        case 't':
            LCD_String("Thu");
            break;
        case 'F':
            LCD_String("Fri");
            break;
        case 's':
            LCD_String("Sat");
            break;
        default:
            break;

    }
    char buffer_TX[] = "Registrando Hora y Fecha\r";
    for (int i = 0; i < 25; i++) {
        //  espera a que el registro de transmisión este disponible o vacio.
        while (!TXSTAbits.TRMT) {
        }
        // escribe el dato que se enviará a través de TX.
        TXREG = buffer_TX[i];
    }

}
//Procedimiento para lectura de hora.
void Verificar_Hora() {
    RTC_Read_Clock(0); /*gives second,minute and hour*/
    I2C_Stop();
    MSdelay(1000);
}


/*void control_motor(){
        if(var_RH_Integral<80){
        PORTBbits.RB6 = 1;
     
    }else if(var_RH_Integral>80){
    PORTBbits.RB6 = 0;
    }else 
        PORTBbits.RB6 = 0;
}
*/