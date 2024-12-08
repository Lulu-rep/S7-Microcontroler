/*
 * chaser.h
 *
 *  Created on: Nov 26, 2024
 *      Author: reppl
 */

#ifndef CHASER_CHASER_H_
#define CHASER_CHASER_H_

#include "stm32l1xx_hal.h"

//Number of maximum LED on the board current = 8 LEDs
#define MAX_LED 8


typedef enum {
	CHASER_OK,
	CHASER_ERROR,
}Chaser_status;


//Structure for the mapping of LED Port and Pin named TypeDefLed
typedef struct{
    GPIO_TypeDef* Port;
    uint16_t Pin;
} TypeDefLed;


Chaser_status init_chaser(TypeDefLed* _TabLed, size_t _leds_sz);

Chaser_status execute_chaser(void);

int decrease_tempo_chaser(int _tempo_selected, TIM_HandleTypeDef* _htim6, uint32_t* _tempos);

int increase_tempo_chaser(int _tempo_selected, TIM_HandleTypeDef* _htim6, uint32_t* _tempos, size_t _tempo_sz);

Chaser_status kill_chaser(TIM_HandleTypeDef* _htim6);

#endif /* CHASER_CHASER_H_ */
