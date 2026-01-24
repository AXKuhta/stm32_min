
#include "stm32h7xx_hal.h"

// https://github.com/GolinskiyKonstantin/STM32_Lib_TFT_ST7789/blob/master/st7789/st7789.h
#define RGB565(r, g, b)         (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

void display_init() {
	HAL_GPIO_Init(GPIOG, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_12, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST }); // Backlight
	HAL_GPIO_Init(GPIOG, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_15, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST }); // Display command
	HAL_GPIO_Init(GPIOG, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_8 , .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST }); // Display NSS
	HAL_GPIO_Init(GPIOG, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_13, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST }); // Display SCK
	HAL_GPIO_Init(GPIOG, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_14, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST }); // Display MOSI

	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_12, 1); // Backlight active high
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, 1); // SPI clock idle high
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_15, 1); // DC also idle high
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8 , 1); // Deselected
}

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

void put_pixels(uint16_t color) {
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

void display_enable() {
	display_cmd(0x11);

	// Init in 16 bit color
	display_cmd(0x3A);
	display_data(0x55);

	display_cmd(0x21);
	display_cmd(0x13);
	display_cmd(0x29);
}

void display_disable() {
	display_cmd(0x10);
}

void display_r() { put_pixels( RGB565(255, 0, 0) ); }
void display_g() { put_pixels( RGB565(0, 255, 0) ); }
void display_b() { put_pixels( RGB565(0, 0, 255) ); }
