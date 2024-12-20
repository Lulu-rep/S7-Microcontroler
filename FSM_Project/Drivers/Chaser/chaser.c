/*
 * chasser.c
 *
 *  Created on: Nov 26, 2024
 *      Author: reppl
 */

#include "chaser.h"

//Definition of global variable for the library
static TypeDefLed *pLeds = NULL;
static size_t leds_sz = 0;
static int led_index = 0;

/**
 *	@brief Init chaser on ISEN32 Board
 *	@param Table of Led available
 *	@param Size of the previous table
 *	@retval Chaser_status either ERROR or OK
 */
Chaser_status init_chaser(TypeDefLed* _TabLed, size_t _leds_sz){
	//parameter checking
	if(_TabLed == NULL) return CHASER_ERROR;
	if(_leds_sz <= 0) return CHASER_ERROR;
	if(_leds_sz > MAX_LED) return CHASER_ERROR;

	//init of variable used in library
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

	//parameter checking
	if(pLeds == NULL) return CHASER_ERROR;
	if(leds_sz <= 0) return CHASER_ERROR;
	if(leds_sz > MAX_LED) return CHASER_ERROR;

	//Toggle the current led
	HAL_GPIO_TogglePin(pLeds[led_index].Port, pLeds[led_index].Pin);

	//Update the led index for next execute
	led_index = (led_index >= leds_sz - 1) ? 0 : led_index +1;

	return CHASER_OK;
}

/**
 * @brief Increase the tempo at which the chaser animation is played (higher tempo = faster speed)
 * @param Current tempo selected in the tempo table
 * @param Handle of the timer used by the animation, current = timer6
 * @param Table that contain all the possible tempo that can be choosen by user
 * @retval index of the new selected tempo
 */
int increase_tempo_chaser(int _tempo_selected, TIM_HandleTypeDef* _htim6, uint32_t* _tempos){
	//Check if possible to further increase the animation speed of the chaser
    if (_tempo_selected > 0) {
        _tempo_selected--;

        //Set the Auto reload register of the timer to the corresponding selected tempo
        __HAL_TIM_SET_AUTORELOAD(_htim6, _tempos[_tempo_selected]);
        //Reset of the counter value to avoid overflow
        __HAL_TIM_SET_COUNTER(_htim6, 0);
    }
    return _tempo_selected;
}

/**
 * @brief Decrease the animation speed of the chaser
 * @param Current tempo selected in the tempo table
 * @param Handle of the timer used by the animation, current = timer6
 * @param Table that contain all the possible tempo that can be choosen by user
 * @param Size of the table given previously
 * @retval index of the new selected tempo
 */
int decrease_tempo_chaser(int _tempo_selected, TIM_HandleTypeDef* _htim6, uint32_t* _tempos, size_t _tempo_sz){
    if (_tempo_selected < _tempo_sz - 1) {
        _tempo_selected++;
        //Set the Auto reload register of the timer to the corresponding selected tempo
        __HAL_TIM_SET_AUTORELOAD(_htim6, _tempos[_tempo_selected]);
        //Reset of the counter value to avoid overflow
        __HAL_TIM_SET_COUNTER(_htim6, 0);
    }
    return _tempo_selected;
}


/**
 * @brief Stop the given timer and reset it into his original state
 * @param Handle of the timer to kill / reset
 * @retval Chaser status
 */
Chaser_status kill_chaser(TIM_HandleTypeDef* _htim6){

	//Parameter checking
	if(pLeds == NULL) return CHASER_ERROR;
	if(leds_sz <= 0) return CHASER_ERROR;
	if(leds_sz > MAX_LED) return CHASER_ERROR;

	led_index = 0;

	//Stop the timer
	if (HAL_TIM_Base_Stop_IT(_htim6) != HAL_OK) {
		return CHASER_ERROR;
	}
	//Reset the timer to it's initial state
	__HAL_TIM_SET_AUTORELOAD(_htim6, 100);
	__HAL_TIM_SET_COUNTER(_htim6, 0);

	//Deactivate all the led
	for(int i = 0; i< leds_sz; i++){
		HAL_GPIO_WritePin(pLeds[i].Port, pLeds[i].Pin, GPIO_PIN_RESET);
	}



	return CHASER_OK;
}
