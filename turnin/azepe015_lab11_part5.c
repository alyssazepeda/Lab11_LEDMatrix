/*	Author: Alyssa Zepeda
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #11  Exercise #5
 *	Exercise Description:https://youtu.be/tXP1FMEQkmE  
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
#define tasksSize 2
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
//Transmit Data
void transmit_data(unsigned char data, unsigned char reg) {
    int i;
    for (i = 0; i < 8 ; ++i) {
   	 // Sets SRCLR to 1 allowing data to be set
   	 // Also clears SRCLK in preparation of sending data
	 if(reg == 1) { PORTC = 0x08;}//C3
	 else {PORTC = 0x20;}//C5
   	 // set SER = next bit of data to be sent.
   	 PORTC |= ((data >> i) & 0x01);
   	 // set SRCLK = 1. Rising edge shifts next bit of data into the shift register
   	 PORTC |= 0x02;  //C1
    }
    // set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
    if(reg == 1) {PORTC |= 0x04;}//C2
    else {PORTC |= 0x10;}//C4
    // clears all lines in preparation of a new transmission
    PORTC = 0x00;
}
////////////////////////////////////////////////////////////
#define A0 ~PINA&0x01
#define A1 ~PINA&0x02
#define A2 ~PINA&0x04
#define A3 ~PINA&0x08
unsigned char p1 = 0x90;
unsigned char p2 = 0xF0;
unsigned char r1 = 0xF0;
unsigned char r2 = 0xF6;
unsigned char pattern[] ={0x00, 0x00}; // LED pattern
unsigned char row[] = {0x00, 0x00}; //Row(s) displaying pattern
enum Square_States {square};
int Square_Tick(int state) {
	// Local Variables
	static unsigned char j = 0;
	//static unsigned char pattern[] ={}; // LED pattern 
	//static unsigned char row[] = {}; //Row(s) displaying pattern		// 0: display pattern on row
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
	transmit_data(pattern[j], 1); // Pattern to display
	transmit_data(row[j], 2); // Row(s) displaying pattern
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
			if(p1 != 0x09 && p2 != 0x0F) {
				p1 = p1 >> 1; 
				p2 = p2 >> 1;
			}
			break;
		case right:
			if(p1 != 0x90 && p2 != 0xF0) {
				p1 = p1 << 1; 
				p2 = p2 << 1;
			}
			break;
		case down: 
			if(r1 != 0xE1 && r2 != 0xED) {
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
        tasks[i].period = 20;
        tasks[i].elapsedTime = tasks[i].period;
        tasks[i].TickFct = &Button_Tick;

	TimerSet(tasksPeriod);
	TimerOn();
	while (1) {}
    	return 1;
}
