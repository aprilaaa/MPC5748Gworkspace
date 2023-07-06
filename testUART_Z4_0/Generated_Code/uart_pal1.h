/* ###################################################################
**     This component module is generated by Processor Expert. Do not modify it.
**     Filename    : uart_pal1.h
**     Project     : testUART_Z4_0
**     Processor   : MPC5748G_324
**     Component   : uart_pal
**     Version     : Component SDK_S32_PA_11, Driver 01.00, CPU db: 3.00.000
**     Repository  : SDK_S32_PA_11
**     Compiler    : GNU C Compiler
**     Date/Time   : 2023-06-20, 15:44, # CodeGen: 2
**     Contents    :
**         UART_GetDefaultConfig    - void UART_GetDefaultConfig(uart_user_config_t * config);
**         UART_Init                - status_t UART_Init(const uart_instance_t * const instance, const...
**         UART_Deinit              - status_t UART_Deinit(const uart_instance_t * const instance);
**         UART_SetBaudRate         - status_t UART_SetBaudRate(const uart_instance_t * const instance, uint32_t...
**         UART_GetBaudRate         - status_t UART_GetBaudRate(const uart_instance_t * const instance, uint32_t  *...
**         UART_SendDataBlocking    - status_t UART_SendDataBlocking(const uart_instance_t * const instance, const...
**         UART_SendData            - status_t UART_SendData(const uart_instance_t * const instance, const uint8_t...
**         UART_AbortSendingData    - status_t UART_AbortSendingData(const uart_instance_t * const instance);
**         UART_GetTransmitStatus   - status_t UART_GetTransmitStatus(const uart_instance_t * const instance,...
**         UART_ReceiveDataBlocking - status_t UART_ReceiveDataBlocking(const uart_instance_t * const instance,...
**         UART_ReceiveData         - status_t UART_ReceiveData(const uart_instance_t * const instance, uint8_t *...
**         UART_AbortReceivingData  - status_t UART_AbortReceivingData(const uart_instance_t * const instance);
**         UART_GetReceiveStatus    - status_t UART_GetReceiveStatus(const uart_instance_t * const instance,...
**         UART_SetRxBuffer         - status_t UART_SetRxBuffer(const uart_instance_t * const instance, uint8_t *...
**         UART_SetTxBuffer         - status_t UART_SetTxBuffer(const uart_instance_t * const instance, const...
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
** @file uart_pal1.h
** @version 01.00
*/         
/*!
**  @addtogroup uart_pal1_module uart_pal1 module documentation
**  @{
*/         
#ifndef uart_pal1_H
#define uart_pal1_H

/* MODULE uart_pal1. */

/* Include inherited beans */
#include "clockMan1.h"
#include "dmaController1.h"
#include "Cpu.h"

/*! @brief Device instance information */
extern const uart_instance_t uart_pal1_instance;

/*! @brief Serial module configurations */
extern const uart_user_config_t uart_pal1_Config0;

/*! @brief Define external callbacks for RX and TX */
extern void uartRxCallback(void *driverState, uart_event_t event, void *userData);

#endif
/* ifndef __uart_pal1_H */
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
