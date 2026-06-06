
#ifndef SJA1110_SPI_H_
#define SJA1110_SPI_H_

/* used communication peripheral modules */
#include "Lpspi_Ip.h"
#define SPI_TIMEOUT_PER_BYTE   10   /* byte data transfer timeout in ms */

#define SJA1110_SPI_Config  	Lpspi_Ip_PhyUnitConfig_SpiPhyUnit_SJA1110_Instance_2
#define SJA1110_SPI_ExtDevice   &Lpspi_Ip_DeviceAttributes_SpiExternalDevice_SJA1110_Instance_2
#define SJA1110_SPI_INSTANCE 2u


#define SPI_MAX_TRANSFER_SIZE 4096

#define SJA1110_SPI_PRINT_INFO 0

#define SPI_SYNC_TRANSFER 0

typedef enum
{
	SJA1110_STATE_INITIALIZING = 0x31,
	SJA1110_STATE_WAITING = 0x33,
    SJA1110_STATE_DOWNLOADING = 0x34,
    SJA1110_STATE_COMPLETED = 0x36,
    SJA1110_STATE_VALIDATING_IMG_META_DATA = 0x37
} SJA1110_Ip_StateType;


Lpspi_Ip_StatusType SJA1110_SPI_Init(void);
uint8_t SJA1110_100BASE_T1_Status_Read(uint8_t *status);
void SJA1110_100BASE_T1_Status_Details( uint8_t status);
SJA1110_Ip_StateType SJA1110_SDL_Send_Payload(uint32_t number,uint8_t *data);
uint8_t DMA_Hw_Err_Clear(void);
void SPI_Callback(uint8 u8Instance, Lpspi_Ip_EventType event);
void SPI_SimpleTest(void);

#endif /* SJA1110_SPI_H_ */
