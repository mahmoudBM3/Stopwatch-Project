/*
 * main.c
 *
 *  Created on: Sep 16, 2022
 *      Author: mabkr
 */
#define delay 2
#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>
unsigned char flag_stop = 0;
unsigned char flag_timer = 0;
unsigned char flag_reset = 0;
unsigned char flag_start = 0;
struct time {
	unsigned char seconds;
	unsigned char minutes;
	unsigned char hours;
} watch;
void INT1_init(void) {
	GICR |= (1 << INT1);
	MCUCR |= (1 << ISC10) | (1 << ISC11);
	GIFR|=(1<<INTF0);
}
void INT0_init(void) {
	GICR |= (1 << INT0);
	MCUCR |= (1 << ISC01);
	GIFR|=(1<<INTF0);
}
void INT2_init(void) {
	GICR |= (1 << INT2);
	GIFR|=(1<<INTF2);
}
void TIMER1_init(void) {
	TCCR1A |= (1 << FOC1A) | (1 << FOC1B);
	TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);
	OCR1A = 976;
	TIMSK |= (1 << OCIE1A);
}
ISR(TIMER1_COMPA_vect) {
	flag_timer = 1;

}
ISR(INT0_vect) {
	flag_reset = 1;
}
ISR(INT1_vect) {
	flag_stop = 1;
}
ISR(INT2_vect) {
	flag_start = 1;
}
void systeminit(void) {
	//setting interrupt pins as inputs
	DDRB &= ~(1 << PB2);
	DDRD &= ~(1 << PD2);
	DDRD &= ~(1 << PD3);
	//enabling internal pullups
	PORTB |= (1 << PB2);
	PORTD |= (1 << PD2);
	//setting Display enables as output
	DDRA = 0x3F;
	PORTA &= 0XC0;
	//setting decoder pins as output
	DDRC = 0x0F;

}
void display(void) {
	unsigned char seconds_digits = watch.seconds % 10;
	unsigned char seconds_tens = watch.seconds / 10;
	unsigned char minutes_digits = watch.minutes % 10;
	unsigned char minutes_tens = watch.minutes / 10;
	unsigned char hours_digits = watch.hours % 10;
	unsigned char hours_tens = watch.hours / 10;
	PORTA &= 0xC0;
	PORTA |= (1 << PA0);
	PORTC = (PORTC & 0xF0) | (seconds_digits & 0x0F);
	_delay_ms(delay);
	PORTA &= ~(1 << PA0);
	PORTA |= (1 << PA1);
	PORTC = (PORTC & 0xF0) | (seconds_tens & 0x0F);
	_delay_ms(delay);
	PORTA &= ~(1 << PA1);
	PORTA |= (1 << PA2);
	PORTC = (PORTC & 0xF0) | (minutes_digits & 0x0F);
	_delay_ms(delay);
	PORTA &= ~(1 << PA2);
	PORTA |= (1 << PA3);
	PORTC = (PORTC & 0xF0) | (minutes_tens & 0x0F);
	_delay_ms(delay);
	PORTA &= ~(1 << PA3);
	PORTA |= (1 << PA4);
	PORTC = (PORTC & 0xF0) | (hours_digits & 0x0F);
	_delay_ms(delay);
	PORTA &= ~(1 << PA4);
	PORTA |= (1 << PA5);
	PORTC = (PORTC & 0xF0) | (hours_digits & 0x0F);
	_delay_ms(delay);
	PORTA &= ~(1 << PA5);
}
int main(void) {

	INT0_init();
	INT1_init();
	INT2_init();
	systeminit();
	TIMER1_init();

	SREG |= (1 << 7);
	while (1) {

		if (flag_timer) {
			watch.seconds++;
			flag_timer = 0;
			if (watch.seconds == 60) {
				watch.seconds = 0;
				watch.minutes++;
			}
			if (watch.minutes == 60) {
				watch.minutes = 0;
				watch.hours++;
			}
		}

		if (flag_stop) {
			TCCR1B &= ~(1 << CS12);
			TCCR1B &= ~(1 << CS10);
			flag_stop = 0;
		}
		if (flag_start) {
			TCCR1B |= (1 << CS12);
			TCCR1B |= (1 << CS10);
			flag_start = 0;
		}
		if (flag_reset) {
			TCNT1 = 0;
			watch.seconds = 0;
			watch.minutes = 0;
			watch.hours = 0;
			flag_reset = 0;
		}

		display();
	}
}
