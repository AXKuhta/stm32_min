
// tinyusb/hw/bsp/stm32h7/family.c
// tinyusb/hw/bsp/stm32h7/boards/stm32h723nucleo/board.h

#include "stm32h7xx_hal.h"
#include "tusb.h"

void OTG_HS_IRQHandler(void) {
	tusb_int_handler(1, true);
}

void usb_hw_init(void) {
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_USB1_OTG_HS_CLK_ENABLE();

	// Only one peripheral
	HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_11, .Mode = GPIO_MODE_AF_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_HIGH, .Alternate = GPIO_AF10_OTG1_HS });
	HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_12, .Mode = GPIO_MODE_AF_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_HIGH, .Alternate = GPIO_AF10_OTG1_HS });


	// Disable VBUS sense (B device) via pin PA9
	USB_OTG_HS->GCCFG &= ~USB_OTG_GCCFG_VBDEN;

	// B-peripheral session valid override enable
	USB_OTG_HS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOEN;
	USB_OTG_HS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOVAL;
}

size_t board_get_unique_id(uint8_t id[], size_t max_len) {
	(void) max_len;
	volatile uint32_t * stm32_uuid = (volatile uint32_t *) UID_BASE;
	uint32_t* id32 = (uint32_t*) (uintptr_t) id;
	uint8_t const len = 12;

	id32[0] = stm32_uuid[0];
	id32[1] = stm32_uuid[1];
	id32[2] = stm32_uuid[2];

	return len;
}

uint32_t board_millis(void) {
	return HAL_GetTick();
}

uint32_t tusb_time_millis_api(void) {
	return board_millis();
}
