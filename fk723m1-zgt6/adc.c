
#include "stm32h7xx_hal.h"
#include "tusb.h"

//
// PB0 is:
// - ADC12_CH9
// - OPAMP1_VINP
//

static ADC_HandleTypeDef hadc1;
static DMA_HandleTypeDef hdma_adc1;

static ADC_ChannelConfTypeDef sConfig;

uint16_t adc_buf[480];

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	if (tud_vendor_write_available() >= 480) tud_vendor_write(adc_buf + 240, 480);
	(void)hadc;
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
	if (tud_vendor_write_available() >= 480) tud_vendor_write(adc_buf, 480);
	(void)hadc;
}

void DMA1_Stream1_IRQHandler(void) {
	HAL_DMA_IRQHandler(hadc1.DMA_Handle);
}

// DMAMUX is nice
void adc_dma_init() {
	__HAL_RCC_DMA1_CLK_ENABLE();

	HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);

	hdma_adc1 = (DMA_HandleTypeDef) {
		.Instance = DMA1_Stream1,
		.Init = {
			.Request             = DMA_REQUEST_ADC1,
			.Direction           = DMA_PERIPH_TO_MEMORY,
			.PeriphInc           = DMA_PINC_DISABLE,
			.MemInc              = DMA_MINC_ENABLE,
			.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
			.MemDataAlignment    = DMA_MDATAALIGN_BYTE,
			.Mode                = DMA_CIRCULAR,
			.Priority            = DMA_PRIORITY_MEDIUM
		}
	};

	if (HAL_DMA_Init(&hdma_adc1) != HAL_OK) {
		while (1) {}
	}

	__HAL_LINKDMA(&hadc1, DMA_Handle, hdma_adc1);
}

// https://github.com/STMicroelectronics/STM32CubeH7/blob/master/Projects/NUCLEO-H743ZI/Examples/ADC/ADC_DMA_Transfer/Src/main.c
// ADC bus clock is 137.5 MHz
void adc_init() {
	__HAL_RCC_ADC12_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_0, .Mode = GPIO_MODE_ANALOG, .Pull = GPIO_NOPULL });

	hadc1 = (ADC_HandleTypeDef) {
		.Instance = ADC1,
		.Init = {
			  .ClockPrescaler           = ADC_CLOCK_SYNC_PCLK_DIV1,      /* Synchronous clock mode, input ADC clock divided by 2, so 68.75 MHz */
			  .Resolution               = ADC_RESOLUTION_8B,            /* 16-bit resolution for converted data */
			  .ScanConvMode             = DISABLE,                       /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
			  .EOCSelection             = ADC_EOC_SINGLE_CONV,           /* EOC flag picked-up to indicate conversion end */
			  .LowPowerAutoWait         = DISABLE,                       /* Auto-delayed conversion feature disabled */
			  .ContinuousConvMode       = ENABLE,                        /* Continuous mode enabled (automatic conversion restart after each conversion) */
			  .NbrOfConversion          = 1,                             /* Parameter discarded because sequencer is disabled */
			  .DiscontinuousConvMode    = DISABLE,                       /* Parameter discarded because sequencer is disabled */
			  .NbrOfDiscConversion      = 1,                             /* Parameter discarded because sequencer is disabled */
			  .ExternalTrigConv         = ADC_SOFTWARE_START,            /* Software start to trig the 1st conversion manually, without external event */
			  .ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE, /* Parameter discarded because software trigger chosen */
			  .ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR, /* ADC DMA circular requested */
			  .Overrun                  = ADC_OVR_DATA_OVERWRITTEN,      /* DR register is overwritten with the last conversion result in case of overrun */
			  .OversamplingMode         = DISABLE                        /* No oversampling */
		}
	};

	if (HAL_ADC_Init(&hadc1) != HAL_OK) {
		while (1) {}
	}

	sConfig = (ADC_ChannelConfTypeDef) {
		.Channel = ADC_CHANNEL_4,
		.Rank = ADC_REGULAR_RANK_1,
		.SamplingTime = ADC_SAMPLETIME_16CYCLES_5,
		.SingleDiff = ADC_SINGLE_ENDED,
		.OffsetNumber = ADC_OFFSET_NONE,
		.Offset = 0
	};

	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		while (1) {}
	}

	// ADC must be stopped for calibration
	if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED) != HAL_OK) {
		while (1) {}
	}

	// Caution: relies on hadc1 being populated
	adc_dma_init();

	if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buf, sizeof(adc_buf)/sizeof(uint8_t)) != HAL_OK) {
		while (1) {}
	}
}
