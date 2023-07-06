/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
/* ###################################################################
**     Filename    : main.c
**     Processor   : MPC574xG
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.c
** @version 01.00
** @brief
**         Main module.
**         This module contains user's application code.
*/         
/*!
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */


/* Including necessary module. Cpu.h contains other modules needed for compiling.*/
#include "Cpu.h"

  volatile int exit_code = 0;
/* User includes (#include below this line is not maintained by Processor Expert) */
#define LED_PORT        PTJ
#define LED_PIN         4U

#define PIT_CHANNEL	    0UL
#define PERIOD_BY_NS	1000000000UL /* nanosecond unit, The period is 1 second */

void pit_0_callback(void * userData) {
	(void)userData;
	PINS_DRV_TogglePins(LED_PORT, (1 << LED_PIN));
}

/*! 
  \brief The main function for the project.
  \details The startup initialization sequence is the following:
 * - startup asm routine
 * - main()
*/
int main(void)
{

  /* Write your local variable definition here */
	uint64_t pitResolution;
  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  #ifdef PEX_RTOS_INIT
    PEX_RTOS_INIT();                   /* Initialization of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */
  /* For example: for(;;) { } */
    /* Initialize clock gate*/

      CLOCK_SYS_Init(g_clockManConfigsArr,   CLOCK_MANAGER_CONFIG_CNT,g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
      CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);



      /* Initialize and configure pins */
      PINS_DRV_Init(NUM_OF_CONFIGURED_PINS, g_pin_mux_InitConfigArr);
      PINS_DRV_SetPins(LED_PORT, (1 << LED_PIN));

      /* Initialize TIMING over PIT */
      TIMING_Init(&timing_pal1_instance, &timing_pal1_InitConfig);

      /* Get tick resolution in nanosecond unit for TIMING over PIT */
      TIMING_GetResolution(&timing_pal1_instance, TIMER_RESOLUTION_TYPE_NANOSECOND, &pitResolution);

      /* Start PIT channel 0 counting with the period is 1 second,
         the period in tick = the period in nanosecond / PIT tick resolution in nanosecond */
      TIMING_StartChannel(&timing_pal1_instance, PIT_CHANNEL, PERIOD_BY_NS/pitResolution);

  	while(1);


  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;) {
    if(exit_code != 0) {
      break;
    }
  }
  return exit_code;
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
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
