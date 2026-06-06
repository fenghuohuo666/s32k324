################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Components/Lib/md5/md5.c \
../src/Components/Lib/md5/platform_util.c 

OBJS += \
./src/Components/Lib/md5/md5.o \
./src/Components/Lib/md5/platform_util.o 

C_DEPS += \
./src/Components/Lib/md5/md5.d \
./src/Components/Lib/md5/platform_util.d 


# Each subdirectory must supply rules for building sources it contributes
src/Components/Lib/md5/%.o: ../src/Components/Lib/md5/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@src/Components/Lib/md5/md5.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


