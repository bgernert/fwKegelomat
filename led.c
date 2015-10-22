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

#include "led.h"

struct kegelomat_led_status {
	volatile uint8_t* led_port;
	uint8_t led_portbit;
	uint8_t state;					// Status der LED
	uint16_t calls;					// Anz. der Aufrufe (1 Aufruf ~ 0.004s)
	uint16_t counter;				// Zähler
	uint16_t limit;					// Anz. wie of die Aktion ausgeführt werden soll
} volatile ledStatus[LED_COUNT];

uint16_t ledLocate = 0;

void kegelomat_led_init()
{
	// LEDs konfigurieren (Als output konfigurieren)
	LEDA_DDR |= (1 << LEDA_DDRBIT);
	LEDB_DDR |= (1 << LEDB_DDRBIT);
	LEDC_DDR |= (1 << LEDC_DDRBIT);
	LEDD_DDR |= (1 << LEDD_DDRBIT);

	// LEDs löschen
	LEDA_PORT |= (1 << LEDA_PORTBIT);
	LEDB_PORT |= (1 << LEDB_PORTBIT);
	LEDC_PORT |= (1 << LEDC_PORTBIT);
	LEDD_PORT |= (1 << LEDD_PORTBIT);

	// Adressen der LEDs übergeben
	ledStatus[0].led_port 		= &LEDA_PORT;
	ledStatus[0].led_portbit	= LEDA_PORTBIT;
	ledStatus[1].led_port 		= &LEDB_PORT;
	ledStatus[1].led_portbit	= LEDB_PORTBIT;
	ledStatus[2].led_port 		= &LEDC_PORT;
	ledStatus[2].led_portbit	= LEDC_PORTBIT;
	ledStatus[3].led_port 		= &LEDD_PORT;
	ledStatus[3].led_portbit	= LEDD_PORTBIT;

	for(uint8_t i = 0; i < LED_COUNT; ++i)
	{
		ledStatus[i].state = 0;
		ledStatus[i].calls = 0;
		ledStatus[i].counter = 0;
		ledStatus[i].limit = 0;
	}
}

void kegelomat_led_timer_call()
{
	for(uint8_t i = 0; i < LED_COUNT; ++i)
	{
		switch(ledStatus[i].state)
		{
		// LED ist undefiniert/nichts ändern
		case 0:
			break;

		// LED einschalten
		case 1:
			ledStatus[i].state = 0;
			*(ledStatus[i].led_port) &= ~(1 << ledStatus[i].led_portbit);
			break;

		// LED ausschalten
		case 2:
			ledStatus[i].state = 0;
			*(ledStatus[i].led_port) |= (1 << ledStatus[i].led_portbit);
			break;

		// LED blinken lassen
		case 3:
			++ledStatus[i].calls;
			if(ledStatus[i].calls == 1)
			{
				++ledStatus[i].counter;
				*(ledStatus[i].led_port) &= ~(1 << ledStatus[i].led_portbit);
			}
			if(ledStatus[i].calls == LED_BLINK_TIMER)
			{
				*(ledStatus[i].led_port) |= (1 << ledStatus[i].led_portbit);
			}
			if(ledStatus[i].calls == (2 * LED_BLINK_TIMER))
			{
				ledStatus[i].calls = 0;
				if(ledStatus[i].limit > 0 \
						&& !(ledStatus[i].counter < ledStatus[i].limit))
				{
					ledStatus[i].state = 2;
					ledStatus[i].limit = 0;
					ledStatus[i].counter = 0;
				}
			}
			break;

		// LED Heartbeat
		case 4:
			++ledStatus[i].calls;
			if(ledStatus[i].calls == 1)
			{
				++ledStatus[i].counter;
				*(ledStatus[i].led_port) &= ~(1 << ledStatus[i].led_portbit);
			}
			if(ledStatus[i].calls == LED_HB_TIMER)
			{
				*(ledStatus[i].led_port) |= (1 << ledStatus[i].led_portbit);
			}
			if(ledStatus[i].calls == (2 * LED_HB_TIMER))
			{
				*(ledStatus[i].led_port) &= ~(1 << ledStatus[i].led_portbit);
			}
			if(ledStatus[i].calls == (3 * LED_HB_TIMER))
			{
				*(ledStatus[i].led_port) |= (1 << ledStatus[i].led_portbit);
			}
			if(ledStatus[i].calls == (3 * LED_HB_TIMER) + LED_HB_SLEEP)
			{
				ledStatus[i].calls = 0;
				if(ledStatus[i].limit > 0 \
						&& !(ledStatus[i].counter < ledStatus[i].limit))
				{
					ledStatus[i].state = 2;
					ledStatus[i].limit = 0;
					ledStatus[i].counter = 0;
				}
			}
			break;

		// Bei allen anderen Werten, LED ausschalten
		default:
			ledStatus[i].state = 2;
			break;
		}
	}
}

void kegelomat_led_on(kegelomat_led led)
{
	ledStatus[led].state = 1;
}

void kegelomat_led_off(kegelomat_led led)
{
	ledStatus[led].state = 2;
}

void kegelomat_led_blink(kegelomat_led led, uint16_t limit)
{
	ledStatus[led].state = 3;
	ledStatus[led].limit = limit;
}

void kegelomat_led_heartbeat(kegelomat_led led, uint16_t limit)
{
	ledStatus[led].state = 4;
	ledStatus[led].limit = limit;
}
