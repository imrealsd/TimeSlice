#include "main.h"



void userTask1(void) {

	while (1) {
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
		HAL_Delay(1000);
	}
}


void userTask2(void) {

	while (1) {
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
		HAL_Delay(500);
	}
}
