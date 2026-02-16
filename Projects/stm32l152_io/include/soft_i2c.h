#ifndef SOFT_I2C_H
#define SOFT_I2C_H

#include "main.h"

/* Prototipos de funciones públicas */
void SoftI2C_Scan_All(void);
float Read_Temp(void);
void I2C1_ClearBus(void);

#endif /* SOFT_I2C_H */
