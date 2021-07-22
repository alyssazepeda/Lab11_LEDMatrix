/*	Author: Alyssa Zepeda
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #11  Exercise #2
 *	Exercise Description: https://youtu.be/IIzwt3bAQnI
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "timer.h"
#endif

//Struct
#define tasksSize 1
//struct
typedef struct task {
        int state;
        unsigned long period;
        unsigned long elapsedTime;
        int (*TickFct)(int);
} task;
task tasks[tasksSize];
//GCD of the periods
const unsigned long tasksPeriod = 20;

//////////////////////////////////////////////////////////////
//timer.h functions that needed to be added in main.c
void TimerISR() {
        unsigned char i;
        for (i = 0; i < tasksSize; ++i) {
                if ( tasks[i].elapsedTime >= tasks[i].period ) {
                        tasks[i].state = tasks[i].TickFct(tasks[i].state);
                        tasks[i].elapsedTime = 0;
                }
                tasks[i].elapsedTime += tasksPeriod;
        }
}
ISR(TIMER1_COMPA_vect) {
    _avr_timer_cntcurr--;
    if (_avr_timer_cntcurr == 0){
        TimerISR();
        _avr_timer_cntcurr = _avr_timer_M;
    }
}
/////////////////////////////////////////////////////////////
#define A0 ~PINA&0x01
#define A1 ~PINA&0x02
enum LED_States {check, left, right, release};
int LED_Tick(int state) {
	// Local Variables
	static unsigned char pattern = 0x80; // LED pattern 
	static unsigned char row = 0xE0; // Row(s) displaying pattern.
	// 0: display pattern on row
	// 1: do NOT display pattern on row
	// Transitions
	switch (state) {
		case check: 
			if(A0) {
				state = left;
			}
			else if(A1) {
				state = right;
			}
			else {state = check;}
			break;
		case left: state = release; break;
		case right: state = release; break;
		case release: state = (!(A1) && !(A0)) ? check : release; break;
		default: state = check; break;
	}
	// Actions
	switch (state) {
		case check: break;
		case release: break;
		case left:
			if(pattern != 0x80) {pattern = (pattern << 1);}
			break;
		case right: 
			if (pattern != 0x01) {pattern = (pattern >> 1);}
			break;
		default: break;
	}
	PORTC = pattern; // Pattern to display
	PORTD = row; // Row(s) displaying pattern
	return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
    /* Insert your solution below */
    	unsigned char i = 0;
	tasks[i].state = check;
	tasks[i].period = tasksPeriod;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &LED_Tick;

	TimerSet(tasksPeriod);
	TimerOn();
	while (1) {}
    	return 1;
}
