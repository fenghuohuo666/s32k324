################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Components/BSP/board.c 

OBJS += \
./src/Components/BSP/board.o 

C_DEPS += \
./src/Components/BSP/board.d 


# Each subdirectory must supply rules for building sources it contributes
src/Components/BSP/%.o: ../src/Components/BSP/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@src/Components/BSP/board.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


