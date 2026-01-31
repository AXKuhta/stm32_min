
#include "stm32h7xx_hal.h"

OPAMP_HandleTypeDef opamp1;
OPAMP_HandleTypeDef opamp2;

void bias_init() {
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

	HAL_GPIO_Init(GPIOG, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_1, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST });
	HAL_GPIO_Init(GPIOE, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_10, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST });
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, 1);
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_1, 0);
}

// OPAMP_PGA_GAIN_2_OR_MINUS_1,
// OPAMP_PGA_GAIN_16_OR_MINUS_15,

// Opamp1:
// PE9 +
// PE8 bias
// PE7 output
void opamp2_init() {
	__HAL_RCC_OPAMP_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

	HAL_GPIO_Init(GPIOE, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_9, .Mode = GPIO_MODE_ANALOG, .Pull = GPIO_NOPULL });
	HAL_GPIO_Init(GPIOE, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_8, .Mode = GPIO_MODE_ANALOG, .Pull = GPIO_NOPULL });
	HAL_GPIO_Init(GPIOE, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_7, .Mode = GPIO_MODE_ANALOG, .Pull = GPIO_NOPULL });

	opamp2 = (OPAMP_HandleTypeDef) {
		.Instance = OPAMP2,
		.Init = {
			.PowerMode = OPAMP_POWERMODE_NORMAL,
			.Mode = OPAMP_PGA_MODE,
			.PgaConnect = OPAMP_PGA_CONNECT_INVERTINGINPUT_IO0_BIAS,
			.NonInvertingInput = OPAMP_NONINVERTINGINPUT_IO0,
			.PgaGain = OPAMP_PGA_GAIN_16_OR_MINUS_15,
			.UserTrimming = OPAMP_TRIMMING_FACTORY
		}
	};

	if(HAL_OK != HAL_OPAMP_Init(&opamp2)) { while (1) {} }
	if(HAL_OK != HAL_OPAMP_SelfCalibrate(&opamp2)) { while (1) {} }
	if(HAL_OK != HAL_OPAMP_Start(&opamp2)) { while (1) {} }
}

void analog_init() {
	bias_init();
	opamp2_init();
}
