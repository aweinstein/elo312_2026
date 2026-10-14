/***************************************************
 * Nombre del modulo: example_keyboard.h
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
 *
 ***************************************************/

/*<--------------- Include section ------------------->*/

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "keyboard.h"
#include "main.h"
#include "gpio.h"
#include "usart.h"

/*--->> Private Constants <<--------------------------------------------------*/


/*--->> Private Data Types <<-------------------------------------------------*/


/*--->> Private Data <<-------------------------------------------------------*/


/*--->> Private Prototypes <<-------------------------------------------------*/


/*--->> API Implementation <<-------------------------------------------------*/

void example_keyboard_1(uint8_t key)
{
  static uint8_t tx_buffer[20];
  if(key > 6)
  {
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
  }

  snprintf((char*)tx_buffer, 20, "Tecla: %d\n", key);

  HAL_UART_Transmit(&huart2, tx_buffer, strlen((const char*)tx_buffer),1000);

}

void example_keyboard_check_key_isr()
{
  static uint8_t but_tic = 0;

  if(but_tic == 9)
  {
    keyboard_tec_matrix();
    but_tic = 0;
  }
  else
  {
    but_tic += 1;
  }
}
