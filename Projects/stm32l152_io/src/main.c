/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "usbd_cdc_if.h"
#include <stdio.h>
#include "usb_device.h"
#include <string.h>
#include "datalog.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#undef GREEN_Pin
#undef GREEN_GPIO_Port
#undef RED_Pin
#undef RED_GPIO_Port

#define GREEN_Pin GPIO_PIN_0
#define GREEN_GPIO_Port GPIOC
#define RED_Pin GPIO_PIN_8
#define RED_GPIO_Port GPIOB

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

LCD_HandleTypeDef hlcd;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

PCD_HandleTypeDef hpcd_USB_FS;

/* USER CODE BEGIN PV */

uint8_t Counter = 0;
static void MX_LCD_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI1_Init(void);
// static void MX_USB_PCD_Init(void);

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_LCD_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI1_Init(void);
/* static void MX_USB_PCD_Init(void); */
/* USER CODE BEGIN PFP */
void I2C_Scan(void);
void I2C1_ClearBus(void);
void SoftI2C_Scan_All(void);
float Read_Temp(void);
int __io_putchar(int ch);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern USBD_HandleTypeDef hUsbDeviceFS;

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

void I2C_Scan(void) {
    HAL_StatusTypeDef result;
    uint8_t devices_found = 0;

    printf("\r\n--- Iniciando Escaneo I2C1 Hardware (Config Actual: PB6/PB7) ---\r\n");
    for (uint16_t i = 1; i < 128; i++) {
        result = HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(i << 1), 2, 10);
        if (result == HAL_OK) {
            printf("I2C1: Dispositivo encontrado en: 0x%02X\r\n", i);
            devices_found++;
        } else if (i == 0x48) {
            printf("I2C1: Fallo en 0x48 (Sensor). Error Code: %d\r\n", result);
        }
    }

    printf("\r\n--- Iniciando Escaneo I2C2 (PB10/PB11) ---\r\n");
    for (uint16_t i = 1; i < 128; i++) {
        result = HAL_I2C_IsDeviceReady(&hi2c2, (uint16_t)(i << 1), 2, 10);
        if (result == HAL_OK) {
            printf("I2C2: Dispositivo encontrado en: 0x%02X\r\n", i);
            devices_found++;
        }
    }

    if (devices_found == 0) printf("No se encontraron dispositivos I2C.\r\n");
    printf("--- Fin Escaneo ---\r\n");
}

/* --- Implementación I2C por Software (Bit Banging) Dinámico --- */
void Soft_Delay(void) {
    for (volatile int i = 0; i < 100; i++) { __NOP(); }
}

void Soft_Init(GPIO_TypeDef* port, uint16_t scl, uint16_t sda) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if(port == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = scl | sda;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(port, scl, GPIO_PIN_SET);
    HAL_GPIO_WritePin(port, sda, GPIO_PIN_SET);
}

void Soft_Start(GPIO_TypeDef* port, uint16_t scl, uint16_t sda) {
    HAL_GPIO_WritePin(port, sda, GPIO_PIN_SET);
    HAL_GPIO_WritePin(port, scl, GPIO_PIN_SET);
    Soft_Delay();
    HAL_GPIO_WritePin(port, sda, GPIO_PIN_RESET);
    Soft_Delay();
    HAL_GPIO_WritePin(port, scl, GPIO_PIN_RESET);
}

void Soft_Stop(GPIO_TypeDef* port, uint16_t scl, uint16_t sda) {
    HAL_GPIO_WritePin(port, sda, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(port, scl, GPIO_PIN_SET);
    Soft_Delay();
    HAL_GPIO_WritePin(port, sda, GPIO_PIN_SET);
    Soft_Delay();
}

uint8_t Soft_WriteByte(GPIO_TypeDef* port, uint16_t scl, uint16_t sda, uint8_t data) {
    for(uint8_t i = 0; i < 8; i++) {
        if(data & 0x80) HAL_GPIO_WritePin(port, sda, GPIO_PIN_SET);
        else HAL_GPIO_WritePin(port, sda, GPIO_PIN_RESET);
        data <<= 1;
        Soft_Delay();
        HAL_GPIO_WritePin(port, scl, GPIO_PIN_SET);
        Soft_Delay();
        HAL_GPIO_WritePin(port, scl, GPIO_PIN_RESET);
    }
    // ACK
    HAL_GPIO_WritePin(port, sda, GPIO_PIN_SET);
    Soft_Delay();
    HAL_GPIO_WritePin(port, scl, GPIO_PIN_SET);
    Soft_Delay();
    uint8_t ack = (HAL_GPIO_ReadPin(port, sda) == GPIO_PIN_RESET);
    HAL_GPIO_WritePin(port, scl, GPIO_PIN_RESET);
    return ack;
}

void SoftI2C_Scan_Port(GPIO_TypeDef* port, uint16_t scl, uint16_t sda, const char* name) {
    printf("\r\n--- Escaneando %s (SCL: PB%d, SDA: PB%d) ---\r\n", 
           name, 
           (scl == GPIO_PIN_6) ? 6 : (scl == GPIO_PIN_8) ? 8 : (scl == GPIO_PIN_10) ? 10 : 0,
           (sda == GPIO_PIN_7) ? 7 : (sda == GPIO_PIN_9) ? 9 : (sda == GPIO_PIN_11) ? 11 : 0);

    Soft_Init(port, scl, sda);

    /* Diagnóstico de Líneas */
    uint8_t sda_state = HAL_GPIO_ReadPin(port, sda);
    uint8_t scl_state = HAL_GPIO_ReadPin(port, scl);
    
    printf("Estado Inicial -> SDA: %d, SCL: %d\r\n", sda_state, scl_state);

    if (sda_state == GPIO_PIN_RESET) {
        printf("ERROR: SDA pegado a LOW. Saltando...\r\n");
        return;
    }

    uint8_t found = 0;
    for(uint16_t i = 1; i < 128; i++) {
        Soft_Start(port, scl, sda);
        if(Soft_WriteByte(port, scl, sda, i << 1)) {
            printf("DISPOSITIVO ENCONTRADO EN 0x%02X !!!\r\n", i);
            found++;
        }
        Soft_Stop(port, scl, sda);
        Soft_Delay();
    }
    if (found == 0) printf("No se encontraron dispositivos.\r\n");
}

void SoftI2C_Scan_All(void) {
    SoftI2C_Scan_Port(GPIOB, GPIO_PIN_6, GPIO_PIN_7, "I2C1 Std");
    SoftI2C_Scan_Port(GPIOB, GPIO_PIN_6, GPIO_PIN_9, "I2C1 Mix");
    SoftI2C_Scan_Port(GPIOB, GPIO_PIN_8, GPIO_PIN_9, "I2C1 Alt");
    SoftI2C_Scan_Port(GPIOB, GPIO_PIN_10, GPIO_PIN_11, "I2C2 Std");
    
    /* Restaurar configuración original de pines */
    MX_I2C1_Init();
    MX_I2C2_Init();
    
    /* Restaurar configuración del LED Rojo (PB8) por si lo usamos */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = RED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(RED_GPIO_Port, &GPIO_InitStruct);
}

/* Función para leer temperatura del sensor I2C (LM75/PCT2075) */
float Read_Temp(void) {
    uint8_t buf[2];
    int16_t val;
    float temp_c;
    
    /* Dirección 0x48 desplazada a la izquierda = 0x90 */
    if (HAL_I2C_Master_Receive(&hi2c1, (0x48 << 1), buf, 2, 100) == HAL_OK) {
        /* Convertir datos (Big Endian) */
        val = (int16_t)((buf[0] << 8) | buf[1]);
        /* Los sensores LM75 suelen tener resolución de 0.5°C o 0.125°C. 
           El formato es complemento a 2, desplazado 8 bits (o 5 para 11-bit).
           Para LM75 estándar: val / 256.0 funciona bien. */
        temp_c = val / 256.0f;
        return temp_c;
    }
    else {
        printf("Error lectura I2C: 0x%08lX (State: %d)\r\n", HAL_I2C_GetError(&hi2c1), hi2c1.State);
    }
    return -999.0f; /* Error */
}

/* Función para desbloquear el bus I2C si un esclavo lo mantiene ocupado (SDA Low) */
void I2C1_ClearBus(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 1. Habilitar reloj GPIOB
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // 2. Configurar SCL (PB6) y SDA (PB7) como Salida Open-Drain
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 3. Poner SDA Alto
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);

    // 4. Generar hasta 20 pulsos de reloj en SCL para liberar esclavos
    for (int i = 0; i < 20; i++) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); // SCL Low
        HAL_Delay(1);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);   // SCL High
        HAL_Delay(1);
        
        /* Si SDA se libera (se pone en ALTO), paramos */
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_SET) {
            break;
        }
    }

    // 5. Generar condición de STOP
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);

    /* IMPORTANTE: Des-inicializar los pines para liberarlos antes de que el periférico I2C los tome */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7);
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
  I2C1_ClearBus(); /* Intentar recuperar el bus antes de inicializar */
  HAL_Delay(10);   /* Pequeña pausa para estabilizar líneas */
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_LCD_Init();
  MX_RTC_Init();
  MX_SPI1_Init();
  /* MX_USB_PCD_Init(); */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  HAL_LCD_Clear(&hlcd); /* Limpiar basura de la pantalla LCD */
  HAL_Delay(1000);
  printf("\r\n=== INICIO DE PRUEBAS TEMPTALE ULTRA ===\r\n");
  printf("Activando pines auxiliares (PB2, PB12, PB4, PB5, PA15, PD2)...\r\n");
  printf("USB CDC: OK\r\n");
  

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint32_t last_tick = 0;
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
  float current_temp = 0.0f;

  while (1)
  {
    if (HAL_GPIO_ReadPin(START_GPIO_Port, START_Pin) == GPIO_PIN_RESET) {
        HAL_GPIO_WritePin(GREEN_GPIO_Port, GREEN_Pin, GPIO_PIN_SET);
        printf("Boton START presionado -> LED VERDE ON\r\n");
        I2C_Scan(); /* Ejecutar escáner bajo demanda */
        
        current_temp = Read_Temp();
        printf("Lectura Sensor I2C: %.2f C\r\n", current_temp);
        
        SoftI2C_Scan_All(); /* Ejecutar escáner por software en TODOS los pines */
        SPI_Flash_ReadID(); /* Ejecutar prueba SPI */
        Log_ReadAll(); /* Leer logs guardados */
        HAL_Delay(200);
    } else {
        HAL_GPIO_WritePin(GREEN_GPIO_Port, GREEN_Pin, GPIO_PIN_RESET);
    }

    if (HAL_GPIO_ReadPin(STOP_GPIO_Port, STOP_Pin) == GPIO_PIN_RESET) {
        HAL_GPIO_WritePin(RED_GPIO_Port, RED_Pin, GPIO_PIN_SET);
        printf("Boton STOP presionado -> LED ROJO ON\r\n");
        
        /* Guardar un log con temperatura real */
        current_temp = Read_Temp();
        Log_SaveCurrent(current_temp);
        
        HAL_Delay(200);
    } else {
        HAL_GPIO_WritePin(RED_GPIO_Port, RED_Pin, GPIO_PIN_RESET);
    }

    if (HAL_GetTick() - last_tick > 1000) {
        last_tick = HAL_GetTick();
        HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
        printf("RTC Time: %02d:%02d:%02d\r\n", sTime.Hours, sTime.Minutes, sTime.Seconds);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_LCD;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.LCDClockSelection = RCC_RTCCLKSOURCE_LSE;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* 1. Habilitar reloj ANTES de resetear (Crucial) */
  __HAL_RCC_I2C1_CLK_ENABLE();
  /* 2. Resetear el periférico I2C1 para desbloquearlo si está en estado BUSY */
  __HAL_RCC_I2C1_FORCE_RESET();
  HAL_Delay(2);
  __HAL_RCC_I2C1_RELEASE_RESET();

  hi2c2.Instance = I2C2;
  hi2c1.Init.ClockSpeed = 10000; /* Bajamos a 10kHz para máxima estabilidad */
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

}

/**
  * @brief LCD Initialization Function
  * @param None
  * @retval None
  */
static void MX_LCD_Init(void)
{

  /* USER CODE BEGIN LCD_Init 0 */

  /* USER CODE END LCD_Init 0 */

  /* USER CODE BEGIN LCD_Init 1 */

  /* USER CODE END LCD_Init 1 */
  hlcd.Instance = LCD;
  hlcd.Init.Prescaler = LCD_PRESCALER_1;
  hlcd.Init.Divider = LCD_DIVIDER_16;
  hlcd.Init.Duty = LCD_DUTY_1_4; /* Cambiado a 1/4 Duty para evitar segmentos fantasma */
  hlcd.Init.Bias = LCD_BIAS_1_3; /* Cambiado a 1/3 Bias (estándar 3V) */
  hlcd.Init.VoltageSource = LCD_VOLTAGESOURCE_INTERNAL;
  hlcd.Init.Contrast = LCD_CONTRASTLEVEL_0;
  hlcd.Init.DeadTime = LCD_DEADTIME_0;
  hlcd.Init.PulseOnDuration = LCD_PULSEONDURATION_0;
  hlcd.Init.MuxSegment = LCD_MUXSEGMENT_ENABLE;
  hlcd.Init.BlinkMode = LCD_BLINKMODE_OFF;
  hlcd.Init.BlinkFrequency = LCD_BLINKFREQUENCY_DIV8;
  if (HAL_LCD_Init(&hlcd) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LCD_Init 2 */

  /* USER CODE END LCD_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  RTC_AlarmTypeDef sAlarm = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the Alarm A
  */
  sAlarm.AlarmTime.Hours = 0x0;
  sAlarm.AlarmTime.Minutes = 0x0;
  sAlarm.AlarmTime.Seconds = 0x0;
  sAlarm.AlarmTime.SubSeconds = 0x0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 0x1;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USB Initialization Function
  * @param None
  * @retval None
  */
#if 0
static void MX_USB_PCD_Init(void)
{

  /* USER CODE BEGIN USB_Init 0 */

  /* USER CODE END USB_Init 0 */

  /* USER CODE BEGIN USB_Init 1 */

  /* USER CODE END USB_Init 1 */
  hpcd_USB_FS.Instance = USB;
  hpcd_USB_FS.Init.dev_endpoints = 8;
  hpcd_USB_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_FS.Init.battery_charging_enable = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_Init 2 */

  /* USER CODE END USB_Init 2 */

}
#endif

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GREEN_GPIO_Port, GREEN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RED_GPIO_Port, RED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level : Pines sospechosos de Enable/Power */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2|GPIO_PIN_12|GPIO_PIN_4|GPIO_PIN_5, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level : PA4 (SPI CS) */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

  /*Configure GPIO pin : GREEN_Pin */
  GPIO_InitStruct.Pin = GREEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GREEN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : START_Pin */
  GPIO_InitStruct.Pin = START_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
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
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  /* Configurar PA4 como Salida Push-Pull para CS de SPI */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Inicializar PA15 (JTDI) y PD2 como Salidas High por si son Enable */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

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
#ifdef USE_FULL_ASSERT
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
