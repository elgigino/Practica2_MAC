/************************************************************************************
* This file defines how the MAC/PHY is configured by the Application. This includes
* setting up the type of device (RFD, FFD, etc) and the number of buffers available
* for the MAC/PHY.
*
*
* (c) Copyright 2005, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/

#ifndef _APPTOMACPHYCONFIG_H_
#define _APPTOMACPHYCONFIG_H_

//**********************************************************************************
//**********************************************************************************
// Select one device type per below. 
// If none selected default type is Type_FFD (all features). The type can be
// defined by the build environment (compiler settings).
//**********************************************************************************
//**********************************************************************************
//#define Type_RFDNBNS
//#define Type_RFDNBNV
//#define Type_RFDNB
//#define Type_RFD
//#define Type_FFDNBNS
//#define Type_FFDNBNV
//#define Type_FFDNB
//#define Type_FFDNGTS
//#define Type_FFD
//#define Type_FFDPNBNV

// New stack target specific compiler defines (no value) "Type_..."
// FFD types
//#define Type_FFDZSNGTSNV
//#define Type_FFDZSNGTSNVNS
//#define Type_FFDNGTSNVNS
//#define Type_FFDZSNBNV
//#define Type_FFDZSNBNVNS
//#define Type_FFDNBNVNS
// FFDP types
//#define Type_FFDPZSNGTSNV
//#define Type_FFDPZSNGTSNVNS
//#define Type_FFDPNGTSNVNS
//#define Type_FFDPZSNBNV
//#define Type_FFDPZSNBNVNS
//#define Type_FFDPNBNVNS
// RFD types
//#define Type_RFDZSNV
//#define Type_RFDZSNVNS
//#define Type_RFDNVNS
//#define Type_RFDZSNBNV
//#define Type_RFDZSNBNVNS
//#define Type_RFDNBNVNS

#if !defined(Type_RFDNBNS) && !defined(Type_RFDNBNV) && !defined(Type_RFDNB) && !defined(Type_RFD) && !defined(Type_FFDNBNS) && \
    !defined(Type_FFDNBNV) && !defined(Type_FFDNB) && !defined(Type_FFDNGTS) && !defined(Type_FFD) && !defined(Type_FFDPNBNV) && \
    !defined(Type_FFDZSNGTSNV) && !defined(Type_FFDZSNGTSNVNS) && !defined(Type_FFDNGTSNVNS) && \
    !defined(Type_FFDZSNBNV) && !defined(Type_FFDZSNBNVNS) && !defined(Type_FFDZSNBNVNSCE) && !defined(Type_FFDZSNBNVNSCE_PROM) && !defined(Type_FFDNBNVNS) && \
    !defined(Type_FFDPZSNGTSNV) && !defined(Type_FFDPZSNGTSNVNS) && !defined(Type_FFDPNGTSNVNS) && \
    !defined(Type_FFDPZSNBNV) && !defined(Type_FFDPZSNBNVNS) && !defined(Type_FFDPNBNVNS) && \
    !defined(Type_RFDZSNV) && !defined(Type_RFDZSNVNS) && !defined(Type_RFDNVNS) && \
    !defined(Type_RFDZSNBNV) && !defined(Type_RFDZSNBNVNS) && !defined(Type_RFDNBNVNS) && !defined(Type_RFDZSNBNVNSCE)
#define Type_FFD
#endif // Type_***


//**********************************************************************************
//**********************************************************************************
// Define MAC buffer pools depending on device type. The number of buffers in each
// pool depends on the supported features. The device type also determines
// whether or not ACL buffers must be allocated for MAC security.
//**********************************************************************************
//**********************************************************************************
#if defined(Type_RFDNB) || defined(Type_RFDNBNV) || \
    defined(Type_RFDZSNBNV)
  // Reduced Function Device, no beacons (NB), no MLME parameter verification (NV).
  #define gBigMsgsMacOnly_d         0 // No MAC private pool
  #define gMmNumPools_c             2 // One small + one large buffer pool

  #ifndef gTotalSmallMsgs_d
  #define gTotalSmallMsgs_d         5 // Number of control messages
  #endif /* gTotalSmallMsgs_d */

  #ifndef gTotalBigMsgs_d
  #define gTotalBigMsgs_d           3 // Number of data messages
  #endif /* gTotalBigMsgs_d */
  
  #ifndef gMAC2006_d
  #define gNumAclEntryDescriptors_c 4 // Number of ACL entries in MAC PIB
  #else
  #define gNumKeyTableEntries_c           2
  
  #define gNumKeyIdLookupEntries_c        2
  #define gNumKeyDeviceListEntries_c      2
  #define gNumKeyUsageListEntries_c       2

  #define gNumDeviceTableEntries_c        2
  #define gNumSecurityLevelTableEntries_c 2
  #endif //gMAC2006_d
  
#endif

#if defined(Type_RFDNBNS) || \
    defined(Type_RFDZSNVNS) || defined(Type_RFDNVNS) || defined(Type_RFDZSNBNVNS) || defined(Type_RFDNBNVNS) || defined(Type_RFDZSNBNVNSCE)
  // Reduced Function Device, no beacons (NB), no MAC security (NS).
  #define gBigMsgsMacOnly_d         0 // No MAC private pool
  #define gMmNumPools_c             2 // One small + one large buffer pool
  
  #ifndef gTotalSmallMsgs_d
  #define gTotalSmallMsgs_d         5 // Number of control messages
  #endif /* gTotalSmallMsgs_d */

  #ifndef gTotalBigMsgs_d
  #define gTotalBigMsgs_d           6 // Number of data messages
  #endif /* gTotalBigMsgs_d */
  #ifndef gMAC2006_d
  #define gNumAclEntryDescriptors_c 0 // No security -> no ACL entries.
  #else
  #define gNumKeyTableEntries_c           0
  
  #define gNumKeyIdLookupEntries_c        0
  #define gNumKeyDeviceListEntries_c      0
  #define gNumKeyUsageListEntries_c       0

  #define gNumDeviceTableEntries_c        0
  #define gNumSecurityLevelTableEntries_c 0
  #endif  //gMAC2006_d
#endif

#if defined(Type_RFD) || \
    defined(Type_RFDZSNV) 
  // Reduced Function Device (end-device only).
  #define gBigMsgsMacOnly_d         0 // No MAC private pool
  #define gMmNumPools_c             2 // One small + one large buffer pool
  
  #ifndef gTotalSmallMsgs_d
  #define gTotalSmallMsgs_d         5 // Number of control messages
  #endif /* gTotalSmallMsgs_d */
  #ifndef gTotalBigMsgs_d
  #define gTotalBigMsgs_d           4 // Number of data messages
  #endif /* gTotalBigMsgs_d */
  
  #ifndef gMAC2006_d
  #define gNumAclEntryDescriptors_c 4 // Number of ACL entries in MAC PIB
  #else
  #define gNumKeyTableEntries_c           2
  
  #define gNumKeyIdLookupEntries_c        2
  #define gNumKeyDeviceListEntries_c      2
  #define gNumKeyUsageListEntries_c       2

  #define gNumDeviceTableEntries_c        2
  #define gNumSecurityLevelTableEntries_c 2
  #endif  //gMAC2006_d
#endif

#if defined(Type_FFDNB) || defined(Type_FFDNBNV) || defined(Type_FFDPNBNV) || \
    defined(Type_FFDZSNBNV) || defined(Type_FFDPZSNBNV)
  // Full Function Device, no beacons (NB), no MLME parameter verification (NV), 
  // PAN Coordinator Only (P).
  #define gBigMsgsMacOnly_d         1 // One buffer is MAC private.
  #define gMmNumPools_c             3 // One small + one large + one large MAC private buffer pool

  #ifndef gTotalSmallMsgs_d
  #define gTotalSmallMsgs_d         5 // Number of control messages
  #endif /* gTotalSmallMsgs_d */

  #ifndef gTotalBigMsgs_d
  #define gTotalBigMsgs_d           4 // Number of data messages
  #endif /* gTotalBigMsgs_d */
  #ifndef gMAC2006_d
  #define gNumAclEntryDescriptors_c 4 // Number of ACL entries in MAC PIB
  #else
  #define gNumKeyTableEntries_c           2
  
  #define gNumKeyIdLookupEntries_c        2
  #define gNumKeyDeviceListEntries_c      2
  #define gNumKeyUsageListEntries_c       2

  #define gNumDeviceTableEntries_c        2
  #define gNumSecurityLevelTableEntries_c 2
  #endif  //gMAC2006_d
#endif

#if defined(Type_FFDNBNS) || \
    defined(Type_FFDZSNGTSNVNS) || defined(Type_FFDNGTSNVNS) || defined(Type_FFDZSNBNVNS) || defined(Type_FFDZSNBNVNSCE) || defined(Type_FFDNBNVNS) || \
    defined(Type_FFDPZSNGTSNVNS) || defined(Type_FFDPNGTSNVNS) || defined(Type_FFDPZSNBNVNS) || defined(Type_FFDPNBNVNS)
  // Full Function Device, no beacons (NB), no MAC security (NS), 
  #define gBigMsgsMacOnly_d         1 // One buffer is MAC private.
  #define gMmNumPools_c             3 // One small + one large + one large MAC private buffer pool
  
  #ifndef gTotalSmallMsgs_d
  #define gTotalSmallMsgs_d         5 // Number of control messages
  #endif /* gTotalSmallMsgs_d */

  #ifndef gTotalBigMsgs_d
  #define gTotalBigMsgs_d           5 // Number of data messages
  #endif /* gTotalBigMsgs_d */
  
  #ifndef gMAC2006_d
  #define gNumAclEntryDescriptors_c 0 // No security -> no ACL entries.
  #else
  #define gNumKeyTableEntries_c           0
  
  #define gNumKeyIdLookupEntries_c        0
  #define gNumKeyDeviceListEntries_c      0
  #define gNumKeyUsageListEntries_c       0

  #define gNumDeviceTableEntries_c        0
  #define gNumSecurityLevelTableEntries_c 0
  #endif  //gMAC2006_d
#endif 

#if defined(Type_FFDZSNBNVNSCE_PROM)
  #define gBigMsgsMacOnly_d         1 // One buffer is MAC private.
  #define gMmNumPools_c             3 // One small + one large + one large MAC private buffer pool
  
  #ifndef gTotalSmallMsgs_d
  #define gTotalSmallMsgs_d         5 // Number of control messages
  #endif /* gTotalSmallMsgs_d */

  #ifndef gTotalBigMsgs_d
  #define gTotalBigMsgs_d           10 // Number of data messages
  #endif /* gTotalBigMsgs_d */
  
  #ifndef gMAC2006_d
  #define gNumAclEntryDescriptors_c 0 // No security -> no ACL entries.
  #else
  #define gNumKeyTableEntries_c           0
  
  #define gNumKeyIdLookupEntries_c        0
  #define gNumKeyDeviceListEntries_c      0
  #define gNumKeyUsageListEntries_c       0

  #define gNumDeviceTableEntries_c        0
  #define gNumSecurityLevelTableEntries_c 0  
  #endif  //gMAC2006_d
#endif


#if defined(Type_FFD) || defined(Type_FFDNGTS) || \
    defined(Type_FFDZSNGTSNV) || defined(Type_FFDPZSNGTSNV)
  // Full Function Device, no GTS (NGTS), 
  #define gBigMsgsMacOnly_d         1 // One buffer is MAC private.
  #define gMmNumPools_c             3 // One small + one large + one large MAC private buffer pool

  #ifndef gTotalSmallMsgs_d
  #define gTotalSmallMsgs_d         5 // Number of control messages
  #endif /* gTotalSmallMsgs_d */

  #ifndef gTotalBigMsgs_d
  #define gTotalBigMsgs_d           5 // Number of data messages
  #endif /* gTotalBigMsgs_d */
  
  #ifndef gMAC2006_d
  #define gNumAclEntryDescriptors_c 4 // Number of ACL entries in MAC PIB
  #else
  #define gNumKeyTableEntries_c           2
  
  #define gNumKeyIdLookupEntries_c        2
  #define gNumKeyDeviceListEntries_c      2
  #define gNumKeyUsageListEntries_c       2

  #define gNumDeviceTableEntries_c        2
  #define gNumSecurityLevelTableEntries_c 2  
  

  #endif  //gMAC2006_d
  
#endif 


  // Allows application to select if ASP shall be included
#define gAspCapability_d            1

  // Telec functionality for the Japanese market
#ifdef NO_TELEC
  #define gAspJapanTelecCapability_d 0
#else
  #define gAspJapanTelecCapability_d (1 && gAspCapability_d)
#endif

  // Instruct the MAC if we are running with a task scheduler or with a mainloop (Mlme_Main)
#ifdef NO_SCHEDULER
#define gSchedulerIntegration_d 0
#else
#define gSchedulerIntegration_d 1
#endif

  /* Enable debug msg tracking code that allows to track who allocated or freed a messag last time
     and its status
  */
#define MsgTracking_d 0

#endif //_APPTOMACPHYCONFIG_H_
