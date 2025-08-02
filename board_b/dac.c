
#include "stm32h7xx_hal.h"

static DAC_HandleTypeDef hdac1;
static DAC_ChannelConfTypeDef sConfig;
static TIM_HandleTypeDef htim;
static DMA_HandleTypeDef hdma_dac1;

// Debugging aid
DMA_Stream_TypeDef* dma_stream = DMA1_Stream5;

void TIM6_DAC_IRQHandler() {
	HAL_TIM_IRQHandler(&htim);
	HAL_DAC_IRQHandler(&hdac1);
}

// Fires often
void DMA1_Stream5_IRQHandler() {
	HAL_DMA_IRQHandler(hdac1.DMA_Handle1);
}

// If stored in RAM, we get hit with a DMA transfer error instantly as DAC tries to fetch the initial value
// Why..?
// Store it in flash for now
static const uint16_t buffer[5] = {0, 1024, 2048, 3072, 4095};

// We'd like Fs = 48 kHz
// Timer runs at 240 MHz right now
// 240 MHz / 48 kHz = 5000
// So Period = 4999
//
// Verify frequency by:
// - Enabling interrupts
// - Counting wraps in IRQ handler
// - Referencing against value of uwTick
void tim_init() {
	__HAL_RCC_TIM6_CLK_ENABLE();

	htim = (TIM_HandleTypeDef){
		.Instance = TIM6,
		.Init = {
			.Period = 4999,
			.Prescaler = 0,
			.ClockDivision = 0,
			.CounterMode = TIM_COUNTERMODE_UP,
			.RepetitionCounter = 0
		}
	};

	TIM_MasterConfigTypeDef sMasterConfig = {
		.MasterOutputTrigger = TIM_TRGO_UPDATE,
		.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE
	};

	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
	HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 0, 0);

	HAL_TIM_Base_Init(&htim);

	HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig);

	HAL_TIM_Base_Start(&htim);
}

// https://github.com/STMicroelectronics/STM32CubeH7/blob/master/Projects/NUCLEO-H723ZG/Examples/DAC/DAC_SignalsGeneration/Src/stm32h7xx_hal_msp.c
void dma_init() {
	__HAL_RCC_DMA1_CLK_ENABLE();

	hdma_dac1 = (DMA_HandleTypeDef) {
		.Instance = DMA1_Stream5,
		.Init = {
			.Request  = DMA_REQUEST_DAC1,
			.Direction = DMA_MEMORY_TO_PERIPH,
			.PeriphInc = DMA_PINC_DISABLE,
			.MemInc = DMA_MINC_ENABLE,
			.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD,
			.MemDataAlignment = DMA_MDATAALIGN_HALFWORD,
			.Mode = DMA_CIRCULAR,
			.Priority = DMA_PRIORITY_HIGH
		}
	};

	if (HAL_DMA_Init(&hdma_dac1) != HAL_OK) {
		while (1) {}
	};

	__HAL_LINKDMA(&hdac1, DMA_Handle1, hdma_dac1);

	HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
}

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

	dma_init();
	tim_init();

	// DAC_TRIGGER_NONE		Value effective as soon as set
	// DAC_TRIGGER_SOFTWARE		Value effective after SWTRIG in HAL_DAC_Start()
	// DAC_TRIGGER_T6_TRGO		Value effective on timer 6 UP (fetched prior?)
	sConfig = (DAC_ChannelConfTypeDef) {
		.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE,
		.DAC_Trigger = DAC_TRIGGER_T6_TRGO,
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

	if (HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *)buffer, sizeof(buffer)/sizeof(uint16_t), DAC_ALIGN_12B_R) != HAL_OK) {
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
