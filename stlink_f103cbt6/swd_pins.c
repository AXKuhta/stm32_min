#include "stm32f1xx_hal.h"

void initPin_PP(void* port, int pin) {
  HAL_GPIO_Init(port, &(GPIO_InitTypeDef) { .Pin = pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_HIGH});
}

void initPin_InputF(void* port, int pin) {
  HAL_GPIO_Init(port, &(GPIO_InitTypeDef) { .Pin = pin, .Mode = GPIO_MODE_INPUT, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_HIGH});
}

void setPin_PP(void* port, int pin, int state) {
  HAL_GPIO_WritePin(port, pin, state);
}

int readPin(void* port, int pin) {
  return HAL_GPIO_ReadPin(port, pin);
}
