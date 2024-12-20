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
#include "stdio.h"
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
TIM_HandleTypeDef htim6;

/* USER CODE BEGIN PV */
volatile uint32_t btn1_irq_cnt;
volatile uint32_t btn2_irq_cnt;
volatile uint32_t btn3_irq_cnt;
volatile uint32_t btn4_irq_cnt;


//Definition of the table containing the available tempo for the chaser animation
uint32_t tempos[] = { 10, 50, 100, 500, 1000 };
size_t tempo_sz = sizeof(tempos) / sizeof(uint32_t);

//Definition of the table containing the available tempo for the jukebox animation
uint32_t tempos_jukebox[] = {50, 100, 150, 200,300,350,400 };
size_t tempo_jukebox_sz = sizeof(tempos_jukebox) / sizeof(uint32_t);

//Set the default value for each animation
int tempo_selected = 2;

int tempo_jukebox_selected =3;

int note_selected = 0;

int speed_selected = 2;

//Definition of the partition that will be played on the buzzer for the jukebox animation
char *jukebox_partition[] = {
	    "A5", "B5", "C5", "A5", "E5", MUTE, "E5", "D5",MUTE,MUTE,
	    "A5", "B5", "C5", "A5", "D5", MUTE, "D5", "C5","C5", MUTE,MUTE,MUTE,MUTE
	};
//Size of the partition
size_t jukebox_partition_sz = sizeof(jukebox_partition)/ sizeof(char*);
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM6_Init(void);
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
	if(new_state == STATE_CHASER) actual_animation = 0;
	if(new_state == STATE_BUZZER) actual_animation = 1;
	if(new_state == STATE_JUKEBOX) actual_animation = 2;
	if(new_state == STATE_CUSTOM) actual_animation = 3;
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
		if (htim6.State == HAL_TIM_STATE_READY){
			if (HAL_TIM_Base_Start_IT(&htim6) != HAL_OK) {
				Error_Handler();
			}
		}
	}
}

void state_buzzer(void){
	if(execution_state == NOT_EXECUTED){
		if (htim3.State == HAL_TIM_STATE_READY){
			if (HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_2) != HAL_OK){
				Error_Handler();
			}
		}
		execute_buzzer(&htim3, note_selected);
	}
}

void state_jukebox(void){
	if(execution_state == NOT_EXECUTED){
		increase_tempo_jukebox(tempo_jukebox_selected, &htim6, tempos_jukebox);
		if (htim3.State == HAL_TIM_STATE_READY && htim6.State == HAL_TIM_STATE_READY){
            if (start_jukebox(&htim3, &htim6) != JUKEBOX_OK){
                Error_Handler();
            }
        }

	}
}

void state_custom(void){
	if(execution_state == NOT_EXECUTED){
		if (htim3.State == HAL_TIM_STATE_READY){
			if (execute_motor(&htim3, speed_selected) == MOTOR_ERROR){
				Error_Handler();
			}
		}
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

/**
 * @Brief Definition of the callback for the button
 * @param GPIO Pin of the pressed button
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	//Set the debounce time to 200ms
	const uint32_t t_debounce = 200;

	if (GPIO_Pin == BTN1_Pin){
		static uint32_t t_ref_debounce_btn1 = 0;
		if ((HAL_GetTick() - t_ref_debounce_btn1) >= t_debounce){
			t_ref_debounce_btn1 = HAL_GetTick();
			btn1_irq_cnt++;
		}
	}
	else if (GPIO_Pin == BTN2_Pin){
		static uint32_t t_ref_debounce_btn2 = 0;
		if ((HAL_GetTick() - t_ref_debounce_btn2) >= t_debounce){
			t_ref_debounce_btn2 = HAL_GetTick();
			btn2_irq_cnt++;
		}
	}
	else if (GPIO_Pin == BTN3_Pin){
		static uint32_t t_ref_debounce_btn3 = 0;
		if ((HAL_GetTick() - t_ref_debounce_btn3) >= t_debounce){
			t_ref_debounce_btn3 = HAL_GetTick();
			btn3_irq_cnt++;
		}
	}
	else if (GPIO_Pin == BTN4_Pin){
		static uint32_t t_ref_debounce_btn4 = 0;
		if ((HAL_GetTick() - t_ref_debounce_btn4) >= t_debounce){
			t_ref_debounce_btn4 = HAL_GetTick();
			btn4_irq_cnt++;
		}
	}
}

/**
 * @Brief redefinition of the Period Elapsed callback to be customely used
 * @param Handle of the timer that has finiched to count
 * @retval none
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

	if(htim == &htim6){
		//If the timer 6 has finished and the current state is the chaser => execute the chaser animation
		if(current_state == STATE_CHASER){
			if(execute_chaser() != CHASER_OK){
						Error_Handler();
					}
		}
		//Else if the current state if the jukebox => execute the jukebox
		else if(current_state == STATE_JUKEBOX){
		            if(execute_jukebox(jukebox_partition, &htim3) != JUKEBOX_OK){
		                Error_Handler();
		            }
		        }
	}
	if(htim == &htim3){
		//If the timer 3 has finished execute the buzzer
		if(execute_buzzer(&htim3, note_selected) != BUZZER_OK){
			Error_Handler();
		}
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
	//Definition of the table of available LED of the ISEN32 Board
	TypeDefLed TabLed[8] = { 0 };

	TabLed[0].Pin = L0_Pin;
	TabLed[0].Port = L0_GPIO_Port;

	TabLed[1].Pin = L1_Pin;
	TabLed[1].Port = L1_GPIO_Port;

	TabLed[2].Pin = L2_Pin;
	TabLed[2].Port = L2_GPIO_Port;

	TabLed[3].Pin = L3_Pin;
	TabLed[3].Port = L3_GPIO_Port;

	TabLed[4].Pin = L4_Pin;
	TabLed[4].Port = L4_GPIO_Port;

	TabLed[5].Pin = L5_Pin;
	TabLed[5].Port = L5_GPIO_Port;

	TabLed[6].Pin = L6_Pin;
	TabLed[6].Port = L6_GPIO_Port;

	TabLed[7].Pin = L7_Pin;
	TabLed[7].Port = L7_GPIO_Port;

	//Definition of the size of the table of LED
	size_t TabLed_sz = sizeof(TabLed) / sizeof(TypeDefLed);

	//Defnition of the available note that can be played using the buzzer
	TypeDef_Note notes[] = {
			{ "F#4", 369.99, 0 },
			{ "G4", 392.00, 0 },
			{ "G#4", 415.30, 0 },
			{ "A4", 440.00, 0 },
			{ "A#4", 466.16, 0 },
			{ "B4", 493.88, 0 },

			{ "C5", 523.25, 0 },
			{ "C#5", 554.37, 0 },
			{ "D5", 587.33, 0 },
			{ "Eb5", 622.25, 0 },
			{ "E5", 659.25, 0 },
			{ "F5", 698.46, 0 },
			{ "F#5", 739.99, 0 },
			{ "G5", 783.99, 0 },
			{ "G#5", 830.61, 0 },
			{ "A5", 880.00, 0 },
			{ "A#5", 932.33, 0 },
			{ "B5", 987.77, 0 },

			{ "C6", 1046.50, 0 },
			{ "C#6", 1108.73, 0 },
			{ "D6", 1174.66, 0 },
			{ "Eb6", 1244.51, 0 },
			{ "E6", 1318.51, 0 },
			{ "F6", 1396.91, 0 },
			{ "F#6", 1479.98, 0 },
			{ "G6", 1567.98, 0 },
			{ "G#6", 1661.22, 0 },
			{ "A6", 1760.00, 0 },
			{ "A#6", 1864.66, 0 },
			{ "B6", 1975.53, 0 }

	};
	//Size of the previous tab
	size_t notes_sz = sizeof(notes) / sizeof(TypeDef_Note);

	//Definition of the table of available speed for the motor
	TypeDef_Speed speeds[] = {
			{0.0, 0},
			{0.25, 0},
			{0.5, 0},
			{0.75, 0},
			{1.0, 0}
	};
	//Size of the motor tab
	size_t speeds_sz = sizeof(speeds) / sizeof(TypeDef_Speed);
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
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  //Initialization of all state machine and all the animation with check
  init_fsm();

  if(init_chaser(TabLed, TabLed_sz) != CHASER_OK){
	  Error_Handler();
  }
  if(init_buzzer(notes, notes_sz) != BUZZER_OK){
	  Error_Handler();
  }
  if(init_jukebox(notes, notes_sz, jukebox_partition_sz) != JUKEBOX_OK){
	  Error_Handler();
  }
  if(init_motor(speeds, speeds_sz, &htim3) != MOTOR_OK){
    	  Error_Handler();
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  //Change state if needed
	  fsm_project();

	  //Button 1 : set to previous animation
	  if(btn1_irq_cnt){

		  btn1_irq_cnt--;

		  //In case of reaching beyond the first animation, set to the last animation
		  target_animation = (target_animation > 0) ? target_animation - 1 : 3;

		  user_input = USER_INPUT_OK;

		  if (current_state == STATE_CHASER){
			  //Kill the chaser if it is the current animation
			  if (kill_chaser(&htim6) != CHASER_OK){
				  Error_Handler();
			  }
			  tempo_selected = 2;
		  }else if (current_state == STATE_BUZZER){
			  //Kill the buzzer if it is the current animation
			  if (kill_buzzer(&htim3) != BUZZER_OK){
				  Error_Handler();
			  }
			  note_selected = 0;
		  }else if(current_state == STATE_JUKEBOX){
			  //Kill the jukebox if it is the current animation
			  if(kill_jukebox(&htim3,&htim6) != JUKEBOX_OK){
				  Error_Handler();
			  }
			  tempo_jukebox_selected = 3;
		  }else if (current_state == STATE_CUSTOM){
			  //Kill the motor if it is the current animation
			  if (kill_motor(&htim3) != MOTOR_OK){
				  Error_Handler();
			  }
			  speed_selected = 2;
		  }
	  }
	  //Button 2 : set to next animation
	  if(btn2_irq_cnt){

		  btn2_irq_cnt--;

		  //In case of reaching beyond the last animation, set to the first animation
		  target_animation = (target_animation < 3) ? target_animation + 1 : 0;
		  user_input = USER_INPUT_OK;
		  if (current_state == STATE_CHASER){
			  //Kill the chaser if it is the current animation
			  if (kill_chaser(&htim6) != CHASER_OK){
				  Error_Handler();
			  }
			  tempo_selected = 2;
		  }else if (current_state == STATE_BUZZER){
			  //Kill the buzzer if it is the current animation
			  if (kill_buzzer(&htim3) != BUZZER_OK){
				  Error_Handler();
			  }
			  note_selected = 0;
		  } else if(current_state == STATE_JUKEBOX){
			  //Kill the jukebox if it is the current animation
			  if(kill_jukebox(&htim3,&htim6) != JUKEBOX_OK){
				  Error_Handler();
			  }
			  tempo_jukebox_selected = 3;
		  }else if (current_state == STATE_CUSTOM){
			  //Kill the motor if it is the current animation
			  if (kill_motor(&htim3) != MOTOR_OK){
				  Error_Handler();
			  }
			  speed_selected = 2;
		  }
	  }
	  //Button 3 : apply the "-" modifier to the current animation
	  if(btn3_irq_cnt){

		  btn3_irq_cnt--;

		  if (current_state == STATE_CHASER){
			  tempo_selected = decrease_tempo_chaser(tempo_selected, &htim6, tempos, tempo_sz);
		  }

		  else if (current_state == STATE_BUZZER){
			  note_selected = previous_note(note_selected, &htim3);
		  }

		  else if (current_state == STATE_JUKEBOX){
			  tempo_jukebox_selected = decrease_tempo_jukebox(tempo_jukebox_selected, &htim6, tempos_jukebox,tempo_jukebox_sz);
		  }

		  else if (current_state == STATE_CUSTOM){
			  speed_selected = speed_down(speed_selected, &htim3);
		  }
	  }
	  //Button 4 : apply the "+" modifier to the current animation
	  if(btn4_irq_cnt){

		  btn4_irq_cnt--;

		  if (current_state == STATE_CHASER){
			  tempo_selected = increase_tempo_chaser(tempo_selected, &htim6, tempos);
		  }

		  else if (current_state == STATE_BUZZER){
			  note_selected = next_note(note_selected, &htim3);
		  }

		  else if(current_state == STATE_JUKEBOX){
			  tempo_jukebox_selected = increase_tempo_jukebox(tempo_jukebox_selected, &htim6, tempos_jukebox);
		  }

		  else if (current_state == STATE_CUSTOM){
			  speed_selected = speed_up(speed_selected, &htim3);
		  }
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
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
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
  sConfigOC.Pulse = 65535;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.Pulse = 4800;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 23999;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 100;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

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
