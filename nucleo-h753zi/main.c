#include "stm32h7xx_hal.h"

void init_clock(void);
void init_gc2145();

void gc2145_capture();

uint32_t captures = 0;

int main() {
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

	HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_0, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST });
	HAL_GPIO_Init(GPIOE, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_1, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST });
	HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_14, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST });
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 1);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_1, 1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);

	HAL_Init();

	init_clock();

	init_gc2145();

	while (1) {
		gc2145_capture();

		captures++;
	};
}
