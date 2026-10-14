/*
 * gpio_utils.h
 *
 *  Created on: Nov 2, 2023
 *      Author: mauricio
 */

#ifndef INC_GPIO_UTILS_H_
#define INC_GPIO_UTILS_H_


#define BIT0    0x0001
#define BIT1    0x0002
#define BIT2    0x0004
#define BIT3    0x0008
#define BIT4    0x0010
#define BIT5    0x0020
#define BIT6    0x0040
#define BIT7    0x0080


/**
 * @brief Estructura para encapsular un pin con su respectivo puerto
 * */
typedef struct
{
  GPIO_TypeDef* gpio_port;    /*!< Puntero al puerto del pin*/
  uint16_t gpio_pin;          /*!< Número del pin*/
} data_pin_t;

#endif /* INC_GPIO_UTILS_H_ */
