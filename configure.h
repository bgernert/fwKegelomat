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

#ifndef SRC_CONFIGURE_H_
#define SRC_CONFIGURE_H_

/**
 * Version
 */

#define FW_VERSION		4

/* Version ENDE */

/**
 * Alive Timer
 */

#define ALIVE_TIMER		300000	// ~ Alle 5 Minuten

/* Alive Timer ENDE */

/**
 * Ports und Register für die LEDs
 */

// Anzahl LEDs
#define LED_COUNT		4

// LED A (grün)
#define LEDA_PORT		PORTB
#define LEDA_PORTBIT	PORTB1
#define LEDA_DDR		DDRB
#define LEDA_DDRBIT		DDB1

// LED B (gelb)
#define LEDB_PORT		PORTC
#define LEDB_PORTBIT	PORTC4
#define LEDB_DDR		DDRC
#define LEDB_DDRBIT		DDC4

// LED C (gelb)
#define LEDC_PORT		PORTC
#define LEDC_PORTBIT	PORTC5
#define LEDC_DDR		DDRC
#define LEDC_DDRBIT		DDC5

// LED D (rot)
#define LEDD_PORT		PORTD
#define LEDD_PORTBIT	PORTD3
#define LEDD_DDR		DDRD
#define LEDD_DDRBIT		DDD3

/**
 * Zeit-Einstellungen für die LEDs
 */

#define LED_HB_TIMER	150		// Zeit eines Heartbeat/Pause zwischen Heartbeat
#define LED_HB_SLEEP	700		// Zeit zwischen zwei HB-Durchgängen
#define LED_BLINK_TIMER	150		// Zeit LED an/LED aus

/* LEDs Ende */

/**
 * Ports für die Adresse des CAN-Bus
 *
 * PORT   = Port
 * DDR    = Data-Direction-Register
 * PINREG = Register der Pins
 * PIN    = Nummer des Pins
 */

// Jumper 0
#define CANADDR_0_PORT		PORTD
#define CANADDR_0_DDR		DDRD
#define CANADDR_0_PINREG	PIND
#define CANADDR_0_PIN		PD4

// Jumper 1
#define CANADDR_1_PORT		PORTD
#define CANADDR_1_DDR		DDRD
#define CANADDR_1_PINREG	PIND
#define CANADDR_1_PIN		PD5

// Jumper 2
#define CANADDR_2_PORT		PORTD
#define CANADDR_2_DDR		DDRD
#define CANADDR_2_PINREG	PIND
#define CANADDR_2_PIN		PD6

// Jumper 3
#define CANADDR_3_PORT		PORTD
#define CANADDR_3_DDR		DDRD
#define CANADDR_3_PINREG	PIND
#define CANADDR_3_PIN		PD7

/**
 * Einstellungen für den CAN-Bus
 */

// Siehe 'contrib/can.h' für mögliche Werte
#define CANBITRATE			BITRATE_50_KBPS

// Port des externen CAN Interrupts
#define CANINTR				INT0

// Externer Interrupt Vector des CAN-Bus
#define CANINTRV			INT0_vect

/** CAN-Bus ENDE */


/**
 * Einstellungen für die Relais
 */

// Data Bit
#define RELAY_DDR_DATA		DDRC
#define RELAY_DDRPIN_DATA	DDC0
#define RELAY_PORT_DATA		PORTC
#define RELAY_PORTPIN_DATA	PORTC0

// Shift Clock
#define RELAY_DDR_SHC		DDRC
#define RELAY_DDRPIN_SHC	DDC1
#define RELAY_PORT_SHC		PORTC
#define RELAY_PORTPIN_SHC	PORTC1

// Store Clock
#define RELAY_DDR_STC		DDRC
#define RELAY_DDRPIN_STC	DDC3
#define RELAY_PORT_STC		PORTC
#define RELAY_PORTPIN_STC	PORTC3

// Reset
#define RELAY_DDR_RESET		DDRC
#define RELAY_DDRPIN_RESET	DDC2
#define RELAY_PORT_RESET	PORTC
#define RELAY_PORTPIN_RESET	PORTC2

/**
 * Punktesteuerung mit Relais
 */

#define RELAY_BUTTON_SCORE_TIME_ON 	50
#define RELAY_BUTTON_SCORE_TIME_OFF	125

#define RELAY_BUTTON_SCORE_DEL		2
#define RELAY_BUTTON_SCORE_1		3
#define RELAY_BUTTON_SCORE_10		4
#define RELAY_BUTTON_SCORE_100		5
#define RELAY_BUTTON_SCORE_1000		6

/** Relais ENDE */

/**
 * Einstellungen für den UART
 */

// Baud-Rate, als unsigned long (UL) angeben
#define UART_BAUDRATE		666UL

// Berechnung der realen Baudrate
#define UART_UBRR_BAUD   	(F_CPU/16/UART_BAUDRATE-1)

/** UART Ende */

/*
 * Nachrichten-Typen
 *
 * CAN-Nachrichten IDs bestehen aus 11 Bits
 * Je größer die ID, umso wichtiger die Nachricht!
 * 4 Bit werden für die Adressierung benötigt!
 * 1 Bit für Gruppennachrichten
 * -> 6 Bit bleiben für den Typ der Nachricht über!
 *    0bxxxxxxGAAAA
 */

#define CAN_MSG_KEEPALIVE			0b00000000	// Keepalive Nachricht
#define CAN_MSG_BOOTUP				0b00000001	// Boot Nachricht
#define CAN_MSG_SEND_ALIVE			0b00000010	// Keepalive Nachricht angef.
#define CAN_MSG_ERROR				0b00000100	// Fehler
#define CAN_MSG_ANSWER_SET_RELAY	0b00001000	// Relais unbegrenzt gesetzt
#define CAN_MSG_SET_RELAY			0b00001001	// Relais unbegrenzt setzen
#define CAN_MSG_ANSWER_TSET_RELAY	0b00001010	// Relais begrenzt gesetzt
#define CAN_MSG_TSET_RELAY			0b00001011	// Relais begrenzt setzen
#define CAN_MSG_ANSWER_SSET_RELAY 	0b00010000	// Gesamtpunktestand gesetzt
#define CAN_MSG_SSET_RELAY	 		0b00010001	// Gesamtpunktestand setzen
#define CAN_MSG_NEW_SCORE			0b00111111	// Neuer Wurf

/** Nachrichten-Typen ENDE */

/*
 * Fehler-Typen
 *
 * Eine 1 Byte große Fehlernachricht, die die Art
 * des Fehler genauer definiert.
 */

#define ERR_UART				0b00000000
#define ERR_CAN					0b00000001
#define ERR_UNKNOWN_FUNCTION	0b00000010
#define ERR_MISSING_PARAMETER   0b00000011
#define ERR_QUEUE_FULL			0b00000100

/** Fehler-Typen ENDE */

#endif /* SRC_CONFIGURE_H_ */
