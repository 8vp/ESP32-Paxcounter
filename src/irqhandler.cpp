#include "irqhandler.h"

// Local logging tag
static const char TAG[] = __FILE__;

// irq handler task, handles all our application level interrupts
void irqHandler(void *pvParameters) {

  configASSERT(((uint32_t)pvParameters) == 1); // FreeRTOS check

  uint32_t InterruptStatus;

  // task remains in blocked state until it is notified by an irq
  for (;;) {
    xTaskNotifyWait(0x00,             // Don't clear any bits on entry
                    ULONG_MAX,        // Clear all bits on exit
                    &InterruptStatus, // Receives the notification value
                    portMAX_DELAY);   // wait forever

// button pressed?
#ifdef HAS_BUTTON
    if (InterruptStatus & BUTTON_IRQ)
      readButton();
#endif

// display needs refresh?
#ifdef HAS_DISPLAY
    if (InterruptStatus & DISPLAY_IRQ)
      refreshtheDisplay();
#endif

    // are cyclic tasks due?
    if (InterruptStatus & CYCLIC_IRQ) {
      doHousekeeping();
    }

    // time to be synced?
    if (InterruptStatus & TIMESYNC_IRQ) {
      setTime(timeProvider());
    }

    // is time to send the payload?
    if (InterruptStatus & SENDCOUNTER_IRQ)
      sendCounter();
  }
  vTaskDelete(NULL); // shoud never be reached
}

// esp32 hardware timer triggered interrupt service routines
// they notify the irq handler task

#ifdef HAS_DISPLAY
void IRAM_ATTR DisplayIRQ() {
  xTaskNotifyFromISR(irqHandlerTask, DISPLAY_IRQ, eSetBits, NULL);
  portYIELD_FROM_ISR();
}
#endif

#ifdef HAS_BUTTON
void IRAM_ATTR ButtonIRQ() {
  xTaskNotifyFromISR(irqHandlerTask, BUTTON_IRQ, eSetBits, NULL);
  portYIELD_FROM_ISR();
}
#endif
