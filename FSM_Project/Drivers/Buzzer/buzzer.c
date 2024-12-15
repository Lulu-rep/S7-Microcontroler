/*
 * buzzer.c
 *
 *  Created on: Nov 27, 2024
 *      Author: Adrien
 */

#include "buzzer.h"

static TypeDef_Note *pNotes = NULL;
static size_t notes_sz = 0;

Buzzer_status init_buzzer(TypeDef_Note* _TabNote, size_t _notes_sz){
	if(_TabNote == NULL) return BUZZER_ERROR;
	if(_notes_sz <= 0) return BUZZER_ERROR;

	for(int i = 0; i < _notes_sz; i++){
		double note_frequency = _TabNote[i].frequency;
		double tim3_clk_freq = (double) HAL_RCC_GetHCLKFreq();
		_TabNote[i].ARR = (uint16_t)((tim3_clk_freq/note_frequency) - 1.0 );
	}

	pNotes = _TabNote;
	notes_sz = _notes_sz;


	return BUZZER_OK;
}

Buzzer_status execute_buzzer(TIM_HandleTypeDef * _htim, int _note_selected){
	if(pNotes == NULL) return BUZZER_ERROR;
	if(notes_sz <= 0) return BUZZER_ERROR;

	__HAL_TIM_SET_COMPARE(_htim, TIM_CHANNEL_2, CCRX_200US);
	__HAL_TIM_SET_AUTORELOAD(_htim, pNotes[_note_selected].ARR);

	if (TIM_CHANNEL_STATE_GET(_htim, TIM_CHANNEL_2) == HAL_TIM_CHANNEL_STATE_BUSY){
		return BUZZER_BUSY;
	}

	if (HAL_TIM_Base_Start(_htim) != HAL_OK) {
		return BUZZER_ERROR;
	}

	if(HAL_TIM_PWM_Start(_htim, TIM_CHANNEL_2) != HAL_OK){
		return BUZZER_ERROR;
	}

	return BUZZER_OK;
}

int next_note(int _note_selected, TIM_HandleTypeDef* _htim){
	if (_note_selected < notes_sz - 1) {
		_note_selected++;
		__HAL_TIM_SET_AUTORELOAD(_htim, pNotes[_note_selected].ARR);
		__HAL_TIM_SET_COUNTER(_htim, 0);
	}
	return _note_selected;
}

int previous_note(int _note_selected, TIM_HandleTypeDef* _htim){
	if (_note_selected > 0) {
		_note_selected--;
		__HAL_TIM_SET_AUTORELOAD(_htim, pNotes[_note_selected].ARR);
		__HAL_TIM_SET_COUNTER(_htim, 0);
	}
	return _note_selected;
}

Buzzer_status kill_buzzer(TIM_HandleTypeDef * _htim){

	__HAL_TIM_SET_AUTORELOAD(_htim, 65535);
	__HAL_TIM_SET_COUNTER(_htim, 0);

	if(HAL_TIM_PWM_Stop(_htim, TIM_CHANNEL_2) != HAL_OK){
		return BUZZER_ERROR;
	}

	if (HAL_TIM_Base_Stop(_htim) != HAL_OK) {
		return BUZZER_ERROR;
	}

	return BUZZER_OK;
}
