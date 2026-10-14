/**
 ******************************************************************************
 * Nombre del modulo: display.c
 * REV 3
 *
 * Modulo creado para la asignatura ELO312:
 * Laboratorio de Estructura de Computadores
 * del departamento de Electrónica de la Universidad
 * Técnica Federico Santa María. El uso o copia
 * está permitido y se agracede mantener el nombre
 * de los creadores.
 *
 * Escrito inicialmente el 01/01/2004 Por Michael Kusch & Wolfgang Freund
 * Modificado el 19/09/2014           Por Mauricio Solís.
 * Modificado el 28/09/2019			  Por Rodrigo Jimenez. (Adaptacion a STM32)
 * Modificado el 07/11/2023			  Por Mauricio Solís. (se arregla un problema
 *                                    con display_get_data)
 *
 * Descripción del módulo:
 * Modulo driver display
 * Contiene las funciones que permiten manejar un display
 * LCD.
 ******************************************************************************
 */
#include "main.h"
#include "display.h"
#include "gpio_utils.h"


/*----------------------------------------------------------------------------*/
/** PRIVATE_TUNABLES                                                          */
/**                                                                           */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/** PRIVATE_Definitions                                                       */
/**                                                                           */
/*----------------------------------------------------------------------------*/
const char new_char0[]={		      BIT2,
                               BIT3 +        BIT1,
                                      BIT2,
                        BIT4 + BIT3 + BIT2 + BIT1 + BIT0,
                                      BIT2,
                               BIT3 +        BIT1,
                               BIT3 +        BIT1,
                        BIT4 + BIT3 +        BIT1 + BIT0};

/*----------------------------------------------------------------------------*/
/** PRIVATE_Macros                                                            */
/**                                                                           */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/** PRIVATE_Types                                                             */
/**                                                                           */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/** PRIVATE_Functions                                                         */
/**                                                                           */
/*----------------------------------------------------------------------------*/
static void _lcd_set_bit(uint16_t pin, GPIO_TypeDef* port);
static void _lcd_rst_bit(uint16_t pin, GPIO_TypeDef* port);
static void _lcd_write(uint8_t dat);
static uint8_t _lcd_read();
static void _lcd_enable();


static uint8_t _data_read(void);
static void _data_write(unsigned char);
static void _data_input(void);
static void _data_output(void);
unsigned char _wait_bf(void);

/*----------------------------------------------------------------------------*/
/** PRIVATE_Data                                                              */
/**                                                                           */
/*----------------------------------------------------------------------------*/
data_pin_t lcd_data_pin[8] = {{D0_GPIO_Port, D0_Pin},
							  {D1_GPIO_Port, D1_Pin},
							  {D2_GPIO_Port, D2_Pin},
							  {D3_GPIO_Port, D3_Pin},
							  {D4_GPIO_Port, D4_Pin},
							  {D5_GPIO_Port, D5_Pin},
							  {D6_GPIO_Port, D6_Pin},
							  {D7_GPIO_Port, D7_Pin}};

/*----------------------------------------------------------------------------*/
/** PUBLIC_API                                                                */
/**                                                                           */
/*----------------------------------------------------------------------------*/

/**
 * API: udelay
 **/

void udelay(unsigned int x)
{
  if((x/1000) < 1)
  {
    HAL_Delay(1);
  }
  else
  {
    HAL_Delay(x/1000);
  }
}



/**
 * API: display_clear
 **/
void display_clear(void)
{
  display_send_cmd(0x01);
}


/**
 * API: display_right_shift
 **/
void display_right_shift(void)
{
  display_send_cmd(0x1D);
}


/**
 * API: display_left_shift
 **/
void display_left_shift(void)
{
  display_send_cmd(0x18);
}


/**
 * API: display_get_pos
 **/
unsigned char display_get_pos(void)
{
  unsigned char val;
  _wait_bf();
  _lcd_set_bit(LCD_RW, RW_GPIO_Port);
  _lcd_rst_bit(LCD_RS, RS_GPIO_Port);
  val = _lcd_read();
  return val & 0x7F;
}

/**
 * API: display_get_data
 **/
char display_get_data(void)
{
  char value;
  _wait_bf();
  _lcd_set_bit(LCD_RS, RS_GPIO_Port );
  _lcd_set_bit(LCD_RW, RW_GPIO_Port);
  value = _lcd_read();
  _lcd_rst_bit(LCD_RS, RS_GPIO_Port);
  _lcd_rst_bit(LCD_RW, RW_GPIO_Port);
  return value;
}

/**
 * API: display_init
 **/
void display_init(void)
{
  udelay(15000);
  _data_input();

  _lcd_rst_bit(LCD_RS, RS_GPIO_Port);
  _lcd_rst_bit(LCD_RW, RW_GPIO_Port);

  _lcd_write(0x30);
  udelay(500);

  _lcd_write(0x30);
  udelay(120);

  _lcd_write(0x30);
  udelay(200);

  _lcd_write(0x38);
  udelay(200);
  _lcd_write(0x0F);

  udelay(200);
  _lcd_write(0x1);
  udelay(5000);
  _lcd_write(0x06);
  udelay(200);
}

/**
 * API: display_set_pos
 **/
void display_set_pos(unsigned char add)
{
  display_send_cmd(0x80 + add);
}

/**
 * API: display_lcd_addr
 **/
unsigned char display_lcd_addr(int line)
{
  switch (LCD_LINES)
  {
  case 2:
    return (line > 0) ? 0x40 : 0x0;

  case 4:
    if (line == 0) return 0x0;
    else if (line == 1) return 0x40;
    else if (line == 2) return LCD_WIDTH;
    return (0x40 + LCD_WIDTH);

  default:
    return 0;
  }
}

/**
 * API: display_send_cmd
 **/
void display_send_cmd(unsigned char value)
{
  _wait_bf();
  _lcd_rst_bit(LCD_RS, RS_GPIO_Port);
  _lcd_rst_bit(LCD_RW, RW_GPIO_Port);
  _lcd_write(value);
}

/**
 * API: display_send_data
 **/
void display_send_data(unsigned char value)
{
  _wait_bf();
  _lcd_set_bit(LCD_RS, RS_GPIO_Port);
  _lcd_rst_bit(LCD_RW, RW_GPIO_Port);
  _lcd_write(value);
}

/**
 * API: display_putchar
 **/
int display_putchar(int c)
{
  if (c == '\n')
  {
    unsigned char pos = display_get_pos();

    if(pos >= display_lcd_addr(LCD_LINES - 1))
      display_lcd_scroll_up();

    display_set_pos(display_lcd_addr(LCD_LINES - 1));
    return c;
  }

  if (c == '\r')
  {
    unsigned char pos = display_get_pos();
    unsigned char line_start = display_lcd_addr(display_lcd_line(pos));

    display_set_pos(line_start);

    for(int i = 0; i < LCD_WIDTH; ++i)
    {
      display_send_data(' ');
    }

    display_set_pos(line_start);
    return c;
  }

  if (c == '\t')
  {
    display_send_data(' ');

    while(display_get_pos() % 4 != 0)
      display_send_data(' ');

    return c;
  }

  if (c == '\b')
  {
    unsigned char pos = display_get_pos();
    unsigned char prev_pos = pos;
    int line = display_lcd_line(pos);

    if(pos == display_lcd_addr(line))
    {
      if(line == 0) return c;
      prev_pos = display_lcd_addr(line - 1) + LCD_WIDTH - 1;
    }
    else
    {
      prev_pos = pos - 1;
    }

    display_set_pos(prev_pos);
    display_send_data(' ');
    display_set_pos(prev_pos);
    return c;
  }

  display_send_data((unsigned char) c);
  return c;
}

/**
 * API: display_lcd_line
 **/
int display_lcd_line(unsigned char addr)
{
  int line;

  for (line = 0; line < LCD_LINES; line++)
    if (addr >= display_lcd_addr(line) &&
        addr < display_lcd_addr(line) + LCD_WIDTH)
      return line;

  return -1;
}

/**
 * API: display_lcd_clear
 **/
void display_lcd_clear(int from, int len)
{
  int i;

  display_set_pos(from);
  for (i = 0; i < len; i++)
    display_send_data(' ');

  display_set_pos(from);
}

/**
 * API: display_lcd_copy
 **/
int display_lcd_copy(unsigned char from, unsigned char to, int len)
{
  char data[LCD_WIDTH];
  int i;

  if ((len > LCD_WIDTH) || (from > (0x7f - len))
      || (to > (0x7f - len))) return -1;

  display_set_pos(from);
  for (i = 0; i < len; i++)
    data[i] = display_get_data();

  display_set_pos(to);
  for (i = 0; i < len; i++)
    display_send_data(data[i]);

  return 0;
}

/**
 * API: display_lcd_scroll_up
 **/
void display_lcd_scroll_up(void)
{
  int line;

  for (line = 0; line < LCD_LINES - 1; line++)
  {
    display_lcd_copy(display_lcd_addr(line + 1), display_lcd_addr(line), LCD_WIDTH);
  }

  display_lcd_clear(display_lcd_addr(line), LCD_WIDTH);
}


/**
 * API: display_test_write_cgram_ms
 **/
void display_test_write_cgram_ms(void)
{
  display_register_character(0x1, new_char0);
}

/**
 * API: display_register_character
 **/
void display_register_character(unsigned char idx, const char *bitmap)
{
  display_send_cmd(0x40 + (idx << 3));

  for(int i = 0; i < 8; ++i)
  {
    display_send_data(bitmap[i]);
  }
}

/*----------------------------------------------------------------------------*/
/** PRIVATE_Functions                                                         */
/**                                                                           */
/*----------------------------------------------------------------------------*/
/**************************************************
 * Nombre        : _lcd_set_bit
 * returns       : void
 * Creada por    : Mauricio Solís
 * Fecha creación: 14/10/2024
 * Descripción   : Hace un set en un pin/puerto especificado.
 **************************************************/
static void _lcd_set_bit(uint16_t pin, GPIO_TypeDef* port)
{
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
}

/**************************************************
 * Nombre        : _lcd_rst_bit
 * returns       : void
 * Creada por    : Mauricio Solís
 * Fecha creación: 14/10/2024
 * Descripción   : Hace un reset en un pin/puerto especificado.
 **************************************************/
static void _lcd_rst_bit(uint16_t pin, GPIO_TypeDef* port)
{
  HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}

/**************************************************
 * Nombre        : _lcd_write
 * returns       : void
 * Creada por    : Mauricio Solís
 * Fecha creación: 14/10/2024
 * Descripción   : Escribe el dato especificado en el argumento
 **************************************************/
static void _lcd_write(uint8_t dat)
{
  _data_output();
  _data_write(dat);
  _lcd_enable();
}

/**************************************************
 * Nombre        : _lcd_read
 * returns       : la palabra que forman los pines del LCD
 * Creada por    : Mauricio Solís
 * Fecha creación: 14/10/2024
 * Descripción   : Lee la data presente en los pines del LCD
**************************************************/
static uint8_t _lcd_read()
{
  uint8_t x;
  _lcd_set_bit(LCD_E, E_GPIO_Port);
  HAL_Delay(1);
  (x) = _data_read();
  _lcd_rst_bit(LCD_E, E_GPIO_Port);
  HAL_Delay(1);
  return x;
}

/**************************************************
 * Nombre        : _lcd_enable
 * returns       : void
 * Creada por    : Mauricio Solís
 * Fecha creación: 14/10/2024
 * Descripción   : Habilita la escritura/lectura en el LCD
**************************************************/
static void _lcd_enable()
{
  _lcd_set_bit(LCD_E, E_GPIO_Port);
  HAL_Delay(1);
  _lcd_rst_bit(LCD_E, E_GPIO_Port);
}

/**************************************************
 * Nombre        : unsigned char _wait_bf(void)
 * returns		 : Dirección actual del cursor
 * Creada por	 : Michael Kusch
 * Fecha creación: 01/01/2004
 * Descripción	 : Espera a que el display esté listo
 * 				   para recibir nuevos comandos o datos
 **************************************************/
unsigned char _wait_bf(void)
{
  unsigned char busy;

  _lcd_set_bit(LCD_RW, RW_GPIO_Port);
  _lcd_rst_bit(LCD_RS, RS_GPIO_Port);

  do
  {
    busy = _lcd_read();
  }
  while(busy & 0x80);

  _lcd_rst_bit(LCD_RW,RW_GPIO_Port);
  return 0;
}


/**************************************************
 * Nombre    	 : void data_output(void)
 * returns		 : void
 * Creada por	 : Mauricio Solís
 * Fecha creación: 02/10/2023
 * Descripción	 : Establece los puertos y pines de
 *                 datos respectivos como salidas.
 **************************************************/
static void _data_output()
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  uint8_t i;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  for(i = 0; i < 8; i++)
  {
    GPIO_InitStruct.Pin = lcd_data_pin[i].gpio_pin;
    HAL_GPIO_Init(lcd_data_pin[i].gpio_port, &GPIO_InitStruct);
  }
}


/**************************************************
 * Nombre    	 : void data_input(void)
 * returns		 : void
 * Creada por	 : Rodrigo Jimenez
 * Fecha creación: 28/09/2019
 * Descripción	 : Establece los puertos y pines
 * 				   de datos respectivos como entradas.
 **************************************************/
static void _data_input()
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  uint8_t i;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  for(i = 0; i < 8; i++)
  {
    GPIO_InitStruct.Pin = lcd_data_pin[i].gpio_pin;
    HAL_GPIO_Init(lcd_data_pin[i].gpio_port, &GPIO_InitStruct);
  }

}

static void _data_write(uint8_t x)
{
  uint8_t i;
  GPIO_PinState dat;

  for(i = 0; i < NUMBER_OF_DATA_PIN; i++)
  {
    dat = ((x & (1 << i))) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(lcd_data_pin[i].gpio_port, lcd_data_pin[i].gpio_pin, dat);
  }
}

static uint8_t _data_read()
{

  uint8_t i;
  uint8_t ret_val;
  uint8_t val_read = 0;
  _data_input();

  for(i = 0; i < 8; i++)
  {
    val_read = (HAL_GPIO_ReadPin(lcd_data_pin[i].gpio_port, lcd_data_pin[i].gpio_pin) == GPIO_PIN_SET);
    if(val_read == 1)
    {
      ret_val |= (val_read << i);
    }
  }
  return ret_val;

}

/*----------------------------------------------------------------------------*/
/** */
/*--->> END: Private Functions <<---------------------------------------------*/

