#include "datalog.h"
#include <stdio.h>
#include <string.h>

/* Macros privadas para el manejo de memoria */
#define FLASH_LOG_SECTOR_ADDR 0x000000 /* Usamos el Sector 0 para logs */
#define FLASH_SECTOR_SIZE     4096
#define MAX_LOGS_PER_SECTOR   (FLASH_SECTOR_SIZE / sizeof(DataLog_t))

/* Variables externas necesarias (definidas en main.c) */
extern SPI_HandleTypeDef hspi1;
extern RTC_HandleTypeDef hrtc;

/* Variable global interna para el índice */
static uint32_t current_log_idx = 0;

/* Prototipos de funciones privadas (helpers) */
static void SPI_Flash_WriteEnable(void);
static void SPI_Flash_WaitForWriteEnd(void);
static void SPI_Flash_EraseSector(uint32_t SectorAddr);
static void SPI_Flash_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
static void SPI_Flash_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);

/* --- Implementación de funciones públicas --- */

/* Función para leer el ID de la memoria SPI Winbond W25X40 */
void SPI_Flash_ReadID(void) {
    uint8_t cmd = 0x9F; /* JEDEC ID Command */
    uint8_t rx_buf[3] = {0};

    printf("\r\n--- Leyendo ID Memoria SPI (Winbond W25X40) ---\r\n");

    /* 1. Seleccionar Chip (CS Low) */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

    /* 2. Enviar comando */
    if (HAL_SPI_Transmit(&hspi1, &cmd, 1, 100) != HAL_OK) {
        printf("Error SPI: Fallo al transmitir comando.\r\n");
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
        return;
    }

    /* 3. Recibir 3 bytes (Manufacturer, Memory Type, Capacity) */
    if (HAL_SPI_Receive(&hspi1, rx_buf, 3, 100) != HAL_OK) {
        printf("Error SPI: Fallo al recibir datos.\r\n");
    }

    /* 4. Deseleccionar Chip (CS High) */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

    printf("Manuf ID: 0x%02X (Winbond espera 0xEF)\r\n", rx_buf[0]);
    printf("Mem Type: 0x%02X\r\n", rx_buf[1]);
    printf("Capacity: 0x%02X\r\n", rx_buf[2]);
}

void SPI_Flash_Test(void) {
    uint8_t writeData[] = "Log Temp: 25.4C";
    uint8_t readData[32] = {0};
    uint32_t addr = 0x000000; /* Sector 0 */
    
    printf("\r\n--- Test Escritura Flash ---\r\n");
    printf("Borrando Sector 0...\r\n");
    SPI_Flash_EraseSector(addr);
    printf("Escribiendo: %s\r\n", writeData);
    SPI_Flash_WritePage(writeData, addr, strlen((char*)writeData));
    printf("Leyendo...\r\n");
    SPI_Flash_ReadBuffer(readData, addr, strlen((char*)writeData));
    printf("Dato Leido: %s\r\n", readData);
}

void Log_SaveCurrent(float temperature) {
    DataLog_t log;
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;

    /* 1. Obtener fecha y hora del RTC */
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    /* 2. Llenar estructura */
    log.Year = sDate.Year;
    log.Month = sDate.Month;
    log.Day = sDate.Date;
    log.Hours = sTime.Hours;
    log.Minutes = sTime.Minutes;
    log.Seconds = sTime.Seconds;
    log.Temp_x100 = (int16_t)(temperature * 100.0f);

    /* 3. Calcular dirección de escritura */
    uint32_t write_addr = FLASH_LOG_SECTOR_ADDR + (current_log_idx * sizeof(DataLog_t));

    /* 4. Si estamos al inicio del sector (índice 0), borrarlo primero */
    if (current_log_idx == 0) {
        printf("--- Buffer Lleno/Inicio: Borrando Sector de Logs ---\r\n");
        SPI_Flash_EraseSector(FLASH_LOG_SECTOR_ADDR);
    }

    /* 5. Escribir en Flash */
    SPI_Flash_WritePage((uint8_t*)&log, write_addr, sizeof(DataLog_t));
    
    printf("Log Guardado [%lu]: %02d/%02d/%02d %02d:%02d:%02d - Temp: %.2f C\r\n", 
            current_log_idx, log.Day, log.Month, log.Year, log.Hours, log.Minutes, log.Seconds, temperature);

    /* 6. Incrementar índice circular */
    current_log_idx++;
    if (current_log_idx >= MAX_LOGS_PER_SECTOR) {
        current_log_idx = 0; /* Vuelta al principio */
    }
}

void Log_ReadAll(void) {
    DataLog_t log;
    printf("\r\n--- Leyendo Todos los Logs ---\r\n");
    for (uint32_t i = 0; i < MAX_LOGS_PER_SECTOR; i++) {
        uint32_t addr = FLASH_LOG_SECTOR_ADDR + (i * sizeof(DataLog_t));
        SPI_Flash_ReadBuffer((uint8_t*)&log, addr, sizeof(DataLog_t));
        
        /* Si encontramos memoria vacía (0xFF), terminamos de leer */
        if (log.Day == 0xFF && log.Month == 0xFF) break;
        
        printf("[%lu] %02d/%02d/%02d %02d:%02d:%02d - Temp: %.2f C\r\n", 
            i, log.Day, log.Month, log.Year, log.Hours, log.Minutes, log.Seconds, (float)log.Temp_x100 / 100.0f);
    }
    printf("--- Fin de Lectura ---\r\n");
}

/* --- Implementación de funciones privadas --- */

static void SPI_Flash_WriteEnable(void) {
    uint8_t cmd = 0x06; /* Write Enable Command */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // CS Low
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);   // CS High
}

static void SPI_Flash_WaitForWriteEnd(void) {
    uint8_t cmd = 0x05; /* Read Status Register */
    uint8_t status = 0;
    do {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
        HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
        HAL_SPI_Receive(&hspi1, &status, 1, 100);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    } while ((status & 0x01) == 0x01); /* Esperar mientras el bit BUSY (Bit 0) esté en 1 */
}

static void SPI_Flash_EraseSector(uint32_t SectorAddr) {
    SPI_Flash_WriteEnable();
    SPI_Flash_WaitForWriteEnd(); // Asegurar que Write Enable terminó
    
    uint8_t cmd[4];
    cmd[0] = 0x20; /* Sector Erase (4KB) */
    cmd[1] = (SectorAddr & 0xFF0000) >> 16;
    cmd[2] = (SectorAddr & 0xFF00) >> 8;
    cmd[3] = SectorAddr & 0xFF;
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, cmd, 4, 100);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    
    SPI_Flash_WaitForWriteEnd();
}

static void SPI_Flash_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite) {
    SPI_Flash_WriteEnable();
    
    uint8_t cmd[4];
    cmd[0] = 0x02; /* Page Program */
    cmd[1] = (WriteAddr & 0xFF0000) >> 16;
    cmd[2] = (WriteAddr & 0xFF00) >> 8;
    cmd[3] = WriteAddr & 0xFF;
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, cmd, 4, 100);
    HAL_SPI_Transmit(&hspi1, pBuffer, NumByteToWrite, 1000);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    
    SPI_Flash_WaitForWriteEnd();
}

static void SPI_Flash_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead) {
    uint8_t cmd[4];
    cmd[0] = 0x03; /* Read Data */
    cmd[1] = (ReadAddr & 0xFF0000) >> 16;
    cmd[2] = (ReadAddr & 0xFF00) >> 8;
    cmd[3] = ReadAddr & 0xFF;
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, cmd, 4, 100);
    HAL_SPI_Receive(&hspi1, pBuffer, NumByteToRead, 1000);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}
