/**
 * GENERATED FILE
 */
#include <kernel/irq.h>

#include "main.h"
#include "stm32f7xx_it.h"

#define EMBOX_STM32_IRQ_HANDLER(prefix) \
	static irq_return_t embox_##prefix##_IRQHandler(unsigned int irq_nr, void *data) { \
		prefix##_IRQHandler(); \
		return IRQ_HANDLED; \
	}

EMBOX_STM32_IRQ_HANDLER(OTG_HS)
EMBOX_STM32_IRQ_HANDLER(SDMMC1)
EMBOX_STM32_IRQ_HANDLER(DMA2_Stream6)
EMBOX_STM32_IRQ_HANDLER(DMA2_Stream3)


extern void (*cortexm_external_clock_hnd)(void);

int embox_stm32_setup_irq_handlers(void) {
	int res = 0;

	cortexm_external_clock_hnd = SysTick_Handler;

	res |= irq_attach(OTG_HS_IRQn + 16, embox_OTG_HS_IRQHandler, 0, NULL, "OTG_HS_IRQHandler");
	res |= irq_attach(SDMMC1_IRQn + 16, embox_SDMMC1_IRQHandler, 0, NULL, "SDMMC1_IRQHandler");
	res |= irq_attach(DMA2_Stream6_IRQn + 16, embox_DMA2_Stream6_IRQHandler, 0, NULL, "DMA2_Stream6_IRQHandler");
	res |= irq_attach(DMA2_Stream3_IRQn + 16, embox_DMA2_Stream3_IRQHandler, 0, NULL, "DMA2_Stream3_IRQHandler");


	return res;
}
