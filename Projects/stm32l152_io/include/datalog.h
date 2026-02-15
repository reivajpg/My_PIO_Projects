#ifndef DATALOG_H
#define DATALOG_H

#include "main.h"

/* Estructura de datos para el Log (8 bytes) */
typedef struct {
    uint8_t Year;
    uint8_t Month;
    uint8_t Day;
    uint8_t Hours;
    uint8_t Minutes;
    uint8_t Seconds;
    int16_t Temp_x100; /* Temperatura * 100 (ej: 2545 = 25.45°C) */
} DataLog_t;

/* Prototipos de funciones públicas */
void SPI_Flash_ReadID(void);
void SPI_Flash_Test(void);
void Log_SaveCurrent(float temperature);
void Log_ReadAll(void);

#endif /* DATALOG_H */
