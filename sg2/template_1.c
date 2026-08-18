#include "stm32l476xx.h"

int main(void)
{
	/*Habilitar reloj para GPIOA*/
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    /*Modificar registro MODER para configurar puerto A pin 5 como salida*/
    //GPIOA->... fill in

    while (1)
    {
    	/*Modificar registro BSRR para levantar el bit 5 del puerto A*/


    	/*Modificar registro BSRR para bajar el bit 5 del puerto A*/

    }
}