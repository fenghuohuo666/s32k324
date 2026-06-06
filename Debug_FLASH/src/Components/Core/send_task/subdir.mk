################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Components/Core/send_task/send_task.c 

OBJS += \
./src/Components/Core/send_task/send_task.o 

C_DEPS += \
./src/Components/Core/send_task/send_task.d 


# Each subdirectory must supply rules for building sources it contributes
src/Components/Core/send_task/%.o: ../src/Components/Core/send_task/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@src/Components/Core/send_task/send_task.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


