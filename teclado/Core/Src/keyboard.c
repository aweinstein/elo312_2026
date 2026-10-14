/***************************************************
 * Nombre del modulo: keyboard.c
 * REV 3
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
 * Modificado el 15/10/2024			  Por Mauricio Solís. PullUp internas.
 *
 * Descripción del módulo:
 * Implementación del módulo driver teclado matricial
 * Contiene las funciones que permiten manejar el teclado matricial.
 *
 * Nota: Se debe implentar una interrupción de timer cada 10 [ms] o
 * se puede usar la interrupción del HAL_Tick y contar 10 interrupciones.
 * Lo anterior para llama a la función keyboard_tec_matrix();
 ***************************************************/

/*<--------------- Include section ------------------->*/

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "keyboard.h"
#include "main.h"
#include "gpio.h"
#include "usart.h"
#include "example_keyboard.h"
#include "gpio_utils.h"

/*--->> Private Constants <<--------------------------------------------------*/


/*--->> Private Data Types <<-------------------------------------------------*/


/*--->> Private Data <<-------------------------------------------------------*/
static uint8_t butmat_clk = 0;         /*!< Cuántas interrupciones se lleva presionado el boton*/
static uint8_t butmat_prev = 0xFF;     /*!< Tecla de la lectura anterior, FF => ninguna.*/

//static GPIO_TypeDef* tec_port[8];      /*!< Todos y cada uno de los puertos que se usarán*/
//static uint16_t tec_pin[8];            /*!< Todos y cada uno de los pines que se usarán*/


static bool key_inited = false;        /*!< Indica si el driver fue inicializado*/
static data_pin_t tec_data_pin[8] = { {TEC0_GPIO_Port, TEC0_Pin},
                                      {TEC1_GPIO_Port, TEC1_Pin},
                                      {TEC2_GPIO_Port, TEC2_Pin},
                                      {TEC3_GPIO_Port, TEC3_Pin},
                                      {TEC4_GPIO_Port, TEC4_Pin},
                                      {TEC5_GPIO_Port, TEC5_Pin},
                                      {TEC6_GPIO_Port, TEC6_Pin},
                                      {TEC7_GPIO_Port, TEC7_Pin},
                                      };

/*--->> Private Prototypes <<-------------------------------------------------*/
static void _keyboard_button_action(uint8_t key);
static void _set_output_port(uint8_t iteration);


/*--->> API Implementation <<-------------------------------------------------*/

/**
 * API: keyboard_init
 */
void keyboard_init()
{
  key_inited = true;
}

/**
 * API: keyboard_deinit
 */
void keyboard_deinit()
{
  key_inited = false;
}

/**
 * API: keyboard_tec_matrix
 */
void keyboard_tec_matrix(void)
{
  uint8_t butmat = 0xFF;        // tecla de 0 a 15
  uint8_t temp, k;
  uint8_t actual_position = 0;
  uint8_t pins_state[8];
  uint8_t port_state;

  if(key_inited == false)
  {
    return;
  }

  temp = 0x00;

  for(actual_position = 0; actual_position < 4; actual_position++)
  {
    _set_output_port(actual_position);
    for(uint8_t i = 0; i < 8; i++)
    {
      pins_state[i] = (HAL_GPIO_ReadPin(tec_data_pin[i].gpio_port, tec_data_pin[i].gpio_pin)== GPIO_PIN_SET)? 1 : 0;
    }
    port_state = (pins_state[7] << 7) |
                 (pins_state[6] << 6) |
                 (pins_state[5] << 5) |
                 (pins_state[4] << 4) |
                 (pins_state[3] << 3) |
                 (pins_state[2] << 2) |
                 (pins_state[1] << 1) |
                 (pins_state[0] << 0);
    k = ((~port_state) & 0x0F) | (0x10 << actual_position);

    if(k & 0x0F)
    {
      temp = k;
    }

  }

  if (temp)                        // hubo tecla presionada?
  {
    switch	(temp & 0xF0)
    {
      case BIT4:
        butmat = 0;	 // fila 1
        break;
      case BIT5:
        butmat = 4;	 // fila 2
        break;
      case BIT6:
        butmat = 8;	 // fila 3
        break;
      case BIT7:
        butmat = 12; // fila 4
        break;
    }

    switch	(temp & 0x0F)
    {
      case BIT0:
        butmat += 0; // col 1
        break;
      case BIT1:
        butmat += 1; // col 2
        break;
      case BIT2:
        butmat += 2; // col 3
        break;
      case BIT3:
        butmat += 3; // col 4
        break;
    }

    if (butmat != butmat_prev)               // botón presionado es distinto al anterior?
    {
      butmat_prev = butmat;
      butmat_clk = 0;
      _keyboard_button_action(butmat);
    }
    else
    {
      if (++butmat_clk == 100) // presionada por más de 100*10[ms] (1 [s])?
      {
        _keyboard_button_action(butmat);   // actuar como si se volvió a presionar el botón
        butmat_clk -= 33;                 // por cada 0.5 [s] que siga presionado el botón
      }
    }
  }
  else
    butmat_prev = 0xFF;                       // ningún botón presionado
}


/*--->> Private Functions <<-------------------------------------------------*/
/**
 * @brief:        Esta función es llamada cuando una tecla
 *                ha sido presionada.
 * @retval:       void
 * @param tecla:  Tecla presionada
 */
void _keyboard_button_action(uint8_t tecla)
{
  example_keyboard_1(tecla);

}

/**
 * @brief:        Esta función es la encargada de reconfigurar los pines
 * @param iteration: La itercaión en el escaneo de la matriz.
 * returns:       void
 * @param tecla:  Tecla presionada
 */
void _set_output_port(uint8_t iteration)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  data_pin_t * kb_in[3];
  data_pin_t * kb_out;

  switch(iteration)
  {
    case 0:/*Se debe setear como salida el pin TEC4 y en cero y el resto como entrada*/
      kb_in[0] = &tec_data_pin[5];
      kb_in[1] = &tec_data_pin[6];
      kb_in[2] = &tec_data_pin[7];

      kb_out = &tec_data_pin[4];

      break;

    case 1:
      kb_in[0] = &tec_data_pin[4];
      kb_in[1] = &tec_data_pin[6];
      kb_in[2] = &tec_data_pin[7];

      kb_out = &tec_data_pin[5];

      break;

    case 2:
      kb_in[0] = &tec_data_pin[4];
      kb_in[1] = &tec_data_pin[5];
      kb_in[2] = &tec_data_pin[7];

      kb_out = &tec_data_pin[6];

      break;

    case 3:
      kb_in[0] = &tec_data_pin[4];
      kb_in[1] = &tec_data_pin[5];
      kb_in[2] = &tec_data_pin[6];

      kb_out = &tec_data_pin[7];

      break;

    default:
      kb_in[0] = &tec_data_pin[4];
      kb_in[1] = &tec_data_pin[6];
      kb_in[2] = &tec_data_pin[7];

      kb_out = &tec_data_pin[5];
      break;
  }


  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;

  GPIO_InitStruct.Pin = kb_in[0]->gpio_pin;
  HAL_GPIO_Init(kb_in[0]->gpio_port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = kb_in[1]->gpio_pin;
  HAL_GPIO_Init(kb_in[1]->gpio_port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = kb_in[2]->gpio_pin;
  HAL_GPIO_Init(kb_in[2]->gpio_port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = kb_out->gpio_pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(kb_out->gpio_port, &GPIO_InitStruct);

  HAL_GPIO_WritePin(kb_out->gpio_port, kb_out->gpio_pin, GPIO_PIN_RESET);

}

