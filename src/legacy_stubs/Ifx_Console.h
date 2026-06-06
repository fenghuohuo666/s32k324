#ifndef IFX_CONSOLE_H
#define IFX_CONSOLE_H
#include "Ifx_Types.h"
#include "IfxStdIf_DPipe.h"
typedef struct { IfxStdIf_DPipe *io; } Ifx_Console;
void Ifx_Console_init(Ifx_Console *console, IfxStdIf_DPipe *io);
#endif
