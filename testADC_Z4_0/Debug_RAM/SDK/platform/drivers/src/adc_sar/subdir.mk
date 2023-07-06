################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../SDK/platform/drivers/src/adc_sar/adc_sar_driver.c 

OBJS += \
./SDK/platform/drivers/src/adc_sar/adc_sar_driver.o 

C_DEPS += \
./SDK/platform/drivers/src/adc_sar/adc_sar_driver.d 


# Each subdirectory must supply rules for building sources it contributes
SDK/platform/drivers/src/adc_sar/%.o: ../SDK/platform/drivers/src/adc_sar/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@SDK/platform/drivers/src/adc_sar/adc_sar_driver.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


