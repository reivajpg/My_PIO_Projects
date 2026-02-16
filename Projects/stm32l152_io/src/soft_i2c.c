#include "soft_i2c.h"
#include <stdio.h>

/* Defines locales para el LED Rojo (usado en Scan All) */
#define RED_Pin GPIO_PIN_8
#define RED_GPIO_Port GPIOB

/* Prototipos privados */
static void Soft_Delay(void);
static void Soft_Init(GPIO_TypeDef* port, uint16_t scl, uint16_t sda);
static void Soft_Start(GPIO_TypeDef* port, uint16_t scl, uint16_t sda);
static void Soft_Stop(GPIO_TypeDef* port, uint16_t scl, uint16_t sda);
static uint8_t Soft_WriteByte(GPIO_TypeDef* port, uint16_t scl, uint16_t sda, uint8_t data);
static uint8_t Soft_ReadByte(GPIO_TypeDef* port, uint16_t scl, uint16_t sda, uint8_t ack);
static void SoftI2C_Scan_Port(GPIO_TypeDef* port, uint16_t scl, uint16_t sda, const char* name);

/* --- Implementación de funciones públicas --- */

/* Función para leer temperatura del sensor I2C (LM75/PCT2075) */
float Read_Temp(void) {
    uint8_t msb, lsb;
    int16_t val;
    
    /* Usamos I2C por Software en PB6/PB7 para evitar bloqueos del hardware */
    Soft_Init(GPIOB, GPIO_PIN_6, GPIO_PIN_7);
    Soft_Start(GPIOB, GPIO_PIN_6, GPIO_PIN_7);

    /* Enviar Dirección 0x48 + Read (1) = 0x91 */
    if (Soft_WriteByte(GPIOB, GPIO_PIN_6, GPIO_PIN_7, (0x48 << 1) | 1)) {
        /* Leer MSB con ACK */
        msb = Soft_ReadByte(GPIOB, GPIO_PIN_6, GPIO_PIN_7, 1);
        /* Leer LSB con NACK (fin de transmisión) */
        lsb = Soft_ReadByte(GPIOB, GPIO_PIN_6, GPIO_PIN_7, 0);
        
        Soft_Stop(GPIOB, GPIO_PIN_6, GPIO_PIN_7);

        /* Convertir (LM75 estándar) */
        val = (int16_t)((msb << 8) | lsb);
        return val / 256.0f;
    }
    
    Soft_Stop(GPIOB, GPIO_PIN_6, GPIO_PIN_7);
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

    /* IMPORTANTE: Des-inicializar los pines para liberarlos */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7);
}

void SoftI2C_Scan_All(void) {
    SoftI2C_Scan_Port(GPIOB, GPIO_PIN_6, GPIO_PIN_7, "I2C1 Std");
    SoftI2C_Scan_Port(GPIOB, GPIO_PIN_6, GPIO_PIN_9, "I2C1 Mix");
    SoftI2C_Scan_Port(GPIOB, GPIO_PIN_8, GPIO_PIN_9, "I2C1 Alt");
    SoftI2C_Scan_Port(GPIOB, GPIO_PIN_10, GPIO_PIN_11, "I2C2 Std");
    
    /* Restaurar configuración del LED Rojo (PB8) por si lo usamos */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = RED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(RED_GPIO_Port, &GPIO_InitStruct);
}

/* --- Implementación de funciones privadas --- */

static void Soft_Delay(void) {
    for (volatile int i = 0; i < 100; i++) { __NOP(); }
}

static void Soft_Init(GPIO_TypeDef* port, uint16_t scl, uint16_t sda) {
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

static void Soft_Start(GPIO_TypeDef* port, uint16_t scl, uint16_t sda) {
    HAL_GPIO_WritePin(port, sda, GPIO_PIN_SET);
    HAL_GPIO_WritePin(port, scl, GPIO_PIN_SET);
    Soft_Delay();
    HAL_GPIO_WritePin(port, sda, GPIO_PIN_RESET);
    Soft_Delay();
    HAL_GPIO_WritePin(port, scl, GPIO_PIN_RESET);
}

static void Soft_Stop(GPIO_TypeDef* port, uint16_t scl, uint16_t sda) {
    HAL_GPIO_WritePin(port, sda, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(port, scl, GPIO_PIN_SET);
    Soft_Delay();
    HAL_GPIO_WritePin(port, sda, GPIO_PIN_SET);
    Soft_Delay();
}

static uint8_t Soft_WriteByte(GPIO_TypeDef* port, uint16_t scl, uint16_t sda, uint8_t data) {
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

static uint8_t Soft_ReadByte(GPIO_TypeDef* port, uint16_t scl, uint16_t sda, uint8_t ack) {
    uint8_t data = 0;
    HAL_GPIO_WritePin(port, sda, GPIO_PIN_SET); // Liberar SDA para leer
    for(int i = 0; i < 8; i++) {
        data <<= 1;
        Soft_Delay();
        HAL_GPIO_WritePin(port, scl, GPIO_PIN_SET); // Clock High
        Soft_Delay();
        if(HAL_GPIO_ReadPin(port, sda) == GPIO_PIN_SET) {
            data |= 1;
        }
        HAL_GPIO_WritePin(port, scl, GPIO_PIN_RESET); // Clock Low
    }
    // Enviar ACK/NACK
    if(ack) HAL_GPIO_WritePin(port, sda, GPIO_PIN_RESET); // ACK
    else HAL_GPIO_WritePin(port, sda, GPIO_PIN_SET);      // NACK
    
    Soft_Delay();
    HAL_GPIO_WritePin(port, scl, GPIO_PIN_SET);
    Soft_Delay();
    HAL_GPIO_WritePin(port, scl, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(port, sda, GPIO_PIN_SET); // Liberar SDA al final
    return data;
}

static void SoftI2C_Scan_Port(GPIO_TypeDef* port, uint16_t scl, uint16_t sda, const char* name) {
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
