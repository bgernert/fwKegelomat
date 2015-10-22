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

#include <avr/interrupt.h>

#include "configure.h"
#include "led.h"
#include "can.h"
#include "relay.h"
#include "uart.h"

struct flags_t {
	int clock:1;
} __attribute__((packed)) volatile flags;

void clock_init(void)
{
	TCCR0B = (1 << CS00) | (1 << CS01);
	TIMSK0 |= (1 << TOIE0);
}

// Timer für die Alive-Nachrichten
uint32_t aliveTimer;

void kegelomat_alive_timer_call()
{
	++aliveTimer;
	if(aliveTimer >= ALIVE_TIMER)
	{
		kegelomat_can_send_keepalive();
		aliveTimer = 0;
	}
}

ISR(TIMER0_OVF_vect)
{
	flags.clock = 1;
}

int main()
{
	kegelomat_led_init();
	kegelomat_can_init();
	kegelomat_relay_init();
	kegelomat_uart_init();

	clock_init();

	// Heartbeat auf LED A (grün)
	kegelomat_led_heartbeat(LED_A, 0);

	// Wakeup-Nachricht senden
	kegelomat_can_send_bootup();

	while(1)
	{
		// CAN-Bus abarbeiten
		kegelomat_can_doWork();

		// UART abarbeiten
		kegelomat_uart_doWork();

		// Calls der Uhr abarbeiten
		if(flags.clock)
		{
			flags.clock = 0;
			kegelomat_led_timer_call();
			kegelomat_alive_timer_call();
			kegelomat_relay_timer_call();
		}
	}
}
