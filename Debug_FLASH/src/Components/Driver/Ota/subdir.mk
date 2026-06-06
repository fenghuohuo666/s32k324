################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Components/Driver/Ota/ota.c 

OBJS += \
./src/Components/Driver/Ota/ota.o 

C_DEPS += \
./src/Components/Driver/Ota/ota.d 


# Each subdirectory must supply rules for building sources it contributes
src/Components/Driver/Ota/%.o: ../src/Components/Driver/Ota/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@src/Components/Driver/Ota/ota.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


