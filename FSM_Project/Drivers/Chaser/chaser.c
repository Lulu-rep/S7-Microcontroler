/*
 * chasser.c
 *
 *  Created on: Nov 26, 2024
 *      Author: reppl
 */

#include "chaser.h"

static TypeDefLed *pLeds = NULL;
static size_t leds_sz = 0;
static int led_index = 0;

/**
 *	@brief Init chaser on ISEN32 Board
 *	@param none
 *	@retval Chaser_status either ERROR or OK
 */
Chaser_status init_chaser(TypeDefLed* _TabLed, size_t _leds_sz){
	if(_TabLed == NULL) return CHASER_ERROR;
	if(_leds_sz <= 0) return CHASER_ERROR;
	if(_leds_sz > MAX_LED) return CHASER_ERROR;

	pLeds = _TabLed;
	leds_sz = _leds_sz;

	return CHASER_OK;
}

/**
 *	@brief Execute the chasser on ISEN32 Board's LEDs 1 to 7
 *	@param	none
 *	@retval Chaser_status either ERROR or OK
 */
Chaser_status execute_chaser(void){

	if(pLeds == NULL) return CHASER_ERROR;
	if(leds_sz <= 0) return CHASER_ERROR;
	if(leds_sz > MAX_LED) return CHASER_ERROR;

	HAL_GPIO_TogglePin(pLeds[led_index].Port, pLeds[led_index].Pin);

	led_index = (led_index >= leds_sz - 1) ? 0 : led_index +1;

	return CHASER_OK;
}

int decrease_tempo_chaser(int _tempo_selected, TIM_HandleTypeDef _htim6, uint32_t* _tempos){
    if (_tempo_selected > 0) {
        _tempo_selected--;
        __HAL_TIM_SET_AUTORELOAD(&_htim6, _tempos[_tempo_selected]);
        __HAL_TIM_SET_COUNTER(&_htim6, 0);
    }
    return _tempo_selected;
}

int increase_tempo_chaser(int _tempo_selected, TIM_HandleTypeDef _htim6, uint32_t* _tempos, size_t _tempo_sz){
    if (_tempo_selected < _tempo_sz - 1) {
        _tempo_selected++;
        __HAL_TIM_SET_AUTORELOAD(&_htim6, _tempos[_tempo_selected]);
        __HAL_TIM_SET_COUNTER(&_htim6, 0);
    }
    return _tempo_selected;
}

Chaser_status kill_chaser(TIM_HandleTypeDef* _htim6){

	if(pLeds == NULL) return CHASER_ERROR;
	if(leds_sz <= 0) return CHASER_ERROR;
	if(leds_sz > MAX_LED) return CHASER_ERROR;

	led_index = 0;

	if (HAL_TIM_Base_Stop_IT(_htim6) != HAL_OK) {
		return CHASER_ERROR;
	}

	for(int i = 0; i< leds_sz; i++){
		HAL_GPIO_WritePin(pLeds[i].Port, pLeds[i].Pin, GPIO_PIN_RESET);
	}



	return CHASER_OK;
}
