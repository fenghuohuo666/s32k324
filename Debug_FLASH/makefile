################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/legacy_stubs/subdir.mk
-include src/Components/Lib/ringbuffer/subdir.mk
-include src/Components/Lib/md5/subdir.mk
-include src/Components/Lib/log/subdir.mk
-include src/Components/Lib/bitmap/subdir.mk
-include src/Components/Driver/Ota/subdir.mk
-include src/Components/Core/send_task/subdir.mk
-include src/Components/Core/schedule_task/subdir.mk
-include src/Components/Core/receive_task/subdir.mk
-include src/Components/Core/rcpbuild/subdir.mk
-include src/Components/Core/subdir.mk
-include src/Components/BSP/subdir.mk
-include src/subdir.mk
-include generate/src/subdir.mk
-include board/subdir.mk
-include RTD/src/subdir.mk
-include Project_Settings/Startup_Code/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_SIZE += \
S32K324_SPI_DMA_SJA1110_Load_firmware_M7_0.siz \


# All Target
all: S32K324_SPI_DMA_SJA1110_Load_firmware_M7_0.elf secondary-outputs

# Tool invocations
S32K324_SPI_DMA_SJA1110_Load_firmware_M7_0.elf: $(OBJS) D:/s32k_workspace/S32K324_SPI_DMA_SJA1110_Load_firmware_M7_0/Project_Settings/Linker_Files/linker_flash_c0_s32k324.ld $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Standard S32DS C Linker'
	arm-none-eabi-gcc -o "S32K324_SPI_DMA_SJA1110_Load_firmware_M7_0.elf" "@S32K324_SPI_DMA_SJA1110_Load_firmware_M7_0.args"  $(USER_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

S32K324_SPI_DMA_SJA1110_Load_firmware_M7_0.siz: S32K324_SPI_DMA_SJA1110_Load_firmware_M7_0.elf
	@echo 'Invoking: Standard S32DS Print Size'
	arm-none-eabi-size --format=berkeley S32K324_SPI_DMA_SJA1110_Load_firmware_M7_0.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) ./*
	-@echo ' '

secondary-outputs: $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets
