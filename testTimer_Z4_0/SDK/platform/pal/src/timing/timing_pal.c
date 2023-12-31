/*
 * Copyright 2017-2019 NXP
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

/*!
 * @file timing_pal.c
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 1.3, Taking address of near auto variable.
 * The code is not dynamically linked. An absolute stack address is obtained
 * when taking the address of the near auto variable. A source of error in
 * writing dynamic code is that the stack segment may be different from the data
 * segment.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, External could be made static.
 * Function is defined for usage by application code.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.9, Could define variable at block scope
 * Workaround for MISRA false positive reported for Rule 9.1 - array conceivably not initialized.
 * The local array is already initialized at definition.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.1, Unpermitted operand to operator.
 * This is required to get the right returned status from the
 * initialization function.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.4, Conversion between a pointer and
 * integer type.
 * The cast is required to initialize a pointer with an unsigned long define,
 * representing an address.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.6, Cast from unsigned int to pointer.
 * The cast is required to initialize a pointer with an unsigned long define,
 * representing an address.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.3, Cast performed between a pointer to object type
 * and a pointer to a different object type.
 * This is needed for the extension of the user configuration structure, for which the actual type
 * cannot be known.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.5, Conversion from pointer to void to pointer to other type
 * This is needed for the extension of the user configuration structure, for which the actual type
 * cannot be known.
 *
 */

#include <stddef.h>
#include "timing_pal.h"
#include "timing_irq.h"
#include "device_registers.h"
#include "interrupt_manager.h"
#include "clock_manager.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief Runtime state of the Timer channel
 *
 * This structure is used by the driver for its internal logic
 * The application should make no assumptions about the content of this structure
 */
typedef struct
{
/*! @cond DRIVER_INTERNAL_USE_ONLY */
    uint32_t period;            /*!< To save timer channel period */
    uint32_t chanStartVal;      /*!< To save the moment that timer channel start new period */
    timer_chan_type_t chanType; /*!< To save timer channel notification type */
    timer_callback_t callback;  /*!< To save callback for channels notification */
    void * callbackParam;       /*!< To save callback parameter pointer.*/
    bool enableNotification;    /*!< To save enable channels notification */
/*! @endcond */
} timer_chan_state_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
#if (defined(TIMING_OVER_LPIT))
    /* Table to save LPIT channel runtime state */
    static timer_chan_state_t s_lpitState[LPIT_INSTANCE_COUNT][LPIT_TMR_COUNT];
    /* Table to save LPIT IRQ number */
    static const IRQn_Type s_lpitIrq[LPIT_INSTANCE_COUNT][LPIT_TMR_COUNT] = {LPIT_IRQS};
    /* Table to save LPIT clock source name */
    static clock_names_t s_lpitClockName[LPIT_INSTANCE_COUNT] = LPIT_CLOCK_NAMES;
    /* The maximum value of compare register */
    #define LPIT_COMPARE_MAX (LPIT_TMR_TVAL_TMR_VAL_MASK)
#endif

#if (defined(TIMING_OVER_LPTMR))
    /* The maximum value of LPTMR channel number */
    #define LPTMR_TMR_COUNT LPTMR_IRQS_CH_COUNT
    /* Table to save LPTMR channel runtime state */
    static timer_chan_state_t s_lptmrState[LPTMR_INSTANCE_COUNT][LPTMR_TMR_COUNT];
    /* Table to save LPTMR IRQ number */
    static const IRQn_Type s_lptmrIrq[LPTMR_INSTANCE_COUNT][LPTMR_TMR_COUNT]  = {LPTMR_IRQS};
    /* Table to save LPTMR clock source name */
    static lptmr_clocksource_t s_lptmrClockSource[LPTMR_INSTANCE_COUNT];
    /* Table to save LPTMR prescaler */
    static lptmr_prescaler_t s_lptmrPrescaler[LPTMR_INSTANCE_COUNT];
    /* Table to save LPTMR bypass prescaler enable */
    static bool s_lptmrBypassPrescaler[LPTMR_INSTANCE_COUNT];
    /* The maximum value of compare register */
    #define LPTMR_COMPARE_MAX (LPTMR_CMR_COMPARE_MASK)
#endif

#if (defined(TIMING_OVER_FTM))
    /*! @brief Table of base addresses for FTM instances. */
    FTM_Type * const ftmBase[FTM_INSTANCE_COUNT] = FTM_BASE_PTRS;
    /* Table to save FTM IRQ number */
    static const IRQn_Type s_ftmIrq[FTM_INSTANCE_COUNT][FEATURE_FTM_CHANNEL_COUNT] = FTM_IRQS;
    /* Table to save FTM channel runtime state */
    static timer_chan_state_t s_ftmState[FTM_INSTANCE_COUNT][FEATURE_FTM_CHANNEL_COUNT];
    /* Table to save FTM channel running status */
    bool g_ftmChannelRunning[FTM_INSTANCE_COUNT][FEATURE_FTM_CHANNEL_COUNT];
    /* The maximum value of compare register */
    #define FTM_COMPARE_MAX (FTM_CNT_COUNT_MASK)
#endif

#if (defined(TIMING_OVER_PIT))
    /* Table to save PIT channel runtime state */
    static timer_chan_state_t s_pitState[PIT_INSTANCE_COUNT][PIT_TIMER_COUNT];
    /* Table to save PIT IRQ number */
    static const IRQn_Type s_pitIrq[PIT_INSTANCE_COUNT][PIT_IRQS_CH_COUNT] = PIT_IRQS;
    /* Table to save PIT clock source name */
    static clock_names_t s_pitClockName[PIT_INSTANCE_COUNT] = PIT_CLOCK_NAMES;
    /* The maximum value of compare register */
    #define PIT_COMPARE_MAX (PIT_LDVAL_TSV_MASK)
#endif

#if (defined(TIMING_OVER_STM))
    /* Table to save STM channel runtime state */
    static timer_chan_state_t s_stmState[STM_INSTANCE_COUNT][STM_CHANNEL_COUNT];
    /* Table to save STM IRQ number */
    static const IRQn_Type s_stmIrq[STM_INSTANCE_COUNT][STM_CHANNEL_COUNT] = STM_IRQS;
#if FEATURE_STM_HAS_CLOCK_SELECTION
    /* Table to save STM clock source name */
    static stm_clock_source_t s_stmClockSource[STM_INSTANCE_COUNT];
#endif
    /* Table to save STM clock source name */
    static clock_names_t s_stmClockSourceName[] = STM_CLOCK_NAMES;
    /* Table to save STM prescaler */
    static uint8_t s_stmPrescaler[STM_INSTANCE_COUNT];
    /* The maximum value of compare register */
    #define STM_COMPARE_MAX (STM_CMP_CMP_MASK)
#endif

#if (defined(TIMING_OVER_ETIMER))
    /* Table to save ETIMER channel runtime state */
    static timer_chan_state_t s_etimerState[ETIMER_INSTANCE_COUNT][ETIMER_CH_COUNT];
    /* Table to save ETIMER IRQ number */
    static const IRQn_Type s_etimerIrq[ETIMER_INSTANCE_COUNT][ETIMER_CH_MAX_IRQS] = ETIMER_IRQS;
    /* Table to save ETIMER clock source name */
    static clock_names_t s_etimerClockName[ETIMER_INSTANCE_COUNT] = ETIMER_CLOCK_NAMES;
    /* Table to save ETIMER prescaler */
    static etimer_input_source_t s_etimerPrescaler[ETIMER_INSTANCE_COUNT];
    /* The maximum value of compare register */
    #define ETIMER_COMPARE_MAX (ETIMER_LOAD_LOAD_MASK)
#endif
/*******************************************************************************
 * Private Functions
 ******************************************************************************/
/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_InitChannelState
 * Description   : This function initializes timer channel state to default value .
 *
 *END**************************************************************************/
static void TIMING_InitChannelState(timer_chan_state_t * channelState, uint32_t channelNum)
{
    DEV_ASSERT(channelState != NULL);
    timer_chan_state_t * state = channelState;
    uint32_t channel = 0U;

    /* Initialize channel state to default value */
    for (channel = 0U; channel < channelNum; channel++)
    {
        state->period = 0U;
        state->chanStartVal = 0U;
        state->chanType = TIMER_CHAN_TYPE_CONTINUOUS;
        state->callback = NULL;
        state->callbackParam = NULL;
        state->enableNotification = false;
        state++;
    }
}

#if (defined(TIMING_OVER_LPIT))
/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_InitLpit
 * Description   : This function initializes TIMING over LPIT.
 *END**************************************************************************/
static status_t TIMING_InitLpit(uint32_t instance,
                                const timer_config_t * config)
{
    DEV_ASSERT(instance < LPIT_INSTANCE_COUNT);

    status_t status = STATUS_SUCCESS;
    lpit_user_config_t lpitConfig;
    lpit_user_channel_config_t channelConfig;
    timer_chan_state_t * channelState;
    uint32_t chanIndex;
    uint8_t index;

    /* Set global structure */
    lpitConfig.enableRunInDebug = true;
    lpitConfig.enableRunInDoze = true;
    /* Set channel configuration structure */
    channelConfig.timerMode = LPIT_PERIODIC_COUNTER;
    channelConfig.periodUnits = LPIT_PERIOD_UNITS_COUNTS;
    channelConfig.period = 0U;
    channelConfig.triggerSource = LPIT_TRIGGER_SOURCE_EXTERNAL;
    channelConfig.triggerSelect = 0U;
    channelConfig.enableReloadOnTrigger = false;
    channelConfig.enableStopOnInterrupt = false;
    channelConfig.enableStartOnTrigger = false;
    channelConfig.chainChannel = false;
    channelConfig.isInterruptEnabled = true;
    /* Initialize LPIT instance */
    LPIT_DRV_Init(instance, &lpitConfig);
    /* Initialize LPIT channels */
    for (index = 0U; index < config->numChan; index++)
    {
        chanIndex = config->chanConfigArray[index].channel;
        channelState = &s_lpitState[instance][chanIndex];
        /* Initialize LPIT channels */
        status = LPIT_DRV_InitChannel(instance, chanIndex, &channelConfig);
        /* Save runtime state structure of timer channel */
        channelState->chanType = config->chanConfigArray[index].chanType;
        channelState->callback = config->chanConfigArray[index].callback;
        channelState->callbackParam = config->chanConfigArray[index].callbackParam;
        channelState->enableNotification = false;
        /* Enable LPIT interrupt */
        INT_SYS_EnableIRQ(s_lpitIrq[instance][chanIndex]);
    }

    return status;
}
#endif

#if (defined(TIMING_OVER_LPTMR))
/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_InitLptmr
 * Description   : This function initializes TIMING over LPTMR.
 *END**************************************************************************/
static status_t TIMING_InitLptmr(uint32_t instance,
                                 const timer_config_t * config)
{
    DEV_ASSERT(instance < LPTMR_INSTANCE_COUNT);

    status_t status = STATUS_SUCCESS;
    lptmr_config_t lptmrConfig;
    timer_chan_state_t * channelState;
    uint8_t chanIndex;
    uint8_t index;

    /* Set lptmr structure */
    lptmrConfig.dmaRequest = false;
    lptmrConfig.interruptEnable = true;
    lptmrConfig.freeRun = false;
    lptmrConfig.workMode = LPTMR_WORKMODE_TIMER;
    lptmrConfig.clockSelect = ((extension_lptmr_for_timer_t*)(config->extension))->clockSelect;
    lptmrConfig.prescaler = ((extension_lptmr_for_timer_t*)(config->extension))->prescaler;
    lptmrConfig.bypassPrescaler = ((extension_lptmr_for_timer_t*)(config->extension))->bypassPrescaler;
    lptmrConfig.counterUnits = LPTMR_COUNTER_UNITS_TICKS;
    lptmrConfig.pinSelect = LPTMR_PINSELECT_TRGMUX;
    lptmrConfig.pinPolarity = LPTMR_PINPOLARITY_RISING;
    lptmrConfig.compareValue = 0U;
    /* Save LPTMR clock source name */
    s_lptmrClockSource[instance] = ((extension_lptmr_for_timer_t*)(config->extension))->clockSelect;
    /* Save LPTMR prescaler */
    s_lptmrPrescaler[instance] = ((extension_lptmr_for_timer_t*)(config->extension))->prescaler;
    /* Save LPTMR bypass enable */
    s_lptmrBypassPrescaler[instance] = ((extension_lptmr_for_timer_t*)(config->extension))->bypassPrescaler;
    /* Initialize LPTMR instance */
    LPTMR_DRV_Init(instance, &lptmrConfig, false);

    for (index = 0U; index < config->numChan; index++)
    {
        chanIndex = config->chanConfigArray[index].channel;
        DEV_ASSERT(chanIndex < LPTMR_TMR_COUNT);
        channelState = &s_lptmrState[instance][chanIndex];
        /* Save runtime state structure of timer channel */
        channelState->chanType = config->chanConfigArray[index].chanType;
        channelState->callback = config->chanConfigArray[index].callback;
        channelState->callbackParam = config->chanConfigArray[index].callbackParam;
        channelState->enableNotification = false;
        /* Enable LPTMR interrupt */
        INT_SYS_EnableIRQ(s_lptmrIrq[instance][chanIndex]);
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_GetLptmrFreq
 * Description   : This function gets LPTMR clock frequency.
 *END**************************************************************************/
static void TIMING_GetLptmrFreq(uint32_t instance, uint32_t * frequency, uint64_t * prescaler)
{
    status_t clkErr;
    clock_names_t inputClockName = SIRCDIV2_CLK;

    switch(s_lptmrClockSource[instance])
    {
    case LPTMR_CLOCKSOURCE_SIRCDIV2:
        inputClockName = SIRCDIV2_CLK;
        break;
    case LPTMR_CLOCKSOURCE_1KHZ_LPO:
        inputClockName = SIM_LPO_1K_CLK;
        break;
    case LPTMR_CLOCKSOURCE_RTC:
        inputClockName = SIM_RTCCLK_CLK;
        break;
    case LPTMR_CLOCKSOURCE_PCC:
        inputClockName = LPTMR0_CLK;
        break;
    default:
        /* Invalid clock source */
        DEV_ASSERT(false);
        break;
    }
    /* Gets current functional clock frequency of LPTMR instance */
    clkErr = CLOCK_SYS_GetFreq(inputClockName, frequency);
    /* Checks the functional clock module is available */
    (void)clkErr;
    DEV_ASSERT(clkErr == STATUS_SUCCESS);
    DEV_ASSERT(*frequency > 0U);

    if (!s_lptmrBypassPrescaler[instance])
    {
        *prescaler = (*prescaler) << ((uint8_t)s_lptmrPrescaler[instance] + 1U);
    }
}
#endif

#if (defined(TIMING_OVER_FTM))
/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_InitFtm
 * Description   : This function initializes TIMING over FTM.
 *END**************************************************************************/
static status_t TIMING_InitFtm(uint32_t instance,
                               const timer_config_t * config)
{
    DEV_ASSERT(instance < FTM_INSTANCE_COUNT);

    status_t status = STATUS_SUCCESS;
    static ftm_state_t ftmState[FTM_INSTANCE_COUNT];
    ftm_user_config_t ftmConfig;
    ftm_output_cmp_param_t outputCmpConfig;
    ftm_output_cmp_ch_param_t chnConfigArray[FEATURE_FTM_CHANNEL_COUNT];
    timer_chan_state_t * channelState;
    uint8_t chanIndex;
    uint8_t index;

    /* Set ftm structure */
    ftmConfig.syncMethod.softwareSync = true;
    ftmConfig.syncMethod.hardwareSync0 = false;
    ftmConfig.syncMethod.hardwareSync1 = false;
    ftmConfig.syncMethod.hardwareSync2 = false;
    ftmConfig.syncMethod.maxLoadingPoint = true;
    ftmConfig.syncMethod.minLoadingPoint = false;
    ftmConfig.syncMethod.inverterSync = FTM_SYSTEM_CLOCK;
    ftmConfig.syncMethod.outRegSync = FTM_SYSTEM_CLOCK;
    ftmConfig.syncMethod.maskRegSync = FTM_SYSTEM_CLOCK;
    ftmConfig.syncMethod.initCounterSync = FTM_SYSTEM_CLOCK;
    ftmConfig.syncMethod.autoClearTrigger = false;
    ftmConfig.syncMethod.syncPoint = FTM_UPDATE_NOW;
    ftmConfig.ftmMode = FTM_MODE_OUTPUT_COMPARE;
    ftmConfig.ftmPrescaler = ((extension_ftm_for_timer_t*)(config->extension))->prescaler;
    ftmConfig.ftmClockSource = ((extension_ftm_for_timer_t*)(config->extension))->clockSelect;
    ftmConfig.BDMMode = FTM_BDM_MODE_00;
    ftmConfig.isTofIsrEnabled = false;
    ftmConfig.enableInitializationTrigger = false;

    /* Initialize running status of FTM channels */
    for (index = 0U; index < FEATURE_FTM_CHANNEL_COUNT; index++)
    {
        g_ftmChannelRunning[instance][index] = false;
    }
    /* Set output compare configuration structure */
    for (index = 0U; index < config->numChan; index++)
    {
        chnConfigArray[index].hwChannelId = config->chanConfigArray[index].channel;
        chnConfigArray[index].chMode = FTM_TOGGLE_ON_MATCH;
        chnConfigArray[index].comparedValue = FTM_COMPARE_MAX;
        chnConfigArray[index].enableExternalTrigger = false;
    }
    outputCmpConfig.nNumOutputChannels = config->numChan;
    outputCmpConfig.mode = FTM_MODE_OUTPUT_COMPARE;
    outputCmpConfig.maxCountValue = ((extension_ftm_for_timer_t*)(config->extension))->finalValue;
    outputCmpConfig.outputChannelConfig = chnConfigArray;

    /* Initialize FTM instance */
    status = FTM_DRV_Init(instance, &ftmConfig, &ftmState[instance]);
    status |= FTM_DRV_InitOutputCompare(instance, &outputCmpConfig);

    if (status == STATUS_SUCCESS)
    {
        for (index = 0U; index < config->numChan; index++)
        {
            chanIndex = config->chanConfigArray[index].channel;
            channelState = &s_ftmState[instance][chanIndex];
            /* Save runtime state structure of timer channel */
            channelState->chanType = config->chanConfigArray[index].chanType;
            channelState->callback = config->chanConfigArray[index].callback;
            channelState->callbackParam = config->chanConfigArray[index].callbackParam;
            channelState->enableNotification = false;
            /* Install FTM irq handler */
            INT_SYS_InstallHandler(s_ftmIrq[instance][chanIndex],
                                   s_timingOverFtmIsr[instance][chanIndex], (isr_t*)0);
            /* Enable FTM interrupt */
            INT_SYS_EnableIRQ(s_ftmIrq[instance][chanIndex]);
        }
    }
    else
    {
        status = STATUS_ERROR;
    }

    return status;
}
#endif

#if (defined(TIMING_OVER_PIT))
/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_InitPit
 * Description   : This function initializes TIMING over PIT.
 *END**************************************************************************/
static status_t TIMING_InitPit(uint32_t instance,
                               const timer_config_t * config)
{
    DEV_ASSERT(instance < PIT_INSTANCE_COUNT);

    status_t status = STATUS_SUCCESS;
    pit_config_t pitConfig;
    pit_channel_config_t channelConfig;
    timer_chan_state_t * channelState;
    uint8_t chanIndex;
    uint8_t index;

    /* Set global structure */
#if FEATURE_PIT_HAS_RTI_CHANNEL
    pitConfig.enableRTITimer = false;
#endif
    pitConfig.enableStandardTimers = true;
    /* This is workaround for the issue: PIT and STM modules can not work if the stop in debug mode is enabled */
#ifndef TIMING_PAL_HAS_PIT_STM_NOT_WORK_IN_DEBUG_MODE
    pitConfig.stopRunInDebug = true;
#else
    pitConfig.stopRunInDebug = false;
#endif
    /* Set channel configuration structure */
    channelConfig.hwChannel = 0U;
    channelConfig.periodUnit = PIT_PERIOD_UNITS_COUNTS;
    channelConfig.period = 0U;
    channelConfig.enableChain = false;
    channelConfig.enableInterrupt = true;

    /* Initialize PIT instance */
    PIT_DRV_Init(instance, &pitConfig);

    for (index = 0U; index < config->numChan; index++)
    {
        chanIndex = config->chanConfigArray[index].channel;
        channelState = &s_pitState[instance][chanIndex];
        /* Initialize PIT channels */
        channelConfig.hwChannel = chanIndex;
        status = PIT_DRV_InitChannel(instance, &channelConfig);
        /* Save runtime state structure of timer channel */
        channelState->chanType = config->chanConfigArray[index].chanType;
        channelState->callback = config->chanConfigArray[index].callback;
        channelState->callbackParam = config->chanConfigArray[index].callbackParam;
        channelState->enableNotification = false;
        /* Enable PIT interrupt */
        INT_SYS_EnableIRQ(s_pitIrq[instance][chanIndex]);
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_IsPitChannelRunning
 * Description   : This function checks whether the timer channel is running .
 *
 *END**************************************************************************/
static bool TIMING_IsPitChannelRunning(uint32_t instance,
                                       const uint8_t channel)
{
    DEV_ASSERT(instance < PIT_INSTANCE_COUNT);
    DEV_ASSERT(channel < PIT_TIMER_COUNT);
    /* Table of base addresses for PIT instances */
    static PIT_Type * const pitBase[PIT_INSTANCE_COUNT] = PIT_BASE_PTRS;
    const PIT_Type * base;
    bool isRunning;
    base = pitBase[instance];
    isRunning = ((base->TIMER[channel].TCTRL & PIT_TCTRL_TEN_MASK) == 1u) ? true : false;
    return isRunning;
}
#endif

#if (defined(TIMING_OVER_STM))
/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_InitStm
 * Description   : This function initializes TIMING over STM.
 *END**************************************************************************/
static status_t TIMING_InitStm(uint32_t instance,
                               const timer_config_t * config)
{
    DEV_ASSERT(instance < STM_INSTANCE_COUNT);

    status_t status = STATUS_SUCCESS;
    stm_config_t stmConfig;
    timer_chan_state_t * channelState;
    uint8_t chanIndex;
    uint8_t index;

    /* Set stm structure */
#if FEATURE_STM_HAS_CLOCK_SELECTION
    stmConfig.clockSource = ((extension_stm_for_timer_t*)(config->extension))->clockSelect;
    /* Save LPTMR clock source name */
    s_stmClockSource[instance] = ((extension_stm_for_timer_t*)(config->extension))->clockSelect;
#endif
    stmConfig.clockPrescaler = ((extension_stm_for_timer_t*)(config->extension))->prescaler;

    /* This is workaround for the issue: PIT and STM modules can not work if the stop in debug mode is enabled */
#ifndef TIMING_PAL_HAS_PIT_STM_NOT_WORK_IN_DEBUG_MODE
    stmConfig.stopInDebugMode = true;
#else
    stmConfig.stopInDebugMode = false;
#endif

    stmConfig.startValue = 0U;
    /* Save LPTMR prescaler */
    s_stmPrescaler[instance] = ((extension_stm_for_timer_t*)(config->extension))->prescaler;
    /* Initialize STM instance */
    STM_DRV_Init(instance, &stmConfig);

    for (index = 0U; index < config->numChan; index++)
    {
        chanIndex = config->chanConfigArray[index].channel;
        channelState = &s_stmState[instance][chanIndex];
        /* Save runtime state structure of timer channel */
        channelState->chanType = config->chanConfigArray[index].chanType;
        channelState->callback = config->chanConfigArray[index].callback;
        channelState->callbackParam = config->chanConfigArray[index].callbackParam;
        channelState->enableNotification = false;
        /* Enable STM interrupt */
        INT_SYS_EnableIRQ(s_stmIrq[instance][chanIndex]);
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_GetStmFreq
 * Description   : This function gets STM clock frequency.
 *END**************************************************************************/
static void TIMING_GetStmFreq(uint32_t instance, uint32_t * frequency, uint64_t * prescaler)
{

    status_t clkErr;
    clock_names_t inputClockName = END_OF_PERIPHERAL_CLKS;

#if FEATURE_STM_HAS_CLOCK_SELECTION
    switch(s_stmClockSource[instance])
    {
    case STM_CLOCK_SYSTEM:
        inputClockName = s_stmClockSourceName[0];
        break;
    case STM_CLOCK_FXOSC:
        inputClockName = s_stmClockSourceName[1];
        break;
    default:
        /* Invalid clock source */
        DEV_ASSERT(false);
        break;
    }
#else
    inputClockName = s_stmClockSourceName[0];
#endif
    /* Gets current functional clock frequency of STM instance */
    clkErr = CLOCK_SYS_GetFreq(inputClockName, frequency);
    /* Checks the functional clock module is available */
    (void)clkErr;
    DEV_ASSERT(clkErr == STATUS_SUCCESS);
    DEV_ASSERT(*frequency > 0U);
    *prescaler = (uint64_t)s_stmPrescaler[instance] + 1UL;
}
#endif

#if (defined(TIMING_OVER_ETIMER))
/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_InitEtimer
 * Description   : This function initializes TIMING over ETIMER.
 *END**************************************************************************/
static status_t TIMING_InitEtimer(uint32_t instance,
                                  const timer_config_t * config)
{
    DEV_ASSERT(instance < ETIMER_INSTANCE_COUNT);

    status_t status = STATUS_SUCCESS;
    etimer_user_channel_config_t channelConfig;
    timer_chan_state_t * channelState;
    uint8_t chanIndex;
    uint8_t index;

    /* Set channel configuration structure */
    ETIMER_DRV_GetDefaultChannelConfig(&channelConfig);
    channelConfig.primaryInput.source = ((extension_etimer_for_timer_t*)(config->extension))->clockSelect;
    channelConfig.compareMode = ETIMER_CMPMODE_COMP1_UP_COMP2_DOWN;
    channelConfig.interruptEnableMask = ETIMER_INTDMA_TCF1IE_MASK;
    channelConfig.countOnce = false;
    channelConfig.countLength = true;
    channelConfig.compareValues[0] = 0xFFFF;
    channelConfig.compareValues[1] = 0xFFFF;
    /* Save ETIMER prescaler */
    s_etimerPrescaler[instance] = ((extension_etimer_for_timer_t*)(config->extension))->clockSelect;
    /* Initialize ETIMER instance */
    ETIMER_DRV_Init((uint16_t)instance);
    /* Initialize ETIMER channels */
    for (index = 0U; index < config->numChan; index++)
    {
        chanIndex = config->chanConfigArray[index].channel;
        channelState = &s_etimerState[instance][chanIndex];
        /* Initialize ETIMER channels */
        ETIMER_DRV_InitChannel((uint16_t)instance, chanIndex, &channelConfig);
        /* Counter is halt when enters debug mode */
        ETIMER_DRV_ChannelDebugBehaviour((uint16_t)instance, chanIndex, ETIMER_DEBUG_HALT);
        /* Save runtime state structure of timer channel */
        channelState->chanType = config->chanConfigArray[index].chanType;
        channelState->callback = config->chanConfigArray[index].callback;
        channelState->callbackParam = config->chanConfigArray[index].callbackParam;
        channelState->enableNotification = false;
        /* Install ETIMER irq handler */
        INT_SYS_InstallHandler(s_etimerIrq[instance][chanIndex],
                               s_timingOverEtimerIsr[instance][chanIndex], (isr_t*)0);
        /* Enable ETIMER channel interrupt */
        INT_SYS_EnableIRQ(s_etimerIrq[instance][chanIndex]);
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_GetEtimerFreq
 * Description   : This function gets ETIMER clock frequency.
 *END**************************************************************************/
static void TIMING_GetEtimerFreq(uint32_t instance, uint32_t * frequency, uint64_t * prescaler)
{
    status_t clkErr;

    /* Gets current functional clock frequency of ETIMER instance */
    clkErr = CLOCK_SYS_GetFreq(s_etimerClockName[instance], frequency);
    /* Checks the functional clock module is available */
    (void)clkErr;
    DEV_ASSERT(clkErr == STATUS_SUCCESS);
    DEV_ASSERT(*frequency > 0U);

    switch(s_etimerPrescaler[instance])
    {
    case ETIMER_IN_SRC_CLK_DIV_1:
        *prescaler = 1;
        break;
    case ETIMER_IN_SRC_CLK_DIV_2:
        *prescaler = 2;
        break;
    case ETIMER_IN_SRC_CLK_DIV_4:
        *prescaler = 4;
        break;
    case ETIMER_IN_SRC_CLK_DIV_8:
        *prescaler = 8;
        break;
    case ETIMER_IN_SRC_CLK_DIV_16:
        *prescaler = 16;
        break;
    case ETIMER_IN_SRC_CLK_DIV_32:
        *prescaler = 32;
        break;
    case ETIMER_IN_SRC_CLK_DIV_64:
        *prescaler = 64;
        break;
    case ETIMER_IN_SRC_CLK_DIV_128:
        *prescaler = 128;
        break;
    default:
        /* Invalid clock source */
        DEV_ASSERT(false);
        break;
    }
}
#endif
/*******************************************************************************
 * FUCNTION FUNCTION
 ******************************************************************************/

/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_Init
 * Description   : Initialize timer instance.
 * This function initializes clock source, prescaler of the timer instance(except LPIT, PIT), the final
 * value of counter (only FTM). This function also setups notification type and callback function of timer channel.
 * The timer instance number and its configuration structure shall be passed as arguments.
 * Timer channels do not start counting by default after calling this function.
 * The function TIMING_StartChannel must be called to start the timer channel counting.
 *
 * Implements    : TIMING_Init_Activity
 *END**************************************************************************/
status_t TIMING_Init(const timing_instance_t * const instance,
                     const timer_config_t * const config)
{
    DEV_ASSERT(instance != NULL);
    DEV_ASSERT(config != NULL);
    status_t status = STATUS_ERROR;
    /* Define TIMING PAL over LPIT */
#if (defined (TIMING_OVER_LPIT))
    if (instance->instType == TIMING_INST_TYPE_LPIT)
    {
        DEV_ASSERT(instance->instIdx < LPIT_INSTANCE_COUNT);
        /* Initialize channel state to default value */
        TIMING_InitChannelState(&s_lpitState[instance->instIdx][0], LPIT_TMR_COUNT);
        /* Initialize TIMING over LPIT */
        status = TIMING_InitLpit(instance->instIdx, config);
    }
    else
#endif

    /* Define TIMING PAL over LPTMR */
#if (defined (TIMING_OVER_LPTMR))
    if (instance->instType == TIMING_INST_TYPE_LPTMR)
    {
        DEV_ASSERT(config->extension != NULL);
        DEV_ASSERT(instance->instIdx < LPTMR_INSTANCE_COUNT);
        /* Initialize channel state to default value */
        TIMING_InitChannelState(&s_lptmrState[instance->instIdx][0], LPTMR_TMR_COUNT);
        /* Initialize TIMING over LPTMR */
        status = TIMING_InitLptmr(instance->instIdx, config);
    }
    else
#endif

    /* Define TIMING PAL over FTM */
#if (defined (TIMING_OVER_FTM))
    if (instance->instType == TIMING_INST_TYPE_FTM)
    {
        DEV_ASSERT(config->extension != NULL);
        DEV_ASSERT(instance->instIdx < FTM_INSTANCE_COUNT);
        /* Initialize channel state to default value */
        TIMING_InitChannelState(&s_ftmState[instance->instIdx][0], FEATURE_FTM_CHANNEL_COUNT);
        /* Initialize TIMING over FTM */
        status = TIMING_InitFtm(instance->instIdx, config);
    }
    else
#endif

    /* Define TIMING PAL over PIT */
#if (defined (TIMING_OVER_PIT))
    if (instance->instType == TIMING_INST_TYPE_PIT)
    {
        DEV_ASSERT(instance->instIdx < PIT_INSTANCE_COUNT);
        /* Initialize channel state to default value */
        TIMING_InitChannelState(&s_pitState[instance->instIdx][0], PIT_TIMER_COUNT);
        /* Initialize TIMING over LPIT */
        status = TIMING_InitPit(instance->instIdx, config);
    }
    else
#endif

    /* Define TIMING PAL over STM */
#if (defined (TIMING_OVER_STM))
    if (instance->instType == TIMING_INST_TYPE_STM)
    {
        DEV_ASSERT(config->extension != NULL);
        DEV_ASSERT(instance->instIdx < STM_INSTANCE_COUNT);
        /* Initialize channel state to default value */
        TIMING_InitChannelState(&s_stmState[instance->instIdx][0], STM_CHANNEL_COUNT);
        /* Initialize TIMING over STM */
        status = TIMING_InitStm(instance->instIdx, config);
    }
    else
#endif

#if (defined (TIMING_OVER_ETIMER))
    if (instance->instType == TIMING_INST_TYPE_ETIMER)
    {
        DEV_ASSERT(config->extension != NULL);
        DEV_ASSERT(instance->instIdx < ETIMER_INSTANCE_COUNT);
        /* Initialize channel state to default value */
        TIMING_InitChannelState(&s_etimerState[instance->instIdx][0], ETIMER_CH_COUNT);
        /* Initialize TIMING over ETIMER */
        status = TIMING_InitEtimer(instance->instIdx, config);
    }
    else
#endif
    {
        /* Do nothing */
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_Deinit
 * Description   : De-initialize timer instance.
 * This function de-initializes timer instance.
 * In order to use the timer instance again, TIMING_Init must be called.
 *
 * Implements    : TIMING_Deinit_Activity
 *END**************************************************************************/
void TIMING_Deinit(const timing_instance_t * const instance)
{
    DEV_ASSERT(instance != NULL);

    /* Define TIMING PAL over LPIT */
#if (defined (TIMING_OVER_LPIT))
    if (instance->instType == TIMING_INST_TYPE_LPIT)
    {
        DEV_ASSERT(instance->instIdx < LPIT_INSTANCE_COUNT);
        /* De-Initialize LPIT instance*/
        LPIT_DRV_Deinit(instance->instIdx);
        /* Set channel state to default value */
        TIMING_InitChannelState(&s_lpitState[instance->instIdx][0], LPIT_TMR_COUNT);
    }
    else
#endif

    /* Define TIMING PAL over LPTMR */
#if (defined (TIMING_OVER_LPTMR))
    if (instance->instType == TIMING_INST_TYPE_LPTMR)
    {
        DEV_ASSERT(instance->instIdx < LPTMR_INSTANCE_COUNT);
        /* De-Initialize LPTMR instance*/
        LPTMR_DRV_Deinit(instance->instIdx);
        /* Set channel state to default value */
        TIMING_InitChannelState(&s_lptmrState[instance->instIdx][0], LPTMR_TMR_COUNT);
        s_lptmrClockSource[instance->instIdx] = LPTMR_CLOCKSOURCE_SIRCDIV2;
        s_lptmrPrescaler[instance->instIdx] = LPTMR_PRESCALE_2;
        s_lptmrBypassPrescaler[instance->instIdx] = false;
    }
    else
#endif

    /* Define TIMING PAL over FTM */
#if (defined (TIMING_OVER_FTM))
    if (instance->instType == TIMING_INST_TYPE_FTM)
    {
        DEV_ASSERT(instance->instIdx < FTM_INSTANCE_COUNT);
        status_t retVal;
        /* De-Initialize FTM instance*/
        retVal = FTM_DRV_Deinit(instance->instIdx);
        (void)retVal;
        /* Set channel state to default value */
        TIMING_InitChannelState(&s_ftmState[instance->instIdx][0], FEATURE_FTM_CHANNEL_COUNT);
    }
    else
#endif

    /* Define TIMING PAL over PIT */
#if (defined (TIMING_OVER_PIT))
    if (instance->instType == TIMING_INST_TYPE_PIT)
    {
        DEV_ASSERT(instance->instIdx < PIT_INSTANCE_COUNT);
        /* De-Initialize PIT instance*/
        PIT_DRV_Deinit(instance->instIdx);
        /* Set channel state to default value */
        TIMING_InitChannelState(&s_pitState[instance->instIdx][0], PIT_TIMER_COUNT);
    }
    else
#endif

    /* Define TIMING PAL over STM */
#if (defined (TIMING_OVER_STM))
    if (instance->instType == TIMING_INST_TYPE_STM)
    {
        DEV_ASSERT(instance->instIdx < STM_INSTANCE_COUNT);
        /* De-Initialize STM instance*/
        STM_DRV_Deinit(instance->instIdx);
        /* Set channel state to default value */
        TIMING_InitChannelState(&s_stmState[instance->instIdx][0], STM_CHANNEL_COUNT);
#if FEATURE_STM_HAS_CLOCK_SELECTION
        s_stmClockSource[instance->instIdx] = STM_CLOCK_SYSTEM;
#endif
        s_stmPrescaler[instance->instIdx] = 0U;
    }
    else
#endif

#if (defined (TIMING_OVER_ETIMER))
    if (instance->instType == TIMING_INST_TYPE_ETIMER)
    {
        DEV_ASSERT(instance->instIdx < ETIMER_INSTANCE_COUNT);
        /* De-Initialize ETIMER instance*/
        ETIMER_DRV_Deinit((uint16_t)(instance->instIdx));
        /* Set channel state to default value */
        TIMING_InitChannelState(&s_etimerState[instance->instIdx][0], ETIMER_CH_COUNT);
        s_etimerPrescaler[instance->instIdx] = ETIMER_IN_SRC_CNT0_IN;
    }
    else
#endif
    {
        DEV_ASSERT(false);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_StartChannel
 * Description   : Starts the timer channel counting.
 * This function starts channel counting with a new period in ticks.
 * Note that:
 * - If the timer is PIT or LPIT, if timer channel is running with a period, this function will not immediately restart
 *   timer channel with a new period, instead the new period will be loaded after the current period expires.
 *   In order to abort the current timer channel period and start timer channel with a new period, the timer channel
 *   must be stopped and started again with new period.
 *   Additional, if call this function with new period while timer channel is running, calling TIMING_GetElapsed and TIMING_GetRemaining
 *   may return unexpected result.
 * - If the timer is FTM, this function start channel by enable channel interrupt generation.
 * - LPTMR and FTM is 16 bit timer, so the input period must be smaller than 65535.
 * - LPTMR and FTM is 16 bit timer, so the input period must be smaller than 65535.
 *
 * Implements    : TIMING_StartChannel_Activity
 *END**************************************************************************/
void TIMING_StartChannel(const timing_instance_t * const instance,
                         const uint8_t channel,
                         const uint32_t periodTicks)
{
    DEV_ASSERT(instance != NULL);

    timer_chan_state_t * channelState;

    /* Define TIMING PAL over LPIT */
#if (defined (TIMING_OVER_LPIT))
    if (instance->instType == TIMING_INST_TYPE_LPIT)
    {
        uint32_t channelMask = 1UL << channel;

        /* Set the channel compare value */
        LPIT_DRV_SetTimerPeriodByCount(instance->instIdx, channel, periodTicks);
        /* Start the channel counting */
        LPIT_DRV_StartTimerChannels(instance->instIdx, channelMask);

        channelState = &s_lpitState[instance->instIdx][channel];
        /* Save the period of channel */
        channelState->period = periodTicks;
        /* Enable notification */
        channelState->enableNotification = true;
    }
    else
#endif

    /* Define TIMING PAL over LPTMR */
#if (defined (TIMING_OVER_LPTMR))
    if (instance->instType == TIMING_INST_TYPE_LPTMR)
    {
        DEV_ASSERT(periodTicks <= LPTMR_COMPARE_MAX);
        DEV_ASSERT(channel < LPTMR_TMR_COUNT);
        uint32_t lptmrInstance = instance->instIdx;
        status_t retVal;

        /* Stop the channel counting */
        LPTMR_DRV_StopCounter(lptmrInstance);
        /* Set the channel compare value */
        retVal = LPTMR_DRV_SetCompareValueByCount(lptmrInstance, (uint16_t)periodTicks);
        (void)retVal;
        /* Start the channel counting */
        LPTMR_DRV_StartCounter(lptmrInstance);

        channelState = &s_lptmrState[lptmrInstance][channel];
        /* Save the period of channel */
        channelState->period = periodTicks;
        /* Enable notification */
        channelState->enableNotification = true;
    }
    else
#endif

    /* Define TIMING PAL over FTM */
#if (defined (TIMING_OVER_FTM))
    if (instance->instType == TIMING_INST_TYPE_FTM)
    {
        uint32_t ftmInstance = instance->instIdx;
        FTM_Type * const base = ftmBase[ftmInstance];
        uint32_t currentCounter;
        status_t retVal;

        DEV_ASSERT(periodTicks <= FTM_DRV_GetMod(base));

        /* Clear the channel interrupt flag which may be set after executed initialization timing function or a previous channel match event */
        FTM_DRV_ClearChnEventStatus(base, channel);
        /* Get current counter*/
        currentCounter = FTM_DRV_GetCounter(base);
        /* Update compare value of the channel */
        retVal = FTM_DRV_UpdateOutputCompareChannel(ftmInstance, channel, (uint16_t)periodTicks, FTM_RELATIVE_VALUE, false);
        /* Enable the channel by enable interrupt generation */
        retVal = FTM_DRV_EnableInterrupts(ftmInstance, (1UL << channel));
        /* Update channel running status */
        g_ftmChannelRunning[ftmInstance][channel] = true;
        (void)retVal;
        /* Save the start value of channel at the moment the start channel function is called */
        channelState = &s_ftmState[ftmInstance][channel];
        channelState->chanStartVal = currentCounter;
        /* Save the period of channel */
        channelState->period = periodTicks;
        /* Enable notification */
        channelState->enableNotification = true;
    }
    else
#endif

    /* Define TIMING PAL over PIT */
#if (defined (TIMING_OVER_PIT))
    if (instance->instType == TIMING_INST_TYPE_PIT)
    {
        const uint32_t pitInstance = instance->instIdx;
        /* Set the channel compare value */
        PIT_DRV_SetTimerPeriodByCount(pitInstance, channel, periodTicks);
        /* Start the channel if the channel is not running */
        if (!TIMING_IsPitChannelRunning(pitInstance, channel))
        {
            PIT_DRV_StartChannel(pitInstance, channel);
        }
        else
        {
            /* Do nothing */
        }

        channelState = &s_pitState[pitInstance][channel];
        /* Save the period of channel */
        channelState->period = periodTicks;
        /* Enable notification */
        channelState->enableNotification = true;
    }
    else
#endif

    /* Define TIMING PAL over STM */
#if (defined (TIMING_OVER_STM))
    if (instance->instType == TIMING_INST_TYPE_STM)
    {
        uint32_t stmInstance = instance->instIdx;
        uint32_t currentCounter;
        uint32_t compareValue;

        /* Get current counter value */
        currentCounter = STM_DRV_GetCounterValue(stmInstance);
        /* Calculate the channel compare value */
        if ((STM_COMPARE_MAX - currentCounter) >= periodTicks)
        {
            /* The distance from current value to max of compare register is enough */
            compareValue = currentCounter + periodTicks;
        }
        else
        {
            /* The distance is not enough, calculates a new value for compare register */
            compareValue = periodTicks - (STM_COMPARE_MAX - currentCounter);
        }
        /* Configure channel compare value */
        STM_DRV_ConfigChannel(stmInstance, channel, compareValue);
        /* Start counter */
        STM_DRV_StartTimer(stmInstance);
        /* Save the start value of channel at the moment the start channel function is called */
        channelState = &s_stmState[stmInstance][channel];
        channelState->chanStartVal = currentCounter;
        /* Save the period of channel */
        channelState->period = periodTicks;
        /* Enable notification */
        channelState->enableNotification = true;
    }
    else
#endif

    /* Define TIMING PAL over ETIMER */
#if (defined (TIMING_OVER_ETIMER))
    if (instance->instType == TIMING_INST_TYPE_ETIMER)
    {
        DEV_ASSERT(periodTicks <= ETIMER_COMPARE_MAX);
        uint16_t channelMask = (uint16_t)(1UL << channel);

        /* Set the channel compare value */
        ETIMER_DRV_SetCompareThreshold((uint16_t)(instance->instIdx), channel, (uint16_t)periodTicks, 0xFFFFU);
        /* Reset counter to 0 */
        ETIMER_DRV_SetTimerTicks((uint16_t)(instance->instIdx), channel, 0U);
        /* Start the channel counting */
        ETIMER_DRV_StartTimerChannels((uint16_t)(instance->instIdx), channelMask);

        channelState = &s_etimerState[instance->instIdx][channel];
        /* Save the period of channel */
        channelState->period = periodTicks;
        /* Enable notification */
        channelState->enableNotification = true;
    }
    else
#endif
    {
        DEV_ASSERT(false);
    }

}

/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_StopChannel
 * Description   : Stop the timer channel counting.
 * This function stop channel counting. Note that if the timer is FTM,
 * this function stop channel by disable channel interrupt generation.
 *
 * Implements    : TIMING_StopChannel_Activity
 *END**************************************************************************/
void TIMING_StopChannel(const timing_instance_t * const instance,
                        const uint8_t channel)
{
    DEV_ASSERT(instance != NULL);

    timer_chan_state_t * channelState;

    /* Define TIMING PAL over LPIT */
#if (defined (TIMING_OVER_LPIT))
    if (instance->instType == TIMING_INST_TYPE_LPIT)
    {
        /* Stop the channel counting */
        LPIT_DRV_StopTimerChannels(instance->instIdx, (1UL << channel));

        channelState = &s_lpitState[instance->instIdx][channel];
        /* Disable notification */
        channelState->enableNotification = false;
    }
    else
#endif

    /* Define TIMING PAL over LPTMR */
#if (defined (TIMING_OVER_LPTMR))
    if (instance->instType == TIMING_INST_TYPE_LPTMR)
    {
        DEV_ASSERT(channel < LPTMR_TMR_COUNT);

        /* Stop the channel counting */
        LPTMR_DRV_StopCounter(instance->instIdx);

        channelState = &s_lptmrState[instance->instIdx][channel];
        /* Disable notification */
        channelState->enableNotification = false;
    }
    else
#endif

    /* Define TIMING PAL over FTM */
#if (defined (TIMING_OVER_FTM))
    if (instance->instType == TIMING_INST_TYPE_FTM)
    {
        DEV_ASSERT(channel < FEATURE_FTM_CHANNEL_COUNT);
        /* Stop the channel by disable interrupt generation */
        FTM_DRV_DisableInterrupts(instance->instIdx, (1UL << channel));
        /* Update channel running status */
        g_ftmChannelRunning[instance->instIdx][channel] = false;
        channelState = &s_ftmState[instance->instIdx][channel];
        /* Disable notification */
        channelState->enableNotification = false;
    }
    else
#endif

    /* Define TIMING PAL over PIT */
#if (defined (TIMING_OVER_PIT))
    if (instance->instType == TIMING_INST_TYPE_PIT)
    {
        /* Stop the channel counting */
        PIT_DRV_StopChannel(instance->instIdx, channel);

        channelState = &s_pitState[instance->instIdx][channel];
        /* Disable notification */
        channelState->enableNotification = false;
    }
    else
#endif

    /* Define TIMING PAL over STM */
#if (defined (TIMING_OVER_STM))
    if (instance->instType == TIMING_INST_TYPE_STM)
    {
        /* Stop the channel counting */
        STM_DRV_DisableChannel(instance->instIdx, channel);

         /* Save the start value of channel at the moment the start channel function is called */
        channelState = &s_stmState[instance->instIdx][channel];
        /* Enable notification */
        channelState->enableNotification = false;
    }
    else
#endif

    /* Define TIMING PAL over ETIMER */
#if (defined (TIMING_OVER_ETIMER))
    if (instance->instType == TIMING_INST_TYPE_ETIMER)
    {
        uint16_t channelMask = (uint16_t)(1UL << channel);
        /* Stop the channel counting */
        ETIMER_DRV_StopTimerChannels((uint16_t)(instance->instIdx), channelMask);

        channelState = &s_etimerState[instance->instIdx][channel];
        /* Disable notification */
        channelState->enableNotification = false;
    }
    else
#endif
    {
        DEV_ASSERT(false);
    }

}

/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_GetElapsed
 * Description   : Get elapsed ticks.
 * This function gets elapsed time since the last event by ticks. The elapsed time by nanosecond, microsecond or
 * millisecond is the result of this function multiplies by the result of the TIMING_GetResolution
 * function.
 *
 * Implements    : TIMING_GetElapsed_Activity
 *END**************************************************************************/
uint32_t TIMING_GetElapsed(const timing_instance_t * const instance,
                           const uint8_t channel)
{
    DEV_ASSERT(instance != NULL);

    uint32_t currentCounter = 0U;
    uint32_t timeElapsed = 0U;

    /* Define TIMING PAL over LPIT */
#if (defined (TIMING_OVER_LPIT))
    if (instance->instType == TIMING_INST_TYPE_LPIT)
    {
        const timer_chan_state_t * lpitChannelState;
        /* Get current channel counter value */
        currentCounter = LPIT_DRV_GetCurrentTimerCount(instance->instIdx, channel);

        lpitChannelState = &s_lpitState[instance->instIdx][channel];
        /* Calculate timer elapsed */
        timeElapsed = lpitChannelState->period - currentCounter;
    }
    else
#endif

    /* Define TIMING PAL over LPTMR */
#if (defined (TIMING_OVER_LPTMR))
    if (instance->instType == TIMING_INST_TYPE_LPTMR)
    {
        (void)channel;
        (void)currentCounter;

        /* Time elapsed is current counter value */
        timeElapsed = LPTMR_DRV_GetCounterValueByCount(instance->instIdx);
    }
    else
#endif

    /* Define TIMING PAL over FTM */
#if (defined (TIMING_OVER_FTM))
    if (instance->instType == TIMING_INST_TYPE_FTM)
    {
        DEV_ASSERT((instance->instIdx) < FTM_INSTANCE_COUNT);
        DEV_ASSERT(channel < FEATURE_FTM_CHANNEL_COUNT);

        uint32_t ftmInstance = instance->instIdx;
        const FTM_Type * const base = ftmBase[ftmInstance];
        uint16_t finalValue;
        const timer_chan_state_t * ftmChannelState;

        /* Get current FTM counter value */
        currentCounter = FTM_DRV_GetCounter(base);
        /* Get the final value of counter */
        finalValue = FTM_DRV_GetMod(base);
        ftmChannelState = &s_ftmState[ftmInstance][channel];
        /* Calculate time elapsed */
        if (currentCounter >= ftmChannelState->chanStartVal)
        {
            /* In case the counter is smaller than the final value of counter */
            timeElapsed = currentCounter - ftmChannelState->chanStartVal;
        }
        else
        {
            /* In case the counter is over the final value of counter */
            timeElapsed = (finalValue - ftmChannelState->chanStartVal) + currentCounter;
        }
    }
    else
#endif

    /* Define TIMING PAL over PIT */
#if (defined (TIMING_OVER_PIT))
    if (instance->instType == TIMING_INST_TYPE_PIT)
    {
        uint32_t pitInstance = instance->instIdx;
        const timer_chan_state_t * pitChannelState;

        /* Get current channel counter value */
        currentCounter = PIT_DRV_GetCurrentTimerCount(pitInstance, channel);

        pitChannelState = &s_pitState[pitInstance][channel];
        /* Calculate time elapsed */
        timeElapsed = pitChannelState->period - currentCounter;
    }
    else
#endif

    /* Define TIMING PAL over STM */
#if (defined (TIMING_OVER_STM))
    if (instance->instType == TIMING_INST_TYPE_STM)
    {
        DEV_ASSERT(channel < STM_CHANNEL_COUNT);

        uint32_t stmInstance = instance->instIdx;
        const timer_chan_state_t * stmChannelState;

        /* Get current counter value */
        currentCounter = STM_DRV_GetCounterValue(stmInstance);
        stmChannelState = &s_stmState[stmInstance][channel];
        /* Calculate time elapsed */
        if (currentCounter >= (stmChannelState->chanStartVal))
        {
            /* In case the counter is smaller than the final value of counter */
            timeElapsed = currentCounter - stmChannelState->chanStartVal;
        }
        else
        {
            /* In case the counter is over the final value of counter */
            timeElapsed = (STM_COMPARE_MAX - stmChannelState->chanStartVal) + currentCounter;
        }
    }
    else
#endif

/* Define TIMING PAL over ETIMER */
#if (defined (TIMING_OVER_ETIMER))
    if (instance->instType == TIMING_INST_TYPE_ETIMER)
    {
        (void)currentCounter;
        /* Get current channel counter value */
        timeElapsed = ETIMER_DRV_GetTimerTicks((uint16_t)(instance->instIdx), channel);
    }
    else
#endif
    {
        DEV_ASSERT(false);
    }

    return timeElapsed;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_GetRemaining
 * Description   : Get remaining ticks.
 * This function gets remaining time to next event by ticks. The remaining time by nanosecond, microsecond or
 * millisecond is the result of this function multiplies by the result of the TIMING_GetResolution
 * function.
 *
 * Implements    : TIMING_GetRemaining_Activity
 *END**************************************************************************/
uint32_t TIMING_GetRemaining(const timing_instance_t * const instance,
                             const uint8_t channel)
{
    DEV_ASSERT(instance != NULL);

    uint32_t timeElapsed = 0U;
    uint32_t timeRemain = 0U;

    /* Define TIMING PAL over LPIT */
#if (defined (TIMING_OVER_LPIT))
    if (instance->instType == TIMING_INST_TYPE_LPIT)
    {
        (void)timeElapsed;
        /* Get the remaining time */
        timeRemain = LPIT_DRV_GetCurrentTimerCount(instance->instIdx, channel);
    }
    else
#endif

    /* Define TIMING PAL over LPTMR */
#if (defined (TIMING_OVER_LPTMR))
    if (instance->instType == TIMING_INST_TYPE_LPTMR)
    {
        DEV_ASSERT(channel < LPTMR_TMR_COUNT);
        uint32_t lptmrInstance = instance->instIdx;
        const timer_chan_state_t * lptmrChannelState;

        /* Time elapsed is current counter value */
        timeElapsed = LPTMR_DRV_GetCounterValueByCount(lptmrInstance);

        lptmrChannelState = &s_lptmrState[lptmrInstance][channel];
        /* Calculate the remaining time */
        timeRemain = lptmrChannelState->period - timeElapsed;
    }
    else
#endif

    /* Define TIMING PAL over FTM */
#if (defined (TIMING_OVER_FTM))
    if (instance->instType == TIMING_INST_TYPE_FTM)
    {
        DEV_ASSERT((instance->instIdx) < FTM_INSTANCE_COUNT);
        DEV_ASSERT(channel < FEATURE_FTM_CHANNEL_COUNT);

        uint32_t ftmInstance = instance->instIdx;
        const FTM_Type * const base = ftmBase[ftmInstance];
        uint16_t ftmCurrentCounter = 0U;
        uint16_t finalValue;
        const timer_chan_state_t * ftmChannelState;

        /* Get current FTM counter value */
        ftmCurrentCounter = FTM_DRV_GetCounter(base);
        /* Get the final value of counter */
        finalValue = FTM_DRV_GetMod(base);
        ftmChannelState = &s_ftmState[ftmInstance][channel];
        /* Calculate time elapsed */
        if (ftmCurrentCounter >= ftmChannelState->chanStartVal)
        {
            /* In case the counter is smaller than the final value of counter */
            timeElapsed = ftmCurrentCounter - ftmChannelState->chanStartVal;
        }
        else
        {
            /* In case the counter is over the final value of counter */
            timeElapsed = (finalValue - ftmChannelState->chanStartVal) + ftmCurrentCounter;
        }
        /* Get the remaining time */
        timeRemain = ftmChannelState->period - timeElapsed;
    }
    else
#endif

    /* Define TIMING PAL over PIT */
#if (defined (TIMING_OVER_PIT))
    if (instance->instType == TIMING_INST_TYPE_PIT)
    {
        (void)timeElapsed;

        /* Get the remaining time */
        timeRemain = PIT_DRV_GetCurrentTimerCount(instance->instIdx, channel);
    }
    else
#endif

    /* Define TIMING PAL over STM */
#if (defined (TIMING_OVER_STM))
    if (instance->instType == TIMING_INST_TYPE_STM)
    {
        DEV_ASSERT(channel < STM_CHANNEL_COUNT);

        uint32_t stmInstance = instance->instIdx;
        const timer_chan_state_t * stmChannelState;

        /* Get current counter value */
        uint32_t stmCurrentCounter = STM_DRV_GetCounterValue(stmInstance);
        stmChannelState = &s_stmState[stmInstance][channel];
        /* Calculate time elapsed */
        if ((stmCurrentCounter >= stmChannelState->chanStartVal))
        {
            /* In case the counter is smaller than the final value of counter */
            timeElapsed = stmCurrentCounter - stmChannelState->chanStartVal;
        }
        else
        {
            /* In case the counter is over the final value of counter */
            timeElapsed = (STM_COMPARE_MAX - stmChannelState->chanStartVal) + stmCurrentCounter;
        }
        /* Get the remaining time */
        timeRemain = stmChannelState->period - timeElapsed;
    }
    else
#endif

   /* Define TIMING PAL over ETIMER */
#if (defined (TIMING_OVER_ETIMER))
    if (instance->instType == TIMING_INST_TYPE_ETIMER)
    {
        const timer_chan_state_t * etimerChannelState;
        /* Get current channel counter value */
        timeElapsed = ETIMER_DRV_GetTimerTicks((uint16_t)(instance->instIdx), channel);

        etimerChannelState = &s_etimerState[instance->instIdx][channel];
        /* Calculate timer elapsed */
        timeRemain = etimerChannelState->period - timeElapsed;
    }
    else
#endif
    {
        DEV_ASSERT(false);
    }

    return timeRemain;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_EnableNotification
 * Description   : Enable channel notifications.
 * This function enables channel notification.
 *
 * Implements    : TIMING_EnableNotification_Activity
 *END**************************************************************************/
void TIMING_EnableNotification(const timing_instance_t * const instance,
                               const uint8_t channel)
{
    DEV_ASSERT(instance != NULL);

    timer_chan_state_t * channelState;

    /* Define TIMING PAL over LPIT */
#if (defined (TIMING_OVER_LPIT))
    if (instance->instType == TIMING_INST_TYPE_LPIT)
    {
        DEV_ASSERT((instance->instIdx) < LPIT_INSTANCE_COUNT);
        DEV_ASSERT(channel < LPIT_TMR_COUNT);

        channelState = &s_lpitState[instance->instIdx][channel];
        /* Enable notification */
        channelState->enableNotification = true;
    }
    else
#endif

    /* Define TIMING PAL over LPTMR */
#if (defined (TIMING_OVER_LPTMR))
    if (instance->instType == TIMING_INST_TYPE_LPTMR)
    {
        DEV_ASSERT((instance->instIdx) < LPTMR_INSTANCE_COUNT);
        DEV_ASSERT(channel < LPTMR_TMR_COUNT);

        uint32_t lptmrInstance = instance->instIdx;

        channelState = &s_lptmrState[lptmrInstance][channel];
        /* Enable notification */
        channelState->enableNotification = true;

    }
    else
#endif

    /* Define TIMING PAL over FTM */
#if (defined (TIMING_OVER_FTM))
    if (instance->instType == TIMING_INST_TYPE_FTM)
    {
        DEV_ASSERT((instance->instIdx) < FTM_INSTANCE_COUNT);
        DEV_ASSERT(channel < FEATURE_FTM_CHANNEL_COUNT);

        uint32_t ftmInstance = instance->instIdx;

        channelState = &s_ftmState[ftmInstance][channel];
        /* Enable notification */
        channelState->enableNotification = true;
    }
    else
#endif

    /* Define TIMING PAL over PIT */
#if (defined (TIMING_OVER_PIT))
    if (instance->instType == TIMING_INST_TYPE_PIT)
    {
        DEV_ASSERT((instance->instIdx) < PIT_INSTANCE_COUNT);
        DEV_ASSERT(channel < PIT_TIMER_COUNT);

        uint32_t pitInstance = instance->instIdx;

        channelState = &s_pitState[pitInstance][channel];
        /* Enable notification */
        channelState->enableNotification = true;
    }
    else
#endif

    /* Define TIMING PAL over STM */
#if (defined (TIMING_OVER_STM))
    if (instance->instType == TIMING_INST_TYPE_STM)
    {
        DEV_ASSERT((instance->instIdx) < STM_INSTANCE_COUNT);
        DEV_ASSERT(channel < STM_CHANNEL_COUNT);

        uint32_t stmInstance = instance->instIdx;

        channelState = &s_stmState[stmInstance][channel];
        /* Enable notification */
        channelState->enableNotification = true;
    }
    else
#endif

/* Define TIMING PAL over ETIMER */
#if (defined (TIMING_OVER_ETIMER))
    if (instance->instType == TIMING_INST_TYPE_ETIMER)
    {
        DEV_ASSERT((instance->instIdx) < ETIMER_INSTANCE_COUNT);
        DEV_ASSERT(channel < ETIMER_CH_COUNT);

        channelState = &s_etimerState[instance->instIdx][channel];
        /* Enable notification */
        channelState->enableNotification = true;
    }
    else
#endif
    {
        DEV_ASSERT(false);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_DisableNotification
 * Description   : Disable channel notifications
 * This function disables channel notification.
 *
 * Implements    : TIMING_DisableNotification_Activity
 *END**************************************************************************/
void TIMING_DisableNotification(const timing_instance_t * const instance,
                                const uint8_t channel)
{
    DEV_ASSERT(instance != NULL);

    timer_chan_state_t * channelState;

    /* Define TIMING PAL over LPIT */
#if (defined (TIMING_OVER_LPIT))
    if (instance->instType == TIMING_INST_TYPE_LPIT)
    {
        DEV_ASSERT((instance->instIdx) < LPIT_INSTANCE_COUNT);
        DEV_ASSERT(channel < LPIT_TMR_COUNT);

        channelState = &s_lpitState[instance->instIdx][channel];
        /* Disable notification */
        channelState->enableNotification = false;
    }
    else
#endif

    /* Define TIMING PAL over LPTMR */
#if (defined (TIMING_OVER_LPTMR))
    if (instance->instType == TIMING_INST_TYPE_LPTMR)
    {
        DEV_ASSERT((instance->instIdx) < LPTMR_INSTANCE_COUNT);
        DEV_ASSERT(channel < LPTMR_TMR_COUNT);

        channelState = &s_lptmrState[instance->instIdx][channel];
        /* Disable notification */
        channelState->enableNotification = false;

    }
    else
#endif

    /* Define TIMING PAL over FTM */
#if (defined (TIMING_OVER_FTM))
    if (instance->instType == TIMING_INST_TYPE_FTM)
    {
        DEV_ASSERT((instance->instIdx) < FTM_INSTANCE_COUNT);
        DEV_ASSERT(channel < FEATURE_FTM_CHANNEL_COUNT);

        channelState = &s_ftmState[instance->instIdx][channel];
        /* Disable notification */
        channelState->enableNotification = false;
    }
    else
#endif

    /* Define TIMING PAL over PIT */
#if (defined (TIMING_OVER_PIT))
    if (instance->instType == TIMING_INST_TYPE_PIT)
    {
        DEV_ASSERT((instance->instIdx) < PIT_INSTANCE_COUNT);
        DEV_ASSERT(channel < PIT_TIMER_COUNT);

        channelState = &s_pitState[instance->instIdx][channel];
        /* Disable notification */
        channelState->enableNotification = false;
    }
    else
#endif

    /* Define TIMING PAL over STM */
#if (defined (TIMING_OVER_STM))
    if (instance->instType == TIMING_INST_TYPE_STM)
    {
        DEV_ASSERT((instance->instIdx) < STM_INSTANCE_COUNT);
        DEV_ASSERT(channel < STM_CHANNEL_COUNT);

        channelState = &s_stmState[instance->instIdx][channel];
        /* Disable notification */
        channelState->enableNotification = false;
    }
    else
#endif

/* Define TIMING PAL over ETIMER */
#if (defined (TIMING_OVER_ETIMER))
    if (instance->instType == TIMING_INST_TYPE_ETIMER)
    {
        DEV_ASSERT((instance->instIdx) < ETIMER_INSTANCE_COUNT);
        DEV_ASSERT(channel < ETIMER_CH_COUNT);

        channelState = &s_etimerState[instance->instIdx][channel];
        /* Enable notification */
        channelState->enableNotification = false;
    }
    else
#endif
    {
        DEV_ASSERT(false);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_GetResolution
 * Description   : Get tick resolution
 * This function gets tick resolution in engineering units (nanosecond, microsecond or millisecond).
 * The result of this function is used to calculate period, remaining time or elapsed time in engineering units.
 *
 * Implements    : TIMING_GetResolution_Activity
 *END**************************************************************************/
status_t TIMING_GetResolution(const timing_instance_t * const instance,
                              const timer_resolution_type_t type,
                              uint64_t * const resolution)
{
    DEV_ASSERT(instance != NULL);
    DEV_ASSERT(resolution != NULL);

    status_t status = STATUS_SUCCESS;
    uint32_t clkFrequency = 0U;
    uint64_t prescaler = 1U;

    /* Define TIMING PAL over LPIT */
#if (defined (TIMING_OVER_LPIT))
    if (instance->instType == TIMING_INST_TYPE_LPIT)
    {
        status_t clkErr;

        DEV_ASSERT((instance->instIdx) < LPIT_INSTANCE_COUNT);
        /* Gets current functional clock frequency of LPIT instance */
        clkErr = CLOCK_SYS_GetFreq(s_lpitClockName[instance->instIdx], &clkFrequency);
        /* Checks the functional clock module is available */
        (void)clkErr;
        DEV_ASSERT(clkErr == STATUS_SUCCESS);
        DEV_ASSERT(clkFrequency > 0U);
    }
    else
#endif

    /* Define TIMING PAL over LPTMR */
#if (defined (TIMING_OVER_LPTMR))
    if (instance->instType == TIMING_INST_TYPE_LPTMR)
    {
        DEV_ASSERT((instance->instIdx) < LPTMR_INSTANCE_COUNT);
        /* Get lptmr clock frequency */
        TIMING_GetLptmrFreq(instance->instIdx, &clkFrequency, &prescaler);
    }
    else
#endif

    /* Define TIMING PAL over FTM */
#if (defined (TIMING_OVER_FTM))
    if (instance->instType == TIMING_INST_TYPE_FTM)
    {
        DEV_ASSERT((instance->instIdx) < FTM_INSTANCE_COUNT);

        uint32_t ftmInstance = instance->instIdx;

        /* Gets current functional clock frequency of FTM instance */
        clkFrequency = FTM_DRV_GetFrequency(ftmInstance);
        DEV_ASSERT(clkFrequency > 0U);
    }
    else
#endif

    /* Define TIMING PAL over PIT */
#if (defined (TIMING_OVER_PIT))
    if (instance->instType == TIMING_INST_TYPE_PIT)
    {
        DEV_ASSERT((instance->instIdx) < PIT_INSTANCE_COUNT);

        status_t clkErr;
        uint32_t pitInstance = instance->instIdx;
        /* Gets current functional clock frequency of PIT instance */
        clkErr = CLOCK_SYS_GetFreq(s_pitClockName[pitInstance], &clkFrequency);
        /* Checks the functional clock module is available */
        (void)clkErr;
        DEV_ASSERT(clkErr == STATUS_SUCCESS);
        DEV_ASSERT(clkFrequency > 0U);
    }
    else
#endif

    /* Define TIMING PAL over STM */
#if (defined (TIMING_OVER_STM))
    if (instance->instType == TIMING_INST_TYPE_STM)
    {
        DEV_ASSERT((instance->instIdx) < STM_INSTANCE_COUNT);
        /* Get stm clock frequency */
        TIMING_GetStmFreq(instance->instIdx, &clkFrequency, &prescaler);
    }
    else
#endif

    /* Define TIMING PAL over ETIMER */
#if (defined (TIMING_OVER_ETIMER))
    if (instance->instType == TIMING_INST_TYPE_ETIMER)
    {

        DEV_ASSERT((instance->instIdx) < ETIMER_INSTANCE_COUNT);
        /* Get etimer clock frequency */
        TIMING_GetEtimerFreq(instance->instIdx, &clkFrequency, &prescaler);
    }
    else
#endif
    {
        DEV_ASSERT(false);
    }

    if (type == TIMER_RESOLUTION_TYPE_NANOSECOND)
    {
        /* For better precision, add half of the frequency before the division */
        *resolution = ((1000000000U * prescaler) + ((uint64_t)clkFrequency >> 1U)) / clkFrequency;
    }
    else if (type == TIMER_RESOLUTION_TYPE_MICROSECOND)
    {
        /* For better precision, add half of the frequency before the division */
        *resolution = ((1000000U * prescaler) + ((uint64_t)clkFrequency >> 1U)) / clkFrequency;
    }
    else
    {
        /* For better precision, add half of the frequency before the division */
        *resolution = ((1000U * prescaler) + ((uint64_t)clkFrequency >> 1U)) / clkFrequency;
    }

    if(*resolution == 0U)
    {
        status = STATUS_ERROR;
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_GetMaxPeriod
 * Description   : Get max period in engineering units
 * This function get max period in engineering units.
 *
 * Implements    : TIMING_GetMaxPeriod_Activity
 *END**************************************************************************/
status_t TIMING_GetMaxPeriod(const timing_instance_t * const instance,
                             const timer_resolution_type_t type,
                             uint64_t * const maxPeriod)
{
    DEV_ASSERT(instance != NULL);
    DEV_ASSERT(maxPeriod != NULL);

    status_t status = STATUS_SUCCESS;
    uint32_t clkFrequency = 0U;
    uint64_t prescaler = 1U;
    uint64_t maxCountValue = 0U;

    /* Define TIMING PAL over LPIT */
#if (defined (TIMING_OVER_LPIT))
    if (instance->instType == TIMING_INST_TYPE_LPIT)
    {
        status_t clkErr;

        DEV_ASSERT((instance->instIdx) < LPIT_INSTANCE_COUNT);
        /* Set max count value of LPIT */
        maxCountValue = (uint64_t)LPIT_COMPARE_MAX + 1UL;
        /* Gets current functional clock frequency of LPIT instance */
        clkErr = CLOCK_SYS_GetFreq(s_lpitClockName[instance->instIdx], &clkFrequency);
        /* Checks the functional clock module is available */
        (void)clkErr;
        DEV_ASSERT(clkErr == STATUS_SUCCESS);
        DEV_ASSERT(clkFrequency > 0U);
    }
    else
#endif

    /* Define TIMING PAL over LPTMR */
#if (defined (TIMING_OVER_LPTMR))
    if (instance->instType == TIMING_INST_TYPE_LPTMR)
    {
        DEV_ASSERT((instance->instIdx) < LPTMR_INSTANCE_COUNT);
        /* Set max count value of LPTMR */
        maxCountValue = LPTMR_COMPARE_MAX + 1UL;
        /* Get lptmr clock frequency */
        TIMING_GetLptmrFreq(instance->instIdx, &clkFrequency, &prescaler);
    }
    else
#endif

    /* Define TIMING PAL over FTM */
#if (defined (TIMING_OVER_FTM))
    if (instance->instType == TIMING_INST_TYPE_FTM)
    {
        uint32_t ftmInstance = instance->instIdx;

        /* Set max count value of FTM */
        maxCountValue = FTM_COMPARE_MAX + 1UL;
        /* Gets current functional clock frequency of FTM instance */
        clkFrequency = FTM_DRV_GetFrequency(ftmInstance);
        DEV_ASSERT(clkFrequency > 0U);
        status = STATUS_SUCCESS;
    }
    else
#endif

    /* Define TIMING PAL over PIT */
#if (defined (TIMING_OVER_PIT))
    if (instance->instType == TIMING_INST_TYPE_PIT)
    {
        DEV_ASSERT((instance->instIdx) < PIT_INSTANCE_COUNT);

        status_t clkErr;
        uint32_t pitInstance = instance->instIdx;

        /* Set max count value of PIT */
        maxCountValue = (uint64_t)PIT_COMPARE_MAX + 1UL;
        /* Gets current functional clock frequency of PIT instance */
        clkErr = CLOCK_SYS_GetFreq(s_pitClockName[pitInstance], &clkFrequency);
        /* Checks the functional clock module is available */
        (void)clkErr;
        DEV_ASSERT(clkErr == STATUS_SUCCESS);
        DEV_ASSERT(clkFrequency > 0U);
    }
    else
#endif

    /* Define TIMING PAL over STM */
#if (defined (TIMING_OVER_STM))
    if (instance->instType == TIMING_INST_TYPE_STM)
    {
        DEV_ASSERT((instance->instIdx) < STM_INSTANCE_COUNT);
        /* Set max count value of STM */
        maxCountValue = (uint64_t)STM_COMPARE_MAX + 1UL;
        /* Get stm clock frequency */
        TIMING_GetStmFreq(instance->instIdx, &clkFrequency, &prescaler);
    }
    else
#endif

    /* Define TIMING PAL over ETIMER */
#if (defined (TIMING_OVER_ETIMER))
    if (instance->instType == TIMING_INST_TYPE_ETIMER)
    {
        DEV_ASSERT((instance->instIdx) < ETIMER_INSTANCE_COUNT);
        /* Set max count value of ETIMER */
        maxCountValue = ETIMER_COMPARE_MAX + 1UL;
         /* Get etimer clock frequency */
        TIMING_GetEtimerFreq(instance->instIdx, &clkFrequency, &prescaler);
    }
    else
#endif
    {
        DEV_ASSERT(false);
    }

    if (type == TIMER_RESOLUTION_TYPE_NANOSECOND)
    {
        /* For better precision, add half of the frequency before the division.
           To avoid overflow, divided the clock frequency before multiples by the max count value */
        *maxPeriod = (((1000000000U * prescaler) + ((uint64_t)clkFrequency >> 1U)) / clkFrequency) * maxCountValue;
    }
    else if (type == TIMER_RESOLUTION_TYPE_MICROSECOND)
    {
        /* For better precision, add half of the frequency before the division. */
        *maxPeriod = ((1000000U * prescaler * maxCountValue) + ((uint64_t)clkFrequency >> 1U)) / clkFrequency;
    }
    else
    {   /* For better precision, add half of the frequency before the division. */
        *maxPeriod = ((1000U * prescaler * maxCountValue) +  ((uint64_t)clkFrequency >> 1U)) / clkFrequency;
    }

    if(*maxPeriod == 0U)
    {
        status = STATUS_ERROR;
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_InstallCallback
 * Description   : Install new callback function and callback parameter for timer channel.
 *
 * Implements    : TIMING_InstallCallback_Activity
 *END**************************************************************************/
void TIMING_InstallCallback(const timing_instance_t * const instance,
                           const uint8_t channel,
                           const timer_callback_t callback,
                           void * const callbackParam)
{
    DEV_ASSERT(instance != NULL);

    timer_chan_state_t * channelState;

    /* Define TIMING PAL over LPIT */
#if (defined (TIMING_OVER_LPIT))
    if (instance->instType == TIMING_INST_TYPE_LPIT)
    {
        DEV_ASSERT((instance->instIdx) < LPIT_INSTANCE_COUNT);
        DEV_ASSERT(channel < LPIT_TMR_COUNT);

        channelState = &s_lpitState[instance->instIdx][channel];

        /* Disable interrupt generating to avoid timer event appears just between
           the write of the new function address and the new parameter */
        INT_SYS_DisableIRQ(s_lpitIrq[instance->instIdx][channel]);
        /* update new callback function */
        channelState->callback = callback;
        /* Update new callback parameter */
        channelState->callbackParam = callbackParam;
        /* Enable interrupt after the updating of callback and parameter*/
        INT_SYS_EnableIRQ(s_lpitIrq[instance->instIdx][channel]);
    }
    else
#endif

    /* Define TIMING PAL over LPTMR */
#if (defined (TIMING_OVER_LPTMR))
    if (instance->instType == TIMING_INST_TYPE_LPTMR)
    {
        DEV_ASSERT((instance->instIdx) < LPTMR_INSTANCE_COUNT);
        DEV_ASSERT(channel < LPTMR_TMR_COUNT);

        channelState = &s_lptmrState[instance->instIdx][channel];

        /* Disable interrupt generating to avoid timer event appears just between
           the write of the new function address and the new parameter */
        INT_SYS_DisableIRQ(s_lptmrIrq[instance->instIdx][channel]);
        /* update new callback function */
        channelState->callback = callback;
        /* Update new callback parameter */
        channelState->callbackParam = callbackParam;
        /* Enable interrupt after the updating of callback and parameter*/
        INT_SYS_EnableIRQ(s_lptmrIrq[instance->instIdx][channel]);
    }
    else
#endif

    /* Define TIMING PAL over FTM */
#if (defined (TIMING_OVER_FTM))
    if (instance->instType == TIMING_INST_TYPE_FTM)
    {
        DEV_ASSERT((instance->instIdx) < FTM_INSTANCE_COUNT);
        DEV_ASSERT(channel < FEATURE_FTM_CHANNEL_COUNT);

        channelState = &s_ftmState[instance->instIdx][channel];

        /* Disable interrupt generating to avoid timer event appears just between
           the write of the new function address and the new parameter */
        INT_SYS_DisableIRQ(s_ftmIrq[instance->instIdx][channel]);
        /* update new callback function */
        channelState->callback = callback;
        /* Update new callback parameter */
        channelState->callbackParam = callbackParam;
        /* Enable interrupt after the updating of callback and parameter*/
        INT_SYS_EnableIRQ(s_ftmIrq[instance->instIdx][channel]);
    }
    else
#endif

    /* Define TIMING PAL over PIT */
#if (defined (TIMING_OVER_PIT))
    if (instance->instType == TIMING_INST_TYPE_PIT)
    {
        DEV_ASSERT((instance->instIdx) < PIT_INSTANCE_COUNT);
        DEV_ASSERT(channel < PIT_TIMER_COUNT);

        channelState = &s_pitState[instance->instIdx][channel];

        /* Disable interrupt generating to avoid timer event appears just between
           the write of the new function address and the new parameter */
        INT_SYS_DisableIRQ(s_pitIrq[instance->instIdx][channel]);
        /* update new callback function */
        channelState->callback = callback;
        /* Update new callback parameter */
        channelState->callbackParam = callbackParam;
        /* Enable interrupt after the updating of callback and parameter*/
        INT_SYS_EnableIRQ(s_pitIrq[instance->instIdx][channel]);
    }
    else
#endif

    /* Define TIMING PAL over STM */
#if (defined (TIMING_OVER_STM))
    if (instance->instType == TIMING_INST_TYPE_STM)
    {
        DEV_ASSERT((instance->instIdx) < STM_INSTANCE_COUNT);
        DEV_ASSERT(channel < STM_CHANNEL_COUNT);

        channelState = &s_stmState[instance->instIdx][channel];

        /* Disable interrupt generating to avoid timer event appears just between
           the write of the new function address and the new parameter */
        INT_SYS_DisableIRQ(s_stmIrq[instance->instIdx][channel]);
        /* update new callback function */
        channelState->callback = callback;
        /* Update new callback parameter */
        channelState->callbackParam = callbackParam;
        /* Enable interrupt after the updating of callback and parameter*/
        INT_SYS_EnableIRQ(s_stmIrq[instance->instIdx][channel]);
    }
    else
#endif

    /* Define TIMING PAL over ETIMER */
#if (defined (TIMING_OVER_ETIMER))
    if (instance->instType == TIMING_INST_TYPE_ETIMER)
    {
        DEV_ASSERT((instance->instIdx) < ETIMER_INSTANCE_COUNT);
        DEV_ASSERT(channel < ETIMER_CH_COUNT);

        channelState = &s_etimerState[instance->instIdx][channel];

        /* Disable interrupt generating to avoid timer event appears just between
           the write of the new function address and the new parameter */
        INT_SYS_DisableIRQ(s_etimerIrq[instance->instIdx][channel]);
        /* update new callback function */
        channelState->callback = callback;
        /* Update new callback parameter */
        channelState->callbackParam = callbackParam;
        /* Enable interrupt after the updating of callback and parameter*/
        INT_SYS_EnableIRQ(s_etimerIrq[instance->instIdx][channel]);
    }
    else
#endif
    {
        DEV_ASSERT(false);
    }

}

#if (defined (TIMING_OVER_LPIT))
/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_Lpit_IrqHandler
 * Description   : Interrupt handler for TIMING over LPIT.
 * This is not a public API as it is called by IRQ whenever an interrupt
 * occurs.
 *
 *END**************************************************************************/
void TIMING_Lpit_IrqHandler(uint32_t instance, uint8_t channel)
{
    const timer_chan_state_t * channelState = &s_lpitState[instance][channel];

    if ((channelState->callback != NULL) && (channelState->enableNotification))
    {
        /* Call to callback function */
        (channelState->callback)(channelState->callbackParam);

        if (channelState->chanType == TIMER_CHAN_TYPE_ONESHOT)
        {
            uint32_t channelMask = 1UL << channel;
            /* Stop the channel counting */
            LPIT_DRV_StopTimerChannels(instance, channelMask);
        }
    }
    LPIT_DRV_ClearInterruptFlagTimerChannels(instance, (1UL << channel));

}
#endif

/* Define TIMING PAL over LPTMR */
#if (defined (TIMING_OVER_LPTMR))
/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_Lptmr_IrqHandler
 * Description   : Interrupt handler for TIMING over LPTMR.
 * This is not a public API as it is called by IRQ whenever an interrupt
 * occurs.
 *
 *END**************************************************************************/
void TIMING_Lptmr_IrqHandler(uint32_t instance, uint8_t channel)
{
    const timer_chan_state_t * channelState = &s_lptmrState[instance][channel];

    if ((channelState->callback != NULL) && (channelState->enableNotification))
    {
        /* Call to callback function */
        (channelState->callback)(channelState->callbackParam);

        if (channelState->chanType == TIMER_CHAN_TYPE_ONESHOT)
        {
            /* Stop the channel counting */
            LPTMR_DRV_StopCounter(instance);
        }
    }
    LPTMR_DRV_ClearCompareFlag(instance);
}
#endif

/* Define TIMING PAL over FTM */
#if (defined (TIMING_OVER_FTM))
/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_Ftm_IrqHandler
 * Description   : Interrupt handler for TIMING over FTM.
 * This is not a public API as it is called by IRQ whenever an interrupt
 * occurs.
 *
 *END**************************************************************************/
void TIMING_Ftm_IrqHandler(uint32_t instance, uint8_t channel)
{
     FTM_Type * const base = ftmBase[instance];
    timer_chan_state_t * channelState = &s_ftmState[instance][channel];

    if ((channelState->callback != NULL) && (channelState->enableNotification))
    {
        /* Call to callback function */
        (channelState->callback)(channelState->callbackParam);
    }
    /* Check notification type */
    if (channelState->chanType == TIMER_CHAN_TYPE_ONESHOT)
    {
        /* Stop the channel by disable interrupt generation */
        FTM_DRV_DisableInterrupts(instance, (1UL << channel));
        /* Update channel running status */
        g_ftmChannelRunning[instance][channel] = false;
    }
    else
    {
        status_t status;
        uint32_t currentCmpValue = 0U;
        uint32_t currentPeriod = channelState->period;
        uint32_t nexCompareValue = 0U;
        uint32_t finalValue;

        /* Get the final value of counter */
        finalValue = FTM_DRV_GetMod(base);
        /* Get current compare value of the channel */
        currentCmpValue = FTM_DRV_GetChnCountVal(base, channel);
        /* Calculate the next compare value of the channel */
        if ((finalValue - currentCmpValue) > currentPeriod)
        {
            nexCompareValue = currentCmpValue + currentPeriod;
        }
        else
        {
            nexCompareValue = currentPeriod - (finalValue - currentCmpValue);
        }
        /* Update next compare value to the channel */
        status = FTM_DRV_UpdateOutputCompareChannel(instance, channel, (uint16_t)nexCompareValue, FTM_ABSOLUTE_VALUE, false);
        (void)status;
        /* Save the start value of channel at the moment new period is started */
        channelState->chanStartVal = currentCmpValue;
    }
    /* Clear interrupt flag */
    FTM_DRV_ClearChnEventStatus(base, channel);
}
#endif

/* Define TIMING PAL over PIT */
#if (defined (TIMING_OVER_PIT))
/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_Pit_IrqHandler
 * Description   : Interrupt handler for TIMING over PIT.
 * This is not a public API as it is called by IRQ whenever an interrupt
 * occurs.
 *
 *END**************************************************************************/
void TIMING_Pit_IrqHandler(uint32_t instance, uint8_t channel)
{
    const timer_chan_state_t * channelState = &s_pitState[instance][channel];

    if ((channelState->callback != NULL) && (channelState->enableNotification))
    {
        /* Call to callback function */
        (channelState->callback)(channelState->callbackParam);
        /* Check notification type */
        if (channelState->chanType == TIMER_CHAN_TYPE_ONESHOT)
        {
            /* Stop the channel counting */
            PIT_DRV_StopChannel(instance, channel);
        }
    }
    PIT_DRV_ClearStatusFlags(instance, channel);
}
#endif

/* Define TIMING PAL over STM */
#if (defined (TIMING_OVER_STM))
/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_Stm_IrqHandler
 * Description   : Interrupt handler for TIMING over STM.
 * This is not a public API as it is called by IRQ whenever an interrupt
 * occurs.
 *
 *END**************************************************************************/
void TIMING_Stm_IrqHandler(uint32_t instance, uint8_t channel)
{
#ifndef STM_0
    uint32_t stmInstance = instance - 1U;
#else
    uint32_t stmInstance = instance;
#endif
    uint32_t currentCounter = 0U;
    timer_chan_state_t * channelState = &s_stmState[stmInstance][channel];

    /* Get current counter value */
    currentCounter = STM_DRV_GetCounterValue(stmInstance);

    if ((channelState->callback != NULL) && (channelState->enableNotification))
    {
        /* Call to callback function */
        (channelState->callback)(channelState->callbackParam);
        /* Save the start value of channel at the moment new period is started */
        channelState->chanStartVal = currentCounter;
    }
    /* Check notification type */
    if (channelState->chanType == TIMER_CHAN_TYPE_ONESHOT)
    {
        /* Stop the channel counting */
        STM_DRV_DisableChannel(stmInstance, channel);
    }
    else
    {
        /* Update next compare value of the channel*/
        STM_DRV_IncrementTicks(stmInstance, channel, channelState->period);
    }
    STM_DRV_ClearStatusFlags(stmInstance, channel);
}
#endif

#if (defined (TIMING_OVER_ETIMER))
/*FUNCTION**********************************************************************
 *
 * Function Name : TIMING_Etimer_IrqHandler
 * Description   : Interrupt handler for TIMING over ETIMER.
 * This is not a public API as it is called by IRQ whenever an interrupt
 * occurs.
 *
 *END**************************************************************************/
void TIMING_Etimer_IrqHandler(uint32_t instance, uint8_t channel)
{
#ifndef ETIMER_0
    uint32_t etimerInstance = instance - 1U;
#else
    uint32_t etimerInstance = instance;
#endif
    const timer_chan_state_t * channelState = &s_etimerState[etimerInstance][channel];
    /* Place the clear status flag function at here to avoid the double interrupt event when the etimer clock is very low */
    ETIMER_DRV_ClearInterruptStatus((uint16_t)etimerInstance, ETIMER_CH_IRQ_FLAGS_TCF1, channel);

    if ((channelState->callback != NULL) && (channelState->enableNotification))
    {
        /* Call to callback function */
        (channelState->callback)(channelState->callbackParam);

        if (channelState->chanType == TIMER_CHAN_TYPE_ONESHOT)
        {
            uint16_t channelMask = (uint16_t)(1UL << channel);
            /* Stop the channel counting */
            ETIMER_DRV_StopTimerChannels((uint16_t)etimerInstance, channelMask);
        }
    }

}
#endif
/*******************************************************************************
 * EOF
 ******************************************************************************/
