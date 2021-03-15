#ifdef F_CPU
#undef F_CPU
#endif

#define F_CPU 8000000


#ifndef SW_UART_H
#define SW_UART_H

#include <avr/io.h>
#include <util/delay.h>

#define SW_UART_NUMBER_SIZE signed char
#define SW_UART_BAUD_RATE    9600											// baud rate
#define SW_UART_BAUD_DELAY  (1000000UL/SW_UART_BAUD_RATE-1)					// do NOT edit

#define SW_UART_Tx_DDR		DDRB
#define SW_UART_Tx_PORT		PORTB
#define SW_UART_Tx_NUMBER	0
#define SW_UART_Rx_DDR		DDRD
#define SW_UART_Rx_PIN		PIND
#define SW_UART_Rx_NUMBER	7

void SW_UART_Init(void);													// inicializace SW UARTu
void SW_UART_Write_Byte(SW_UART_NUMBER_SIZE x);								// odeslat byte
SW_UART_NUMBER_SIZE SW_UART_Read_Byte(void);								// okamzity prijem bytu, pro preruseni atd.
SW_UART_NUMBER_SIZE SW_UART_Read_Byte_Waiting(void);						// prijem bytu vcetne cekani na start bit
void SW_UART_Write_Data(SW_UART_NUMBER_SIZE* data, unsigned char count);	// odeslani dat
void SW_UART_Read_Data(SW_UART_NUMBER_SIZE* data, unsigned char count);		// prijem dat S CEKANIM na start bit

#endif
