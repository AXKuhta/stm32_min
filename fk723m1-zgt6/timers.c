
#include "stm32h7xx_hal.h"

TIM_HandleTypeDef emit_timer; // TIM2 CH4 on PB11
TIM_HandleTypeDef chop_timer; // TIM1 CH3 on PE13

void TIM2_IRQHandler() {
	HAL_TIM_IRQHandler(&emit_timer);
}

void TIM1_UP_IRQHandler() {
	HAL_TIM_IRQHandler(&chop_timer);
}

void emit_timer_init(uint16_t period) {
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_TIM2_CLK_ENABLE();

	HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_11, .Mode = GPIO_MODE_AF_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST, .Alternate = GPIO_AF1_TIM2 });

	chop_timer = (TIM_HandleTypeDef) {
		.Instance = TIM2,
		.Init = {
			.Prescaler = 0,
			.CounterMode = TIM_COUNTERMODE_UP,
			.Period = period,
			.ClockDivision = TIM_CLOCKDIVISION_DIV1,
		}
	};

	HAL_TIM_PWM_Init(&chop_timer);

	// PWM1 = starts high
	// PWM2 = starts low
	HAL_TIM_PWM_ConfigChannel(&chop_timer, &(TIM_OC_InitTypeDef){
		.OCMode = TIM_OCMODE_PWM2,
		.Pulse = period/2 + 1, // 50% duty cycle
		.OCPolarity = TIM_OCPOLARITY_HIGH,
		.OCNPolarity = TIM_OCPOLARITY_LOW,
		.OCFastMode = TIM_OCFAST_DISABLE
	}, TIM_CHANNEL_4);

	HAL_NVIC_EnableIRQ(TIM2_IRQn);
	HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);

	HAL_TIM_PWM_Start(&chop_timer, TIM_CHANNEL_4);
}

void chop_timer_init(uint16_t period) {
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_TIM1_CLK_ENABLE();

	HAL_GPIO_Init(GPIOE, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_11, .Mode = GPIO_MODE_AF_OD, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST, .Alternate = GPIO_AF1_TIM1 });
	HAL_GPIO_Init(GPIOE, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_10, .Mode = GPIO_MODE_AF_OD, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST, .Alternate = GPIO_AF1_TIM1 });

	chop_timer = (TIM_HandleTypeDef) {
		.Instance = TIM1,
		.Init = {
			.Prescaler = 0,
			.CounterMode = TIM_COUNTERMODE_UP,
			.Period = period,
			.ClockDivision = TIM_CLOCKDIVISION_DIV1,
		}
	};

	HAL_TIM_PWM_Init(&chop_timer);

	// PWM1 = starts high
	// PWM2 = starts low
	HAL_TIM_PWM_ConfigChannel(&chop_timer, &(TIM_OC_InitTypeDef){
		.OCMode = TIM_OCMODE_PWM2,
		.Pulse = period/2 + 1, // 50% duty cycle
		.OCPolarity = TIM_OCPOLARITY_HIGH,
		.OCNPolarity = TIM_OCPOLARITY_LOW,
		.OCFastMode = TIM_OCFAST_DISABLE
	}, TIM_CHANNEL_2);

	HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);
	HAL_NVIC_SetPriority(TIM1_UP_IRQn, 0, 0);

	HAL_TIM_PWM_Start(&chop_timer, TIM_CHANNEL_2);
	HAL_TIMEx_PWMN_Start(&chop_timer, TIM_CHANNEL_2);
}

void timers_init() {
	//chop_timer_init(2 - 1); // 850 kHz
	//chop_timer_init(26 - 1); // 850 kHz
	//emit_timer_init(25 - 1); // 1.0 MHz

	chop_timer_init(2 - 1); // 850 kHz
	//emit_timer_init(275 - 1); // 1.0 MHz
}
