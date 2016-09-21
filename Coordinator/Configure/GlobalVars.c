/************************************************************************************
* This module implements the memory allocation, list, and message modules. The memory 
* allocation is build around N (1-4) pools with various memory allocation unit (block)
* sizes. Each pool consists of an anchor with head and tail pointers. The memory blocks
* are all linked to the anchor using a single chained list. Thus each block has a next
* pointer. The user of the functions in this module never has to be concerned with the
* list overhead since this is handled transparently. The block pointer which the user
* receives when allocating memory is pointing to the address after the next-pointer.
*
* FIFO Queues are implemented using the same list functions as used by the memory
* (de)allocation functions. The queue data object is simply an anchor (anchor_t).
* List_AddTail is used for putting allocated blocks on the queue, and List_RemoveHead
* will detach the block from the queue. Before using a queue anchor it must have been
* initialized with List_ClearAnchor. No extra header is required in order to put a
* block in a queue. However, messages should contain type information beside the
* message data so that the message handler at the receiver can reckognize the message.
*
* Messages are sent by allocating a block using MSG_Alloc, and 
* call the Service Access Point (SAP) of the receiver. If the SAP handles specific
* messages synchronously (returns with result immideately) then the block may be
* allocated on the stack of the calling function. The message types which allows this
* are specified in the design documents.
*
* Copyright (c) 2006, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*************************************************************************************/
#include "MsgSystem.h"

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

  // The heap for MAC, NWK and application memory blocks.
uint8_t maMacHeap[mMmTotalPoolSize_c];

  // Memory pool info and anchors.
pools_t maMmPools[gMmNumPools_c];

  // Const array used during initialization. Describes the memory layout.
  // Pools must occur in the table in ascending order according to their size.
  // If coordinator capability is required, then there must always exist an
  // extra MAC pool for receiving data frames. This pool is private to the MAC.
  // An example of a memory layout for coordinators/routers:
  // Pool3]:
  //   SmallMessages[M], // Command messages of each ~22 bytes
  //   BigMessages[N-1], // Data messages of each ~134 bytes
  //   BigMessages[1],   // One data message of ~134 bytes, MAC private.
  // MSG_Alloc ensures that only the MAC has access to the private pool.

  // An example of a memory layout for devices:
  // Pool2]:
  //   SmallMessages[M], // Command messages of each ~22 bytes
  //   BigMessages[N],   // Data messages of each ~134 bytes
  //
const poolInfo_t poolInfo[gMmNumPools_c] = {
  gMmPoolSize0_c, mMmBlockSize0_c, mMmBlockSize1_c,
#if gMmNumPools_c > 1
  gMmPoolSize1_c, mMmBlockSize1_c, mMmBlockSize2_c,
#if gMmNumPools_c > 2
  gMmPoolSize2_c, mMmBlockSize2_c, 0, 
#if gMmNumPools_c > 3
  gMmPoolSize3_c, mMmBlockSize3_c, 0
#endif // gMmNumPools_c > 3
#endif // gMmNumPools_c > 2
#endif // gMmNumPools_c > 1
};

const pools_t *pLastPool = &maMmPools[mMmLastPoolIdx_c];
#if gBigMsgsMacOnly_d
  const pools_t *pSecondLastPool = &maMmPools[mMmLastPoolIdx_c-1];
#endif

#ifndef gMAC2006_d
  // Application allocated space for MAC PIB ACL Entry descriptors.
  #if gNumAclEntryDescriptors_c > 0
    aclEntryDescriptor_t gPIBaclEntryDescriptorSet[gNumAclEntryDescriptors_c];

     // Set number of ACL entries. Used by the MAC.
    const uint8_t gNumAclEntryDescriptors = gNumAclEntryDescriptors_c;
  #endif // gNumAclEntryDescriptors_c
#else
  
  #if gNumKeyTableEntries_c > 0
    KeyDescriptor_t gPIBKeyTable[gNumKeyTableEntries_c];
    const uint8_t gNumKeyTableEntries = gNumKeyTableEntries_c;
    
    /* Allocate KeyIdLookupDescriptor_t, KeyDeviceDescriptor_t, KeyUsageDescriptor_t */ 
    /* These arrays are part of KeyDescriptor_t structure */
    /* Allocate a continuous space for each array based on the gNumKeyTableEntries_c */
    /* The MAC PIB will initialize the pointers accordingly */
    
    #if gNumKeyIdLookupEntries_c > 0
       KeyIdLookupDescriptor_t gPIBKeyIdLookupDescriptorTable[gNumKeyIdLookupEntries_c * gNumKeyTableEntries_c];
       const uint8_t gNumKeyIdLookupEntries = gNumKeyIdLookupEntries_c; /* The number of elements in each virtual array inside the gPIBKeyIdLookupDescriptorTable */
    #endif //gNumKeyIdLookupEntries_c
    
    #if gNumKeyDeviceListEntries_c > 0
       KeyDeviceDescriptor_t  gPIBKeyDeviceDescriptorTable[gNumKeyDeviceListEntries_c * gNumKeyTableEntries_c];
       const uint8_t gNumKeyDeviceListEntries = gNumKeyDeviceListEntries_c; /* The number of elements for each virtual array inside the gPIBKeyDeviceDescriptorTable */
    #endif //gNumKeyDeviceListEntries_c
    
    #if gNumKeyUsageListEntries_c > 0
       KeyUsageDescriptor_t  gPIBKeyUsageDescriptorTable[gNumKeyUsageListEntries_c * gNumKeyTableEntries_c];
       const uint8_t gNumKeyUsageListEntries = gNumKeyUsageListEntries_c; /* The number of elements for each virtual array inside the gPIBKeyDeviceDescriptorTable */
    #endif //gNumKeyUsageListEntries_c
  #endif //gNumKeyTableEntries_c
  
  #if gNumDeviceTableEntries_c > 0
    DeviceDescriptor_t gPIBDeviceTable[gNumDeviceTableEntries_c];
    const uint8_t gNumDeviceTableEntries = gNumDeviceTableEntries_c;    
  #endif //gNumDeviceTableEntries_c

  #if gNumSecurityLevelTableEntries_c > 0
    SecurityLevelDescriptor_t gPIBSecurityLevelTable[gNumSecurityLevelTableEntries_c];
    const uint8_t gNumSecurityLevelTableEntries = gNumSecurityLevelTableEntries_c;    
  #endif //gNumKeyTableEntries_d


#endif  //gMAC2006_d

#if MsgTracking_d
const uint8_t TotalNoOfMsgs_c = (gTotalBigMsgs_d + gTotalSmallMsgs_d);

MsgTracking_t MsgTrackingArray[(gTotalBigMsgs_d + gTotalSmallMsgs_d)] = 0;
uint8_t NoOfWrongAddrs = 0;
#endif

#ifdef PROCESSOR_QE128
 typedef uint32_t AppReturnAddr_t;
#else
 typedef uint16_t AppReturnAddr_t; 
#endif 
/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/


/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/
/************************************************************************************
* Initialize Msg tracking array. It is called by the Msg system when the MAC is reset
*
* Interface assumptions:
*   None
*   
* Return value:
*   None.
* 
************************************************************************************/

void MM_InitMsgTracking(void) {
#if MsgTracking_d
   FLib_MemSet((uint8_t* )&MsgTrackingArray[0],0,(sizeof(MsgTracking_t)/sizeof(uint8_t)) * TotalNoOfMsgs_c);
   NoOfWrongAddrs = 0;
#endif   
}
/************************************************************************************
* Initialize Msg tracking array a current index with a Msg address.
* This functions is called for once for every Msg when the MAC is reset.
*
* Interface assumptions:
*   None
*   
* Return value:
*   None.
* 
************************************************************************************/
void   MM_AddMsgToTrackingArray(uint8_t Index, uint16_t Addr) {
#if MsgTracking_d
      MsgTrackingArray[Index].MsgAddr = Addr;  
#else
  /*To prevent compiler warinngs:*/
  (void) Index;
  (void) Addr;  
#endif      
}
/************************************************************************************
* This function finds the index of a Msg in the Tracking array
* -1 is returned if Msg not found and the NoOfWrongAddrs is incremented.
*
* Interface assumptions:
*   None
*   
* Return value:
*   Index of message in tracking array
* 
************************************************************************************/  
int8_t MM_GetMsgIndex(const uint16_t MsgAddr)
{
#if MsgTracking_d
  uint8_t i;    
  /*Search through the Tracking array*/ 
  for (i = 0; i < TotalNoOfMsgs_c; i++) 
  {      
    /*If Msg Addr found then exit with Index*/
    if (MsgAddr == MsgTrackingArray[i].MsgAddr)
      return i;
  }
  /*Msg addr not found, increment error counter and return*/  
  NoOfWrongAddrs++;
  return -1;
#else  
  /*To prevent compiler warinngs:*/
 (void) MsgAddr;
 return -1;
#endif 
}

/************************************************************************************
* This function is called when ever a Msg is freed or allocated and updates
* the tracking information for that particular Msg
* 
*
* Interface assumptions:
*   None
*   
* Return value:
*   Success status
* 
************************************************************************************/  
bool_t MM_UpdateMsgTracking(const void *pBlock, const bool_t Alloc, const uint8_t offset)
{
#if MsgTracking_d
  uint16_t StackPointer = 0;
  int8_t index = MM_GetMsgIndex((uint16_t)pBlock);
    
  if (index >= 0)
  {          
    MsgTrackingArray[index].AllocStatus = Alloc;
    
    /*Get stack pointer so we can find return addr of orginal free/alloc caller*/
    _asm
    {
      tsx        
      sthx StackPointer
    }
    /*Update MsgTracking array counters and return address of alloc or free */  
    if (Alloc) {
      MsgTrackingArray[index].AllocCounter++;
      MsgTrackingArray[index].AllocAddr = (AppReturnAddr_t) (*((volatile uint16_t*) (StackPointer - 1 + offset)));
      
      #ifdef PROCESSOR_QE128      
         /* Copy the PPAGE if the calling function has a paged address */
         *((uint8_t*)&(MsgTrackingArray[index].AllocAddr) + 1) = *(uint8_t*)(StackPointer - 2 + offset);
      #endif 
    }
    else {
      MsgTrackingArray[index].FreeCounter++;        
      MsgTrackingArray[index].FreeAddr = (AppReturnAddr_t) (*((volatile uint16_t*) (StackPointer - 1 + offset)));      
     
      #ifdef PROCESSOR_QE128      
         /* Copy the PPAGE if the calling function has a paged address */
         *((uint8_t*)&(MsgTrackingArray[index].FreeAddr) + 1) = *(uint8_t*)(StackPointer - 2 + offset);
      #endif 
    }
    return TRUE;
      
  }      
  else  
    return FALSE;
#else
  /*To prevent compiler warinngs:*/
  (void) pBlock;
  (void) Alloc;
  (void) offset;
  return FALSE;
#endif 
}

