/*
 * jukebox.c
 *
 *  Created on: Dec 9, 2024
 *      Author: reppl
 */

#include "jukebox.h"


static TypeDef_Note *pNotes = NULL;
static size_t notes_sz = 0;
static size_t jukebox_partition_sz =0;

Jukebox_status init_jukebox(TypeDef_Note* _TabNote, size_t _notes_sz, size_t _jukebox_partition_sz){
	if(_TabNote == NULL) return JUKEBOX_ERROR;
	if(_notes_sz <= 0) return JUKEBOX_ERROR;

	for(int i = 0; i < _notes_sz; i++){
		double note_frequency = _TabNote[i].frequency;
		double tim3_clk_freq = (double) HAL_RCC_GetHCLKFreq();
		_TabNote[i].ARR = (uint16_t)((tim3_clk_freq/note_frequency) - 1.0 );
	}

	pNotes = _TabNote;
	notes_sz = _notes_sz;
	jukebox_partition_sz = _jukebox_partition_sz;

	return JUKEBOX_OK;
}

Jukebox_status execute_jukebox(char* _Jukebox_partition[], TIM_HandleTypeDef* _htim){
    static int note_index = 0;

    jukebox_play_note_by_name(_htim, pNotes, notes_sz, _Jukebox_partition[note_index]);

    note_index++;

    if (note_index >= jukebox_partition_sz) {
        note_index = 0;
    }
    return JUKEBOX_OK;
}

Jukebox_status jukebox_play_note(TIM_HandleTypeDef *_htim, TypeDef_Note *_note) {
    HAL_StatusTypeDef tim3_status = HAL_OK;
    __HAL_TIM_SET_COMPARE(_htim, TIM_CHANNEL_2, CCRX_200US);
    __HAL_TIM_SET_AUTORELOAD(_htim, _note->ARR);

    if (TIM_CHANNEL_STATE_GET(_htim, TIM_CHANNEL_2) == HAL_TIM_CHANNEL_STATE_BUSY) {
        return JUKEBOX_BUSY;
    }
    tim3_status = HAL_TIM_PWM_Start(_htim, TIM_CHANNEL_2);
    if (tim3_status != HAL_OK) {
        printf("Failed to start PWM: %d\r\n", tim3_status);
        return JUKEBOX_ERROR;
    }
    printf("Playing note: %s\r\n", _note->name);
    return JUKEBOX_OK;
}

Jukebox_status jukebox_mute(TIM_HandleTypeDef *_htim) {
    HAL_StatusTypeDef tim3_status = HAL_OK;
    tim3_status = HAL_TIM_PWM_Stop(_htim, TIM_CHANNEL_2);
    if (tim3_status != HAL_OK) {
        printf("Failed to stop PWM: %d\r\n", tim3_status);
        return JUKEBOX_ERROR;
    }
    printf("Buzzer muted.\r\n");
    return JUKEBOX_OK;
}

Jukebox_status jukebox_play_note_by_name(TIM_HandleTypeDef *_htim, TypeDef_Note *_notes,
		size_t _notes_sz,char* _name) {
	if (strcmp(_name, MUTE) == 0) {
		jukebox_mute(_htim);
		return JUKEBOX_OK;
	}
	for (int i = 0; i < _notes_sz; i++) {
		const char *note_name = _notes[i].name;
		if (strcmp(note_name, _name) == 0) {
			jukebox_play_note(_htim, &(_notes[i]));
			return JUKEBOX_OK;
		}
	}
	return JUKEBOX_ERROR;
}

Jukebox_status kill_jukebox(TIM_HandleTypeDef* _htim3,TIM_HandleTypeDef* _htim6) {
	__HAL_TIM_SET_AUTORELOAD(_htim3, 65535);
	__HAL_TIM_SET_COUNTER(_htim3, 0);

	__HAL_TIM_SET_AUTORELOAD(_htim6, 100);
	__HAL_TIM_SET_COUNTER(_htim6, 0);
	if (HAL_TIM_Base_Stop_IT(_htim3) != HAL_OK) {
		return JUKEBOX_ERROR;
	}
	if (HAL_TIM_PWM_Stop(_htim3, TIM_CHANNEL_2) != HAL_OK) {
		return JUKEBOX_ERROR;
	}
	return JUKEBOX_OK;
}

Jukebox_status start_jukebox(TIM_HandleTypeDef* _htim3,TIM_HandleTypeDef* _htim6) {
	if (TIM_CHANNEL_STATE_GET(_htim3, TIM_CHANNEL_2)
			== HAL_TIM_CHANNEL_STATE_READY) {
		if (HAL_TIM_PWM_Start(_htim3, TIM_CHANNEL_2) != HAL_OK) {
			return JUKEBOX_ERROR;
		}
	}
	if (_htim6->State == HAL_TIM_STATE_READY) {
		if (HAL_TIM_Base_Start_IT(_htim6) != HAL_OK) {
			return JUKEBOX_ERROR;
		}
	}
	return JUKEBOX_OK;
}

int increase_tempo_jukebox(int _tempo_selected, TIM_HandleTypeDef* _htim6, uint32_t* _tempos){
    if (_tempo_selected > 0) {
        _tempo_selected--;
        __HAL_TIM_SET_AUTORELOAD(_htim6, _tempos[_tempo_selected]);
        __HAL_TIM_SET_COUNTER(_htim6, 0);
    }
    return _tempo_selected;
}

int decrease_tempo_jukebox(int _tempo_selected, TIM_HandleTypeDef* _htim6, uint32_t* _tempos, size_t _tempo_sz){
    if (_tempo_selected < _tempo_sz - 1) {
        _tempo_selected++;
        __HAL_TIM_SET_AUTORELOAD(_htim6, _tempos[_tempo_selected]);
        __HAL_TIM_SET_COUNTER(_htim6, 0);
    }
    return _tempo_selected;
}
