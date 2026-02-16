#include "lcd_display.h"
#include <stdio.h>

extern LCD_HandleTypeDef hlcd;

/* 
 * TABLA DE MAPEO (A AJUSTAR POR EL USUARIO)
 * Esta tabla define qué bits de la RAM del LCD encienden los segmentos de cada dígito.
 * Formato: {COM, SEG_BIT} para cada segmento (A, B, C, D, E, F, G)
 * 
 * Nota: Como no tenemos el esquema del vidrio LCD, estos valores son PLACESHOLDERS.
 * Usa LCD_Test_All_Segments para descubrir el mapeo real.
 */

/* Definición simple de un dígito de 7 segmentos */
typedef struct {
    uint8_t com[7]; // COM index (0..3) para cada segmento A..G
    uint8_t seg[7]; // SEG mask (bit shift) para cada segmento A..G
} Digit_Map_t;

/* 
 * EJEMPLO DE MAPEO (HIPOTÉTICO) 
 * Necesitarás ajustar esto observando qué segmentos se encienden.
 */
const Digit_Map_t Digit1 = { /* Dígito de las decenas */
    .com = {0, 0, 1, 1, 2, 2, 0}, // A, B, C, D, E, F, G
    .seg = {0, 1, 1, 0, 0, 1, 2}  // Bits de segmento (simplificado)
};

/* Patrones de 7 segmentos para números 0-9 (Hex: G F E D C B A) */
const uint8_t Number_Patterns[] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};

void LCD_Clear(void) {
    HAL_LCD_Clear(&hlcd);
}

/* Función para escribir un bit específico en la RAM del LCD */
void LCD_WritePixel(uint8_t com, uint32_t seg_mask, uint8_t state) {
    if (state) {
        /* HAL_LCD_Write usa el registro RAM (COM) y la máscara del segmento */
        /* Nota: En STM32L1, RAM[0]=COM0, RAM[2]=COM1, RAM[4]=COM2, RAM[6]=COM3 (Interleaved) */
        /* O RAM[0]=COM0, RAM[1]=COM1... dependiendo de la config. */
        /* Usaremos la función HAL que abstrae esto un poco, pero requiere el registro correcto */
        
        /* Escribir directamente en registros para mayor control si HAL falla */
        /* LCD->RAM[com] |= seg_mask; */
        
        /* Usando HAL (RegisterIndex suele ser COM index) */
        HAL_LCD_Write(&hlcd, com, ~(0U), seg_mask); 
        /* Nota: HAL_LCD_Write hace un Write total, no OR. Necesitamos Read-Modify-Write */
        /* Pero HAL no tiene Read. Asumimos escritura directa a RAM es mejor: */
        LCD->RAM[com * 2] |= seg_mask; // *2 por espaciado de registros en algunos modelos
    } else {
        LCD->RAM[com * 2] &= ~seg_mask;
    }
    HAL_LCD_UpdateDisplayRequest(&hlcd);
}

void LCD_Test_All_Segments(void) {
    printf("--- Test LCD: Encendiendo todos los segmentos ---\r\n");
    LCD_Clear();
    
    /* Recorrer los 4 COMs posibles */
    for (int com = 0; com < 4; com++) {
        /* Recorrer los 32 Segmentos posibles */
        for (int seg = 0; seg < 32; seg++) {
            uint32_t mask = (1 << seg);
            
            /* Encender */
            LCD->RAM[com * 2] |= mask;
            HAL_LCD_UpdateDisplayRequest(&hlcd);
            HAL_Delay(50); // Velocidad rápida para barrido
            
            /* Apagar (comentar para dejar rastro y ver todo encendido al final) */
            // LCD->RAM[com * 2] &= ~mask;
            // HAL_LCD_UpdateDisplayRequest(&hlcd);
        }
    }
    printf("--- Fin Test LCD ---\r\n");
}

void LCD_ShowTemp(float temperature) {
    /* 
     * Esta función es un placeholder. 
     * Hasta que no mapees los segmentos con LCD_Test_All_Segments,
     * solo encenderemos algunos segmentos fijos para indicar "Actividad".
     */
    
    // Ejemplo: Encender el primer segmento del COM0 y COM1 para decir "Hola"
    LCD->RAM[0] |= (1<<2) | (1<<3); // COM0, SEG2, SEG3
    LCD->RAM[2] |= (1<<2) | (1<<3); // COM1, SEG2, SEG3
    
    HAL_LCD_UpdateDisplayRequest(&hlcd);
    
    /* 
     * TODO: Implementar lógica real:
     * 1. Convertir float a int (x10) -> 209
     * 2. Extraer dígitos: 2, 0, 9
     * 3. Buscar patrón en Number_Patterns
     * 4. Escribir bits correspondientes según Digit_Map_t
     */
}
