/*
 * SJA1110_APP.c
 *
 *  Created on: 2022Äê3ÔÂ10ÈÕ
 *      Author: nxf58909
 */

//printf Header
#include "stdio.h"

#include "SJA1110_APP.h"

	SJA1110_Ip_StateType SJA1110_status;
	uint8_t cnt = 10;
	uint8_t SJA1110_needAppFlag = 0;
	uint8_t SDL_bootAbortFlag = 0;
	uint32_t length = FIRMWARE_LENGTH;
	uint32_t SJA1110_SPI_transferSize = SJA1110_SPI_TRANSFER_SIZE;
	//dinesh
	uint32_t inner_counter;
	SJA1110_Ip_StateType SJA1110_status_dummy;
	SJA1110_Ip_StateType SJA1110_status_stream;

#pragma GCC section bss ".mcal_bss_no_cacheable"
	__attribute__(( aligned(32) )) uint8_t dummyData[4] ; //= {0,0,0,0};
	__attribute__(( aligned(32) )) uint8_t SJA1110_FW_Stream_Header[4]; // = {0x00,0x00,0xdd,0x11};/*SJA1110 firmware stream header. Add 2 dummy bytes to align 4 bytes status value*/
#pragma GCC section bss

/*@Brief: This function demonstrate the SJA1110 SDL bootloader example.*/
void SJA1110_SDL_Boot()
{

//	SJA1110_Ip_StateType SJA1110_status;
//	uint8_t cnt = 10;
//	uint8_t SJA1110_needAppFlag = 0;
//	uint8_t SDL_bootAbortFlag = 0;
//	uint32_t length = FIRMWARE_LENGTH;
	// uint8_t dummyData[4] = {0,0,0,0};
//	uint32_t SJA1110_SPI_transferSize = SJA1110_SPI_TRANSFER_SIZE;
	uint8_t * Pointer_Payload = (uint8_t *)0x00500000; /*This is the address where SJA1110 firmware is stored,please modify accordingly.*/

    //uint8_t SJA1110_FW_Stream_Header[4] = {0x00,0x00,0xdd,0x11};/*SJA1110 firmware stream header. Add 2 dummy bytes to align 4 bytes status value*/

	dummyData[0] = 0;
	dummyData[1] = 0;
	dummyData[2] = 0;
	dummyData[3] = 0;

	SJA1110_FW_Stream_Header[0] = 0x00;
	SJA1110_FW_Stream_Header[1] = 0x00;
	SJA1110_FW_Stream_Header[2] = 0xdd;
	SJA1110_FW_Stream_Header[3] = 0x11;
    /*Poll SJA1110 Status to find out if SJA1110 needs App-Image from Companion.*/
	while(SJA1110_STATE_WAITING != SJA1110_status && --cnt)
	{
		SJA1110_status_dummy = SJA1110_status = SJA1110_SDL_Send_Payload(4,dummyData);
	    /*some delay can be added accordingly.*/
	    printf("status = 0x%x,Tried %d times.\n",SJA1110_status,cnt);
	}

	/*If get the WAITING STATE, that means SJA1110 needs the APP image from Companion*/
	if(0 != cnt)
	{
		SJA1110_needAppFlag = 1;
	}

	if(1 == SJA1110_needAppFlag)
	{
		/*Send the stream header*/
		SJA1110_status_stream = SJA1110_status = SJA1110_SDL_Send_Payload(4,SJA1110_FW_Stream_Header);

		while(SJA1110_SPI_transferSize <= length)
		{
			/*Send firmware of SJA1110 by SPI*/
			SJA1110_status = SJA1110_SDL_Send_Payload(SJA1110_SPI_transferSize,Pointer_Payload);

			/*The state should be VALIDATING_IMG_META_DATA then DOWNLOADING when firmware is downloading,
			 * if the state stays in SJA1110_STATE_INITIALIZING or SJA1110_STATE_WAITING, that means
			 * there is an error condition.Need to abort the transmission*/
			if(SJA1110_STATE_WAITING == SJA1110_status || SJA1110_STATE_INITIALIZING == SJA1110_status)
			{
				printf("The SDL Boot is aborted.The firmware is invalid or SWT Reset occurred in SJA1110\n");
				SDL_bootAbortFlag = 1;
				break;
			}
			/*offset the length and data*/
			length -= SJA1110_SPI_transferSize;
			Pointer_Payload += SJA1110_SPI_transferSize;
			//DInesh
			inner_counter++;
		}
		/*If the SDL Boot is aborted, skip the rest procedure.*/
		if(0 == SDL_bootAbortFlag)
		{
			/*send the rest data*/
			if(0 != length)
			{
				SJA1110_status = SJA1110_SDL_Send_Payload(length,Pointer_Payload);
			}

			/*get the complete status*/
			cnt = 10;
			while(SJA1110_STATE_COMPLETED != SJA1110_status && --cnt)
			{
				SJA1110_status = SJA1110_SDL_Send_Payload(4,dummyData);
			}

			if(0 != cnt)
			{
				printf("Upload SJA1110 firmware by SDL completed.\n");
			}
			else
			{
				printf("Didn't get SJA1110 firmware completed state.State = 0x%x\n",SJA1110_status);
			}
		}

	}
	else
	{
		printf("SJA1110 does not need SDL APP Image.\n");
	}

}
