/* ###################################################################
**     This component module is generated by Processor Expert. Do not modify it.
**     Filename    : pwm_pal1.c
**     Project     : testPWM_Z4_0
**     Processor   : MPC5748G_324
**     Component   : pwm_pal
**     Version     : Component SDK_S32_PA_11, Driver 01.00, CPU db: 3.00.000
**     Repository  : SDK_S32_PA_11
**     Compiler    : GNU C Compiler
**     Date/Time   : 2023-06-26, 16:24, # CodeGen: 18
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
** @file pwm_pal1.c
** @version 01.00
*/         
/*!
**  @addtogroup pwm_pal1_module pwm_pal1 module documentation
**  @{
*/         
#include "pwm_pal1.h"
#include "pwm_pal.h"

/*
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, External could be made static.
 * This structure is defined for usage by application code.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 8.4, external symbol defined without a prior
 * declaration.
 * This symbol is not a part of the public symbols.
 *
 */

 /* Timebases */
pwm_emios_timebase_t pwm_pal1_EMIOS_CNT_BUSA_DRIVEN_Timebase =
{
    .name = EMIOS_CNT_BUSA_DRIVEN,
    .internalPrescaler = EMIOS_CLOCK_DIVID_BY_1,
};
 
 /* Channels */
pwm_channel_t pwm_pal1Channels[1] =
{
    {
        .channel                       = 0,
        .channelType                   = PWM_EDGE_ALIGNED,
        .period                        = 5000,
        .duty                          = 0,
        .polarity                      = PWM_ACTIVE_HIGH,
        .timebase                      = &pwm_pal1_EMIOS_CNT_BUSA_DRIVEN_Timebase,
        .insertDeadtime                = false,
        .deadtime                      = 0,
        .deadtimeEdge                  = PWM_DEADTIME_TRAIL_EDGE,
        .deadtimePrescaler             = EMIOS_CLOCK_DIVID_BY_1,
    },
};
 

  /* Initialization configuration */
pwm_global_config_t pwm_pal1Configs =
{
    .pwmChannels          = pwm_pal1Channels,
    .numberOfPwmChannels  = 1,
};
 
  /* Instance configuration */
pwm_instance_t pwm_pal1Instance =
{
    .instType  = PWM_INST_TYPE_EMIOS,
    .instIdx   = 0,
};
  
 
/* END pwm_pal1. */

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

