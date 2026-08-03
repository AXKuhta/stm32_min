#include "stm32h7xx_hal.h"

void init_clock(void);

int main() {
	HAL_Init();

	init_clock();

	while (1) {};
}
