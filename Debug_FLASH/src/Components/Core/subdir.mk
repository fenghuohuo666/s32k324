################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Components/Core/device.c 

OBJS += \
./src/Components/Core/device.o 

C_DEPS += \
./src/Components/Core/device.d 


# Each subdirectory must supply rules for building sources it contributes
src/Components/Core/%.o: ../src/Components/Core/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@src/Components/Core/device.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


