/*****************************************************************************
* MyWirelessApp Demo Non Beacon End Device application.
*
* (c) Copyright 2008, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

#include "MApp.h"
#include "Sound.h"
#include "NVM_Interface.h"
#include "TimerTask.h"

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

/* If there are too many pending packets to be send over the air, */
/* receive mMaxKeysToReceive_c chars. */
/* The chars will be send over the air when there are no pending packets*/
#define mMaxKeysToReceive_c 32

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/

/* Forward declarations of helper functions */
static void    App_CommRxCallBack(void);
static uint8_t App_StartScan(uint8_t scanType);
static uint8_t App_HandleScanActiveConfirm(nwkMessage_t *pMsg);
static uint8_t App_WaitMsg(nwkMessage_t *pMsg, uint8_t msgType);
static uint8_t App_SendAssociateRequest(void);
static uint8_t App_HandleAssociateConfirm(nwkMessage_t *pMsg);
static uint8_t App_HandleMlmeInput(nwkMessage_t *pMsg);
static void    App_HandleMcpsInput(mcpsToNwkMessage_t *pMsgIn);
static void    App_TransmitCommData(void);
static void    AppPollWaitTimeout(uint8_t tmr);
#if (gMC1323xMatrixKBD_d == TRUE)
static void    App_HandleKeys(uint8_t events, uint8_t pressedKey);
#else
static void    App_HandleKeys(key_event_t events);
#endif //gMC1323xMatrixKBD_d

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

/* Information about the PAN we are part of */
static panDescriptor_t mCoordInfo;

/* This is either the short address assigned by the PAN coordinator
   during association, or our own extended MAC address. */
static uint8_t maMyAddress[8];
/* The devices address mode. If 2, then maMyAddress contains the short
   address assigned by the PAN coordinator. If 3, then maMyAddress is
   equal to the extended address. */
static uint8_t mAddrMode;

/* Data request packet for sending serial terminal interface input to the coordinator */
static nwkToMcpsMessage_t *mpPacket;

/* The MSDU handle is a unique data packet identifier */
static uint8_t mMsduHandle;

/* Number of pending data packets */
static uint8_t mcPendingPackets;

/* Signals that an MLME-Poll request is pending, and that we must wait for 
   the MLME-Poll confirm message before sending the next poll request. */
static bool_t mWaitPollConfirm;

/* Time between MLME-Poll requests */
static uint16_t mPollInterval;

/* Application input queues */
static anchor_t mMlmeNwkInputQueue;
static anchor_t mMcpsNwkInputQueue;

static tmrTimerID_t mTimer_c = gTmrInvalidTimerID_c;

static bool_t Comm_Flag = FALSE;
bool_t Timer_Flag = FALSE;
uint8_t Buffer[12] = "Counter: ";
extern tsTaskID_t TimerTaskID; 
/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/* The current state of the applications state machine */
uint8_t gState;

/* This data set contains application variables to be preserved across resets */
NvDataItemDescription_t const gaNvAppDataSet[] = {
  {NULL, 0}       /* Required end-of-table marker. */
};

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

void DeepSleepWakeupStackProc(void);

/*****************************************************************************
* Initialization function for the App Task. This is called during
* initialization and should contain any application specific initialization
* (ie. hardware initialization/setup, table initialization, power up
* notificaiton.
*
* Interface assumptions: None
*
* Return value: None
*
*****************************************************************************/
void MApp_init(void)
{ 
  
  /* The initial application state */
  gState = stateInit;  
  /* Reset number of pending packets */
  mcPendingPackets = 0;
  
  /* Allow sending a poll request */  
  mWaitPollConfirm = FALSE;    
  
  /* Initialize the poll interval */
  mPollInterval = mDefaultValueOfPollIntervalSlow_c;

  /* Initialize the MAC 802.15.4 extended address */
  Init_MacExtendedAddress();
  mTimer_c = TMR_AllocateTimer();
  /* register keyboard callback function */
  KBD_Init(App_HandleKeys);
  /* Initialize SPI Module */
  SPI_Init();
  /* initialize LCD Module */
  LCD_Init();
  /* initialize LED Module */
  LED_Init();
  /* Initialize the LPM module */
  PWRLib_Init();
  /* Initialize the serial terminal interface so that we can print out status messages */
  Comm_SetBaud(gUartDefaultBaud_c);
  Comm_SetRxCallBack(App_CommRxCallBack);
  
  /* initialize buzzer (NCB, SRB only) */  
  BuzzerInit();  
  /* Prepare input queues.*/
  MSG_InitQueue(&mMlmeNwkInputQueue); 
  MSG_InitQueue(&mMcpsNwkInputQueue);
  /* Enable MCU interrupts */
  IrqControlLib_EnableAllIrqs();
  /*signal app ready*/  
  Led1Flashing();
  Led2Flashing();
  Led3Flashing();
  Led4Flashing();
    
  CommUtil_Print("\n\rPress any switch on board to start running the application.\n\r", gAllowToBlock_d);  
  LCD_ClearDisplay();
  LCD_WriteString(1,"Press any key");
  LCD_WriteString(2,"to start.");
  
  TimerTaskInit();
#ifdef FFD
  BUtl_SetReceiverOn();
#endif
}

/*****************************************************************************
*Mac Application Task event processor.  This function is called to
* process all events for the task. Events include timers, messages and any
* other user defined events
*
* Interface assumptions: None
*
* Return value: None
*****************************************************************************/
void AppTask(event_t events) 
{ 

  /* Pointer for storing the messages from MLME, MCPS, and ASP. */
  void *pMsgIn;
  /* Stores the status code returned by some functions. */
  uint8_t rc;  
  pMsgIn = NULL;
  
  /* Dequeue the MLME message */
  if (events & gAppEvtMessageFromMLME_c)
  {
    /* Get the message from MLME */
    pMsgIn = MSG_DeQueue(&mMlmeNwkInputQueue);
    
    /* Any time a beacon might arrive. Always handle the beacon frame first */
    if (pMsgIn)
    {               
      rc = App_WaitMsg(pMsgIn, gNwkBeaconNotifyInd_c);
      if(rc == errorNoError)
      {
        /* ALWAYS free the beacon frame contained in the beacon notify indication.*/
        /* ALSO the application can use the beacon payload.*/
        MSG_Free(((nwkMessage_t *)pMsgIn)->msgData.beaconNotifyInd.pBufferRoot);
        CommUtil_Print("Received an MLME-Beacon Notify Indication\n\r", gAllowToBlock_d);
      }
    }
  }
  
  /* The application state machine */
  switch(gState)
  {
  case stateInit:    
    /* Print a welcome message to the serial terminal interface */
    CommUtil_Print("\n\rMyWirelessApp Demo Non Beacon End Device application is initialized and ready.\n\r\n\r", gAllowToBlock_d);            
    /* Goto Active Scan state. */
    gState = stateScanActiveStart;
    TS_SendEvent(gAppTaskID_c, gAppEvtDummyEvent_c);    
    break;
    
  case stateScanActiveStart:
    /* Start the Active scan, and goto wait for confirm state. */
    CommUtil_Print("Start scanning for a PAN coordinator\n\r", gAllowToBlock_d);
    /*print a message on the LCD also*/
    LCD_ClearDisplay();
    LCD_WriteString(1,"Start scanning");
    LCD_WriteString(2,"for coordinator");  
    rc = App_StartScan(gScanModeActive_c);
    if(rc == errorNoError)
    {
      gState = stateScanActiveWaitConfirm;
    }
    break;
    
  case stateScanActiveWaitConfirm:
    /* Stay in this state until the Scan confirm message
       arrives, and then goto the associate state. */
    if (events & gAppEvtMessageFromMLME_c)
    {
      if (pMsgIn)
      {                     
        rc = App_WaitMsg(pMsgIn, gNwkScanCnf_c);
        if(rc == errorNoError)
        {
          rc = App_HandleScanActiveConfirm(pMsgIn);
          if(rc == errorNoError)
          {
            CommUtil_Print("Found a coordinator with the following properties:\n\r", gAllowToBlock_d);
            CommUtil_Print("----------------------------------------------------", gAllowToBlock_d);
            CommUtil_Print("\n\rAddress...........0x", gAllowToBlock_d); CommUtil_PrintHex(mCoordInfo.coordAddress, mCoordInfo.coordAddrMode == gAddrModeShort_c ? 2 : 8, 0);
            CommUtil_Print("\n\rPAN ID............0x", gAllowToBlock_d); CommUtil_PrintHex(mCoordInfo.coordPanId, 2, 0);
            CommUtil_Print("\n\rLogical Channel...0x", gAllowToBlock_d); CommUtil_PrintHex(&mCoordInfo.logicalChannel, 1, 0);
            CommUtil_Print("\n\rBeacon Spec.......0x", gAllowToBlock_d); CommUtil_PrintHex(mCoordInfo.superFrameSpec, 2, 0);
            CommUtil_Print("\n\rLink Quality......0x", gAllowToBlock_d); CommUtil_PrintHex(&mCoordInfo.linkQuality, 1, 0);
            CommUtil_Print("\n\r\n\r", gAllowToBlock_d);

            gState = stateAssociate;
            TS_SendEvent(gAppTaskID_c, gAppEvtDummyEvent_c);
          }
          else
		      {
            CommUtil_Print("Scan did not find a suitable coordinator\n\r", gAllowToBlock_d);
            /*print a message on the LCD also*/
            LCD_ClearDisplay();
            LCD_WriteString(1,"No coordinator");
            LCD_WriteString(2,"found.");
		      }
        
		   }
      }
    }
    break;

  case stateAssociate:
    /* Associate to the PAN coordinator */
    CommUtil_Print("Associating to PAN coordinator on channel 0x", gAllowToBlock_d);
    CommUtil_PrintHex(&(mCoordInfo.logicalChannel), 1, gPrtHexNewLine_c);
    /*print a message on the LCD also*/
    LCD_ClearDisplay();
    LCD_WriteString(1,"Associating to ");
    LCD_WriteString(2,"PAN coordinator");  
    rc = App_SendAssociateRequest();
    if(rc == errorNoError)
      gState = stateAssociateWaitConfirm;
    break; 

  case stateAssociateWaitConfirm:
    /* Stay in this state until the Associate confirm message
       arrives, and then goto the Listen state. */
    if (events & gAppEvtMessageFromMLME_c)
    {
      if (pMsgIn)
      {   
        rc = App_WaitMsg(pMsgIn, gNwkAssociateCnf_c);
        if(rc == errorNoError)
        {          
          rc = App_HandleAssociateConfirm(pMsgIn);
          if (rc == errorNoError)
          { 
	          CommUtil_Print("Successfully associated with the coordinator.\n\r", gAllowToBlock_d);
	          CommUtil_Print("We were assigned the short address 0x", gAllowToBlock_d);
	          CommUtil_PrintHex(maMyAddress, mAddrMode == gAddrModeShort_c ? 2 : 8, 0);
	          CommUtil_Print("\n\r\n\rReady to send and receive data over the serial terminal interface.\n\r\n\r", gAllowToBlock_d);
	          /*print a message on the LCD also*/
	          LCD_ClearDisplay();
	          LCD_WriteString(1,"Ready to send");
	          LCD_WriteString(2,"and receive data");    
	          /* Startup the timer */
	          TMR_StartSingleShotTimer(mTimer_c, mPollInterval, AppPollWaitTimeout);
	          /* Go to the listen state */
	          gState = stateListen;
	          TS_SendEvent(gAppTaskID_c, gAppEvtDummyEvent_c); 
          }        
          else 
          {
          
	          CommUtil_Print("\n\rAssociate Confirm wasn't successful... \n\r\n\r", gAllowToBlock_d);
	          gState = stateScanActiveStart;
              TS_SendEvent(gAppTaskID_c, gAppEvtDummyEvent_c);
          }
        }
      }
    }
    break; 
    
  case stateListen:
    /* Transmit to coordinator data received from the serial terminal interface. */
    if (events & gAppEvtMessageFromMLME_c)
    {  
      if (pMsgIn)
      {  
        /* Process it */
        rc = App_HandleMlmeInput(pMsgIn);
      }
    } 
    
    if (events & gAppEvtRxFromComm_c)
    {      
      Comm_Flag = TRUE;
      /* get byte from serial terminal interface */
      App_TransmitCommData();
    }  

    
    break;
  }
  
  if(Timer_Flag){
  	App_TransmitCommData();
  	Timer_Flag = FALSE;
  }
  
  if (pMsgIn)
  {
    /* Messages must always be freed. */ 
    MSG_Free(pMsgIn);
  }
  
   /* Handle MCPS confirms and transmit data from serial terminal interface */
  if (events & gAppEvtMessageFromMCPS_c)
  {      
    /* Get the message from MCPS */
    pMsgIn = MSG_DeQueue(&mMcpsNwkInputQueue);
    if (pMsgIn)
    {              
      /* Process it */
      App_HandleMcpsInput(pMsgIn);
      /* Messages from the MCPS must always be freed. */
      MSG_Free(pMsgIn);
    }
  }
  
  /* Check for pending messages in the Queue */ 
  if(MSG_Pending(&mMcpsNwkInputQueue))
    TS_SendEvent(gAppTaskID_c, gAppEvtMessageFromMCPS_c);
  if(MSG_Pending(&mMlmeNwkInputQueue))
    TS_SendEvent(gAppTaskID_c, gAppEvtMessageFromMLME_c);  
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*****************************************************************************
* App_CommRxCallBack
*
* This callback is triggered when a new byte is received over the serial terminal interface
*
*****************************************************************************/
static void App_CommRxCallBack(void) 
{
  uint8_t pressedKey;
	if(stateListen == gState){
    TS_SendEvent(gAppTaskID_c, gAppEvtRxFromComm_c);
  }else{
	  (void)Comm_GetByteFromRxBuffer(&pressedKey);
  }
}

/******************************************************************************
* The App_StartScan(scanType) function will start the scan process of the
* specified type in the MAC. This is accomplished by allocating a MAC message,
* which is then assigned the desired scan parameters and sent to the MLME
* service access point.
* The function may return either of the following values:
*   errorNoError:          The Scan message was sent successfully.
*   errorInvalidParameter: The MLME service access point rejected the
*                          message due to an invalid parameter.
*   errorAllocFailed:      A message buffer could not be allocated.
*
******************************************************************************/
static uint8_t App_StartScan(uint8_t scanType)
{
  mlmeMessage_t *pMsg;
  mlmeScanReq_t *pScanReq;

  CommUtil_Print("Sending the MLME-Scan Request message to the MAC...", gAllowToBlock_d);

  /* Allocate a message for the MLME (We should check for NULL). */
  pMsg = MSG_AllocType(mlmeMessage_t);
  if(pMsg != NULL)
  {
    /* This is a MLME-SCAN.req command */
    pMsg->msgType = gMlmeScanReq_c;
    /* Create the Scan request message data. */
    pScanReq = &pMsg->msgData.scanReq;
    /* gScanModeED_c, gScanModeActive_c, gScanModePassive_c, or gScanModeOrphan_c */
    pScanReq->scanType = scanType;
    /* ChannelsToScan & 0xFF - LSB, always 0x00 */
    pScanReq->scanChannels[0] = (uint8_t)((mDefaultValueOfChannel_c)     & 0xFF);
    /* ChannelsToScan>>8 & 0xFF  */
    pScanReq->scanChannels[1] = (uint8_t)((mDefaultValueOfChannel_c>>8)  & 0xFF);
    /* ChannelsToScan>>16 & 0xFF  */
    pScanReq->scanChannels[2] = (uint8_t)((mDefaultValueOfChannel_c>>16) & 0xFF);
    /* ChannelsToScan>>24 & 0xFF - MSB */
    pScanReq->scanChannels[3] = (uint8_t)((mDefaultValueOfChannel_c>>24) & 0xFF);
    /* Duration per channel 0-14 (dc). T[sec] = (16*960*((2^dc)+1))/1000000.
       A scan duration of 3 on 16 channels approximately takes 2 secs. */
    pScanReq->scanDuration = 3;
#ifdef gMAC2006_d
    pScanReq->securityLevel = 0;
#endif //gMAC2006_d    
    
    /* Send the Scan request to the MLME. */
    if(MSG_Send(NWK_MLME, pMsg) == gSuccess_c)
    {
      CommUtil_Print("Done\n\r", gAllowToBlock_d);
      return errorNoError;
    }
    else
    {
      CommUtil_Print("Invalid parameter!\n\r", gAllowToBlock_d);
      return errorInvalidParameter;
    }
  }
  else
  {
    /* Allocation of a message buffer failed. */
    CommUtil_Print("Message allocation failed!\n\r", gAllowToBlock_d);
    return errorAllocFailed;
  }
}

/******************************************************************************
* The App_HandleScanActiveConfirm(nwkMessage_t *pMsg) function will handle the
* Active Scan confirm message received from the MLME when the Active scan has
* completed. The message contains a list of PAN descriptors. Based on link
* quality inforamtion in the pan descriptors the nearest coordinator is chosen.
* The corresponding pan descriptor is stored in the global variable mCoordInfo. 
*
* The function may return either of the following values:
*   errorNoError:       A suitable pan descriptor was found.
*   errorNoScanResults: No scan results were present in the confirm message.
*
******************************************************************************/
static uint8_t App_HandleScanActiveConfirm(nwkMessage_t *pMsg)
{
  void    *pBlock;
  uint8_t panDescListSize = pMsg->msgData.scanCnf.resultListSize;
  uint8_t rc = errorNoScanResults;
  uint8_t j;
  uint8_t bestLinkQuality = 0;  
  panDescriptorBlock_t *pDescBlock = pMsg->msgData.scanCnf.resList.pPanDescriptorBlocks;  
  panDescriptor_t *pPanDesc;    
  
 
  /* Check if the scan resulted in any coordinator responses. */  
  
  if (panDescListSize > 0)
  {    
    /* Check all PAN descriptors. */
    while (NULL != pDescBlock)
    {
      for (j = 0; j < pDescBlock->descriptorCount; j++)
      {            
        pPanDesc = &pDescBlock->descriptorList[j];

        /* Only attempt to associate if the coordinator
           accepts associations and is non-beacon. */
        if( ( pPanDesc->superFrameSpec[1] & gSuperFrameSpecMsbAssocPermit_c) && 
            ((pPanDesc->superFrameSpec[0] & gSuperFrameSpecLsbBO_c) == 0x0F) )
        {        
          
          /* Find the nearest coordinator using the link quality measure. */
          if(pPanDesc->linkQuality > bestLinkQuality)
          {
            /* Save the information of the coordinator candidate. If we
               find a better candiate, the information will be replaced. */
            FLib_MemCpy(&mCoordInfo, pPanDesc, sizeof(panDescriptor_t));
            bestLinkQuality = pPanDesc->linkQuality;
            rc = errorNoError;
          }
        }      
      }
      
      /* Free current block */
      pBlock = pDescBlock;
      pDescBlock = pDescBlock->pNext;              
      MSG_Free(pBlock);
    }
  }
  
  if (pDescBlock)
      MSG_Free(pDescBlock);
  
  return rc;
}

/******************************************************************************
* The App_SendAssociateRequest(void) will create an Associate Request message
* and send it to the coordinator it wishes to associate to. The function uses
* information gained about the coordinator during the scan procedure.
*
* The function may return either of the following values:
*   errorNoError:          The Associate Request message was sent successfully.
*   errorInvalidParameter: The MLME service access point rejected the
*                          message due to an invalid parameter.
*   errorAllocFailed:      A message buffer could not be allocated.
*
******************************************************************************/
static uint8_t App_SendAssociateRequest(void)
{
  mlmeMessage_t *pMsg;
  mlmeAssociateReq_t *pAssocReq;

  CommUtil_Print("Sending the MLME-Associate Request message to the MAC...", gAllowToBlock_d);
  
  /* Allocate a message for the MLME message. */
  pMsg = MSG_AllocType(mlmeMessage_t);
  if(pMsg != NULL)
  {
    /* This is a MLME-ASSOCIATE.req command. */
    pMsg->msgType = gMlmeAssociateReq_c;
    
    /* Create the Associate request message data. */
    pAssocReq = &pMsg->msgData.associateReq;
 
    /* Use the coordinator info we got from the Active Scan. */
    FLib_MemCpy(pAssocReq->coordAddress, mCoordInfo.coordAddress, 8);
    FLib_MemCpy(pAssocReq->coordPanId,   mCoordInfo.coordPanId, 2);
    pAssocReq->coordAddrMode      = mCoordInfo.coordAddrMode;
    pAssocReq->logicalChannel     = mCoordInfo.logicalChannel;
#ifndef gMAC2006_d    
    pAssocReq->securityEnable     = FALSE;
#else    
    pAssocReq->securityLevel = 0;
#endif //gMAC2006_d    
    /* We want the coordinator to assign a short address to us. */
#ifdef FFD
    pAssocReq->capabilityInfo     = gCapInfoAllocAddr_c | gCapInfoDeviceFfd_c | gCapInfoRxWhenIdle_c;
#else
    pAssocReq->capabilityInfo     = gCapInfoAllocAddr_c;
#endif
    /* Send the Associate Request to the MLME. */
    if(MSG_Send(NWK_MLME, pMsg) == gSuccess_c)
    {
      CommUtil_Print("Done\n\r", gAllowToBlock_d);
      return errorNoError;
    }
    else
    {
      /* One or more parameters in the message were invalid. */
      CommUtil_Print("Invalid parameter!\n\r", gAllowToBlock_d);
      return errorInvalidParameter;
    }
  }
  else
  {
    /* Allocation of a message buffer failed. */
    CommUtil_Print("Message allocation failed!\n\r", gAllowToBlock_d);
    return errorAllocFailed;
  }
}

/******************************************************************************
* The App_HandleAssociateConfirm(nwkMessage_t *pMsg) function will handle the
* Associate confirm message received from the MLME when the Association
* procedure has completed. The message contains the short address that the
* coordinator has assigned to us. This address is 0xfffe if we did not specify
* the gCapInfoAllocAddr_c flag in the capability info field of the Associate
* request. The address and address mode are saved in global variables. They
* will be used in the next demo application when sending data.
*
******************************************************************************/
static uint8_t App_HandleAssociateConfirm(nwkMessage_t *pMsg)
{
  /* This is our own extended address (MAC address). It cannot be modified. */
  extern uint8_t aExtendedAddress[8];
  
  /* If the coordinator assigns a short address of 0xfffe then,
     that means we must use our own extended address in all
     communications with the coordinator. Otherwise, we use
     the short address assigned to us. */
  if ( pMsg->msgData.associateCnf.status == gSuccess_c) 
  {

	  if( (pMsg->msgData.associateCnf.assocShortAddress[0] >= 0xFE) && 
	      (pMsg->msgData.associateCnf.assocShortAddress[1] == 0xFF) )
	  {
	    mAddrMode = gAddrModeLong_c;
	    FLib_MemCpy(maMyAddress, (void *)aExtendedAddress, 8);
	  }
	  else
	  {
	    mAddrMode = gAddrModeShort_c;
	    FLib_MemCpy(maMyAddress, pMsg->msgData.associateCnf.assocShortAddress, 2);
	  }
	    return gSuccess_c;
  } 
  
  else 
  {
	return pMsg->msgData.associateCnf.status; 
  }
}
/******************************************************************************
* The App_HandleMlmeInput(nwkMessage_t *pMsg) function will handle various
* messages from the MLME, e.g. poll confirm.
*
* The function may return either of the following values:
*   errorNoError:   The message was processed.
*   errorNoMessage: The message pointer is NULL.
******************************************************************************/
static uint8_t App_HandleMlmeInput(nwkMessage_t *pMsg)
{
  if(pMsg == NULL)
    return errorNoMessage;
  
  /* Handle the incoming message. The type determines the sort of processing.*/
  switch(pMsg->msgType) {
  case gNwkPollCnf_c:
    if(pMsg->msgData.pollCnf.status != gSuccess_c)
    {
      /* The Poll Confirm status was not successful. Usually this happens if
         no data was available at the coordinator. In this case we start
         polling at a lower rate to conserve power. */
      mPollInterval = mDefaultValueOfPollIntervalSlow_c;
      
      /* If we get to this point, then no data was available, and we
         allow a new poll request. Otherwise, we wait for the data
         indication before allowing the next poll request. */
      mWaitPollConfirm = FALSE;
    }
    break;
  }
  return errorNoError;
}

/******************************************************************************
* The App_HandleMcpsInput(mcpsToNwkMessage_t *pMsgIn) function will handle 
* messages from the MCPS, e.g. Data Confirm, and Data Indication.
*
******************************************************************************/
static void App_HandleMcpsInput(mcpsToNwkMessage_t *pMsgIn)
{
  switch(pMsgIn->msgType)
  {
    /* The MCPS-Data confirm is sent by the MAC to the network 
       or application layer when data has been sent. */
  case gMcpsDataCnf_c:
    if(mcPendingPackets)
      mcPendingPackets--;
    break;

  case gMcpsDataInd_c:
    /* Copy the received data to the serial terminal interface. */
    CommUtil_Tx(pMsgIn->msgData.dataInd.pMsdu, pMsgIn->msgData.dataInd.msduLength);
    /* Since we received data, the coordinator might have more to send. We 
       reduce the polling interval to raise the throughput while data is
       available. */
    mPollInterval = mDefaultValueOfPollIntervalFast_c;
    /* Allow another MLME-Poll request. */
    mWaitPollConfirm = FALSE;
    break;
  }
}

/******************************************************************************
* The App_WaitMsg(nwkMessage_t *pMsg, uint8_t msgType) function does not, as
* the name implies, wait for a message, thus blocking the execution of the
* state machine. Instead the function analyzes the supplied message to 
* determine whether or not the message is of the expected type.
* The function may return either of the following values:
*   errorNoError: The message was of the expected type.
*   errorNoMessage: The message pointer is NULL.
*   errorWrongConfirm: The message is not of the expected type.
*
******************************************************************************/
static uint8_t App_WaitMsg(nwkMessage_t *pMsg, uint8_t msgType)
{
  /* Do we have a message? If not, the exit with error code */
  if(pMsg == NULL)
    return errorNoMessage;

  /* Is it the expected message type? If not then exit with error code */
  if(pMsg->msgType != msgType)
    return errorWrongConfirm;

  /* Found the expected message. Return with success code */
  return errorNoError;
}

/******************************************************************************
* The App_TransmitCommData() function will perform (single/multi buffered)
* data transmissions of data received by the serial terminal interface. Data could also come from
* other sources such as sensors etc. This is completely determined by the
* application. The constant mDefaultValueOfMaxPendingDataPackets_c determine the maximum
* number of packets pending for transmission in the MAC. A global variable
* is incremented each time a data packet is sent to the MCPS, and decremented
* when the corresponding MCPS-Data Confirm message is received. If the counter
* reaches the defined maximum no more data buffers are allocated until the
* counter is decreased below the maximum number of pending packets.
*
* The function uses the coordinator information gained during the Active Scan,
* and the short address assigned to us by coordinator, for building an MCPS-
* Data Request message. The message is sent to the MCPS service access point
* in the MAC.
******************************************************************************/
static void App_TransmitCommData(void)
{   
  static uint8_t keysBuffer[mMaxKeysToReceive_c];
  static uint8_t keysReceived = 0;
  
  
  /* get data from serial terminal interface */
  if( keysReceived < mMaxKeysToReceive_c ) 
  {
    if(Comm_GetByteFromRxBuffer(&keysBuffer[keysReceived])) 
    {
    keysReceived++;
    }
  }
	/* Use multi buffering for increased TX performance. It does not really
     have any effect at a UART baud rate of 19200bps but serves as an
     example of how the throughput may be improved in a real-world 
     application where the data rate is of concern. */
  if( (mcPendingPackets < mDefaultValueOfMaxPendingDataPackets_c) && (mpPacket == NULL) ) 
  {
    /* If the maximum number of pending data buffes is below maximum limit 
       and we do not have a data buffer already then allocate one. */
    mpPacket = MSG_Alloc(gMaxRxTxDataLength_c);
  }

  if(mpPacket != NULL)
  {
    /* get data from serial terminal interface */        
      mpPacket->msgData.dataReq.pMsdu = (uint8_t*)(&(mpPacket->msgData.dataReq.pMsdu)) + sizeof(uint8_t*);
      if(Comm_Flag){
    	  FLib_MemCpy(mpPacket->msgData.dataReq.pMsdu, (uint8_t*) keysBuffer, keysReceived);
    	  mpPacket->msgData.dataReq.msduLength = keysReceived;
    	  Comm_Flag = FALSE;
      }else{
    	  FLib_MemCpy(mpPacket->msgData.dataReq.pMsdu, (uint8_t*) Buffer, 12);
    	  mpPacket->msgData.dataReq.msduLength = 12;
      }
      /* Data was available in the serial terminal interface receive buffer. Now create an
         MCPS-Data Request message containing the serial terminal interface data. */
      mpPacket->msgType = gMcpsDataReq_c;
      /* Create the header using coordinator information gained during 
         the scan procedure. Also use the short address we were assigned
         by the coordinator during association. */
      FLib_MemCpy(mpPacket->msgData.dataReq.dstAddr, mCoordInfo.coordAddress, 8);
      FLib_MemCpy(mpPacket->msgData.dataReq.srcAddr, maMyAddress, 8);
      FLib_MemCpy(mpPacket->msgData.dataReq.dstPanId, mCoordInfo.coordPanId, 2);
      FLib_MemCpy(mpPacket->msgData.dataReq.srcPanId, mCoordInfo.coordPanId, 2);
      mpPacket->msgData.dataReq.dstAddrMode = mCoordInfo.coordAddrMode;
      mpPacket->msgData.dataReq.srcAddrMode = mAddrMode;
      
      /* Request MAC level acknowledgement of the data packet */
      mpPacket->msgData.dataReq.txOptions = gTxOptsAck_c;
      /* Give the data packet a handle. The handle is
         returned in the MCPS-Data Confirm message. */
      mpPacket->msgData.dataReq.msduHandle = mMsduHandle++;
#ifdef gMAC2006_d
      mpPacket->msgData.dataReq.securityLevel = 0;
#endif //gMAC2006_d      
      
      /* Send the Data Request to the MCPS */
      (void)MSG_Send(NWK_MCPS, mpPacket);
      /* Prepare for another data buffer */
      mpPacket = NULL;
      mcPendingPackets++;
      /* Receive another pressed keys */
      keysReceived = 0;
  }
      
  /* If the keysBuffer[] wasn't send over the air because there are too many pending packets, */
  /* try to send it later   */
  if (keysReceived)
  {
  TS_SendEvent(gAppTaskID_c, gAppEvtRxFromComm_c);
  }

}

/******************************************************************************
* The AppPollWaitTimeout() function will check if it is time to send out an
* MLME-Poll request in order to receive data from the coordinator. If its time,
* and we are permitted then a poll request is created and sent.
* 
* The function uses the coordinator information gained during the Active Scan
* for building the MLME-Poll Request message. The message is sent to the MLME
* service access point in the MAC.
******************************************************************************/
static void    AppPollWaitTimeout(uint8_t tmr)
{ 
  
  /* Just to avoid the compiler warning */
  tmr++;
  
  /* Check if we are permitted, and if it is time to send a poll request.
     The poll interval is adjusted dynamically to the current band-width
     requirements. */
  if(mWaitPollConfirm == FALSE)
  {
    /* This is an MLME-POLL.req command. */
    mlmeMessage_t *pMlmeMsg = MSG_AllocType(mlmeMessage_t);
    if(pMlmeMsg)
    {
      /* Create the Poll Request message data. */
      pMlmeMsg->msgType = gMlmePollReq_c;
 
      /* Use the coordinator information we got from the Active Scan. */
      FLib_MemCpy(pMlmeMsg->msgData.pollReq.coordAddress, mCoordInfo.coordAddress, 8);
      FLib_MemCpy(pMlmeMsg->msgData.pollReq.coordPanId, mCoordInfo.coordPanId, 2);
      pMlmeMsg->msgData.pollReq.coordAddrMode = mCoordInfo.coordAddrMode;
#ifndef gMAC2006_d      
      pMlmeMsg->msgData.pollReq.securityEnable = FALSE;
#else
      pMlmeMsg->msgData.pollReq.securityLevel = 0;
#endif //gMAC2006_d      
      
      /* Send the Poll Request to the MLME. */
      if(MSG_Send(NWK_MLME, pMlmeMsg) == gSuccess_c)
      {
        /* Do not allow another Poll request before the confirm is received. */
        mWaitPollConfirm = TRUE;

      }
    }
  }
 /* Restart timer. */
 TMR_StartSingleShotTimer(mTimer_c, mPollInterval, AppPollWaitTimeout);
}

/*****************************************************************************
* Function to handle a generic key press. Called for all keys.
*****************************************************************************/
static void App_HandleGenericKey(void)
{
  if(gState == stateInit)
  {
    StopLed1Flashing();
    StopLed2Flashing();
    StopLed3Flashing();
    StopLed4Flashing();
    Led1Off();
    Led2Off();
    Led3Off();
    Led4Off();
    LCD_ClearDisplay();
    LCD_WriteString(1,"Application");
    LCD_WriteString(2,"    started");     
    TS_SendEvent(gAppTaskID_c, gAppEvtDummyEvent_c);       
  }
  TS_SendEvent(TimerTaskID, TimerTaskEvent2);
 
}

/*****************************************************************************
* Handles all key events for this device.
* Interface assumptions: None
* Return value: None
*****************************************************************************/
#if (gMC1323xMatrixKBD_d == TRUE)
static void App_HandleKeys(key_event_t events, uint8_t pressedKey)   
{
  switch ( pressedKey ) 
	{ 
	  case gKBD_EventSW1_c:
		  LED_Value = 1;
		  break;
	  case gKBD_EventSW2_c:
		  LED_Value = 2;
		  break;
	  case gKBD_EventSW3_c:
		  LED_Value = 3;
		  break;
	  case gKBD_EventSW4_c:
		  LED_Value = 4;
		  break;
	  case gKBD_EventLongSW1_c:
	  case gKBD_EventLongSW2_c:
	  case gKBD_EventLongSW3_c:
	  case gKBD_EventLongSW4_c:
		break;	
	}
 (void)events;
 (void)pressedKey;
 App_HandleGenericKey();
}
#else
static void App_HandleKeys(key_event_t events)
{
  switch ( events ) 
    { 
      case gKBD_EventSW1_c:
    	  LED_Value = 1;
    	  break;
      case gKBD_EventSW2_c:
    	  LED_Value = 2;
    	  break;
      case gKBD_EventSW3_c:
    	  LED_Value = 3;
    	  break;
      case gKBD_EventSW4_c:
    	  LED_Value = 4;
    	  break;
      case gKBD_EventLongSW1_c:
      case gKBD_EventLongSW2_c:
      case gKBD_EventLongSW3_c:
      case gKBD_EventLongSW4_c:
	    break;	
    }    
  App_HandleGenericKey();
}
#endif //gMC1323xMatrixKBD_d
/*****************************************************************************
* The DeepSleepWakeupStackProc(void) function is called each time the 
* application exits the DeepSleep mode .
* 
* Return value:
*     None
*****************************************************************************/
void DeepSleepWakeupStackProc(void){
  return;
}

/******************************************************************************
* The following functions are called by the MAC to put messages into the
* Application's queue. They need to be defined even if they are not used
* in order to avoid linker errors.
******************************************************************************/

uint8_t MLME_NWK_SapHandler(nwkMessage_t * pMsg)
{
  /* Put the incoming MLME message in the applications input queue. */
  MSG_Queue(&mMlmeNwkInputQueue, pMsg);
  TS_SendEvent(gAppTaskID_c, gAppEvtMessageFromMLME_c);
  return gSuccess_c;
}

uint8_t MCPS_NWK_SapHandler(mcpsToNwkMessage_t *pMsg)
{
  /* Put the incoming MCPS message in the applications input queue. */
  MSG_Queue(&mMcpsNwkInputQueue, pMsg);
  TS_SendEvent(gAppTaskID_c, gAppEvtMessageFromMCPS_c);
  return gSuccess_c;
}

uint8_t ASP_APP_SapHandler(aspToAppMsg_t *pMsg)
{
  /* If the message is not handled anywhere it must be freed. */
  MSG_Free(pMsg);
  return gSuccess_c;
}

/******************************************************************************/
