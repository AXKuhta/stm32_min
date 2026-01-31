
#include "stm32h7xx_hal.h"
#include "tusb.h"

void system_init();
void display_init();
void display_enable();
void display_disable();
void display_r();
void display_g();
void display_b();
void usb_hw_init();
void adc_init();
void analog_init();
void timers_init();

int main() {
	__HAL_RCC_GPIOG_CLK_ENABLE();

	HAL_GPIO_Init(GPIOG, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_7, .Mode = GPIO_MODE_OUTPUT_OD, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST });
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, 0); // Led fed from 3.3V

	system_init();

	usb_hw_init();

	tusb_rhport_init_t dev_init = {
		.role = TUSB_ROLE_DEVICE,
		.speed = TUSB_SPEED_AUTO
	};
	tusb_init(1, &dev_init);

	adc_init();
	analog_init();
	timers_init();

	//display_init();
	//display_enable();
	//display_g();

	uint32_t next_msg_at = 1000;

	while (1) {
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, HAL_GetTick() % 1000 < 200);
		tud_task();

		if (HAL_GetTick() > next_msg_at) {
			tud_cdc_write("hello\r\n", 7);
			tud_cdc_write_flush();
			next_msg_at += 1000;
		}
	}
}
