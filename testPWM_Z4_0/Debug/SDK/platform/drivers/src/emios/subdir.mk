################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../SDK/platform/drivers/src/emios/emios_common.c \
../SDK/platform/drivers/src/emios/emios_hw_access.c \
../SDK/platform/drivers/src/emios/emios_mc_driver.c \
../SDK/platform/drivers/src/emios/emios_pwm_driver.c 

OBJS += \
./SDK/platform/drivers/src/emios/emios_common.o \
./SDK/platform/drivers/src/emios/emios_hw_access.o \
./SDK/platform/drivers/src/emios/emios_mc_driver.o \
./SDK/platform/drivers/src/emios/emios_pwm_driver.o 

C_DEPS += \
./SDK/platform/drivers/src/emios/emios_common.d \
./SDK/platform/drivers/src/emios/emios_hw_access.d \
./SDK/platform/drivers/src/emios/emios_mc_driver.d \
./SDK/platform/drivers/src/emios/emios_pwm_driver.d 


# Each subdirectory must supply rules for building sources it contributes
SDK/platform/drivers/src/emios/%.o: ../SDK/platform/drivers/src/emios/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@SDK/platform/drivers/src/emios/emios_common.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


