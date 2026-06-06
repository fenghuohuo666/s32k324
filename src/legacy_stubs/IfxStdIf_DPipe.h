#ifndef IFXSTDIF_DPIPE_H
#define IFXSTDIF_DPIPE_H
#include "Ifx_Types.h"
typedef struct { void *null; } IfxStdIf_DPipe;
void IfxStdIf_DPipe_write(IfxStdIf_DPipe *dpipe, uint8 *data, uint32 *size, uint32 timeout);
boolean IfxStdIf_DPipe_read(IfxStdIf_DPipe *dpipe, uint8 *data, uint32 *size, uint32 timeout);
#endif
