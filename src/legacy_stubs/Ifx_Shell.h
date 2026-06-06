#ifndef IFX_SHELL_H
#define IFX_SHELL_H
#include "Ifx_Types.h"
#include "Ifx_Console.h"
typedef struct { Ifx_Console *console; } Ifx_Shell;
void Ifx_Shell_init(Ifx_Shell *shell, Ifx_Console *console);
void Ifx_Shell_process(Ifx_Shell *shell);
#endif
