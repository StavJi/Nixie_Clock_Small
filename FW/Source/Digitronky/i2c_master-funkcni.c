#ifdef F_CPU
#undef F_CPU
#endif

#define F_CPU 8000000

#include "i2c_master.h"

/****************************************************
    inicializacna funkcia i2c zbernice - master
****************************************************/

void i2c_init(void)
{
    TWSR = 0;                         //bez delicky
    TWBR = ((F_CPU/SCL_CLOCK)-16)/2;  //hodnota TWBR musi byt viac ako 10 t.j. FCPU > 3.6 MHz
}

/****************************************************
    start podmienka
    - smer prenosu
    - na zbernicu zapise adresu zariadenia
    - vrati 0 - po bezchybnom priebehu
    - vrati 1 - neuspesny start
    - vrati 2 - adresa nepotvrdena ACk bitom
****************************************************/

unsigned char i2c_start(unsigned char address, unsigned char dir)
{
	// posle start podmienku
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

	// caka na jej prijatie
	while(!(TWCR & (1<<TWINT)));

	// kontrola TWSR
	if (((TWSR & 0xF8) != TW_START) && ((TWSR & 0xF8) != TW_REP_START)) return 1;

	// odosle adresu zariadenia s urcenim smeru prenosu dat
	TWDR = address | dir;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// caka na dokoncenie presnosu
	while(!(TWCR & (1<<TWINT)));

	// kontrola TWSR
	if (((TWSR & 0xF8) != TW_MT_SLA_ACK) && ((TWSR & 0xF8) != TW_MR_SLA_ACK)) return 2;

	return 0;
}

/****************************************************
    opakovana start podmienka
****************************************************/

unsigned char i2c_rep_start(unsigned char address, unsigned char dir)
{
	return (i2c_start(address,dir));
}

/****************************************************
    stop podmienka
    - vrati 0 - po bezchybnom priebehu
    - vrati 1 - po neuspesnom stop
****************************************************/

unsigned char i2c_stop(void)
{
    	//odosle stop podmienku
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);

	//pocka na jej dokoncenie
	while(!(TWCR & (1<<TWSTO)));

	if ((TWSR & 0xF8) != TW_SR_STOP) return 1;

	return 0;
}

/****************************************************
    vysielanie byte-u na zbernicu
    - vrati 0 - po uspesnom vysielani - potvrdene s ACK
    - vrati 1 - data nepotvrdene ACK bitom
****************************************************/

unsigned char i2c_write(unsigned char data)
{
	//odosle data na predtym adresovane zariadenie
	TWDR = data;
	TWCR = (1<<TWINT)|(1<<TWEN);

	//caka na dokoncenie odosielania
	while(!(TWCR & (1<<TWINT)));

	//skontroluje stav TWSR
	if((TWSR & 0xF8) != TW_MT_DATA_ACK) return 1; 
	return 0; 
}

/****************************************************
    prijem byte-u zo zbernice - nie je posledny
    - vrati prijaty bajt a nasleduje dalsi bajt
****************************************************/

unsigned char i2c_read_Ack(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while(!(TWCR & (1<<TWINT)));

        return TWDR;
}

/****************************************************
    prijem byte-u zo zbernice - posledny bajt
    - vrati prijaty bajt a nasleduje stop bit
****************************************************/

unsigned char i2c_read_nAck(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));

        return TWDR;
}
