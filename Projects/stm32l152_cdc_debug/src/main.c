/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "usb_device.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include <stdio.h>
#include "usbd_cdc_if.h"
#include "stdio.h"

#include <string.h>

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
uint8_t Counter = 0;
char buffer[64]; // Buffer para formatear la cadena
extern USBD_HandleTypeDef hUsbDeviceFS;
volatile uint8_t start_btn_pressed = 0; // Bandera para sincronizar con el main
volatile uint8_t stop_btn_pressed = 0; // Bandera para sincronizar con el main
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */
//extern void initialise_monitor_handles(void);

/* Redirección de printf a USB CDC */
int __io_putchar(int ch)
{
  static uint8_t buf[64];
  static uint8_t idx = 0;

  buf[idx++] = (uint8_t)ch;

  /* Enviar si el buffer está lleno o si encontramos un carácter de nueva línea */
  if (idx >= 64 || ch == '\n' || ch == '\r') {
      /* Verificar si el dispositivo está configurado (conectado) para evitar bloqueo */
      if (hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED) {
          idx = 0;
          return ch;
      }

      /* Esperar a que el USB esté libre para transmitir con Timeout */
      uint32_t start = HAL_GetTick();
      while (CDC_Transmit_FS(buf, idx) == USBD_BUSY) {
          if (HAL_GetTick() - start > 50) { // 50ms timeout para evitar bloqueo infinito
              idx = 0;
              return ch;
          }
      }
      idx = 0;
  }
  
  return ch;
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) // START (PA0) incrementa un contador y alterna el LED verde. // STOP (PA1) reinicia el contador y alterna el LED rojo. // Usa HAL_GetTick() para evitar múltiples activaciones por rebote (software debounce).
{
	static uint32_t last_start_tick = 0;
	static uint32_t last_stop_tick = 0;
	static int counter = 0;

    if (GPIO_Pin == START_Pin) // PA0
    {
    	// Verificamos si pasaron al menos 200 ms desde la última vez
    	if (HAL_GetTick() - last_start_tick > 200) // 200 ms debounce
    	{
    		counter++;
    		HAL_GPIO_TogglePin(GREEN_GPIO_Port, GREEN_Pin); // Toggle LED verde
    		last_start_tick = HAL_GetTick();
            start_btn_pressed = 1; // Levantar bandera. NO usar printf aquí.
    	}
    }
    else if (GPIO_Pin == STOP_Pin) // PA0
    {
        if (HAL_GetTick() - last_stop_tick > 200) // 200 ms debounce
        {
            //counter = 0;
            HAL_GPIO_TogglePin(RED_GPIO_Port, RED_Pin); // Toggle LED rojo
            last_stop_tick = HAL_GetTick();
            stop_btn_pressed = 1; // Levantar bandera. NO usar printf aquí.
        }
    }
    counter = 0;
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
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  //initialise_monitor_handles(); // Inicializar Semihosting
  //printf("Sistema iniciado correctamente.\r\n");
 // uint8_t i=0;
 int START_STATE = 0;
 int GREEN_STATE = 0;
 int STOP_STATE = 0;
 int RED_STATE = 0;
 //int counter = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      /* Procesar la pulsación del botón Start fuera de la interrupción 
      if (start_btn_pressed) {
          start_btn_pressed = 0;
          printf("Boton Start presionado. Contador incrementado: %d\r\n", counter);
      }
      */

      /* Procesar la pulsación del botón Stop fuera de la interrupción 
      if (stop_btn_pressed) {
          stop_btn_pressed = 0;
          printf("Boton Stop presionado. Contador incrementado: %d\r\n", counter);
      }
      */

    START_STATE = HAL_GPIO_ReadPin(GPIOA, START_Pin);
	  if (START_STATE==0)
	    {
		  HAL_GPIO_TogglePin(GREEN_GPIO_Port, GREEN_Pin);
          GREEN_STATE = HAL_GPIO_ReadPin(GREEN_GPIO_Port, GREEN_Pin);
		  printf("Boton Start presionado. GREEN_STATE: %d\r\n", GREEN_STATE );
		  HAL_Delay(500);
	    }


	  STOP_STATE = HAL_GPIO_ReadPin(GPIOA, STOP_Pin);
	  if (STOP_STATE==0)
	    {
		  HAL_GPIO_TogglePin(RED_GPIO_Port, RED_Pin);
          RED_STATE = HAL_GPIO_ReadPin(RED_GPIO_Port, RED_Pin);
		  printf("Boton Stop presionado. RED_STATE: %d\r\n", RED_STATE );
		  HAL_Delay(500);
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
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6; // 16MHz * 6 = 96MHz (VCO)
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV3; // 96MHz / 3 = 32MHz (SysClk)
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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GREEN_GPIO_Port, GREEN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RED_GPIO_Port, RED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : GREEN_Pin */
  GPIO_InitStruct.Pin = GREEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GREEN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : START_Pin */
  GPIO_InitStruct.Pin = START_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  //GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(START_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : STOP_Pin */
  GPIO_InitStruct.Pin = STOP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(STOP_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RED_Pin */
  GPIO_InitStruct.Pin = RED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RED_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  // No necesitamos habilitar NVIC EXTI si estamos usando GPIO_MODE_INPUT (Polling)

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  // Interrupciones deshabilitadas para evitar conflictos con USB printf en este modo simple.

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
