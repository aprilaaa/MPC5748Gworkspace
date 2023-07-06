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

#define BTN_PORT        PTA
#define BTN_PIN         3U
#define BTN_PORT0        PTE
#define BTN_PIN0         12U
#define PORT0	PTA
#define LED0 	7
#define PORT1	PTA
#define LED1	10
#define PORT2	PTH
#define LED2	13
#define PORT3	PTC
#define LED3	4
#define PORT4	PTH
#define LED4	5
#define PORT5   PTJ
#define LED5    4U
#define PORT6	PTA
#define LED6 	0
#define PORT7	PTA
#define LED7 	4
#define GPIO_PORT	PTG
#define GPIO_PIN	2


/*! 
  \brief The main function for the project.
  \details The startup initialization sequence is the following:
 * - startup asm routine
 * - main()
*/
int main(void)
{
  /* Write your local variable definition here */

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  #ifdef PEX_RTOS_INIT
    PEX_RTOS_INIT();                   /* Initialization of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */
  /* For example: for(;;) { } */
    /* Initialize and configure clocks */
       CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT,
                           g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
       CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_FORCIBLE);

       /* Initialize pins */
       PINS_DRV_Init(NUM_OF_CONFIGURED_PINS, g_pin_mux_InitConfigArr);

       while(1) {
       	uint32_t btn_return = (PINS_DRV_ReadPins(BTN_PORT) >> BTN_PIN) & 0x01;//read SW1
       	uint32_t btn_return0 = (PINS_DRV_ReadPins(BTN_PORT0) >> BTN_PIN0) & 0x01;//read SW2
       	if(btn_return == 1) {	//button pressed
       		PINS_DRV_WritePin(PTA, 7, 0);
       		PINS_DRV_WritePin(PORT1, LED1, 0);
       		PINS_DRV_WritePin(PORT2, LED2, 0);
       		PINS_DRV_WritePin(PORT3, LED3, 0);
       		PINS_DRV_WritePin(PORT4, LED4, 0);	//led on
       		PINS_DRV_WritePin(PORT5, LED5, 0);
       		PINS_DRV_WritePin(PORT6, LED6, 0);
       		PINS_DRV_WritePin(PORT7, LED7, 0);
       		PINS_DRV_WritePin(GPIO_PORT, GPIO_PIN, 1);	//PG2 1
       	}
       	if(btn_return0 == 1) {
       		PINS_DRV_WritePin(PORT0, LED0, 1);
       		PINS_DRV_WritePin(PORT1, LED1, 1);
       		PINS_DRV_WritePin(PORT2, LED2, 1);
       		PINS_DRV_WritePin(PORT3, LED3, 1);
       		PINS_DRV_WritePin(PORT4, LED4, 1);	//led off
       		PINS_DRV_WritePin(PORT5, LED5, 1);
       		PINS_DRV_WritePin(PORT6, LED6, 1);
       		PINS_DRV_WritePin(PORT7, LED7, 1);
       		PINS_DRV_WritePin(GPIO_PORT, GPIO_PIN, 0); //PG2 0
       	}
       }

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
