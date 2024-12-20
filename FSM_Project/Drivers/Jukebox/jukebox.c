/*
 * jukebox.c
 *
 *  Created on: Dec 9, 2024
 *      Author: reppl
 */

#include "jukebox.h"

//Definition of the global variables used in this library
static TypeDef_Note *pNotes = NULL;
static size_t notes_sz = 0;
static size_t jukebox_partition_sz =0;

/**
 *	@brief Initialize the jukebox on the micro-controller
 *	@param Table of notes available to be played
 *	@param Size of the previous table
 *	@param Size of the partition that will be played
 *	@retval Jukebox status
 */
Jukebox_status init_jukebox(TypeDef_Note* _TabNote, size_t _notes_sz, size_t _jukebox_partition_sz){
	//Parameter checking
	if(_TabNote == NULL) return JUKEBOX_ERROR;
	if(_notes_sz <= 0) return JUKEBOX_ERROR;
	if(_jukebox_partition_sz <=0) return JUKEBOX_ERROR;

	//Set the ARR for all the desired frequency contained in the notes table
	for(int i = 0; i < _notes_sz; i++){
		double note_frequency = _TabNote[i].frequency;
		double tim3_clk_freq = (double) HAL_RCC_GetHCLKFreq();
		_TabNote[i].ARR = (uint16_t)((tim3_clk_freq/note_frequency) - 1.0 );
	}

	//Set the global variables that will be used in the library
	pNotes = _TabNote;
	notes_sz = _notes_sz;
	jukebox_partition_sz = _jukebox_partition_sz;

	return JUKEBOX_OK;
}
/**
 *	@brief Execute the jukebox by playing tye next note in the partition
 *	@param	Partition to play
 *	@param SIze of the partition
 *	@retval Jukebox status
 */
Jukebox_status execute_jukebox(char* _Jukebox_partition[], TIM_HandleTypeDef* _htim){
    static int note_index = 0;

    //Play the current note on the buzzer
    jukebox_play_note_by_name(_htim, pNotes, notes_sz, _Jukebox_partition[note_index]);

    note_index++;

    //Check if the partition as reached the end
    if (note_index >= jukebox_partition_sz) {
        note_index = 0;
    }
    return JUKEBOX_OK;
}

/**
 *	@brief Play the current note of the partition using the buzzer of the board
 *	@param Handle of the timer used to manage the buzzer (current timer 3)
 *	@param Note to be played
 *	@retval Jukebox status
 */
Jukebox_status jukebox_play_note(TIM_HandleTypeDef *_htim, TypeDef_Note *_note) {
    HAL_StatusTypeDef tim3_status = HAL_OK;
    //Set the timer ARR to values to play the current note
    __HAL_TIM_SET_COMPARE(_htim, TIM_CHANNEL_2, CCRX_200US);
    __HAL_TIM_SET_AUTORELOAD(_htim, _note->ARR);

    //Check if the timer is not busy
    if (TIM_CHANNEL_STATE_GET(_htim, TIM_CHANNEL_2) == HAL_TIM_CHANNEL_STATE_BUSY) {
        return JUKEBOX_BUSY;
    }
    //Start the Timer
    tim3_status = HAL_TIM_PWM_Start(_htim, TIM_CHANNEL_2);
    if (tim3_status != HAL_OK) {
        printf("Failed to start PWM: %d\r\n", tim3_status);
        return JUKEBOX_ERROR;
    }
    printf("Playing note: %s\r\n", _note->name);
    return JUKEBOX_OK;
}

/**
 *	@brief Stop the buzzer to play a mute sound
 *	@param Handle of the timer used to manage the buzzer (current timer 3 channel 2)
 *	@retval Jukebox status
 */
Jukebox_status jukebox_mute(TIM_HandleTypeDef *_htim) {
    HAL_StatusTypeDef tim3_status = HAL_OK;
    //Stop the timer
    tim3_status = HAL_TIM_PWM_Stop(_htim, TIM_CHANNEL_2);
    if (tim3_status != HAL_OK) {
        printf("Failed to stop PWM: %d\r\n", tim3_status);
        return JUKEBOX_ERROR;
    }
    printf("Buzzer muted.\r\n");
    return JUKEBOX_OK;
}

/**
 *	@brief Play one note given it's name
 *	@param Handle of the timer used to manage the buzzer (current timer 3 channel 2)
 *	@param Table of all available notes
 *	@retval Jukebox status
 */
Jukebox_status jukebox_play_note_by_name(TIM_HandleTypeDef *_htim, TypeDef_Note *_notes,
		size_t _notes_sz,char* _name) {
	//Check if the current note is a mute
	if (strcmp(_name, MUTE) == 0) {
		//play the mute sound
		jukebox_mute(_htim);
		return JUKEBOX_OK;
	}
	//Check if the current note is available to play
	for (int i = 0; i < _notes_sz; i++) {
		const char *note_name = _notes[i].name;
		if (strcmp(note_name, _name) == 0) {
			//play the note
			jukebox_play_note(_htim, &(_notes[i]));
			return JUKEBOX_OK;
		}
	}
	return JUKEBOX_ERROR;
}
/**
 * @brief Stop the jukebox's timers and reset them into their original states
 * @param Handle of the timer 3
 * @param Handle of the timer 6
 * @retval Chaser status
 */
Jukebox_status kill_jukebox(TIM_HandleTypeDef* _htim3,TIM_HandleTypeDef* _htim6) {
	//Reset the timer 3
	__HAL_TIM_SET_AUTORELOAD(_htim3, 65535);
	__HAL_TIM_SET_COUNTER(_htim3, 0);

	//Reset the timer 6
	__HAL_TIM_SET_AUTORELOAD(_htim6, 100);
	__HAL_TIM_SET_COUNTER(_htim6, 0);

	//Stop timer 3
	if (HAL_TIM_Base_Stop_IT(_htim3) != HAL_OK) {
		return JUKEBOX_ERROR;
	}
	//Stop timer 6
	if (HAL_TIM_PWM_Stop(_htim3, TIM_CHANNEL_2) != HAL_OK) {
		return JUKEBOX_ERROR;
	}
	return JUKEBOX_OK;
}


/**
 * @brief Start the jukebox's timer upon entering Jukebox state
 * @param Handle of the timer 3
 * @param Handle of the timer 6
 * @retval Chaser status
 */
Jukebox_status start_jukebox(TIM_HandleTypeDef* _htim3,TIM_HandleTypeDef* _htim6) {
	//Start timer 3 channel 2 if not already started
	if (TIM_CHANNEL_STATE_GET(_htim3, TIM_CHANNEL_2)
			== HAL_TIM_CHANNEL_STATE_READY) {
		if (HAL_TIM_PWM_Start(_htim3, TIM_CHANNEL_2) != HAL_OK) {
			return JUKEBOX_ERROR;
		}
	}
	//Start timer 6 if not started
	if (_htim6->State == HAL_TIM_STATE_READY) {
		if (HAL_TIM_Base_Start_IT(_htim6) != HAL_OK) {
			return JUKEBOX_ERROR;
		}
	}
	return JUKEBOX_OK;
}

/**
 * @brief Increase the speed at which the jukebox animation is played
 * @param Current tempo selected in the tempo table
 * @param Handle of the timer used by the animation, current = timer6
 * @param Table that contain all the possible tempo that can be choosen by user
 * @retval index of the new selected tempo
 */
int increase_tempo_jukebox(int _tempo_selected, TIM_HandleTypeDef* _htim6, uint32_t* _tempos){
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
 * @brief Decrease the animation speed of the jukebox
 * @param Current tempo selected in the tempo table
 * @param Handle of the timer used by the animation, current = timer6
 * @param Table that contain all the possible tempo that can be choosen by user
 * @param Size of the table given previously
 * @retval index of the new selected tempo
 */
int decrease_tempo_jukebox(int _tempo_selected, TIM_HandleTypeDef* _htim6, uint32_t* _tempos, size_t _tempo_sz){
    if (_tempo_selected < _tempo_sz - 1) {
        _tempo_selected++;
        //Set the Auto reload register of the timer to the corresponding selected tempo
        __HAL_TIM_SET_AUTORELOAD(_htim6, _tempos[_tempo_selected]);
        //Reset of the counter value to avoid overflow
        __HAL_TIM_SET_COUNTER(_htim6, 0);
    }
    return _tempo_selected;
}
