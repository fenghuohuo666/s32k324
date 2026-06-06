################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Components/Lib/log/log.c 

OBJS += \
./src/Components/Lib/log/log.o 

C_DEPS += \
./src/Components/Lib/log/log.d 


# Each subdirectory must supply rules for building sources it contributes
src/Components/Lib/log/%.o: ../src/Components/Lib/log/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@src/Components/Lib/log/log.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


