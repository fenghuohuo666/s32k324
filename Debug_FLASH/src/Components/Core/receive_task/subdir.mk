################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Components/Core/receive_task/dispose_rcp.c \
../src/Components/Core/receive_task/receive_task.c 

OBJS += \
./src/Components/Core/receive_task/dispose_rcp.o \
./src/Components/Core/receive_task/receive_task.o 

C_DEPS += \
./src/Components/Core/receive_task/dispose_rcp.d \
./src/Components/Core/receive_task/receive_task.d 


# Each subdirectory must supply rules for building sources it contributes
src/Components/Core/receive_task/%.o: ../src/Components/Core/receive_task/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@src/Components/Core/receive_task/dispose_rcp.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


