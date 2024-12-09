/*
 * config.h
 *
 *  Created on: Dec 9, 2024
 *      Author: reppl
 */

#ifndef CONFIG_CONFIG_H_
#define CONFIG_CONFIG_H_

#include "stm32l1xx_hal.h"

typedef struct{
	const char *name;
	double frequency;
	uint16_t ARR;
}TypeDef_Note;

#endif /* CONFIG_CONFIG_H_ */
