
#include "stm32h7xx_hal.h"

void system_init();
void display_init();
void display_enable();
void display_disable();
void display_r();
void display_g();
void display_b();

int main() {
	__HAL_RCC_GPIOG_CLK_ENABLE();

	HAL_GPIO_Init(GPIOG, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_7, .Mode = GPIO_MODE_OUTPUT_OD, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST });
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, 0); // Led fed from 3.3V

	system_init();
	display_init();
	display_enable();
	display_g();

	while (1) {
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, HAL_GetTick() % 1000 < 200);
	}
}
