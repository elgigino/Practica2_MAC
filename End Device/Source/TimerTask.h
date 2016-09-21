/*
 * TimerTask.h
 *
 *  Created on: Sep 19, 2016
 *      Author: Rvega
 */

#ifndef TIMERTASK_H_
#define TIMERTASK_H_

#include "TMR_Interface.h" /* Defines the interface of the Timer driver. */
#include "TS_Interface.h" /* Defines the interface of the TaskScheduler. */
#include "Led.h"

/* Define the available Task's Events */
#define TimerTaskEvent1 (1 << 0)
#define TimerTaskEvent2 (1 << 1)
#define TimerTaskEvent3 (1 << 2)

/* New task priority. */
#define TimerTaskPriority 0x81

void TimerTaskHandler(event_t);
void TimerTaskInit(void);
static void TimerTaskCallback(tmrTimerID_t);


#endif /* TIMERTASK_H_ */
