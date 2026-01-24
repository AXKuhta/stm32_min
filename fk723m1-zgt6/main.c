
#include "stm32h7xx_hal.h"

void system_init();

// MSB first
void display_write(uint8_t byte) {
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, 0);

	for (int i = 0; i < 8; i++) {
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_14, (byte & (0b10000000 >> i)) > 0);
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, 0);
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, 1);
	}
}

void display_cmd(uint8_t byte) {
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8 , 0);
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_15, 0);
	display_write(byte);
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_15, 1);
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8 , 1);
}

void display_data(uint8_t byte) {
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8 , 0);
	display_write(byte);
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8 , 1);
}

void put_pixel(uint16_t color) {
	display_cmd(0x2A);

	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8 , 0);
	display_write(0);
	display_write(0);
	display_write(0);
	display_write(240);
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8 , 1);

	display_cmd(0x2B);

	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8 , 0);
	display_write(0);
	display_write(0);
	display_write(320 >> 8);
	display_write(320 & 0xFF);
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8 , 1);

	display_cmd(0x2C);

	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8 , 0);
	for (int i = 0; i < 240*320; i++) {
		display_write(color >> 8);
		display_write(color & 0xFF);
	}
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8 , 0);
}

int main() {
	__HAL_RCC_GPIOG_CLK_ENABLE();

	HAL_GPIO_Init(GPIOG, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_7, .Mode = GPIO_MODE_OUTPUT_OD, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST });
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, 0); // Led fed from 3.3V

	system_init();

	HAL_GPIO_Init(GPIOG, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_12, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST }); // Backlight
	HAL_GPIO_Init(GPIOG, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_15, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST }); // Display command
	HAL_GPIO_Init(GPIOG, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_8 , .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST }); // Display NSS
	HAL_GPIO_Init(GPIOG, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_13, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST }); // Display SCK
	HAL_GPIO_Init(GPIOG, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_14, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST }); // Display MOSI

	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_12, 1); // Backlight active high
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, 1); // SPI clock idle high
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_15, 1); // DC also idle high
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8 , 0); // Always selected

	HAL_Delay(500);

	while (1) {
		display_cmd(0x11);
		HAL_Delay(100);

		// Init in 16 bit color
		// https://github.com/GolinskiyKonstantin/STM32_Lib_TFT_ST7789/blob/master/st7789/st7789.h
		display_cmd(0x3A);
		display_data(0x55);

		display_cmd(0x13);
		HAL_Delay(100);
		display_cmd(0x29);
		HAL_Delay(100);
		put_pixel( HAL_GetTick() );
		HAL_Delay(100);

		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, HAL_GetTick() % 1000 < 200);

		display_cmd(0x10);
		HAL_Delay(100);
	}
}
