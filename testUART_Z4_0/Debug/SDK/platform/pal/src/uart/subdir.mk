################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../SDK/platform/pal/src/uart/uart_pal.c 

OBJS += \
./SDK/platform/pal/src/uart/uart_pal.o 

C_DEPS += \
./SDK/platform/pal/src/uart/uart_pal.d 


# Each subdirectory must supply rules for building sources it contributes
SDK/platform/pal/src/uart/%.o: ../SDK/platform/pal/src/uart/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@SDK/platform/pal/src/uart/uart_pal.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


