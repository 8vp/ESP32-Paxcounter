#ifndef _IRQHANDLER_H
#define _IRQHANDLER_H

#define DISPLAY_IRQ         0x01
#define BUTTON_IRQ          0x02
#define SENDPAYLOAD_IRQ     0x04
#define CYCLIC_IRQ          0x08

#include "globals.h"
#include "cyclic.h"
#include "button.h"
#include "display.h"
#include "cyclic.h"
#include "senddata.h"

void irqHandler(void *pvParameters);

#endif
