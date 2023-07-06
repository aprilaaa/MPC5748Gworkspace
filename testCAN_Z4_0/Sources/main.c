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

#define TX_STD_MAILBOX (0UL)
#define TX_EXT_MAILBOX (1UL)

can_buff_config_t stdBuffCfg =  {
    .enableFD = false,
    .enableBRS = false,
    .fdPadding = 0U,
    .idType = CAN_MSG_ID_STD,
    .isRemote = false
};

can_buff_config_t extBuffCfg =  {
    .enableFD = false,
    .enableBRS = false,
    .fdPadding = 0U,
    .idType = CAN_MSG_ID_EXT,
    .isRemote = false
};

can_message_t recvMsg;


//refer to can_callback_t
void canRxCallback(uint32_t instance,
                   can_event_t eventType,
                   uint32_t objIdx,
                   void *driverState)
{
  if(eventType == CAN_EVENT_RX_COMPLETE) {
    if(instance == can_pal1_instance.instIdx) {
    	if(recvMsg.id < 0x800) {	//not sure
    		if(recvMsg.data[0] == 0x01) {
    			CAN_Send(&can_pal1_instance, TX_STD_MAILBOX, &recvMsg);	//echo
    		} else {
    			can_message_t message = {
    			    .cs = 0U,
    			    .id = 0x111,
    			    .data[0] = 0x01,
					.data[1] = 0x01,
					.data[2] = 0x01,
					.data[3] = 0x01,
					.data[4] = 0x01,
					.data[5] = 0x01,
					.data[6] = 0x01,
					.data[7] = 0x01,
    			    .length = 8U
    			};
    			CAN_Send(&can_pal1_instance, TX_STD_MAILBOX, &message);
    		}
    	} else {
    		CAN_Send(&can_pal1_instance, TX_EXT_MAILBOX, &recvMsg);
    	}
        CAN_Receive(&can_pal1_instance, objIdx, &recvMsg);
    }
  }
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

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  #ifdef PEX_RTOS_INIT
    PEX_RTOS_INIT();                   /* Initialization of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */
  /* For example: for(;;) { } */

      CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT,
                          g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
      CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_FORCIBLE);
      PINS_DRV_Init(NUM_OF_CONFIGURED_PINS, g_pin_mux_InitConfigArr);

      /* Initialize can_pal1 */
      CAN_Init(&can_pal1_instance, &can_pal1_Config0);

      /* Configure TX buffer with index TX_MAILBOX*/
      CAN_ConfigTxBuff(&can_pal1_instance, TX_STD_MAILBOX, &stdBuffCfg);
      CAN_ConfigTxBuff(&can_pal1_instance, TX_EXT_MAILBOX, &extBuffCfg);

      /* Configure RX buffer with index RX_MAILBOX */
      CAN_ConfigRxBuff(&can_pal1_instance, 2, &stdBuffCfg, 0x123);
      CAN_ConfigRxBuff(&can_pal1_instance, 3, &stdBuffCfg, 0x456);
      CAN_ConfigRxBuff(&can_pal1_instance, 4, &extBuffCfg, 0x1234567);
      CAN_ConfigRxBuff(&can_pal1_instance, 5, &extBuffCfg, 0x1234568);

      CAN_InstallEventCallback(&can_pal1_instance, canRxCallback, NULL);
      CAN_Receive(&can_pal1_instance, 2, &recvMsg);
      CAN_Receive(&can_pal1_instance, 3, &recvMsg);
      CAN_Receive(&can_pal1_instance, 4, &recvMsg);
      CAN_Receive(&can_pal1_instance, 5, &recvMsg);

      while(1) {

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
