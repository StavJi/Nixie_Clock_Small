#ifdef F_CPU
#undef F_CPU
#endif

#define F_CPU 8000000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <avr/pgmspace.h>
#include "i2cmaster.h"

#define KATODA_1_PORT		PORTC		// k 74141 - A
#define KATODA_1_DDR		DDRC
#define KATODA_1_NUMBER		2

#define KATODA_2_PORT		PORTC		// k 74141 - B
#define KATODA_2_DDR		DDRC
#define KATODA_2_NUMBER		0

#define KATODA_3_PORT		PORTB		// k 74141 - C
#define KATODA_3_DDR		DDRB
#define KATODA_3_NUMBER		3

#define KATODA_4_PORT		PORTC		// k 74141 - D
#define KATODA_4_DDR		DDRC
#define KATODA_4_NUMBER		1

#define ANODA_1_PORT		PORTD		// k tranzistorum
#define ANODA_1_DDR			DDRD
#define ANODA_1_NUMBER		0

#define ANODA_2_PORT		PORTD		// k tranzistorum
#define ANODA_2_DDR			DDRD
#define ANODA_2_NUMBER		1

#define ANODA_3_PORT		PORTD		// k tranzistorum
#define ANODA_3_DDR			DDRD
#define ANODA_3_NUMBER		2

#define ANODA_4_PORT		PORTC		// k tranzistorum
#define ANODA_4_DDR			DDRC
#define ANODA_4_NUMBER		3

#define TLACITKO_1_PORT		PORTB		// tlacitko 1
#define TLACITKO_1_DDR		DDRB
#define TLACITKO_1_PIN		PINB
#define TLACITKO_1_NUMBER	1

#define TLACITKO_2_PORT		PORTB		// tlacitko 2
#define TLACITKO_2_DDR		DDRB
#define TLACITKO_2_PIN		PINB
#define TLACITKO_2_NUMBER	2

// ******* NEMENIT ******* //
#define TLACITKO_1_ZMACKNUTO	( (TLACITKO_1_PIN & (1 << TLACITKO_1_NUMBER) ) == 0)					// zjisteni zda doslo ke stisku tlacitka
#define TLACITKO_2_ZMACKNUTO	( (TLACITKO_2_PIN & (1 << TLACITKO_2_NUMBER) ) == 0)

#define TLACITKO_MAX_TIME	15000

#define TLACITKO_MENU		1			// prirazeni funkce tlacitkum
#define TLACITKO_OK			2

#define DIGITS_OFF			KATODA_1_PORT |= (1 << KATODA_1_NUMBER); KATODA_2_PORT |= (1 << KATODA_2_NUMBER); KATODA_3_PORT |= (1 << KATODA_3_NUMBER); KATODA_4_PORT |= (1 << KATODA_4_NUMBER);		

#define ANODE_OFF			ANODA_1_PORT &= ~(1 << ANODA_1_NUMBER);	ANODA_2_PORT &= ~(1 << ANODA_2_NUMBER); ANODA_3_PORT &= ~(1 << ANODA_3_NUMBER);	ANODA_4_PORT &= ~(1 << ANODA_4_NUMBER);
#define ANODE_1_ON			ANODA_1_PORT |= (1 << ANODA_1_NUMBER)
#define ANODE_2_ON			ANODA_2_PORT |= (1 << ANODA_2_NUMBER)
#define ANODE_3_ON			ANODA_3_PORT |= (1 << ANODA_3_NUMBER)
#define ANODE_4_ON			ANODA_4_PORT |= (1 << ANODA_4_NUMBER)

// Adresy RTC

#define DS3231SN			0xD0//0x68	// adresa obvodu
#define RTC_SEC				0x00		// adresa sekund
#define RTC_MIN				0x01		// adresa minut
#define RTC_HOD				0x02		// adresa hodin
#define RTC_DAY				0x04		// den
#define RTC_WEEK_DAY		0x03		// den v tydnu
#define RTC_MONTH			0x05		// mesic + stoleti				
#define RTC_YEAR			0x06		// rok			



// ******* NEMENIT ******* //

// Prevodni tabulka, ktera odpovida rozlozeni na DPS
unsigned char cislice[11]=
{
	//ABCD
	0b00000101,				// 5 - 0
	0b00000001,				// 8 - 1
	0b00001101,				// 7 - 2
	0b00001000,				// 1 - 3
	0b00000100,				// 2 - 4
	0b00000110,				// 6 - 5
	0b00000010,				// 4 - 6
	0b00000000,				// 0 - 7
	0b00001100,				// 3 - 8
	0b00001001,				// 9 - 9
	0b00001111,				// zhasnuto
};

volatile unsigned char disp[6] = {10,10,10,10,10,10};	// globalni promenna pro zobrazovaci rutinu, zapsanim 10 se cislice zhasne
volatile uint8_t pos;									// index prave zobrazovane pozice
volatile uint8_t blikej = 0;							// promenna urcujici zda se ma blikat a kterou cislici aktivni spodni polovina bajtu
volatile uint8_t blikej_cnt = 0;						// pocitadlo pro blikani
volatile uint8_t temp = 0, i2cdata = 0;					// pomocne promenne
volatile uint16_t time_tl_1 = 0;						// promenna urcujici delku zmacknuteho tlacitka
volatile uint16_t time_tl_2 = 0;						// promenna urcujici delku zmacknuteho tlacitka
volatile uint16_t timer = 0;							// casovani pri necinnosti
volatile uint8_t tlacitko_1 = 0;						// co ma dane tlacitko vykonavat
volatile uint8_t tlacitko_2 = 0;						// co ma dane tlacitko vykonavat
volatile uint8_t stop = 0;								// pomocna promenna, ktera zastavi hodiny

volatile uint8_t hodiny = 0, minuty = 0, sekundy = 0;

signed char UART_test[30];								// promenna na test programu po UARTu

void init(void);
void zobraz_cislo(uint8_t cislo);
void read(unsigned char reg);
void write(unsigned char reg);
void read_time(char *dest);

int main(void)
{
    uint8_t previousHour = 0, actualHours = 0;
	
	//SW_UART_Init();
	i2c_init();
	init();
	
	i2cdata = 128;
	
	read(RTC_SEC);
	
	_delay_ms(10);
	
	if(i2cdata > 127)				// hodiny nenastaveny, nastavit nulovy vychozi cas
	{
		i2cdata = 0;
		write(RTC_HOD);
		i2cdata = 0;
		write(RTC_MIN);
		i2cdata = 0;
		write(RTC_SEC);
	}
	
	// Load and save actual hours
	read_time((char*)&disp[0]);
	actualHours = disp[1];
	
	_delay_ms(10);
	
	sei();	// povol globalni preruseni
	
	for(;;)
	{
		
		if(stop)
		{
			blikej = 0b00000011;										// blikej hodiny
			
			while(stop)
			{
				while(TLACITKO_1_ZMACKNUTO || TLACITKO_2_ZMACKNUTO);	// nepokracuj dokud je zmacknute nektere tlacitko
		
				_delay_ms(200);
				tlacitko_1 = 0;
				tlacitko_2 = 0;
				timer = 40000;
				
				while( (tlacitko_2 != TLACITKO_OK) && (timer != 0) )
				{
					if (tlacitko_1 == TLACITKO_OK)
					{
						timer = 40000;
						
						disp[1]++;
												
						if( (disp[1] > 3) && (disp[0] > 1) )
						{
							disp[1] = 0;
							disp[0]++;
						}
						
						if( (disp[1] > 9) && (disp[0] < 2) )
						{
							disp[1] = 0;
							disp[0]++;
						}
						
						if(disp[0] > 2) disp[0] = 0;
						
						while(TLACITKO_1_ZMACKNUTO);
						tlacitko_1 = 0;
					}
				}
				
				if(timer != 0)
				{
					blikej = 0b00001100;					// blikej minuty
					timer = 40000;
				
					while (TLACITKO_2_ZMACKNUTO);			// dokud se nepusti tlacitko nic nedelej
				
					_delay_ms(200);
					tlacitko_1 = 0;
					tlacitko_2 = 0;
								
					while( (tlacitko_2 != TLACITKO_OK) && (timer != 0) )
					{
						if (tlacitko_1 == TLACITKO_OK)
						{
							timer = 40000;
						
							disp[3]++;
						
							if (disp[3] > 9)
							{
								disp[3] = 0;
								disp[2]++;
							}
						
							if(disp[2] > 5) disp[2] = 0;
						
							while(TLACITKO_1_ZMACKNUTO);
							tlacitko_1 = 0;
						}
					}
				
					if (tlacitko_2 == TLACITKO_OK)
					{
						blikej = 0;							// stop blikani
						disp[4] = 0;						// vynulovat sekundy
						disp[5] = 0;
					
						i2cdata = ((disp[0]<<4)+disp[1]);	// slozit BCD hodnotu
						write(RTC_HOD);
						i2cdata = ((disp[2]<<4)+disp[3]);	// slozit BCD hodnotu
						write(RTC_MIN);
						i2cdata = 0;
						write(RTC_SEC);
						
						// Save actual hours
						previousHour = disp[1];
						actualHours = disp[1];
						
						_delay_ms(20);		
					
						blikej = 0b00001111;		
								
						while(TLACITKO_2_ZMACKNUTO);
					
						while(1)
						{
							_delay_ms(3000);
							break;
						}
					}
				}
					blikej = 0;
					stop = 0;						// odchod z menu
				
			}
		}
		else
		{											// normalni stav zobrazuji se hodiny
			//cli();
			previousHour = disp[1];
			read_time((char*)&disp[0]);
			actualHours = disp[1];
			
			// Each hour iterate over each digit to prevent cathode poisoning
			if (previousHour != actualHours)
			{
				for (uint8_t i = 0; i < 10; i++ )
				{
					disp[0] = i;
					disp[1] = i;
					disp[2] = i;
					disp[3] = i;
					_delay_ms(500);
				}
			}
			
		//	sei();
			
/*			SW_UART_Write_Byte(disp[0]+48);
			SW_UART_Write_Byte(disp[1]+48);
			SW_UART_Write_Data(":",1);
			SW_UART_Write_Byte(disp[2]+48);
			SW_UART_Write_Byte(disp[3]+48);
			SW_UART_Write_Data(":",1);
			SW_UART_Write_Byte(disp[4]+48);
			SW_UART_Write_Byte(disp[5]+48);
			SW_UART_Write_Data("\n",1);*/	
							
			_delay_ms(50);		
		}
	}
	
	return 0;										// nikdy sem nedojne ale nejsem prase
}

void init (void)
{
	// vystupy - katody
	KATODA_1_DDR |= (1 << KATODA_1_NUMBER);
	KATODA_2_DDR |= (1 << KATODA_2_NUMBER);
	KATODA_3_DDR |= (1 << KATODA_3_NUMBER);
	KATODA_4_DDR |= (1 << KATODA_4_NUMBER);
	
	// vystupy anody
	ANODA_1_DDR |= (1 << ANODA_1_NUMBER);
	ANODA_2_DDR |= (1 << ANODA_2_NUMBER);
	ANODA_3_DDR |= (1 << ANODA_3_NUMBER);
	ANODA_4_DDR |= (1 << ANODA_4_NUMBER);
	
	// vstupy pro tlacitka
	TLACITKO_1_DDR &= ~(1 << TLACITKO_1_NUMBER);
	TLACITKO_2_DDR &= ~(1 << TLACITKO_2_NUMBER);
	
	// timer 0 - multiplex - cca 330Hz
	TCCR0B |= (1 << CS02);							// preddelicka 256	
	TIMSK0 |= (1 << TOIE0);							// povoleni casovace 2
	TCNT0 = 162;									// 330Hz
	//TCNT0 = 248;									// 3900Hz
	// plny chod 122Hz
	//TCNT0 = 100;									// 200Hz
	
	// timer 2 - 3900 Hz 
	TCCR2B |= (1 << CS21);							// preddelicka 8
	TIMSK2 |= (1 << TOIE2);							// poveleni casovace 2
	
	OSCCAL = 0xAE;									// kalibracni konstanta oscilatoru, nevim jestli funguje
}

void zobraz_cislo(uint8_t cislo)
{
	if (cislice[cislo] & (1 << KATODA_1_NUMBER))	// rozhodnuti zda se bude zapisovat 0 nebo 1
	{
		KATODA_1_PORT |= (1 << KATODA_1_NUMBER);	// zapis log 1
	}
	else
	{
		KATODA_1_PORT &= ~(1 << KATODA_1_NUMBER);	// zapis log 0
	}
	
	if (cislice[cislo] & (1 << KATODA_2_NUMBER))	// rozhodnuti zda se bude zapisovat 0 nebo 1
	{
		KATODA_2_PORT |= (1 << KATODA_2_NUMBER);	// zapis log 1
	}
	else
	{
		KATODA_2_PORT &= ~(1 << KATODA_2_NUMBER);	// zapis log 0
	}
		
	if (cislice[cislo] & (1 << KATODA_3_NUMBER))	// rozhodnuti zda se bude zapisovat 0 nebo 1
	{
		KATODA_3_PORT |= (1 << KATODA_3_NUMBER);	// zapis log 1
	}
	else
	{
		KATODA_3_PORT &= ~(1 << KATODA_3_NUMBER);	// zapis log 0
	}
			
	if (cislice[cislo] & (1 << KATODA_4_NUMBER))	// rozhodnuti zda se bude zapisovat 0 nebo 1
	{
		KATODA_4_PORT |= (1 << KATODA_4_NUMBER);	// zapis log 1
	}
	else
	{
		KATODA_4_PORT &= ~(1 << KATODA_4_NUMBER);	// zapis log 0
	}
}

void read(unsigned char reg)
{
	i2c_start_wait(DS3231SN+I2C_WRITE);					// zapis do DS3231
	i2c_write(reg);									// adresa bunky
	i2c_rep_start(DS3231SN+I2C_READ);				// cteni z DS3231
	i2cdata = i2c_readNak();						// ulozit prectenou hodnotu do globalni promenne
	i2c_stop();		               					// stop
}

void write(unsigned char reg)
{
	i2c_start_wait(DS3231SN+I2C_WRITE);
	i2c_write(reg);	        						// 	vyber adresy( hodiny minuty atd)
	i2c_write(i2cdata);								// 	zapise BCD na vybranou adresu
	i2c_stop(); 									//	stop
}

void read_time(char *dest)							// funkce pro vycteni casu z RTC do zvoleneho pole
{
	read(RTC_SEC);
	dest[5]=(i2cdata & 0x0F);
	dest[4]=((i2cdata >>4)& 0b00000111);
	read(RTC_MIN);
	dest[3]=(i2cdata & 0x0F);
	dest[2]=((i2cdata >>4)& 0b00000111);
	read(RTC_HOD);
	dest[1]=(i2cdata & 0x0F);
	dest[0]=((i2cdata >>4)& 0b00000011);
	blikej = 0;
}

ISR(TIMER0_OVF_vect)								// multiplex
{
	TCNT0 = 162;									// 330Hz
	//TCNT0 = 248;									// 3900Hz
	// plny chod 122Hz
	//TCNT0 = 100;									// 200Hz
	
	ANODE_OFF;
	DIGITS_OFF;				// zhasnout vsechno

	temp = (1 << pos);
	
	blikej_cnt++;
	
	if(blikej_cnt > 340) blikej_cnt = 0;			// perioda blikani cca 1 sekunda

	if(blikej & temp)								// blikat zvolenou cislici
	{
		if(blikej_cnt > 50)							// sviti dele nez je zhasnuto
		{
			zobraz_cislo(disp[pos]);
		}
	}
	else
	{												// blikani cislici je vypnuto
		zobraz_cislo(disp[pos]);					// zobraz trvale
	}
	
	switch (pos)			// vybrat anodu, skenovani tlacitek
	{
		case 0:					// sileny if osetruje situaci, aby nesvitila nula (napr. 09:42), ale aby svitila v situaci 00:15
		if( ( (disp[0] < 10) && (disp[0] > 0) ) || ( (disp[0] < 10) && (disp[0] == 0) && (disp[1] == 0)) )			// pokud neni nic zobrazeno, nepripoji se ani anoda,
		{							// aby neprosvitala sousedni cislice
			ANODE_1_ON;
		}
			
		break;
		case 1:
		if (disp[1] < 10)
		{
			ANODE_2_ON;
		}
		break;
		case 2:
		if (disp[2] < 10)
		{
			ANODE_3_ON;
		}
		break;
		case 3:
		if (disp[3] < 10)
		{
			ANODE_4_ON;
		}
		break;
	}// end switch
	
	
	pos++;							// prepnout dalsi cislici
	if (pos > 3) pos = 0;
}			

ISR(TIMER2_OVF_vect)								// tlacitka
{
	if (TLACITKO_1_ZMACKNUTO)
	{
		time_tl_1++;
	}
	else
	{
		time_tl_1 = 0;
	}
	
	if (TLACITKO_2_ZMACKNUTO)
	{
		time_tl_2++;
	}
	else
	{
		time_tl_2 = 0;
	}
	
	if(stop == 1)
	{
		if(timer != 0)
		{
			timer--;
		}
	}
	
	
	if (stop == 0)
	{
		if(time_tl_1 > 10000) stop = 1;				// menu
		
		if(time_tl_2 > 10000) stop = 1;				// menu
				
		if(time_tl_1 > TLACITKO_MAX_TIME) time_tl_1 = 0;
			
		if(time_tl_2 > TLACITKO_MAX_TIME) time_tl_2 = 0;
	}
	
	if (stop == 1)
	{
		if(time_tl_1 > 40)
		{
			time_tl_1 = 0;
			tlacitko_1 = TLACITKO_OK;
		}
		
		if(time_tl_2 > 40)
		{
			time_tl_2 = 0;
			tlacitko_2 = TLACITKO_OK;
		}
	}
}