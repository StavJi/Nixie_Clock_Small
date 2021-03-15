#include "SW_UART.h"	// hlavicka knihovny

void SW_UART_Init(void){
	SW_UART_Tx_DDR |= (1 << SW_UART_Tx_NUMBER);
	SW_UART_Tx_PORT |= (1 << SW_UART_Tx_NUMBER);
	SW_UART_Rx_DDR &= ~(1 << SW_UART_Rx_NUMBER);
}

void SW_UART_Write_Byte(SW_UART_NUMBER_SIZE x){	// odeslat byt
	unsigned char i = 8;
	SW_UART_Tx_PORT &= ~(1 << SW_UART_Tx_NUMBER);	// start bit
	_delay_us(SW_UART_BAUD_DELAY);
	do{	// casove optimalni smycka
		if(x & 0b00000001){	// optimalni konstrukce odesilani - podminka
			SW_UART_Tx_PORT |= (1 << SW_UART_Tx_NUMBER);	// jednicka
		}else{
			SW_UART_Tx_PORT &= ~(1 << SW_UART_Tx_NUMBER);	// nula
		}
		x = (x >> 1);	// optimalni konstrukce odesilani - posuv
		_delay_us(SW_UART_BAUD_DELAY);
	}while(--i);	// konec casove optimalni smycky
	SW_UART_Tx_PORT |= (1 << SW_UART_Tx_NUMBER);	// stop bit
	_delay_us(SW_UART_BAUD_DELAY);
}

SW_UART_NUMBER_SIZE SW_UART_Read_Byte(void){	// okamzity prijem bytu, pro preruseni atd.
	SW_UART_NUMBER_SIZE x=0;
	unsigned char i = 8;
	_delay_us(SW_UART_BAUD_DELAY/2 + SW_UART_BAUD_DELAY);	// 1,5 periody -> start -> mereni v polovine periody
	do{	// casove optimalni smycka
		x = (x >> 1);	// optimalni konstrukce odesilani - posuv
		if((SW_UART_Rx_PIN & (1 << SW_UART_Rx_NUMBER)) != 0){	// optimalni konstrukce odesilani - cteni
			x |= 0b10000000;	// jednicka
		}else{
			x &= 0b01111111;	// nula
		}
		_delay_us(SW_UART_BAUD_DELAY);
	}while(--i);	// konec casove optimalni smycky
	_delay_us(SW_UART_BAUD_DELAY);	// stop bit
	return x;
}

SW_UART_NUMBER_SIZE SW_UART_Read_Byte_Waiting(void){	// prijem bytu vcetne cekani na start bit
	while((SW_UART_Rx_PIN & (1 << SW_UART_Rx_NUMBER)) != 0);	// cekani na sestupnou hranu
	return SW_UART_Read_Byte();
}

void SW_UART_Write_Data(SW_UART_NUMBER_SIZE* data, unsigned char count){	// odeslani dat
	for (unsigned char i = 0; i < count; i++){
		SW_UART_Write_Byte(data[i]);	
	}
}

void UART_Read_Data(SW_UART_NUMBER_SIZE* data, unsigned char count){	// prijem dat S CEKANIM na start bit
	for (unsigned char i = 0; i < count; i++){
		data[i] = SW_UART_Read_Byte_Waiting();	
	}
}
