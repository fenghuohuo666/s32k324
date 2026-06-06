/*
 * SJA1110_APP.h
 *
 *  Created on: 2022Äê3ÔÂ10ÈÕ
 *      Author: nxf58909
 */

#ifndef SJA1110_APP_H_
#define SJA1110_APP_H_

#include "SJA1110_SPI.h"

// #define FIRMWARE_LENGTH 0x44c88 /*The length of the firmware, including the header 0x20 bytes + payload 0x44c68 bytes */
//#define FIRMWARE_LENGTH    0x216a0  /*The length of the firmware, including the header 0x20 bytes + payload 0x21680 bytes */
#define FIRMWARE_LENGTH    0x9800// 0x92d0 // 0x91c8 //0x96a0//0x96c8  /*The length of the firmware, including the header 0x20 bytes + payload 0x96a8 bytes */
#define SJA1110_SPI_TRANSFER_SIZE 24 //400  //4000 /*The packet size of each SPI transfer, this value need to be divide exactly by 4 and no more than 4096*/

void SJA1110_SDL_Boot(void);

#endif /* SJA1110_APP_H_ */
