/*
 * buzzer.h
 *
 *  Created on: Nov 27, 2024
 *      Author: Adrien
 */

#ifndef BUZZER_BUZZER_H_
#define BUZZER_BUZZER_H_

#include "stm32l1xx_hal.h"

#define CCRX_200US 4800UL


typedef enum {
	BUZZER_OK,
	BUZZER_BUSY,
	BUZZER_ERROR,
}Buzzer_status;

typedef struct{
	const char *name;
	double frequency;
	uint16_t ARR;
}TypeDef_Note;


Buzzer_status init_buzzer(TypeDef_Note* _TabNote, size_t _notes_sz);

Buzzer_status execute_buzzer(TIM_HandleTypeDef * _htim, int _note_selected);

int next_note(int _note_selected, TIM_HandleTypeDef *_htim);

int previous_note(int _note_selected, TIM_HandleTypeDef *_htim);

Buzzer_status kill_buzzer(TIM_HandleTypeDef * _htim);

#endif /* BUZZER_BUZZER_H_ */
