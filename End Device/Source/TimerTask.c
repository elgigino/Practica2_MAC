/*
 * TimerTask.c
 *
 *  Created on: Sep 19, 2016
 *      Author: Rvega
 */

#include "TimerTask.h"
#include "LED.h"
#include "Sound.h"
#include "NVM_Interface.h"

tmrTimerID_t myTimerID;
tsTaskID_t TimerTaskID;
uint8_t LED_Value = 0;

extern Timer_Flag;

void TimerTaskInit(void)
{	
	/* Allocate in memory the timer*/
	myTimerID = TMR_AllocateTimer();
	/* Create the New Task */
	TimerTaskID = TS_CreateTask(TimerTaskPriority, TimerTaskHandler);
	/* Send an Event to myNewTask with the first event */
	TS_SendEvent(TimerTaskID, TimerTaskEvent1);
}

void TimerTaskHandler(event_t events){

	switch(events){
		case TimerTaskEvent1:
				TMR_StartTimer( myTimerID, /* Timer ID allocated in "MyNewTaskInit()", called in MApp_init.c during initialization*/
				gTmrIntervalTimer_c, /* Type of timer: INTERVAL */
				3000, /* Timer's Timeout */
				TimerTaskCallback /* pointer to myTaskTimerCallback function */
				);
				TurnOffLeds(); /* Ensure all LEDs are turned off */
		break;
		case TimerTaskEvent2: 
			LED_TurnOffAllLeds();
			Timer_Flag = TRUE;
			if(LED_Value == 1) LED_TurnOnLed(LED1);
			if(LED_Value == 2) LED_TurnOnLed(LED2);
			if(LED_Value == 3) LED_TurnOnLed(LED3);
			if(LED_Value == 4) LED_TurnOnLed(LED4);
			//(void)App_TransmitCommData();	
			LED_Value ++; 
			if(LED_Value == 5) LED_Value = 1; 
		
		break; 
	}
}

void TimerTaskCallback(tmrTimerID_t timer){
	(void)timer; // this line is just to clear a warn
	TS_SendEvent(myTimerID, TimerTaskEvent2);
}



