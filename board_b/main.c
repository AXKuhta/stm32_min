
#include "stm32h7xx_hal.h"

void system_init();
void dac_init();
void dac_write(uint16_t value);
void adc_init();
void chop_timer_init(uint16_t period);

int main() {
	system_init();
	dac_init();
	adc_init();

	chop_timer_init(1000);

	while (1) {}
}
