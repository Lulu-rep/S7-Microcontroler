/*
 * verstappen.c
 *
 *  Created on: Dec 14, 2024
 *      Author: Adrien
 */

#include "motor.h"

static TypeDef_Speed *pSpeeds = NULL;
static size_t speeds_sz = 0;

Motor_status init_motor(TypeDef_Speed* _TabSpeed, size_t _speeds_sz, TIM_HandleTypeDef * _htim){
	if(_TabSpeed == NULL) return MOTOR_ERROR;
	if(_speeds_sz <= 0) return MOTOR_ERROR;


	int arr_tim3 = __HAL_TIM_GET_AUTORELOAD(_htim);
	for(int i = 0; i < _speeds_sz; i++){
			double speed_ratio = _TabSpeed[i].ratio;
			_TabSpeed[i].Pulse = (uint16_t)(speed_ratio*arr_tim3);
		}

	pSpeeds = _TabSpeed;
	speeds_sz = _speeds_sz;


	return MOTOR_OK;
}

Motor_status execute_motor(TIM_HandleTypeDef * _htim, int _speed_selected){
	if(pSpeeds == NULL) return MOTOR_ERROR;
	if(speeds_sz <= 0) return MOTOR_ERROR;

	__HAL_TIM_SET_COMPARE(_htim, TIM_CHANNEL_1, pSpeeds[_speed_selected].Pulse);

	if (HAL_TIM_Base_Start(_htim) != HAL_OK) {
		return MOTOR_ERROR;
	}

	if(HAL_TIM_PWM_Start(_htim, TIM_CHANNEL_1) != HAL_OK){
		return MOTOR_ERROR;
	}

	return MOTOR_OK;
}

int speed_up(int _speed_selected, TIM_HandleTypeDef* _htim){
	if (_speed_selected < speeds_sz - 1) {
		_speed_selected++;
		__HAL_TIM_SET_COMPARE(_htim, TIM_CHANNEL_1, pSpeeds[_speed_selected].Pulse);
	}
	return _speed_selected;
}

int speed_down(int _speed_selected, TIM_HandleTypeDef* _htim){
	if (_speed_selected > 0) {
		_speed_selected--;
		__HAL_TIM_SET_COMPARE(_htim, TIM_CHANNEL_1, pSpeeds[_speed_selected].Pulse);
	}
	return _speed_selected;
}

Motor_status kill_motor(TIM_HandleTypeDef * _htim){

	__HAL_TIM_SET_AUTORELOAD(_htim, 65535);
	__HAL_TIM_SET_COUNTER(_htim, 0);

	if(HAL_TIM_PWM_Stop(_htim, TIM_CHANNEL_1) != HAL_OK){
		return MOTOR_ERROR;
	}

	if (HAL_TIM_Base_Stop(_htim) != HAL_OK) {
		return MOTOR_ERROR;
	}

	return MOTOR_OK;
}
