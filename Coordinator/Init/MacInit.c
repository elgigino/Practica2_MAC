/******************************************************************************
*  This file is to Initialize the entire MAC.
*
* (c) Copyright 2007, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
******************************************************************************/

#include "EmbeddedTypes.h"
#include "MacInit.h"


/******************************************************************************
*******************************************************************************
* Private macros
*******************************************************************************
******************************************************************************/
/* Number of elements in an array. */
#define NumberOfElements(array)   ((sizeof(array) / (sizeof(array[0]))))

/******************************************************************************
*******************************************************************************
* Private prototypes
*******************************************************************************
******************************************************************************/

void MacTaskInit(void);
void TS_PlaceHolderTaskInit(void);

/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/

typedef struct macTaskEntry_tag {
  tsTaskID_t *pTaskID;
  void (*pInitFunction)(void);
  void (*pEventHandler)(event_t);
  tsTaskPriority_t priority;
} macTaskEntry_t;

/******************************************************************************
*******************************************************************************
* Private memory declarations
*******************************************************************************
******************************************************************************/

#define Task(taskIdGlobal, taskInitFunc, taskMainFunc, priority) \
  { &taskIdGlobal, taskInitFunc, taskMainFunc, priority },
  
macTaskEntry_t const MacTaskTable[] = {
#include "MacTaskTbl.h"
};

/******************************************************************************
*******************************************************************************
* Public memory declarations
*******************************************************************************
******************************************************************************/

tsTaskID_t gMacTaskID_c;


/******************************************************************************
*******************************************************************************
* Public functions
*******************************************************************************
******************************************************************************/

void MacInit(void) {
  MacTaskInit();
}

/******************************************************************************
*******************************************************************************
* Private functions
*******************************************************************************
******************************************************************************/

/* This function can be used in the table in MacTaskTbl.h for any task */
/* that does not have it's own init function. */
void TS_PlaceHolderTaskInit(void) {
}

/*****************************************************************************/

/* Init the MAC task. Add all of them to the kernel's task table, and */
/* call of the init functions. */
void MacTaskInit(void) {
  index_t i;

  /* Add the tasks to the kernel's task table first. That way, if any of the */
  /* init functions want to set events, all of the task ids will be defined. */
  for (i = 0; i < NumberOfElements(MacTaskTable); ++i) {
    *MacTaskTable[i].pTaskID = TS_CreateTask(MacTaskTable[i].priority,
                                             MacTaskTable[i].pEventHandler);
  }

  for (i = 0; i < NumberOfElements(MacTaskTable); ++i) {
    (*MacTaskTable[i].pInitFunction)();
  }
}                                       /* MacTaskInit() */
