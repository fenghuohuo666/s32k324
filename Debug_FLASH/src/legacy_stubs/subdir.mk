################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/legacy_stubs/sbrk.c \
../src/legacy_stubs/tc387_stub.c 

OBJS += \
./src/legacy_stubs/sbrk.o \
./src/legacy_stubs/tc387_stub.o 

C_DEPS += \
./src/legacy_stubs/sbrk.d \
./src/legacy_stubs/tc387_stub.d 


# Each subdirectory must supply rules for building sources it contributes
src/legacy_stubs/%.o: ../src/legacy_stubs/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@src/legacy_stubs/sbrk.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


