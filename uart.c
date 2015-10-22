/*
 *  Copyright (c) 2015 Björn Gernert <mail@bjoern-gernert.de>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "uart.h"

volatile uint8_t uart_msg;
volatile uint8_t uart_new_char;

typedef struct {
	uint8_t active;
	uint8_t position;
	char ball[4];
	char score[2];
	char total0[5];
	char total1[5];
} kegelomat_uart_status;

kegelomat_uart_status uartBuffer;

ISR(USART_RX_vect)
{
    // Daten aus dem Puffer lesen
    uart_new_char = UDR0;

    uart_msg = 1;

    kegelomat_led_blink(LED_B, 1);
}

void kegelomat_uart_init()
{
	// Baudrate einstellen (Normaler Modus)
	UBRR0H = (unsigned char) (UART_UBRR_BAUD >> 8);
	UBRR0L = (unsigned char) UART_UBRR_BAUD;

	// Aktivieren des "Daten empfangen"-Interrupts
	UCSR0B = (1<<RXEN0)|(1<<RXCIE0);

	// Einstellen des Datenformats
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);

	sei();

	uart_msg = 0;

	// Strings null terminieren
	uartBuffer.ball[3] = '\0';
	uartBuffer.score[1] = '\0';
	uartBuffer.total0[4] = '\0';
	uartBuffer.total1[4] = '\0';
}

void kegelomat_uart_doWork()
{
	if(uart_msg > 0)
	{
		uart_msg = 0;

		if(uart_new_char == '\x19')
		{
			// Parsen beginnen
			if(uartBuffer.active == 1)
			{
				kegelomat_uart_error();
			}
			kegelomat_uart_reset_status();
			uartBuffer.active = 1;

		} else {
			if(uartBuffer.active > 0)
			{
				++uartBuffer.position;
				if(uartBuffer.position > 0 && uartBuffer.position < 4)
				{
					uartBuffer.ball[uartBuffer.position-1] = uart_new_char;
				}
				if(uartBuffer.position > 4 && uartBuffer.position < 6)
				{
					uartBuffer.score[0] = uart_new_char;
				}
				if(uartBuffer.position > 6 && uartBuffer.position < 11)
				{
					uartBuffer.total0[uartBuffer.position-7] = uart_new_char;
				}
				if(uartBuffer.position > 12 && uartBuffer.position < 17)
				{
					uartBuffer.total1[uartBuffer.position-13] = uart_new_char;
				}
				if(uartBuffer.position == 16)
				{

					// Struct ist nun gefüllt, Daten über CAN-Bus senden
					if(kegelomat_uart_check_digit(uartBuffer.ball) == false)
					{
						kegelomat_uart_error();
						return;
					}
					if(kegelomat_uart_check_digit(uartBuffer.score) == false)
					{
						kegelomat_uart_error();
						return;
					}
					if(kegelomat_uart_check_digit(uartBuffer.total0) == false)
					{
						kegelomat_uart_error();
						return;
					}
					if(kegelomat_uart_check_digit(uartBuffer.total1) == false)
					{
						kegelomat_uart_error();
						return;
					}

					uint16_t ball = atoi(uartBuffer.ball);
					uint8_t score = atoi(uartBuffer.score);
					uint16_t total0 = atoi(uartBuffer.total0);
					uint16_t total1 = atoi(uartBuffer.total1);

					// Nachricht erstellen
					uint8_t msg[7];

					msg[0] = (ball >> 8);
					msg[1] = (ball & 0xff);
					msg[2] = score;
					msg[3] = (total0 >> 8);
					msg[4] = (total0 & 0xff);
					msg[5] = (total1 >> 8);
					msg[6] = (total1 & 0xff);

					kegelomat_can_send_msg(CAN_MSG_NEW_SCORE, msg, 7);

					kegelomat_uart_reset_status();
				}
			}
		}
	}
}

void kegelomat_uart_reset_status()
{
	memset(&uartBuffer, 0, sizeof(uartBuffer));
}

void kegelomat_uart_error()
{
	kegelomat_led_blink(LED_D, 4);
	kegelomat_can_send_msg(CAN_MSG_ERROR, ERR_UART, 1);
	kegelomat_uart_reset_status();
}

bool kegelomat_uart_check_digit(char* c)
{
	// Jedes Char prüfen
	for(uint8_t i = 0; i < strlen(c); ++i)
	{
		if(!isdigit(c[i]))
			return false;
	}
	return true;
}
