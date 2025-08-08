
#include "stm32h7xx_hal.h"

TIM_HandleTypeDef chop_timer;

// Used for verification purposes by comparing with uwTick
uint32_t tim3_wraps = 0;

void TIM3_IRQHandler() {
	HAL_TIM_IRQHandler(&chop_timer);
	tim3_wraps++;
}

// TIM3 CH4	PB1
// Runs at 240 MHz
void chop_timer_init(uint16_t period) {
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_TIM3_CLK_ENABLE();

	HAL_GPIO_Init(GPIOB, &(GPIO_InitTypeDef){ .Pin = GPIO_PIN_1, .Mode = GPIO_MODE_AF_OD, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FAST, .Alternate = GPIO_AF2_TIM3 });

	chop_timer = (TIM_HandleTypeDef) {
		.Instance = TIM3,
		.Init = {
			.Prescaler = 240 - 1,
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
		.OCFastMode = TIM_OCFAST_DISABLE
	}, TIM_CHANNEL_4);

	HAL_NVIC_EnableIRQ(TIM3_IRQn);
	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);

	HAL_TIM_PWM_Start_IT(&chop_timer, TIM_CHANNEL_4);
}
