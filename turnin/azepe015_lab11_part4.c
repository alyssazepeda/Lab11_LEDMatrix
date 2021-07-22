/*	Author: Alyssa Zepeda
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #11  Exercise #4
 *	Exercise Description: 
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
const unsigned long tasksPeriod = 1;

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
#define A2 ~PINA&0x04
#define A3 ~PINA&0x08
unsigned int p1 = 0x90;
unsigned int p2 = 0xF0;
unsigned int r1 = 0xF0;
unsigned int r2 = 0xF6;
unsigned int pattern[] ={0x90, 0xF0}; // LED pattern
unsigned int row[] = {0xF0, 0xF6}; //Row(s) displaying pattern
enum Square_States {square};
int Square_Tick(int state) {
	// Local Variables
	static unsigned char j = 0;
	//static unsigned char pattern[] ={p1, p2}; // LED pattern 
	//static unsigned char row[] = {r1, r2}; //Row(s) displaying pattern		// 0: display pattern on row
	// 1: do NOT display pattern on row
	pattern[0] = p1;
	pattern[1] = p2;
	row[0] = r1;
	row[1] =r2;
	// Transitions
	switch (state) {
		case square: break;
		default: state = square; break;
	}
	// Actions
	switch (state) {
		case square: j = !j; break;
		default: break;
	}
	PORTC = pattern[j]; // Pattern to display
	PORTD = row[j]; // Row(s) displaying pattern
	return state;
}
enum button_states{check, up, down, left, right, release};
int Button_Tick(int state) {
	switch(state) {
		case check: 
			if(A0) {state = up;}
			else if(A1) {state = down;}
			else if (A2) {state = left;}
			else if(A3) {state = right;}
			else {state = check;}
			break;
		case up: state = release; break;
		case down: state = release; break;
		case left: state = release; break;
		case right: state = release; break;
		case release: 
			state = (!(A0) && !(A1) && !(A2) && !(A3)) ? check : release; 
			break;
		default: state = check; break;
	}
	switch(state) {
		case check: break;
		case release: break;
		case left:
			if(p1 != 0x09 && p2 != 0x0F) {p1 = p1 >> 1; p2 = p2 >> 1;}
			break;
		case right:
			if(p1 != 0x90 && p2 != 0xF0) {p1 = p1 << 1; p2 = p2 >> 1;}
			break;
		case down: 
			if(r1 != 0x71 && r2 != 0x7D) {
				r1 = (r1 << 1) | 0x01;
				r2 = (r2 << 1) | 0x01;
			}
				break;
		case up:
			if(r1 != 0xF0 && r2 != 0xF6) {
				r1 = (r1 >> 1) | 0x80;
				r2 = (r2 >> 1) | 0x80;
			}
				break;
		default: break;
	}
	return state;

}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
    /* Insert your solution below */
    	unsigned char i = 0;
	tasks[i].state = square;
	tasks[i].period = tasksPeriod;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Square_Tick;
	++i;
	tasks[i].state = check;
        tasks[i].period = tasksPeriod;
        tasks[i].elapsedTime = tasks[i].period;
        tasks[i].TickFct = &Button_Tick;

	TimerSet(tasksPeriod);
	TimerOn();
	while (1) {}
    	return 1;
}
