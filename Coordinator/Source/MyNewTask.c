/*
 * MyNewTask.c
 *
 *  Created on: Sep 14, 2016
 *      Author: Rvega
 */

#include "MyNewTask.h"

/* Global Variable to store our TimerID */
tmrTimerID_t myTimerID;
tsTaskID_t myNewTaskID;
/* Local variable to store the current state of the LEDs */
static uint8_t ledsState = 0;
/* Timer Callback prototype */
void myTaskTimerCallback(tmrTimerID_t timer);

/* MyNewTask main function that will handle the events*/
void MyNewTask(event_t events)
{
	/* Depending on the received event */
	switch(events){
	case gMyNewTaskEvent1_c:
		TMR_StartTimer(myTimerID, /* Timer ID allocated in "MyNewTaskInit()",
called in MApp_init.c during initialization*/
				gTmrIntervalTimer_c, /* Type of timer: INTERVAL */
				1000, /* Timer's Timeout */
				myTaskTimerCallback /* pointer to myTaskTimerCallback function */
		);
		TurnOffLeds(); /* Ensure all LEDs are turned off */
		break;
	case gMyNewTaskEvent2_c: /* Event called from myTaskTimerCallback */
		if(!ledsState) {
			TurnOnLeds();
			ledsState = 1;
		}
		else {
			TurnOffLeds();
			ledsState = 0;
		}
		break;
	case gMyNewTaskEvent3_c: /* Event to stop the timer */
		ledsState = 0;
		TurnOffLeds();
		TMR_StopTimer(myTimerID);
		break;
	default:
		break;
	}
}

/* This function is called in MApp_init.c during initialization ( main() )
 * It will INITIALIZE all required components for the task to work and then
 * call an event for the new Task.*/
void MyNewTaskInit(void)
{
	/* Allocate in memory the timer*/
	myTimerID = TMR_AllocateTimer();
	ledsState = 0;
	/* Create the New Task */
	myNewTaskID = TS_CreateTask(gMyNewTaskPriority_c, MyNewTask);
	/* Send an Event to myNewTask with the first event */
	TS_SendEvent(myNewTaskID, gMyNewTaskEvent1_c);
}

/* This is the function called by the Timer each time it expires */
static void myTaskTimerCallback(tmrTimerID_t timer)
{
	(void)timer; // this line is just to clear a warning
	TS_SendEvent(myNewTaskID, gMyNewTaskEvent2_c);
}


/* Public function to send an event to stop the timer */
void MyTaskTimer_Stop(void)
{
	TS_SendEvent(myNewTaskID, gMyNewTaskEvent3_c);
}

