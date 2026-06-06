################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Components/Lib/bitmap/bitmap.c 

OBJS += \
./src/Components/Lib/bitmap/bitmap.o 

C_DEPS += \
./src/Components/Lib/bitmap/bitmap.d 


# Each subdirectory must supply rules for building sources it contributes
src/Components/Lib/bitmap/%.o: ../src/Components/Lib/bitmap/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@src/Components/Lib/bitmap/bitmap.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


