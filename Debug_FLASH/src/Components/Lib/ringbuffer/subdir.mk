################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Components/Lib/ringbuffer/ringbuffer.c 

OBJS += \
./src/Components/Lib/ringbuffer/ringbuffer.o 

C_DEPS += \
./src/Components/Lib/ringbuffer/ringbuffer.d 


# Each subdirectory must supply rules for building sources it contributes
src/Components/Lib/ringbuffer/%.o: ../src/Components/Lib/ringbuffer/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@src/Components/Lib/ringbuffer/ringbuffer.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


