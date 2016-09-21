/*
 * MyNewTask.h
 *
 *  Created on: Sep 14, 2016
 *      Author: Rvega
 */

#ifndef MYNEWTASK_H_
#define MYNEWTASK_H_

#include "TMR_Interface.h" /* Defines the interface of the Timer driver. */
#include "TS_Interface.h" /* Defines the interface of the TaskScheduler. */
#include "Led.h"

/* Define the available Task's Events */
#define gMyNewTaskEvent1_c (1 << 0)
#define gMyNewTaskEvent2_c (1 << 1)
#define gMyNewTaskEvent3_c (1 << 2)

/* New task priority. */
#define gMyNewTaskPriority_c 0x81

/* Prototypes: If not static, can be used outside of this file*/
void MyNewTask(event_t);
void MyNewTaskInit(void);
void MyTaskTimer_Stop(void);
static void myTaskTimerCallback(tmrTimerID_t);



#endif /* MYNEWTASK_H_ */
