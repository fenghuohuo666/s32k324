/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-09-03     Wyj          General Function library
 */

#ifndef __RCP_BUILD_H__
#define __RCP_BUILD_H__

#include "def.h"
#include "type.h"
#include "codec.h"

#define PROTOCOL_VERSION 0x01
#define PROTOCOL_DEVICEID 0x01
#define RCPMESSAGE_HEAD_SIZE 20

RcpMessage *rcp_build_message(uint8_t channel, size_t flag, void* payload, size_t payload_size, size_t payload_type);

#endif /* __RCP_BUILD_H__ */
