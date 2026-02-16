#include "lcd_display.h"
#include <stdio.h>

extern LCD_HandleTypeDef hlcd;

/* 
 * ============================================================================
 *                        ZONA DE CONFIGURACIÓN (MAPEO)
 * ============================================================================
 * Instrucciones:
 * 1. Ejecuta LCD_Test_All_Segments().
 * 2. Mira la pantalla y el monitor serie al mismo tiempo.
 * 3. Cuando se encienda el segmento 'A' (barra superior) del primer dígito,
 *    anota el COM y SEG que muestra el monitor serie.
 * 4. Rellena las estructuras de abajo.
 *    NOTA: Solo se usan COM0 y COM2.
 * 
 * Esquema de 7 Segmentos:
 *      A
 *    F   B
 *      G
 *    E   C
 *      D
 */

typedef struct {
    uint8_t com[7]; // COM index (0..3) para A, B, C, D, E, F, G
    uint8_t seg[7]; // SEG index (0..31) para A, B, C, D, E, F, G
} Digit_Map_t;

/* --- MAPEO DEL DÍGITO 1 (DECENAS) --- */
const Digit_Map_t Digit_Tens = {
    .com = {0, 0, 0, 0, 0, 0, 0}, // Rellenar: A, B, C, D, E, F, G (Usar 0 o 2)
    .seg = {0, 0, 0, 0, 0, 0, 0}  // Rellenar: A, B, C, D, E, F, G
};

/* --- MAPEO DEL DÍGITO 2 (UNIDADES) --- */
const Digit_Map_t Digit_Units = {
    .com = {0, 0, 0, 0, 0, 0, 0},
    .seg = {0, 0, 0, 0, 0, 0, 0}
};

/* --- MAPEO DEL DÍGITO 3 (DECIMALES) --- */
const Digit_Map_t Digit_Decimals = {
    .com = {0, 0, 0, 0, 0, 0, 0},
    .seg = {0, 0, 0, 0, 0, 0, 0}
};

/* --- MAPEO DE SÍMBOLOS ESPECIALES --- */
// Ajusta estos valores según lo que veas (ej. punto decimal, signo menos, grados)
uint8_t Sym_Point_COM = 0, Sym_Point_SEG = 0; 
uint8_t Sym_Minus_COM = 0, Sym_Minus_SEG = 0;
uint8_t Sym_Degree_COM = 0, Sym_Degree_SEG = 0;

/* Patrones numéricos (0-9) para 7 segmentos (G F E D C B A) */
const uint8_t Number_Patterns[] = {
    0x3F, // 0 (0011 1111)
    0x06, // 1 (0000 0110)
    0x5B, // 2 (0101 1011)
    0x4F, // 3 (0100 1111)
    0x66, // 4 (0110 0110)
    0x6D, // 5 (0110 1101)
    0x7D, // 6 (0111 1101)
    0x07, // 7 (0000 0111)
    0x7F, // 8 (0111 1111)
    0x6F  // 9 (0110 1111)
};

/* ============================================================================ */

void LCD_Clear(void) {
    HAL_LCD_Clear(&hlcd);
    HAL_LCD_UpdateDisplayRequest(&hlcd);
}

/* Función interna para escribir un dígito completo */
void LCD_WriteDigit(const Digit_Map_t *map, uint8_t number) {
    if (number > 9) return; // Opcional: manejar error o apagar
    
    uint8_t pattern = Number_Patterns[number];
    
    for (int i = 0; i < 7; i++) {
        // Extraer bit i del patrón (0=A, 1=B, ... 6=G)
        int active = (pattern >> i) & 0x01;
        
        uint8_t com = map->com[i];
        uint8_t seg = map->seg[i];
        uint32_t mask = (1UL << seg);
        
        if (active) {
            LCD->RAM[com] |= mask;
        } else {
            LCD->RAM[com] &= ~mask;
        }
    }
}

/* Función interna para escribir un símbolo */
void LCD_WriteSymbol(uint8_t com, uint8_t seg, uint8_t state) {
    uint32_t mask = (1UL << seg);
    if (state) LCD->RAM[com] |= mask;
    else       LCD->RAM[com] &= ~mask;
}

void LCD_ShowTemp(float temperature) {
    LCD_Clear();
    
    // Manejo de signo negativo
    if (temperature < 0) {
        LCD_WriteSymbol(Sym_Minus_COM, Sym_Minus_SEG, 1);
        temperature = -temperature;
    }

    // Separar dígitos (ej: 24.5)
    int temp_x10 = (int)(temperature * 10);
    int tens = (temp_x10 / 100) % 10;
    int units = (temp_x10 / 10) % 10;
    int decimals = temp_x10 % 10;

    // Escribir dígitos
    if (tens > 0) LCD_WriteDigit(&Digit_Tens, tens); // Supresión de cero a la izquierda
    LCD_WriteDigit(&Digit_Units, units);
    LCD_WriteDigit(&Digit_Decimals, decimals);

    // Símbolos fijos
    LCD_WriteSymbol(Sym_Point_COM, Sym_Point_SEG, 1);   // Punto decimal
    LCD_WriteSymbol(Sym_Degree_COM, Sym_Degree_SEG, 1); // Grados / F / C

    HAL_LCD_UpdateDisplayRequest(&hlcd);
}

void LCD_Test_All_Segments(void) {
    printf("\r\n=== MODO TEST LCD: INICIANDO BARRIDO (Solo COM0 y COM2) ===\r\n");
    printf("Anota COM y SEG de cada segmento que veas.\r\n");
    LCD_Clear();
    
    /* Recorrer solo COM0 y COM2 ya que COM1 y COM3 no están conectados */
    uint8_t active_coms[] = {0, 2};
    
    for (int i = 0; i < 2; i++) {
        int com = active_coms[i];
        
        /* Recorrer los 32 Segmentos posibles */
        for (int seg = 0; seg < 32; seg++) {
            uint32_t mask = (1UL << seg);
            
            /* Encender pixel */
            LCD->RAM[com] |= mask;
            HAL_LCD_UpdateDisplayRequest(&hlcd);
            
            printf("ENCENDIDO: COM %d - SEG %d\r\n", com, seg);
            
            HAL_Delay(500); // 500ms para dar tiempo a mirar
            
            /* Apagar pixel (comentar si quieres que se queden fijos) */
            LCD->RAM[com] &= ~mask;
            HAL_LCD_UpdateDisplayRequest(&hlcd);
        }
    }
    printf("=== FIN BARRIDO ===\r\n");
}
