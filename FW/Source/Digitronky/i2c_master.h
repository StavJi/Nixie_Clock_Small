#ifndef I2C_MASTER_H_INCLUDED
#define I2C_MASTER_H_INCLUDED

/*  kniznica pre i2c zbernicu - TWI AVR
    written by: Luboss17 in 2011
    http://lubosweb.php5.sk                        */

#include <avr/io.h>
#include <compat/twi.h>

#define SCL_CLOCK   100000L  //prenosova rychlost i2c
#define I2C_READ 1
#define I2C_WRITE 0

/****************************************************
    start podmienka
    - smer prenosu
    - na zbernicu zapise adresu zariadenia
    - vrati 0 - po bezchybnom priebehu
    - vrati 1 - neuspesny start
    - vrati 2 - adresa nepotvrdena ACk bitom
****************************************************/

extern void i2c_init(void);

/****************************************************
    start podmienka
    - smer prenosu
    - na zbernicu zapise adresu zariadenia
    - vrati 0 - po bezchybnom priebehu
****************************************************/

extern unsigned char i2c_start(unsigned char address, unsigned char dir);

/****************************************************
    opakovana start podmienka
****************************************************/

extern unsigned char i2c_rep_start(unsigned char address, unsigned char dir);

/****************************************************
    stop podmienka
    - vrati 0 - po bezchybnom priebehu
    - vrati 1 - po neuspesnom stop
****************************************************/

extern unsigned char i2c_stop(void);

/****************************************************
    vysielanie byte-u na zbernicu
    - vrati 0 - po uspesnom vysielani - potvrdene s ACK
    - vrati 1 - data nepotvrdene ACK bitom
****************************************************/

extern unsigned char i2c_write(unsigned char data);

/****************************************************
    prijem byte-u zo zbernice - nie je posledny
    - vrati prijaty bajt a nasleduje dalsi bajt
****************************************************/

extern unsigned char i2c_read_Ack(void);

/****************************************************
    prijem byte-u zo zbernice - posledny bajt
    - vrati prijaty bajt a nasleduje stop bit
****************************************************/

extern unsigned char i2c_read_nAck(void);

#endif // I2C_MASTER_H_INCLUDED
