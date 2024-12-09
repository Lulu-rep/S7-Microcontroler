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


Jukebox_status jukebox_play_note(TIM_HandleTypeDef *_htim, TypeDef_Note *_note);

Jukebox_status jukebox_mute(TIM_HandleTypeDef *_htim);

Jukebox_status jukebox_play_note_by_name(TIM_HandleTypeDef *_htim, TypeDef_Note *_notes,
		size_t _notes_sz, const char *_name);

Jukebox_status stop_music(TIM_HandleTypeDef* _htim);

Jukebox_status start_music(TIM_HandleTypeDef* _htim3,TIM_HandleTypeDef* _htim6);

#endif /* JUKEBOX_JUKEBOX_H_ */
