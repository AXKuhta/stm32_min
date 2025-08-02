
#include "stm32h7xx_hal.h"

static DAC_HandleTypeDef hdac1;
static DAC_ChannelConfTypeDef sConfig;

// https://github.com/STMicroelectronics/STM32CubeH7/blob/master/Projects/NUCLEO-H723ZG/Examples/DAC/DAC_SignalsGeneration/Src/main.c
void dac_init() {
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_DAC12_CLK_ENABLE();

	HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_4, .Mode = GPIO_MODE_ANALOG, .Pull = GPIO_NOPULL });

	hdac1 = (DAC_HandleTypeDef) {
		.Instance = DAC1
	};

	if (HAL_DAC_Init(&hdac1) != HAL_OK) {
		while (1) {}
	}

	// DAC_TRIGGER_NONE		Value effective as soon as set
	// DAC_TRIGGER_SOFTWARE		Value effective after SWTRIG in HAL_DAC_Start()
	sConfig = (DAC_ChannelConfTypeDef) {
		.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE,
		.DAC_Trigger = DAC_TRIGGER_NONE,
		.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE,
		.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_EXTERNAL,
		.DAC_UserTrimming = DAC_TRIMMING_FACTORY,
		.DAC_TrimmingValue = 0
	};

	// Updates .DAC_UserTrimming and .DAC_TrimmingValue
	if (HAL_DACEx_SelfCalibrate(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK) {
		while (1) {}
	}

	if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK) {
		while (1) {}
	}

	if (HAL_DAC_Start(&hdac1, DAC_CHANNEL_1) != HAL_OK) {
		while (1) {}
	}
}

// Max value: 4095
void dac_write(uint16_t value) {
	if (HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, value) != HAL_OK) {
		while (1) {}
	}

	if (HAL_DAC_GetValue(&hdac1, DAC_CHANNEL_1) != value) {
		while (1) {}
	}
}
