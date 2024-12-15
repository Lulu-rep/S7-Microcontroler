/*
 * jukebox.h
 *
 *  Created on: Dec 9, 2024
 *      Author: reppl
 */

#ifndef JUKEBOX_JUKEBOX_H_
#define JUKEBOX_JUKEBOX_H_

#define MUTE "-"
#define CCRX_200US 4800UL

#include "stm32l1xx_hal.h"
#include "config.h"
#include "string.h"
typedef enum {
	JUKEBOX_OK,
	JUKEBOX_BUSY,
	JUKEBOX_ERROR,
}Jukebox_status;

Jukebox_status init_jukebox(TypeDef_Note* _TabNote, size_t _notes_sz, size_t _jukebox_partition_sz);

Jukebox_status execute_jukebox(char* _Jukebox_partition[], TIM_HandleTypeDef* _htim);

Jukebox_status jukebox_play_note(TIM_HandleTypeDef *_htim, TypeDef_Note *_note);

Jukebox_status jukebox_mute(TIM_HandleTypeDef *_htim);

Jukebox_status jukebox_play_note_by_name(TIM_HandleTypeDef *_htim, TypeDef_Note *_notes,
		size_t _notes_sz,char* _name);

Jukebox_status stop_music(TIM_HandleTypeDef* _htim);

Jukebox_status start_music(TIM_HandleTypeDef* _htim3,TIM_HandleTypeDef* _htim6);

int decrease_tempo_jukebox(int _tempo_selected, TIM_HandleTypeDef* _htim6, uint32_t* _tempos);

int increase_tempo_jukebox(int _tempo_selected, TIM_HandleTypeDef* _htim6, uint32_t* _tempos, size_t _tempo_sz);
#endif /* JUKEBOX_JUKEBOX_H_ */
