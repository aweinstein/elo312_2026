/**
 ******************************************************************************
 * @file    display.h
 * @author  Mauricio Solís
 * @date    10/2023
 * @brief   Este módulo fue creado para el laboratorio de Estructura de Computadores
 * 			del departamento de Electrónica de la Universidad Santa María.
 * 			Los principales contribuyentes de este módulo han sido:
 * 			Michael Kusch:   2004
 * 			Rodrigo Jiménez: 2019
 * 			Mauricio Solís:  2014 - 2023
 *
 * @note    Este archivo es de libre disposición.
 ******************************************************************************
 **/


#ifndef _DISPLAY_H_
#define _DISPLAY_H_

/*--->> Includes <<-----------------------------------------------------------*/

/*--->> Definition <<---------------------------------------------------------*/

// defines display layout
#define LCD_LINES 2
#define LCD_WIDTH 16

// defines for control

#define LCD_RS	RS_Pin	// bit meaning: 1 = Data , 0 = Control
#define LCD_RW	RW_Pin	// bit meanings: 0 = Write, 1 = Read
#define LCD_E	E_Pin	// Enable


#define NUMBER_OF_DATA_PIN 8

/*--->> Data types <<---------------------------------------------------------*/


/*--->> API Definition <<-----------------------------------------------------*/

/**
* @brief       Esta función genera un delay activo de x microsegundos
* @param  x    La cantidad de microsegundos.
* @retval	   void
**/
void udelay(unsigned int x);


/**
* @brief      Esta función genera envía el comando para limpiar el
* 			  contenido del display
* @retval     void
**/
void display_clear(void);

/**
* @brief      Esta función corre el "visor" del display en un
* 			  caracter hacia la derecha
* @retval     void
**/
void display_right_shift(void);

/**
* @brief      Esta función corre el "visor" del display en un
* 			  caracter hacia la izquierda
* @retval     void
**/
void display_left_shift(void);


/**
* @brief      Esta función Retorna la posición actual del cursor
* @retval     La posición del cursor
**/
unsigned char display_get_pos(void);

/**
* @brief      Esta función Envía el comando para leer el caracter sobre
* 			  el que está el cursor del display
* @retval     Caracter sobre el que está el cursor del display
**/
char display_get_data(void);


/**
* @brief      Esta función inicializa el display
* @retval     void
**/
void display_init(void);


/**
* @brief      	 Esta función posiciona el cursor.
* @param pos 	 Posición donde se desea poner el cursor.
* @retval    	 void
**/
void display_set_pos(unsigned char pos);

/**
* @brief         Esta función retorna la dirección física de una determinada línea
* @param line    La línea de la cual se desea la dirección
* @retval        La dirección física de la línea.
**/
unsigned char display_lcd_addr(int line);


/**
* @brief         Esta función envía un comando al display
* @param value   El comando a enviar
* @retval        void
**/
void display_send_cmd(unsigned char value);


/**
* @brief         Esta función envía un caracter al display o realiza alguna acción sobre
* 				 este dependiendo del caracter, por ejemplo salto de línea.
* @param value   El caracter a enviar
* @retval        El caracter enviado.
* @note 		 Esta función debe ser llamada desde la función que procesa los caracteres de la función
* 				 printf de la librería stdio.h
**/
int display_putchar(int c);

/**
* @brief         Esta función envía un byte de data al display
* @param value   La data a enviar
* @retval        void
**/
void display_send_data(unsigned char value);


/**
* @brief          Esta función retorna el número de línea de una dirección.
* @param addr     La dirección de la cual se desea conocer la línea.
* @retval         El número de línea
**/
int display_lcd_line(unsigned char addr);

/**
* @brief          Esta función limpia el display desde una dirección
* 				  inicial hasta una cierta cantida de caracteres.
* @param dir_ini  Dirección de inicio.
* @param cantidad Cantidad de caracteres a limpiar.
* @retval         void
**/
void display_lcd_clear(int from, int len);

/**
* @brief          Esta función copia una cierta cantidad de caracteres
* 				  desplegados en el display a otra dirección del display.
* @param from     Dirección de inicio.
* @param to       Dirección donde se copiarán los caracteres.
* @param len      Cantidad de caracteres a copiar
* @retval         void
**/
int display_lcd_copy(unsigned char from, unsigned char to, int len);


/**
* @brief          Esta función copia línea i en línea i - 1 para todo el display
* @retval         void
**/
void display_lcd_scroll_up(void);

/**
* @brief          Esta función escribe un nuevo caracter en la memoria CGRAM
* 				  en este caso particular un robot.
* @retval         void
**/

void display_test_write_cgram_ms(void);

/**
* @brief          Esta función registra un nuevo caracter en la cgram del display
* @param addr     La dirección cgram donde se desea escribir el nuevo caracter.
* @param bitmap   Puntero a un arreglo de 8 bytes donde está descrito el nuevo caracter.
* @retval         void
**/
void display_register_character(unsigned char addr, const char *bitmap);

#endif
