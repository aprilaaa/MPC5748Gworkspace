
/* ###################################################################
**     Filename    : main.c
**     Project     : testADC
**     Processor   : MPC5748G_176
**     Version     : Driver 01.00
**     Compiler    : GNU C Compiler
**     Date/Time   : 2017-03-14, 14:08, # CodeGen: 1
**     Abstract    :
**     Settings    :
**     Contents    :
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
/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "clockMan1.h"
#include "adConv1.h"
#include "pin_mux.h"
volatile int exit_code = 0;
/* User includes (#include below this line is not maintained by Processor Expert) */
#include <stdint.h>
#include <stdbool.h>
/* This example is setup to work by default with DEVKIT. To use it with other boards
   please comment the following line
*/
#define DEVKIT
#ifdef DEVKIT


#define PORT0	PTA
#define LED0	10
#define PORT1	PTA
#define LED1 	7
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

#define ADC_CHAN_NUM (9u)		/* ADC1_P[0] corresponding to PB[4] */
uint16_t result;				/* 读取PB[4]的结果 */
#define ADC_THRESHOLD (512u) 	/* should be around 3.3V/8 = 0.4125V for 12-bit resolution, 
									4096/8 = 512 也就是 3.3V/8 的数字量 */
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
  /* Initialize and configure pins */
  PINS_DRV_Init(NUM_OF_CONFIGURED_PINS, g_pin_mux_InitConfigArr);
  
  /* Initialize clocks */
  CLOCK_SYS_Init(g_clockManConfigsArr,   CLOCK_MANAGER_CONFIG_CNT,
		 g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
  CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);
  
  /* Initialize ADC */
  ADC_DRV_Reset(INST_ADCONV1); //复位
  ADC_DRV_DoCalibration(INST_ADCONV1); //校准
  ADC_DRV_ConfigConverter(INST_ADCONV1,&adConv1_ConvCfg0); //配置
  ADC_DRV_EnableChannel(INST_ADCONV1, ADC_CONV_CHAIN_NORMAL, ADC_CHAN_NUM); //使能
  
  /* Start a loop to read converted values and blink an LED */
  while(1)
  {
    ADC_DRV_StartConversion(INST_ADCONV1, ADC_CONV_CHAIN_NORMAL); //启动
    while(!(ADC_DRV_GetStatusFlags(INST_ADCONV1) & ADC_FLAG_NORMAL_ENDCHAIN)); //等待状态标志
    uint32_t len = ADC_DRV_GetConvResultsToArray(INST_ADCONV1, ADC_CONV_CHAIN_NORMAL, &result, 1u); //获取转换结果
    DEV_ASSERT(len == 1u); /* only one value should be read from hardware registers */
    (void)len;
    ADC_DRV_ClearStatusFlags(INST_ADCONV1, ADC_FLAG_NORMAL_ENDCHAIN);//清除
    uint8_t ledValue = 0x00;

    if(result < (1 * ADC_THRESHOLD))
    {
    	ledValue = 0x01;	//点亮LED0		0b0000 0001
    }
    else if(((1 * ADC_THRESHOLD) <=result) && (result < (2 * ADC_THRESHOLD)))
    {
    	ledValue = 0x02;	//点亮LED1
    }
    else if(((2 * ADC_THRESHOLD) <=result) && (result < (3 * ADC_THRESHOLD)))
    {
    	ledValue = 0x04;	//点亮LED2
    }
    else if(((3 * ADC_THRESHOLD) <=result) && (result < (4 * ADC_THRESHOLD)))
    {
    	ledValue = 0x08;	//点亮LED3
    }
    else if(((4 * ADC_THRESHOLD) <=result) && (result < (5 * ADC_THRESHOLD)))
    {
    	ledValue = 0x10;	//点亮LED4
    }
    else if(((5 * ADC_THRESHOLD) <=result) && (result < (6 * ADC_THRESHOLD)))
    {
    	ledValue = 0x20;	//点亮LED5
    }
    else if(((6 * ADC_THRESHOLD) <=result) && (result < (7 * ADC_THRESHOLD)))
    {
    	ledValue = 0x40;	//点亮LED6
    }
    else if(((7 * ADC_THRESHOLD) <=result) && (result < (8 * ADC_THRESHOLD)))
    {
    	ledValue = 0x80;	//点亮LED7		0b1000 0000
    }
    else
    {

    }
	PINS_DRV_WritePin(PORT0, LED0, (ledValue & 0x01) ? 0 : 1);
	PINS_DRV_WritePin(PORT1, LED1, (ledValue & 0x02) ? 0 : 1);
	PINS_DRV_WritePin(PORT2, LED2, (ledValue & 0x04) ? 0 : 1);
	PINS_DRV_WritePin(PORT3, LED3, (ledValue & 0x08) ? 0 : 1);
	PINS_DRV_WritePin(PORT4, LED4, (ledValue & 0x10) ? 0 : 1);
	PINS_DRV_WritePin(PORT5, LED5, (ledValue & 0x20) ? 0 : 1);
	PINS_DRV_WritePin(PORT6, LED6, (ledValue & 0x40) ? 0 : 1);
	PINS_DRV_WritePin(PORT7, LED7, (ledValue & 0x80) ? 0 : 1);

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
** 
*/

