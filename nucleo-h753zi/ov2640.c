#include "stm32h7xx_hal.h"

//
// Start with this STM32F4 one:
// https://github.com/openmv/openmv/blob/v1.1/src/omv/sccb.c
// https://github.com/openmv/openmv/blob/v1.1/src/omv/ov2640.c
// https://github.com/openmv/openmv/blob/v1.1/src/omv/sensor.c
//
// Afterwards, cherry-pick from more complete one:
// https://github.com/openmv/openmv/blob/v5.0.0/drivers/sensors/ov2640.h
// https://github.com/openmv/openmv/blob/v5.0.0/drivers/sensors/ov2640.c
// https://github.com/openmv/openmv/blob/v5.0.0/ports/stm32/omv_csi.c
//

//
// PC6    DCMI_D0
// PC7    DCMI_D1
// PC8    DCMI_D2
// PC9    DCMI_D3
//
// PE4    DCMI_D4
// PD3    DCMI_D5
// PE5    DCMI_D6
// PE6    DCMI_D7
//
// PA4    DCMI_HSYNC
// PB7    DCMI_VSYNC
//
// PA6    DCMI_PIXCLK
//
// PA5    FLASH
// PE8    PWDN (Active high)
// PE7    RESET (Active low)
//
// PD12   SCL
// PD13   SDA
//

DCMI_HandleTypeDef hdcmi;
I2C_HandleTypeDef sccb;

void DCMI_IRQHandler() {
	HAL_DCMI_IRQHandler(&hdcmi);
}

void init_ov2640() {
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

	HAL_GPIO_Init(GPIOE, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_7, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST });
	HAL_GPIO_Init(GPIOE, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_8, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST });
	HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_5, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST });

	HAL_GPIO_Init(GPIOC, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_6, .Mode = GPIO_MODE_AF_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST, .Alternate = GPIO_AF13_DCMI });
	HAL_GPIO_Init(GPIOC, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_7, .Mode = GPIO_MODE_AF_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST, .Alternate = GPIO_AF13_DCMI });
	HAL_GPIO_Init(GPIOC, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_8, .Mode = GPIO_MODE_AF_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST, .Alternate = GPIO_AF13_DCMI });
	HAL_GPIO_Init(GPIOC, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_9, .Mode = GPIO_MODE_AF_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST, .Alternate = GPIO_AF13_DCMI });

	HAL_GPIO_Init(GPIOE, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_4, .Mode = GPIO_MODE_AF_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST, .Alternate = GPIO_AF13_DCMI });
	HAL_GPIO_Init(GPIOD, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_3, .Mode = GPIO_MODE_AF_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST, .Alternate = GPIO_AF13_DCMI });
	HAL_GPIO_Init(GPIOE, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_5, .Mode = GPIO_MODE_AF_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST, .Alternate = GPIO_AF13_DCMI });
	HAL_GPIO_Init(GPIOE, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_6, .Mode = GPIO_MODE_AF_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST, .Alternate = GPIO_AF13_DCMI });

	HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_4, .Mode = GPIO_MODE_AF_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST, .Alternate = GPIO_AF13_DCMI });
	HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_7, .Mode = GPIO_MODE_AF_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST, .Alternate = GPIO_AF13_DCMI });
	HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_6, .Mode = GPIO_MODE_AF_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST, .Alternate = GPIO_AF13_DCMI });

	HAL_GPIO_Init(GPIOD, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_12, .Mode = GPIO_MODE_AF_OD, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST, .Alternate = GPIO_AF4_I2C4 });
	HAL_GPIO_Init(GPIOD, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_13, .Mode = GPIO_MODE_AF_OD, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST, .Alternate = GPIO_AF4_I2C4 });

	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_7, 0);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 0);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 1);

	HAL_Delay(10);

	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_7, 1);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 0);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 0);

	HAL_Delay(10);

	__HAL_RCC_DCMI_CLK_ENABLE();
	__HAL_RCC_I2C4_CLK_ENABLE();

	sccb = (I2C_HandleTypeDef) {
		.Instance = I2C4,
		.Init = {
			.AddressingMode  = I2C_ADDRESSINGMODE_7BIT,
			.Timing          = 0xB0C33131,
			.DualAddressMode = I2C_DUALADDRESS_DISABLED,
			.GeneralCallMode = I2C_GENERALCALL_ENABLED,
			.NoStretchMode   = I2C_NOSTRETCH_DISABLED,
			.OwnAddress1     = 0xFE,
			.OwnAddress2     = 0xFE
		}
	};

	if (HAL_OK != HAL_I2C_Init(&sccb)) while (1) {};

#define BANK_SEL                0xFF
#define BANK_SEL_DSP            0x00
#define BANK_SEL_SENSOR         0x01

	if (HAL_OK != HAL_I2C_Master_Transmit(&sccb, 0x60, (uint8_t[]){ BANK_SEL, BANK_SEL_SENSOR }, 2, 1000)) while (1) {};

	/*
	hdcmi = (DCMI_HandleTypeDef) {
		.Instance = DCMI,
		.Init = {
			.VSPolarity  = DCMI_VSPOLARITY_LOW,
			.HSPolarity  = DCMI_HSPOLARITY_LOW,
			.PCKPolarity = DCMI_PCKPOLARITY_RISING,
			.SynchroMode = DCMI_SYNCHRO_HARDWARE,
			.CaptureRate = DCMI_CR_ALL_FRAME,
			.ExtendedDataMode = DCMI_EXTEND_DATA_8B,
			.JPEGMode = 0
		}
	};

	HAL_NVIC_SetPriority(DCMI_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DCMI_IRQn);

	if (HAL_OK != HAL_DCMI_Init(&hdcmi)) while (1) {};*/
}
