/***************************************************
 * Nombre del modulo: example_keyboard.c
 *
 * Modulo creado para la asignatura Elo312
 * Laboratorio de Estructura de Computadores
 * del departamento de Electrónica de la Universidad
 * Técnica Federico Santa María. El uso o copia
 * esté permitido y se agracede mantener el nombre
 * de los creadores.
 *
 * Escrito inicialmente el 30/03/2020 Por Mauricio Solís
 *
 * Descripción del módulo:
 * Ejemplo simple de utilización del driver del teclado.
 ***************************************************/

#ifndef __EXAMPLE_KEYBOARD__H
#define __EXAMPLE_KEYBOARD__H

/*--->> Includes <<-----------------------------------------------------------*/

/*--->> Definition <<---------------------------------------------------------*/

/*--->> Data types <<---------------------------------------------------------*/


/*--->> API Definition <<-----------------------------------------------------*/
/**
* @brief          : Esta función debe ser ejecutada cuando se presiona una tecla.
* @param tecla    : La tecla presionada.
* @retval         : void
*/
void example_keyboard_1(uint8_t tecla);

/**
 * @brief         : Esta función debe ser invocada desde SysTick_Handler() o
 * 				    desde una interrupción cada 1 [ms]
 * @retval        : void
*/
void example_keyboard_check_key_isr();

#endif
