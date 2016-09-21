/******************************************************************************
* This public header file provides structure definition for different table
* structure whose size can be configured by the application. 
*
* Copyright (c) 2007, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*******************************************************************************/

#ifndef _MAC_GLOBALS_H_
#define _MAC_GLOBALS_H_

#include "EmbeddedTypes.h"
#include "TS_Interface.h"
#include "TMR_Interface.h"


/******************************************************************************
*******************************************************************************
* Public macros
*******************************************************************************
******************************************************************************/

/*******************************************************************************
********************************************************************************
* Public prototypes
********************************************************************************
*******************************************************************************/


/****************************************************************************
*****************************************************************************
* Public type definitions
*****************************************************************************
****************************************************************************/

/****************************************************************************/

/* Task / kernel identifiers. */

/* Reserved task priority ranges.
 * Task priority   0 is reserved by the kernel for the idle task.
 * Task priority 255 is reserved by the kernel as an invalid priority value.
 *
 * Higher number tasks run first.
 */

/* Reserve some IDs for low priority MAC tasks. */
#define gTsFirstMacTaskPriorityLow_c        0x01

#if gZtcIncluded_d
#define gTsZTCTaskPriority_c                0x02
#endif

#define gTsUartTaskPriority_c               0x03

/* Reserved for application tasks. */
#define gTsFirstApplicationTaskPriority_c   0x40

/* Application main task. */
#define gTsAppTaskPriority_c                0x80

#define gTsMlmeTaskPriority_c               0xC6
#define gTsHighestTaskPriorityPlusOne_c     0xC7

#if gTsTimerTaskPriority_c < gTsHighestTaskPriorityPlusOne_c
#error The Timer Task must be the highest priority task.
#endif


/****************************************************************************
*****************************************************************************
* Public memory declarations
*****************************************************************************
****************************************************************************/


/****************************************************************************/
/* TaskIDs. The MAC library needs to know the IDs for some of the tasks, so
 * it can set events for them. But it is supplied as a binary library, so it
 * can't use #define or enum constants, and the TaskIDs might change. TaskIDs
 * are assigned dynamically by the kernel.
 * Instead, there is integer in RAM for each task that contains that task's
 * ID. This costs some RAM, but the MAC library only needs to know the names
 * of the integers, not the TaskIDs, at link time.
 */

#define Task( taskIdGlobal, taskInitFunc, taskMainFunc, priority ) \
  extern tsTaskID_t taskIdGlobal; \
  extern void taskInitFunc( void ); \
  extern void taskMainFunc( event_t );

#include "MacTaskTbl.h"

/****************************************************************************
*****************************************************************************
* Public functions
*****************************************************************************
****************************************************************************/

#endif /* _MAC_GLOBALS_H_ */
