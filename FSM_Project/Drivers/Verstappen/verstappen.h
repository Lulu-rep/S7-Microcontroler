/*
 * verstappen.h
 *
 *  Created on: Dec 14, 2024
 *      Author: Adrien
 */

#ifndef VERSTAPPEN_VERSTAPPEN_H_
#define VERSTAPPEN_VERSTAPPEN_H_

#include "stm32l1xx_hal.h"

typedef enum {
	MOTOR_OK,
	MOTOR_ERROR,
}Motor_status;

typedef struct{
	double ratio;
	uint16_t Pulse;
}TypeDef_Speed;

Motor_status init_motor(TypeDef_Speed* _TabSpeed, size_t _speeds_sz, TIM_HandleTypeDef * _htim);

Motor_status execute_motor(TIM_HandleTypeDef * _htim, int _speed_selected);

int speed_up(int _speed_selected, TIM_HandleTypeDef* _htim);

int speed_down(int _speed_selected, TIM_HandleTypeDef* _htim);

Motor_status kill_motor(TIM_HandleTypeDef * _htim);



#endif /* VERSTAPPEN_VERSTAPPEN_H_ */
