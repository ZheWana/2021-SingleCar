/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Debug.h"
#include "user.h"
#include "timepiece.h"
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

/* USER CODE BEGIN PV */
uint8_t data, command, crossHistroy, crossCNT = 0;
extern pid leftspd, rightspd, dir;
extern uint8_t mainState, subState, Tflag, delayFlag1, delayPreFlag1, delayFlag2, turnFlag, delayPreFlag2, dottedFlag, getCross, getT, forceTurn, crossState, dataFrame;
extern double shadowDirOut, basicLeftSpd, basicRightSpd;

task_t tasks[] = {
        {0, 10, TaskHandlerDeclare(PIDControl),  0},
        {1, 2,  TaskHandlerDeclare(StateDetect), 0},
        {2, 10, TaskHandlerDeclare(LogOutput),   0},
        {3, 2,  TaskHandlerDeclare(CrossDelay),  0}
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {
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
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    MX_TIM2_Init();
    MX_TIM5_Init();
    MX_TIM1_Init();
    /* USER CODE BEGIN 2 */
    HAL_TIM_Base_Start_IT(&htim3);
    SetInterruptPeriod(1);
    TaskRegister_List(tasks, 4);

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_1);
    HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_2);
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_2);
    HAL_UART_Receive_IT(&huart1, &data, sizeof(data));
    HAL_UART_Receive_IT(&huart2, &command, sizeof(command));

    PID_Init(&leftspd, 170, 25, 0, 0);  //170 25 0 173max
    PID_Init(&rightspd, 170, 25, 0, 0); //170 25 0 173max
    PID_Init(&dir, 0.3, 0, 0, 0);

    //Debug
    mainState = STATE_MedicineRoom;
    subState = GetMedicine_TurnAround;
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        //State
        switch (mainState) {
            case STATE_Debug:                   //调试状�??
                ForceTurn_Start;
                Set_Speed(MAXSpeed, MAXSpeed);
                LED_ON(0);
                ForceTurn(DIR_RIGHT);
                Set_Speed(0, 0);
                ForceTurn_Start;
                HAL_Delay(500);
                ForceTurn(DIR_LEFT);
                Set_Speed(0, 0);
                ForceTurn_Start;
                HAL_Delay(500);
                break;

            case STATE_MedicineRoom:            //药房�???????????
                //prosessor
                LED_ON(LED_GREEN);
                Set_Speed(0, 0);

                //state change
                if (Get_MedicineState() == MedicineExist) {
                    Set_Speed(MAXSpeed, MAXSpeed);
                    mainState = STATE_GoToWaitingPoint;
                }
                break;
            case STATE_GoToWaitingPoint:
                //prosessor
                LED_ON(0);

                //state change
                if (getT) {
                    ForceTurn_Start;
                    CalibrationDelay(60000);
                    mainState = STATE_Waiting;
                }
                break;
            case STATE_Waiting:
                //prosessor
                LED_ON(LED_YELLOW);
                Set_Speed(0, 0);

                //state change
                if (command == 'p') {
                    ForceTurn(DIR_BACK);
                    ForceTurn_Start;
                    Set_Speed(0, 0);
                    HAL_Delay(100);
                    ForceTurn_Stop;
                    mainState = STATE_TransportMedicine;
                }
                break;
            case STATE_TransportMedicine:       //送药�???????????
                // prosessor
                LED_ON(0);
                Set_Speed(MAXSpeed, MAXSpeed);
                if (getCross) {
                    CalibrationDelay(CrossCnt);
                    ForceTurn(crossHistroy & 1 ? DIR_RIGHT : DIR_LEFT);
                    ForceTurn_Start;
                }


                //state change
                if (dottedFlag) {
                    CalibrationDelay(DottedCnt);
                    mainState = STATE_PacientRoom;
                }
                break;
            case STATE_PacientRoom:             //病房�???????????
                //prosessor
                LED_ON(LED_RED);
                ForceTurn_Start;
                Set_Speed(0, 0);

                //state change
                if (Get_MedicineState() == MedicineNone) {
                    ForceTurn_Stop;
                    mainState = STATE_GetMedicineMiddle;
                }
                break;
            case STATE_GetMedicineMiddle:             //取药�???????????
                //prosessor
                LED_ON(0);

                //subState
                switch (subState) {
                    case GetMedicine_TurnAround:
                        //prosessor
                        ForceTurn(DIR_BACK);
                        Set_Speed(0, 0);
                        HAL_Delay(100);
                        Set_Speed(MAXSpeed, MAXSpeed);

                        //state change
                        subState = GetMedicine_Turn;
                        break;
                    case GetMedicine_Turn:
                        //prosessor
                        if (getCross) {
                            CalibrationDelay(CrossCnt);
                            ForceTurn(crossHistroy & 1 ? DIR_RIGHT : DIR_LEFT);

                            //state change
                            subState = GetMedicine_Go;
                        }

                        break;
                    case GetMedicine_Go:
                        //prosessor
                        Set_Speed(MAXSpeed, MAXSpeed);

                        //state change
                        break;
                    default:;
                }

                //state change
                if (dottedFlag) {
                    CalibrationDelay(DottedCnt);
                    mainState = STATE_MedicineRoom;
                }
                break;
            case STATE_GetMedicineFar:
                //prosessor

                //subState
                switch (subState) {
                    case GetMedicine_TurnAround:
                        //prosessor
                        ForceTurn(DIR_BACK);
                        Set_Speed(0, 0);
                        ForceTurn_Start;
                        HAL_Delay(100);
                        Set_Speed(MAXSpeed, MAXSpeed);

                        //state change
                        subState = GetMedicine_Turn;
                        break;
                    case GetMedicine_Turn:
                        //prosessor
                        CalibrationDelay(BackCnt);

                        ForceTurn(crossHistroy & 1 ? DIR_RIGHT : DIR_LEFT);

                        //state change
                        subState = GetMedicine_TurnBig;
                        break;
                    case GetMedicine_TurnBig:
                        //prosessor
                        Set_Speed(MAXSpeed, MAXSpeed);

                        //state change
                        if (getCross) {
                            CalibrationDelay(CrossCnt);
                            ForceTurn(crossHistroy & 1 << 1 ? DIR_RIGHT : DIR_LEFT);
                            subState = GetMedicine_Go;
                        }
                        break;
                    case GetMedicine_Go:
                        //prosessor
                        Set_Speed(MAXSpeed, MAXSpeed);

                        //state change
                        break;
                    default:;
                }

                //state change
                if (dottedFlag) {
                    CalibrationDelay(DottedCnt);
                    mainState = STATE_MedicineRoom;
                }
                break;
            default:;
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
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
    */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 25;
    RCC_OscInitStruct.PLL.PLLN = 168;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
