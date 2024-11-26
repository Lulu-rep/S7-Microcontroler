/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim3;

/* USER CODE BEGIN PV */

volatile uint32_t btn1_irq_cnt;
volatile uint32_t btn2_irq_cnt;
volatile uint32_t btn3_irq_cnt;
volatile uint32_t btn4_irq_cnt;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

int __io_putchar(int ch){
	ITM_SendChar(ch);
	return ch;
}

typedef enum {
	STATE_START, STATE_CHASER, STATE_BUZZER, STATE_JUKEBOX, STATE_CUSTOM
} FSM_States_Enum;

typedef enum {
	USER_INPUT_OK, USER_INPUT_ERROR
} UserInput_Enum;

typedef enum {
	NOT_EXECUTED, EXECUTED,
} ExecutionState_Enum;

UserInput_Enum user_input = USER_INPUT_ERROR;
int16_t actual_animation = 0;
int16_t target_animation = 0;

FSM_States_Enum current_state = STATE_START;
ExecutionState_Enum execution_state = NOT_EXECUTED;

void (*state_callbacks[5])(void);

void set_new_state(FSM_States_Enum new_state){
	current_state = new_state;
	actual_animation = (actual_animation < 3) ? actual_animation + 1 : 0;
	execution_state = NOT_EXECUTED;
}

void state_start(void){
	if(execution_state == NOT_EXECUTED){
		user_input = USER_INPUT_ERROR;
		printf("Starting.\r\n");
	}
}

void state_chaser(void){
	if(execution_state == NOT_EXECUTED){

	}
}

void state_buzzer(void){
	if(execution_state == NOT_EXECUTED){

	}
}

void state_jukebox(void){
	if(execution_state == NOT_EXECUTED){

	}
}

void state_custom(void){
	if(execution_state == NOT_EXECUTED){

	}
}

void init_fsm(void){
	user_input = USER_INPUT_ERROR;
	actual_animation = 0;
	target_animation = 0;

	state_callbacks[STATE_START] = state_start;
	state_callbacks[STATE_CHASER] = state_chaser;
	state_callbacks[STATE_BUZZER] = state_buzzer;
	state_callbacks[STATE_JUKEBOX] = state_jukebox;
	state_callbacks[STATE_CUSTOM] = state_custom;

	set_new_state(STATE_START);
}

void fsm_project(void){
	state_callbacks[current_state]();

	execution_state = EXECUTED;

	switch(current_state){

	case STATE_START:
		set_new_state(STATE_CHASER);
		break;

	case STATE_CHASER:
		if((user_input == USER_INPUT_OK) && (target_animation == 3)){
			set_new_state(STATE_CUSTOM);
		}else if((user_input == USER_INPUT_OK) && (actual_animation < target_animation)){
			set_new_state(STATE_BUZZER);
		}
		break;

	case STATE_BUZZER:
		if((user_input == USER_INPUT_OK) && (actual_animation < target_animation)){
			set_new_state(STATE_JUKEBOX);
		}else if((user_input == USER_INPUT_OK) && (actual_animation > target_animation)){
			set_new_state(STATE_CHASER);
		}
		break;

	case STATE_JUKEBOX:
		if((user_input == USER_INPUT_OK) && (actual_animation < target_animation)){
			set_new_state(STATE_CUSTOM);
		}else if((user_input == USER_INPUT_OK) && (actual_animation > target_animation)){
			set_new_state(STATE_BUZZER);
		}
		break;

	case STATE_CUSTOM:
		if((user_input == USER_INPUT_OK) && (target_animation == 0)){
			set_new_state(STATE_CHASER);
		}else if((user_input == USER_INPUT_OK) && (actual_animation > target_animation)){
			set_new_state(STATE_JUKEBOX);
		}
		break;

	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if (GPIO_Pin == BTN1_Pin){
		btn1_irq_cnt++;
	}
	else if (GPIO_Pin == BTN2_Pin){
		btn2_irq_cnt++;
	}
	else if (GPIO_Pin == BTN3_Pin){
		btn3_irq_cnt++;
	}
	else if (GPIO_Pin == BTN4_Pin){
		btn4_irq_cnt++;
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  init_fsm();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  fsm_project();

	  if(btn1_irq_cnt){
		  btn1_irq_cnt--;
		  target_animation = (target_animation < 3) ? target_animation + 1 : 0;
	  }
	  if(btn2_irq_cnt){
		  btn2_irq_cnt--;
		  target_animation = (target_animation > 0) ? target_animation - 1 : 0;
	  }
	  if(btn3_irq_cnt){
		  btn3_irq_cnt--;
		  //TO DO
	  }
	  if(btn4_irq_cnt){
		  btn4_irq_cnt--;
		  //TO DO
	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL3;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 23999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 4800;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, L0_Pin|L1_Pin|L2_Pin|L3_Pin
                          |L4_Pin|L5_Pin|L6_Pin|L7_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : BTN4_Pin BTN3_Pin */
  GPIO_InitStruct.Pin = BTN4_Pin|BTN3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : L0_Pin L1_Pin L2_Pin L3_Pin
                           L4_Pin L5_Pin L6_Pin L7_Pin */
  GPIO_InitStruct.Pin = L0_Pin|L1_Pin|L2_Pin|L3_Pin
                          |L4_Pin|L5_Pin|L6_Pin|L7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : BTN1_Pin BTN2_Pin */
  GPIO_InitStruct.Pin = BTN1_Pin|BTN2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */