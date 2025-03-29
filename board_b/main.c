
#include "stm32h7xx_hal.h"

extern void system_init();

int main() {
	system_init();

	while(1) {};
}
