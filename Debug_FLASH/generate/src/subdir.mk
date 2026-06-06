################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../generate/src/Axbs_Ip_PBcfg.c \
../generate/src/CDD_Rm_Cfg.c \
../generate/src/CDD_Rm_Ipw_PBcfg.c \
../generate/src/CDD_Rm_PBcfg.c \
../generate/src/Clock_Ip_Cfg.c \
../generate/src/Dma_Ip_Cfg.c \
../generate/src/Dma_Ip_PBcfg.c \
../generate/src/Dma_Mux_Ip_PBcfg.c \
../generate/src/Igf_Port_Ip_Cfg.c \
../generate/src/IntCtrl_Ip_Cfg.c \
../generate/src/Lpspi_Ip_Sa_PBcfg.c \
../generate/src/Lpuart_Lin_Ip_Sa_PBcfg.c \
../generate/src/Mscm_Ip_PBcfg.c \
../generate/src/OsIf_Cfg.c \
../generate/src/Pflash_Ip_Cfg.c \
../generate/src/Pflash_Ip_PBcfg.c \
../generate/src/Pit_Ip_PBcfg.c \
../generate/src/Virt_Wrapper_Ip_PBcfg.c \
../generate/src/Xbic_Ip_PBcfg.c \
../generate/src/Xrdc_Ip_Cfg.c \
../generate/src/Xrdc_Ip_PBcfg.c 

OBJS += \
./generate/src/Axbs_Ip_PBcfg.o \
./generate/src/CDD_Rm_Cfg.o \
./generate/src/CDD_Rm_Ipw_PBcfg.o \
./generate/src/CDD_Rm_PBcfg.o \
./generate/src/Clock_Ip_Cfg.o \
./generate/src/Dma_Ip_Cfg.o \
./generate/src/Dma_Ip_PBcfg.o \
./generate/src/Dma_Mux_Ip_PBcfg.o \
./generate/src/Igf_Port_Ip_Cfg.o \
./generate/src/IntCtrl_Ip_Cfg.o \
./generate/src/Lpspi_Ip_Sa_PBcfg.o \
./generate/src/Lpuart_Lin_Ip_Sa_PBcfg.o \
./generate/src/Mscm_Ip_PBcfg.o \
./generate/src/OsIf_Cfg.o \
./generate/src/Pflash_Ip_Cfg.o \
./generate/src/Pflash_Ip_PBcfg.o \
./generate/src/Pit_Ip_PBcfg.o \
./generate/src/Virt_Wrapper_Ip_PBcfg.o \
./generate/src/Xbic_Ip_PBcfg.o \
./generate/src/Xrdc_Ip_Cfg.o \
./generate/src/Xrdc_Ip_PBcfg.o 

C_DEPS += \
./generate/src/Axbs_Ip_PBcfg.d \
./generate/src/CDD_Rm_Cfg.d \
./generate/src/CDD_Rm_Ipw_PBcfg.d \
./generate/src/CDD_Rm_PBcfg.d \
./generate/src/Clock_Ip_Cfg.d \
./generate/src/Dma_Ip_Cfg.d \
./generate/src/Dma_Ip_PBcfg.d \
./generate/src/Dma_Mux_Ip_PBcfg.d \
./generate/src/Igf_Port_Ip_Cfg.d \
./generate/src/IntCtrl_Ip_Cfg.d \
./generate/src/Lpspi_Ip_Sa_PBcfg.d \
./generate/src/Lpuart_Lin_Ip_Sa_PBcfg.d \
./generate/src/Mscm_Ip_PBcfg.d \
./generate/src/OsIf_Cfg.d \
./generate/src/Pflash_Ip_Cfg.d \
./generate/src/Pflash_Ip_PBcfg.d \
./generate/src/Pit_Ip_PBcfg.d \
./generate/src/Virt_Wrapper_Ip_PBcfg.d \
./generate/src/Xbic_Ip_PBcfg.d \
./generate/src/Xrdc_Ip_Cfg.d \
./generate/src/Xrdc_Ip_PBcfg.d 


# Each subdirectory must supply rules for building sources it contributes
generate/src/%.o: ../generate/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@generate/src/Axbs_Ip_PBcfg.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


