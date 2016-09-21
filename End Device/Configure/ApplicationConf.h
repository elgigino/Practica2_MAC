/*****************************************************************************
* This  file is used by the Application to configure Mac Standalone. 
*
*             
* (c) Copyright 2007, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

#ifndef _MACAPPLICATION_CONFIG_H_
#define _MACAPPLICATION_CONFIG_H_

/*****************************************************************************
******************************************************************************
* Public macros
******************************************************************************
*****************************************************************************/
/* Defines the channels to scan. Each bit represents one channel. Use
   0x07FFF800 to scan all 16 802.15.4 channels in the 2.4GHz band. */
   
/******* DO NOT ALTER THE DEFINITIONS BELOW, PLEASE WRITE YOUR OWN ONE ! ******/
//#define mDefaultValueOfChannel_c 0x00000800 /*ch. 0x0B, 11 dec, 2405MHz */
//#define mDefaultValueOfChannel_c 0x00001000 /*ch. 0x0C, 12 dec, 2410MHz */
//#define mDefaultValueOfChannel_c 0x00002000 /*ch. 0x0D, 13 dec, 2415MHz */
//#define mDefaultValueOfChannel_c 0x00004000 /*ch. 0x0E, 14 dec, 2420MHz */
//#define mDefaultValueOfChannel_c 0x00008000 /*ch. 0x0F, 15 dec, 2425MHz */
#define mDefaultValueOfChannel_c 0x00001000 /*ch. 0x10, 16 dec, 2430MHz */
//#define mDefaultValueOfChannel_c 0x00020000 /*ch. 0x11, 17 dec, 2435MHz */
//#define mDefaultValueOfChannel_c 0x00040000 /*ch. 0x12, 18 dec, 2440MHz */
//#define mDefaultValueOfChannel_c 0x00080000 /*ch. 0x13, 19 dec, 2445MHz */
//#define mDefaultValueOfChannel_c 0x00100000 /*ch. 0x14, 20 dec, 2450MHz */
//#define mDefaultValueOfChannel_c 0x00200000 /*ch. 0x15, 21 dec, 2455MHz */
//#define mDefaultValueOfChannel_c 0x00400000 /*ch. 0x16, 22 dec, 2460MHz */
//#define mDefaultValueOfChannel_c 0x00800000 /*ch. 0x17, 23 dec, 2465MHz */
//#define mDefaultValueOfChannel_c 0x01000000 /*ch. 0x18, 24 dec, 2470MHz */
//#define mDefaultValueOfChannel_c 0x02000000 /*ch. 0x19, 25 dec, 2475MHz */
//#define mDefaultValueOfChannel_c 0x04000000 /*ch. 0x1A, 26 dec, 2480MHz */
/*********************** THANKS :-) ********************************************/

/* Defines the default value of the MAC extended address */
#define mDefaultValueOfExtendedAddress_c    0x00,0x22,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF

/* Defines whether or not the LPM module is included in the application */
#ifndef gLpmIncluded_d
#define gLpmIncluded_d FALSE
#endif

/*****************************************************************************
******************************************************************************
* Public type definitions
******************************************************************************
*****************************************************************************/

/*None*/

/*****************************************************************************
******************************************************************************
* Public prototypes
******************************************************************************
*****************************************************************************/

/*None*/

#endif /* _MACAPPLICATION_CONFIG_H_ */