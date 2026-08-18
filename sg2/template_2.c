#include "stm32l476xx.h"

int main(void)
{
	/*Habilitar reloj para GPIOC*/
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;

    /*Modificar registro MODER para configurar puerto C pin 13 como entrada*/
    //GPIOC->... fill in
    
    /*crear variable para guardar estado del pulsador*/

    while (1)
    {
    	/*Leer registro IDR para obtener el estado del bit 13 del puerto C 
    	 * y guardarlo en la variable creada*/

    }
}
