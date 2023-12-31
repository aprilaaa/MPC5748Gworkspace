/* ###################################################################
**     This component module is generated by Processor Expert. Do not modify it.
**     Filename    : timing_pal1.h
**     Project     : testTimer_Z4_0
**     Processor   : MPC5748G_324
**     Component   : timing_pal
**     Version     : Component SDK_S32_PA_11, Driver 01.00, CPU db: 3.00.000
**     Repository  : SDK_S32_PA_11
**     Compiler    : GNU C Compiler
**     Date/Time   : 2023-06-19, 15:52, # CodeGen: 1
**     Contents    :
**         TIMING_Init                - status_t TIMING_Init(const timing_instance_t * const instance, const...
**         TIMING_Deinit              - void TIMING_Deinit(const timing_instance_t * const instance);
**         TIMING_StartChannel        - void TIMING_StartChannel(const timing_instance_t * const instance, const...
**         TIMING_StopChannel         - void TIMING_StopChannel(const timing_instance_t * const instance, const...
**         TIMING_GetElapsed          - uint32_t TIMING_GetElapsed(const timing_instance_t * const instance, const...
**         TIMING_GetRemaining        - uint32_t TIMING_GetRemaining(const timing_instance_t * const instance, const...
**         TIMING_EnableNotification  - void TIMING_EnableNotification(const timing_instance_t * const instance,...
**         TIMING_DisableNotification - void TIMING_DisableNotification(const timing_instance_t * const instance,...
**         TIMING_GetResolution       - status_t TIMING_GetResolution(const timing_instance_t * const instance, const...
**         TIMING_GetMaxPeriod        - status_t TIMING_GetMaxPeriod(const timing_instance_t * const instance, const...
**         TIMING_InstallCallback     - void TIMING_InstallCallback(const timing_instance_t * const instance, const...
**
**     Copyright 1997 - 2015 Freescale Semiconductor, Inc. 
**     Copyright 2016-2017 NXP 
**     All Rights Reserved.
**     
**     THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
**     IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
**     OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
**     IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
**     INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
**     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
**     SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
**     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
**     STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
**     IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
**     THE POSSIBILITY OF SUCH DAMAGE.
** ###################################################################*/
/*!
** @file timing_pal1.h
** @version 01.00
*/         
/*!
**  @addtogroup timing_pal1_module timing_pal1 module documentation
**  @{
*/         
#ifndef timing_pal1_H
#define timing_pal1_H

/* MODULE timing_pal1. */

/* Include inherited beans */
#include "clockMan1.h"
#include "Cpu.h"
#include "timing_pal.h"


/*! @brief PAL instance information */
extern const timing_instance_t timing_pal1_instance;

/*! @brief Channel configuration array */
extern timer_chan_config_t timing_pal1_channelConfigs[1];
/*! @brief Timer configuration */
extern timer_config_t timing_pal1_InitConfig;
/*! @brief Callback function of channel configuration 0 */
extern void pit_0_callback(void * userData);

#endif
/* ifndef __timing_pal1_H */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.1 [05.21]
**     for the NXP C55 series of microcontrollers.
**
** ###################################################################
*/

