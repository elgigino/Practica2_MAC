/*****************************************************************************
* Applications' initialization module
*
* (c) Copyright 2006, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor Romania A/S.
*
*****************************************************************************/

#include "EmbeddedTypes.h"
#include "Mapp_init.h"
#include "Mac_Globals.h"
#include "MacInit.h"
#include "NVM_Interface.h"
#include "Uart_Interface.h"
#ifdef PROCESSOR_MC13238C
  #include "CDC_Interface.h"
  #include "USB_Interface.h"
#endif
#if gZtcIncluded_d
#include "ZTCInterface.h"
#endif /* gZtcIncluded_d */

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/


/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/

void IdleTask(event_t event);
void AppTask(event_t events);
tsTaskID_t gAppTaskID_c;

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

extern uint8_t aExtendedAddress[8];
extern uint8_t gState;

#if gNvStorageIncluded_d
static bool_t receiverOn;
#endif
/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/
extern void MacPhyInit_WriteExtAddress(uint8_t *pExtendedAddress);

/*****************************************************************************
* main function
*
* Interface assumptions:
* 
* Return value:
* None
*****************************************************************************/

 void main(void)
 {
 #ifdef BOOTLOADER_ENABLED	
  NVM_FindNVRAM();
 #endif	
 
  /* Interrupts should already be off. This is probably redundant. */
  IrqControlLib_DisableAllIrqs();
  Platform_Init();    // Init platform
  Init_802_15_4();

  TS_Init();  
  TMR_Init();                          /* Init the kernel. */
  NvModuleInit();
  Uart_ModuleInit();
  #ifdef PROCESSOR_MC13238C
    USB_Init();
    (void)CDC_ModuleInit();
  #endif
  MacInit();
  #if gZtcIncluded_d
  Ztc_TaskInit();
  #endif /* gZtcIncluded_d */ 

  /*initialize the application*/
 
  gAppTaskID_c = TS_CreateTask(gTsAppTaskPriority_c, AppTask);
 
  MApp_init();
  /*do not allow the device to enter sleep mode*/
  PWR_DisallowDeviceToSleep();   
  /*All LED's are switched OFF*/
  Led1Off();
  Led2Off();
  Led3Off();
  Led4Off();
  /* if the board is mc13213 SRB or NCB turn on LED5 */
  #if (gTargetMC13213SRB_d == 1) || (gTargetMC13213NCB_d == 1) || ((gTarget_UserDefined_d == 1) && (!defined(PROCESSOR_MC1323X)) && (!defined(PROCESSOR_QE128)))
    Led5On();
  #endif
  
  /* Start the task scheduler. Does not return. */
  TS_Scheduler(); 
 
}


#if gNvStorageIncluded_d

void BUtl_SetReceiverOff(void)
{
 /* This function is called by the NVM component at the start of the NV operation */
 mlmeMessage_t msg;	  
 bool_t value; 
 /* get the receiver state */
 
 msg.msgType = gMlmeGetReq_c;
 msg.msgData.getReq.pibAttribute = gMPibRxOnWhenIdle_c;
 msg.msgData.getReq.pibAttributeValue = &value; 
 (void)MSG_Send(NWK_MLME, &msg);
 
 if(value)
 {
  /* Turn off the receiver during NVM operation */
  receiverOn = TRUE;
  
  value = FALSE;
  msg.msgType = gMlmeSetReq_c;
  msg.msgData.setReq.pibAttribute = gMPibRxOnWhenIdle_c;
  msg.msgData.setReq.pibAttributeValue = &value;
  (void)MSG_Send(NWK_MLME, &msg);
 }
 else
 {
  /* Receiver already turned off */
  receiverOn = FALSE;
 }
 
}

void BUtl_SetReceiverOn(void)
{
 /* This function is called by the NVM component at the end of the NV operation */
 mlmeMessage_t msg;	  
 bool_t value = TRUE;
 if(receiverOn == TRUE)
 {  
  /* Restore receiver state */
  msg.msgType = gMlmeSetReq_c;
  msg.msgData.setReq.pibAttribute = gMPibRxOnWhenIdle_c;
  msg.msgData.setReq.pibAttributeValue = &value;
  (void)MSG_Send(NWK_MLME, &msg);
 } 
}

#endif

/*****************************************************************************
* Entry point to the idle task
*
* Interface assumptions:
* Cannot use any call by which this task has to be suspended.This task should
* always be ready to run if there are no other tasks in the system
* 
* Return value:
* None
*****************************************************************************/

void IdleTask(event_t events)
{
  /* Just to avoid the compiler warning */
  (void)events;

 #if gNvStorageIncluded_d
  /* Process NV Storage save-on-idle and save-on-count requests. */
  NvIdle();
 #endif

    
  #if (gLpmIncluded_d == 1)
  /*Low power management*/
    if(PWR_CheckIfDeviceCanGoToSleep())    
      PWR_EnterLowPower();
  #endif /*gLpmIncluded_d*/         
    
}


/*****************************************************************************
* Initialize the MAC 64 bit address
*
* Interface assumptions:
* None
* 
* Return value:
* None
*****************************************************************************/
void Init_MacExtendedAddress( void ) 
{

  uint8_t tempExtAddress[8]= {mDefaultValueOfExtendedAddress_c};
  /*Check if Extended address is already generated(if the Extended Address 
  is All "0xFF", it means that no extended address is genereted);If not set
  it to the value chosen by application layer*/
  if((aExtendedAddress[0] & aExtendedAddress[1]
    & aExtendedAddress[2] & aExtendedAddress[3]
    & aExtendedAddress[4] & aExtendedAddress[5]
    & aExtendedAddress[6] & aExtendedAddress[7]) 
    == 0xFF) 
    {
    /*Set the Extended Address From the App*/
      MacPhyInit_WriteExtAddress(tempExtAddress);
    }
}

/******************************************************************************
* BeeAppAssert
*
* Something failed in the stack or app that can't recover. Stop here. Mostly
* used during debug phase of product. Could be used
******************************************************************************/
void BeeAppAssert
  (
  assertCode_t code /* IN: why the assert failed */
  )
{
  volatile bool_t waitForever = code; /* just set to anything non-zero */
  
  while (waitForever)
    { }
}

/******************************************************************************
* Swap2Bytes - needed for ZTC
*
* Swaps two bytes
******************************************************************************/
uint16_t Swap2Bytes(uint16_t iOldValue)
{
  uint16_t iValue;

  iValue = ((uint8_t *)&iOldValue)[1] << 8;
  iValue += ((uint8_t *)&iOldValue)[0];

  return iValue;
}

/******************************************************************************
* Swap4Bytes - needed for ZTC
*
* Swaps two bytes
******************************************************************************/
uint32_t Swap4Bytes(uint32_t iOldValue)
{
  uint32_t iValue;

  ((uint8_t*)&iValue)[0] = ((uint8_t *)&iOldValue)[3];
  ((uint8_t*)&iValue)[1] = ((uint8_t *)&iOldValue)[2];
  ((uint8_t*)&iValue)[2] = ((uint8_t *)&iOldValue)[1];
  ((uint8_t*)&iValue)[3] = ((uint8_t *)&iOldValue)[0];

  return iValue;
}
/*******************************************************************************/
