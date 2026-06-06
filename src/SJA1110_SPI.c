/*
 * SJA1110_SPI.c
 *
 *  Created on: 2021Äê10ÔÂ18ÈÕ
 *      Author: nxa07657
 */
//printf Header
#include "stdio.h"
#include "Siul2_Port_Ip.h"
#include "Siul2_Dio_Ip.h"

#include "SJA1110_SPI.h"


#define PORT5_MASK  0x08
#define PORT6_MASK  0x10
#define PORT7_MASK  0x20
#define PORT8_MASK  0x40
#define PORT9_MASK  0x80

#define PORT_TESTED  (PORT5_MASK|PORT6_MASK|PORT7_MASK|PORT8_MASK)



Lpspi_Ip_StatusType SJA1110_SPI_Init(void)
{
	Lpspi_Ip_StatusType spi_status = LPSPI_IP_STATUS_SUCCESS;

	/* configure the LPSPI for communication with SJA1110 */
	spi_status = Lpspi_Ip_Init(&SJA1110_SPI_Config);
	/* Enable SPI Interrupt mode, it is polling mode by default.*/
	Lpspi_Ip_UpdateTransferMode(SJA1110_SPI_INSTANCE,LPSPI_IP_INTERRUPT);

	return spi_status;
}

uint8_t SJA1110_100BASE_T1_Status_Read(uint8_t *status)
{
	Lpspi_Ip_StatusType spi_status = LPSPI_IP_STATUS_SUCCESS;
	uint8_t Msg_Header_TxBuffer[4] = {0};

	/* dump read the SJA1110 status via SPI interface */
	spi_status = Lpspi_Ip_SyncTransmit(SJA1110_SPI_ExtDevice, Msg_Header_TxBuffer, status, 1, 1000 * SPI_TIMEOUT_PER_BYTE);

	return spi_status;
}

/**
 * for FCC test, connect SJA1110 100BSE-T1 port 5 with port6 and port 7 with port 8, according to master and slave pairs
 */
void SJA1110_100BASE_T1_Status_Details( uint8_t status)
{
	printf("ENET status value is: %x.\n",status);

	if(PORT_TESTED == (status & PORT_TESTED))
	{
		printf("All testing SJA1110 100BASE-T1 ports work OK!\r\n");
	}
	else /* failed, print the failure/error details */
	{
		/* check port 5 */
		if(0==(status & PORT5_MASK))
		{
			printf("Error: SJA1110 100BASE-T1_Port 5 link failed\r\n");
		}

		/* check port 6 */
		if(0==(status & PORT6_MASK))
		{
			printf("Error: SJA1110 100BASE-T1_Port 6 link failed\r\n");
		}

		/* check port 7 */
		if(0==(status & PORT7_MASK))
		{
			printf("Error: SJA1110 100BASE-T1_Port 7 link failed\r\n");
		}

		/* check port 8 */
		if(0==(status & PORT8_MASK))
		{
			printf("Error: SJA1110 100BASE-T1_Port 8 link failed\r\n");
		}
	}
}


/*********************************************************************************************/
/**
* @brief   SPI transfer event callback.
* @details It is only a demo.Registered in Lpspi_Ip_AsyncTransmit();
*
* @param[in]      u8Instance  	SPI instance
* @return         event         SPI transfer event type
* @implements  SPI_transfer_event_callback_function
*/
/*********************************************************************************************/
void SPI_Callback(uint8 u8Instance, Lpspi_Ip_EventType event)
{
	if(2u == u8Instance)
	{
		if(event == LPSPI_IP_EVENT_END_TRANSFER)
		{
			;
		}
	}

}

//Dinesh
#include "SJA1110_app.h"
uint8_t Msg_Header_RxBuffer__global[4];

#pragma GCC section bss ".mcal_bss_no_cacheable"
__attribute__(( aligned(32) )) uint8_t Packet_status_400[SJA1110_SPI_TRANSFER_SIZE];
__attribute__(( aligned(32) )) uint8_t Msg_Header_RxBuffer[4];
#pragma GCC section bss

extern void DevAssert_local(volatile boolean x);
/****************************************
 * brief: This function transfers the bin file of SJA1110 by SPI.
 * @param: number - The number of the bin file to be sent, including the payload + the header. The number need to be divided exactly by 4 and less than 4096.
 * @param: data - The bin file to be sent.
 ***************************************/
SJA1110_Ip_StateType SJA1110_SDL_Send_Payload(uint32_t number,uint8_t *data)
{
//    uint8_t Msg_Header_RxBuffer[4];
    Lpspi_Ip_HwStatusType SPI_status;

    /*If the number can't be divided exactly by 4 and more than 4096 bytes, assert*/
    DevAssert_local(number%4 == 0 && number <= 4096);

#if SJA1110_SPI_PRINT_INFO
	printf("start SJA1110 firmware uploading.\n");
#endif

    /*set LPSPI2 PCS1 low to start the transfer*/
    Siul2_Dio_Ip_ClearPins(LPSPI2_SJA1110_PCS1_PORT,(1<<LPSPI2_SJA1110_PCS1_PIN));

    if(number > 4)
    {   /*Transfer the major data, no need to receive status.*/
        #if SPI_SYNC_TRANSFER
    	Lpspi_Ip_SyncTransmit(SJA1110_SPI_ExtDevice, data, NULL, number - 4, 1000 * SPI_TIMEOUT_PER_BYTE);
        #else
    	//Lpspi_Ip_AsyncTransmit(SJA1110_SPI_ExtDevice, data, NULL, number - 4, SPI_Callback);
    	//Dinesh
    	Lpspi_Ip_AsyncTransmit(SJA1110_SPI_ExtDevice, data, Packet_status_400, number - 4, SPI_Callback);
    	do
    	{
    		SPI_status = Lpspi_Ip_GetStatus(SJA1110_SPI_INSTANCE);
    	}while(LPSPI_IP_BUSY == SPI_status);
        #endif

	    /*Transfer the last 4 bytes and get the status from SJA1110*/
        #if SPI_SYNC_TRANSFER
	    Lpspi_Ip_SyncTransmit(SJA1110_SPI_ExtDevice, data + number - 4, Msg_Header_RxBuffer, 4, 1000 * SPI_TIMEOUT_PER_BYTE);
        #else
	    Lpspi_Ip_AsyncTransmit(SJA1110_SPI_ExtDevice, data + number - 4, Msg_Header_RxBuffer, 4, SPI_Callback);
	    do
    	{
    		SPI_status = Lpspi_Ip_GetStatus(SJA1110_SPI_INSTANCE);
    	}while(LPSPI_IP_BUSY == SPI_status);
        #endif
    }
	else
	{
		/*the transfer number is 4*/
        #if SPI_SYNC_TRANSFER
		Lpspi_Ip_SyncTransmit(SJA1110_SPI_ExtDevice, data, Msg_Header_RxBuffer, 4, 1000 * SPI_TIMEOUT_PER_BYTE);
        #else
		Lpspi_Ip_AsyncTransmit(SJA1110_SPI_ExtDevice, data, Msg_Header_RxBuffer, 4, SPI_Callback);
    	do
    	{
    		SPI_status = Lpspi_Ip_GetStatus(SJA1110_SPI_INSTANCE);
    	}while(LPSPI_IP_BUSY == SPI_status);
        #endif
	}

	/*set LPSPI2 PCS1 high to end the transfer*/
	Siul2_Dio_Ip_SetPins(LPSPI2_SJA1110_PCS1_PORT,(1<<LPSPI2_SJA1110_PCS1_PIN));

#if SJA1110_SPI_PRINT_INFO
	printf("SJA1110 firmware upload completed.\n");

    /*print the status value from SJA1110 to get the current status*/
	printf("received SPI value: %x,%x,%x,%x.\n,",Msg_Header_RxBuffer[0],Msg_Header_RxBuffer[1],Msg_Header_RxBuffer[2],Msg_Header_RxBuffer[3]);
#endif

	Msg_Header_RxBuffer__global[0] = Msg_Header_RxBuffer[0];
	Msg_Header_RxBuffer__global[1] = Msg_Header_RxBuffer[1];
	Msg_Header_RxBuffer__global[2] = Msg_Header_RxBuffer[2];
	Msg_Header_RxBuffer__global[3] = Msg_Header_RxBuffer[3];
	return Msg_Header_RxBuffer[1];
}

