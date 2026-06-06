/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-09     Alex_min	   first version
 */

#ifndef __OUT_H_
#define __OUT_H_

#include "IfxStdIf_DPipe.h"

extern IfxStdIf_DPipe  g_ascStandardInterface;

/**
 * @brief This function implement the printf function.
 *
 * @param fd is unuesd.
 *
 * @param buf is the printed string.
 *
 * @param buf is the length of the printed string.
 *
 * @return the length of the string that has been sent.
 */
int _write(int fd, void *buf, size_t count)
{
    if (g_ascStandardInterface.txDisabled)
        return 0;

    Ifx_SizeT sent = (Ifx_SizeT)count;

    IfxStdIf_DPipe_write(&g_ascStandardInterface, buf, &sent, TIME_INFINITE);

    return (int)sent;
}

#endif /* __OUT_H_ */
