/***************************************************
 * Nombre del modulo: keyboard.c
 *
 * Modulo creado para la asignatura Elo312
 * Laboratorio de Estructura de Computadores
 * del departamento de Electrónica de la Universidad
 * Técnica Federico Santa María. El uso o copia
 * esté permitido y se agracede mantener el nombre
 * de los creadores.
 *
 * Escrito inicialmente el 01/01/2004 Por Michael Kusch & Wolfgang Freund
 * Modificado el 24/09/2014           Por Mauricio Solís & Andrés Llico.
 * Modificado el 30/03/2020           Por Mauricio Solís.
 *
 * Descripción del módulo:
 * Definición del módulo driver teclado matricial
 * Contiene las funciones que permiten manejar el teclado matricial.
 ***************************************************/

#ifndef __KEYBOARD__H
#define __KEYBOARD__H

/*--->> Includes <<-----------------------------------------------------------*/

/*--->> Definition <<---------------------------------------------------------*/

/*--->> Data types <<---------------------------------------------------------*/



/*--->> API Definition <<-----------------------------------------------------*/
/**
* @brief        : Inicializa el teclado.
* @retval       : void
*/
void keyboard_init();

/**
* @brief        : Desinicializa el teclado.
* @retval       : void
*/
void keyboard_deinit();

/**
 * @brief       : Lee la matriz de botones y
 * 				  llama a la función keyboard_button_action() cuando
 * 				  detecta que se ha presionado una tecla (o se ha mantenido
 * 				  presionada una tecla).
 * @retval      : void
 * @note        : Esta función debe ser llamada cada 10 [ms]
*/
void keyboard_tec_matrix(void);

#endif
