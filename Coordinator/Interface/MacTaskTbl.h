/******************************************************************************
* MacTaskTbl.h
*
* Copyright (c) 2007, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from  Freescale Semiconductor.
*
******************************************************************************/

/* Table describing MAC internal tasks. Application tasks are not
 * considered here. The idle task is required by the kernel and the power
 * management code, and is entered into the kernel's task table by the
 * kernel's initialization procedure.
 */

/* The format of the Task() macro is:
 *  Column 1    taskIdGlobal: the name of a global integer that contains the
 *                ID of this task. These names are used by libraries, such
 *                as the MAC layer, that are generally only provided in
 *                binary form. The names used here must be compatible with
 *                those libraries. Note that these are global integers, not
 *                macros or enum contanst, despite the *_c names.
 *  Column 2    taskInitFunc: Name of a function that will be called to
 *                initialize the task.
 *  Column 3    taskMainFunc: Name of the main function for this task.
 *  Column 4    priority: This task's priority. Must be >= 1 and <= 0xFE.
 *
 * The absolute values of the task priority numbers are not important; only
 * the relative order is significant. Tasks with higher priority numbers
 * run first.
 *
 * Used to initialize beeTaskTable[], and to declare the task ID globals.
 *
 * Note that ZTC, the timer task, and the Idle Task are not initialized via
 * this table.
 */

#ifndef Task
#define Task( taskIdGlobal, taskInitFunc, taskMainFunc, priority )
#endif

/* The order of the entries here is significant. The task initialization */
/* functions will be called in the order of the Task() macros. */

/* The MAC library does not have a task init function. */
Task( gMacTaskID_c,                 TS_PlaceHolderTaskInit,     Mlme_Main,              gTsMlmeTaskPriority_c )

#undef  Task
