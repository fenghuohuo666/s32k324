################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Cpu0_Main.c \
../src/SJA1110_APP.c \
../src/SJA1110_SPI.c \
../src/main.c 

OBJS += \
./src/Cpu0_Main.o \
./src/SJA1110_APP.o \
./src/SJA1110_SPI.o \
./src/main.o 

C_DEPS += \
./src/Cpu0_Main.d \
./src/SJA1110_APP.d \
./src/SJA1110_SPI.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@src/Cpu0_Main.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


