################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Components/Core/schedule_task/schedule.c \
../src/Components/Core/schedule_task/schedule_algo.c 

OBJS += \
./src/Components/Core/schedule_task/schedule.o \
./src/Components/Core/schedule_task/schedule_algo.o 

C_DEPS += \
./src/Components/Core/schedule_task/schedule.d \
./src/Components/Core/schedule_task/schedule_algo.d 


# Each subdirectory must supply rules for building sources it contributes
src/Components/Core/schedule_task/%.o: ../src/Components/Core/schedule_task/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@src/Components/Core/schedule_task/schedule.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


