/************************************************************************************
* This file is owned by the platform/application specific software and basically 
* defines how the 802.15.4 Freescale MAC is configured. The file is included by the
* relevant MAC modules and is necessary for the MAC to compile.
*
*
* (c) Copyright 2006, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/

#ifndef _APPTOPLATFORMCONFIG_H_
#define _APPTOPLATFORMCONFIG_H_

/* Defines whether or not the LPM module is included in the application */
//#define gLpmIncluded_d 1

//**********************************************************************************
// Define one Hardware platform type (if not defined default is Axiom GB60)
//**********************************************************************************

// Check that old target definitions are not used in the project settings
#if (defined(TARGET_DIG528_2) || defined(TARGET_DIG536_2) || \
     defined(TARGET_DIG534_1) || defined(TARGET_AXIOM_GB60) || \
     defined(TARGET_RD01) || defined(TARGET_TOROWEAP))
#error Old target definitions no longer allowed.
#endif

// Check if any target platform has been set in the project
#if !((defined gTargetSARD_d) || (defined gTargetEVB_d) || \
      (defined gTargetMC13213NCB_d) || (defined gTargetMC13213SRB_d) || \
      (defined gTargetAxiomGB60_d)||(defined gTarget_UserDefined_d) || \
      (defined gTargetQE128Demo_d) || (defined gTargetQE128EVB_d) || \
      (defined gTargetMC1323xEVB_d) || (defined gTargetMC1323xREM_d) || \
      (defined gTargetMC1323xTWRRF_d) || \
      (defined gTargetMC1323xRCM_d) || (defined gTargetMC1323xUSBR2_d))

/* Select default target if no target settings were defined in the project */
#define gTargetMC13213SRB_d  0
#define gTargetSARD_d        0
#define gTargetEVB_d         1
#define gTargetMC13213NCB_d  0
#define gTargetAxiomGB60_d   0
#define gTarget_UserDefined_d 0
#define gTargetQE128Demo_d 0
#define gTargetQE128EVB_d  0
#define gTargetMC1323xEVB_d 0
#define gTargetMC1323xREM_d 0
#define gTargetMC1323xRCM_d 0
#define gTargetMC1323xTWRRF_d 0
#define gTargetMC1323xUSBR2_d 0

#endif

/* If a target was selected, set the others to 0 to avoid
   compiler warnings ("assuming default 0") */
#ifndef gTargetMC13213SRB_d
#define gTargetMC13213SRB_d  0
#endif
#ifndef gTargetSARD_d
#define gTargetSARD_d        0
#endif
#ifndef gTargetMC13213NCB_d
#define gTargetMC13213NCB_d  0
#endif
#ifndef gTargetEVB_d
#define gTargetEVB_d         0
#endif
#ifndef gTargetAxiomGB60_d
#define gTargetAxiomGB60_d   0
#endif
#ifndef gTarget_UserDefined_d
#define gTarget_UserDefined_d   0
#endif
#ifndef gTargetQE128Demo_d
#define gTargetQE128Demo_d  0
#endif
#ifndef gTargetQE128EVB_d
#define gTargetQE128EVB_d  0
#endif
#ifndef gTargetMC1323xEVB_d
#define gTargetMC1323xEVB_d  0
#endif
#ifndef gTargetMC1323xREM_d
#define gTargetMC1323xREM_d  0
#endif
#ifndef gTargetMC1323xRCM_d
#define gTargetMC1323xRCM_d  0
#endif
#ifndef gTargetMC1323xTWRRF_d
#define gTargetMC1323xTWRRF_d  0
#endif
#ifndef gTargetMC1323xUSBR2_d
#define gTargetMC1323xUSBR2_d  0
#endif

/*
   jehol1 hack to enable BeeStack target definitions.
   This converts from new ones to old ones
*/
#if (gTargetSARD_d == 1) /* This define must change name to gTargetMC1319xSARD_d later in both MAC and BeeStack */
#define TARGET_DIG536_2
#endif /* gTargetSARD_d */

#if (gTargetEVB_d == 1) /* This define must change name to gTargetMC1319xEVB_d later in both MAC and BeeStack */
#define TARGET_DIG528_2
#endif /* gTargetEVB_d */

#if (gTargetMC13213NCB_d == 1)
#define TARGET_TOROWEAP
#endif /* gTargetMC1321xNCB_d */

#if (gTargetMC13213SRB_d == 1)
#define TARGET_TOROWEAP
#endif /* gTargetMC13213SRB_d */
 
#if (gTargetAxiomGB60_d == 1)
#define TARGET_AXIOM_GB60
#endif /* TARGET_AXIOM_GB60 */

#if (gTargetQE128Demo_d == 1)
#define TARGET_AXIOM_GB60
#endif

#if (gTargetQE128EVB_d == 1)
#define TARGET_TOROWEAP
#endif

#if (gTargetMC1323xEVB_d == 1)
#define TARGET_MC1323x
#endif

#if (gTargetMC1323xREM_d == 1)
#define TARGET_MC1323x
#endif

#if (gTargetMC1323xRCM_d == 1)
#define TARGET_MC1323x
#endif

#if (gTargetMC1323xTWRRF_d == 1)
#define TARGET_MC1323x
#endif

#if (gTargetMC1323xUSBR2_d == 1)
#define TARGET_MC1323x
#endif

#if (gTarget_UserDefined_d == 1) 
#define TARGET_USER_DEFINED
#endif /* gTarget_UserDefined_d */

/*
   Enable target platform.
   Do NOT touch while the hack above is enabled as it will
   cause the BeeStack and MAC to use different target difinitios.
*/
//#define TARGET_DIG528_2 // 13192-EVB
//#define TARGET_DIG536_2 // 13192-SARD
//#define TARGET_DIG534_1 // Sniffer
//#define TARGET_AXIOM_GB60 // 802.15.4 Dev Platform
//#define TARGET_RD01 // For all boards which uses the Freescale reference design 01 layout



//**********************************************************************************
// Set default target hardware platform if none selected:
//**********************************************************************************
#if !(defined(TARGET_DIG528_2) || defined(TARGET_DIG536_2) || defined(TARGET_DIG534_1) || defined(TARGET_AXIOM_GB60) || defined(TARGET_RD01) || defined(TARGET_TOROWEAP))
//#define TARGET_DIG528_2 // 13192-EVB
//#define TARGET_DIG536_2 // 13192-SARD
//#define TARGET_DIG534_1 // Sniffer
//#define TARGET_AXIOM_GB60 // 802.15.4 Dev Platform
//#define TARGET_RD01 // For all boards which uses the Freescale reference design 01 layout
//#define TARGET_TOROWEAP // For the toroweap board
#endif

////////////////////////////////////////
/// Define "PHY Type" (length 7 char) //
////////////////////////////////////////
#if defined TARGET_AXIOM_GB60
  #define PHY_TYPE "AX-0308"
#elif defined TARGET_DIG528_2
  #define PHY_TYPE " DIG528"
#elif defined TARGET_DIG536_2
  #define PHY_TYPE " DIG536"
#elif defined TARGET_RD01
  #define PHY_TYPE " RD01  "
#elif defined TARGET_DIG534_1
  #define PHY_TYPE " DIG534"
#elif defined TARGET_TOROWEAP
  #define PHY_TYPE " TOROWP"
#elif defined TARGET_MC1323x
  #define PHY_TYPE " MC1323x"
#elif defined TARGET_USER_DEFINED
  #define PHY_TYPE " USERD"  
#else
  #error No target defined!!!
#endif


//**********************************************************************************
// PTC application defines
//**********************************************************************************
#ifdef INCLUDE_802_15_4 // Defined by NWK build environment
 #define F8W_NWK_PRESENT 1 // Define if compiling for F8W target - excludes PTC PHY fnc-table init.
#else
 //#define F8W_NWK_PRESENT 1
#endif INCLUDE_802_15_4



//**********************************************************************************
// Select platform clock speed selection depending on MAC device type 
// MAC device type minimum clock values. User can specify a higher clock frequency.
// If none selected default to 16MHz
// Valid values for SYSTEM_CLOCK: 8, 12, 16, 16780

#ifdef Type_RFDNBNS
	#define SYSTEM_CLOCK 16
#endif Type_RFDNBNS

#ifdef Type_RFDNBNV
	#define SYSTEM_CLOCK 16
#endif Type_RFDNBNV

#ifdef  Type_RFDNB
	#define SYSTEM_CLOCK 16
#endif  Type_RFDNB

#ifdef Type_RFD
	#define SYSTEM_CLOCK 16
#endif Type_RFD

#ifdef Type_FFDNBNS
	#define SYSTEM_CLOCK 16
#endif Type_FFDNBNS

#ifdef Type_FFDNBNV
	#define SYSTEM_CLOCK 16
#endif Type_FFDNBNV

#ifdef Type_FFDNB
	#define SYSTEM_CLOCK 16
#endif Type_FFDNB

#ifdef Type_FFDNGTS
	#define SYSTEM_CLOCK 16
#endif Type_FFDNGTS

#ifdef Type_FFD
	#define SYSTEM_CLOCK 16
#endif Type_FFD

// If device type is not predefined use the following definitions:
#ifndef SYSTEM_CLOCK 
  #define SYSTEM_CLOCK 16
#endif /* SYSTEM_CLOCK */

//**********************************************************************************


#endif /* _APPTOPLATFORMCONFIG_H_ */
