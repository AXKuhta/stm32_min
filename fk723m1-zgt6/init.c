
// STM32CubeH7/Projects/NUCLEO-H723ZG/Examples/RCC/RCC_ClockConfig/Src/main.c

#include "stm32h7xx_hal.h"

void Error_Handler(void) {
	while (1) {}
}

// HSE is 25 MHz
// PLL in is 12.5 MHz
static void system_clock_target(double mhz) {
	double inte = (int)(mhz / 12.5);
	double frac = mhz / 12.5 - inte;

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);
	while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

	RCC_OscInitTypeDef RCC_OscInitStruct = (RCC_OscInitTypeDef){
		.OscillatorType = RCC_OSCILLATORTYPE_HSE,
		.HSEState = RCC_HSE_ON,
		.CSIState = RCC_CSI_OFF,
		.PLL = {
			.PLLState = RCC_PLL_ON,
			.PLLSource = RCC_PLLSOURCE_HSE,
			.PLLM = 2,
			.PLLN = inte,
			.PLLFRACN = frac * 8192.0 + 0.5,
			.PLLP = 1,
			.PLLR = 2,
			.PLLQ = 4,
			.PLLVCOSEL = RCC_PLL1VCOWIDE,
			.PLLRGE = RCC_PLL1VCIRANGE_3
		}
	};

	if (HAL_OK != HAL_RCC_OscConfig(&RCC_OscInitStruct)) { while(1); }

	RCC_ClkInitTypeDef RCC_ClkInitStruct = (RCC_ClkInitTypeDef){
		.ClockType = 	RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 | \
				RCC_CLOCKTYPE_PCLK2  | RCC_CLOCKTYPE_D3PCLK1,

		.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK,
		.SYSCLKDivider = RCC_SYSCLK_DIV1,
		.AHBCLKDivider = RCC_HCLK_DIV2,
		.APB3CLKDivider = RCC_APB3_DIV2,
		.APB1CLKDivider = RCC_APB1_DIV2,
		.APB2CLKDivider = RCC_APB2_DIV2,
		.APB4CLKDivider = RCC_APB4_DIV2
	};

	if (HAL_OK != HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3)) { while(1); }

	// We can disable HSI now
	if (HAL_OK != HAL_RCC_OscConfig(& (RCC_OscInitTypeDef){
		.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSI48,
		.HSIState = RCC_HSI_OFF,
		.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT,
		.HSI48State = RCC_HSI48_OFF,
	})) { while(1); }

	// tinyusb/hw/bsp/stm32h7/boards/stm32h723nucleo/board.h
	RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct = (RCC_PeriphCLKInitTypeDef){
		.PeriphClockSelection = RCC_PERIPHCLK_USB,
		.UsbClockSelection = RCC_USBCLKSOURCE_PLL3,
		.PLL3 = {
			.PLL3M = 5,
			.PLL3N = 48,
			.PLL3FRACN = 0,
			.PLL3P = 1,
			.PLL3R = 2,
			.PLL3Q = 5, // PLLQ used for USB
			.PLL3VCOSEL = RCC_PLL1VCOWIDE,
			.PLL3RGE = RCC_PLL1VCIRANGE_2
		}
	};

	if (HAL_OK != HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct)) { while(1); }
}

static void CPU_CACHE_Enable(void) {
	/* Enable I-Cache */
	SCB_EnableICache();

	/* Enable D-Cache */
	SCB_EnableDCache();
}

void HAL_SYSTICK_Callback(void) {
	HAL_IncTick();
}

void system_init() {
	//CPU_CACHE_Enable();
	HAL_Init();

	system_clock_target(426);
}
