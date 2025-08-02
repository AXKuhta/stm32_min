
#include "stm32h7xx_hal.h"

void system_init();
void dac_init();
void dac_write(uint16_t value);

int main() {
	system_init();
	dac_init();

	while (1) {
		for (int i = 0; i < 4096; i++) {
			dac_write(i);
		}
	};
}
