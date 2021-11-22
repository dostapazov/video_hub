/*
 * dio.c
 *
 *  Created on: 7 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "dio.h"
#include "timers.h"

static uint8_t dio_din=0;	//состояние входов после работы функции антидребезга
static uint8_t dio_key=0, dio_key_tmp=0;	//номер нажатой кнопки
static uint16_t dio_filter_st, dio_filter_set=DIO_FILTER_SET; //счётчик мс. пока кнопка удерживается нажатой, заданное количество мс. сколько должна удерживаться кнопка нажатой
static uint32_t dio_time=0;	//переменная времени неизменного состояния кнопки для фиксации нажатия (антидребезг)
static dio_outs_t dio_outs_pins[DIO_OUTS_CNT]={{RST_1RP}, {RST_2RP}, {RST_3RP}, {RST_4RP}, {RST_5RP}};

/**
  * @brief  Читает текущее сосотяние выходов
  *
  * @param  состояние выходов; битовые поля: 0 - выключен, 1 - включен
  */
uint8_t dio_out_read(void)
{uint8_t dout=0, cnt;

	for (cnt=0; cnt<DIO_OUTS_CNT; cnt++)
		{
		dout|=HAL_GPIO_ReadPin(dio_outs_pins[cnt].port, dio_outs_pins[cnt].pin)<<cnt;
		}
	return(dout);
}

/**
  * @brief  Читает текущее сосотяние одного выхода
  *
  * @param  number: номер выхода (от 1 до ...)
  */
uint8_t dio_out_bit_read(uint8_t number)
{
	if (number<DIO_OUTS_CNT)
		return(HAL_GPIO_ReadPin(dio_outs_pins[number].port, dio_outs_pins[number].pin));

	return(0);
}

/**
  * @brief  Установливает заданное значение дискретных выходов
  *
  * @param  data: битавые поля: 0 - включить, 1 - включить
  */
void dio_out_write(uint8_t data)
{uint8_t cnt;
	//(data&0x01)!=0 ? HAL_GPIO_WritePin(RST_1RP, GPIO_PIN_SET) : HAL_GPIO_WritePin(RST_1RP, GPIO_PIN_RESET);
	//(data&0x02)!=0 ? HAL_GPIO_WritePin(RST_2RP, GPIO_PIN_SET) : HAL_GPIO_WritePin(RST_2RP, GPIO_PIN_RESET);
	//(data&0x02)!=0 ? HAL_GPIO_WritePin(RST_3RP, GPIO_PIN_SET) : HAL_GPIO_WritePin(RST_3RP, GPIO_PIN_RESET);
	//(data&0x02)!=0 ? HAL_GPIO_WritePin(RST_4RP, GPIO_PIN_SET) : HAL_GPIO_WritePin(RST_4RP, GPIO_PIN_RESET);
	//(data&0x02)!=0 ? HAL_GPIO_WritePin(RST_5RP, GPIO_PIN_SET) : HAL_GPIO_WritePin(RST_5RP, GPIO_PIN_RESET);
	for (cnt=0; cnt<DIO_OUTS_CNT; cnt++)
			{
			HAL_GPIO_WritePin(dio_outs_pins[cnt].port, dio_outs_pins[cnt].pin, (data>>cnt)&1);
			}
}

/**
  * @brief  Установливает заданное значение указанного выхода
  *
  * @param  number: номер выхода (от 1 до ...)
  * @param  data: битавые поля: 0 - включить, 1 - включить
  */
void dio_out_bit_write(uint8_t number, uint8_t data)
{	if (number<DIO_OUTS_CNT)
		HAL_GPIO_WritePin(dio_outs_pins[number].port, dio_outs_pins[number].pin, data);
}

/**
  * @brief  Читает текущее состояние входов
  *
  * @retval  состояние входов; битовые поля: 0 - выключен, 1 - включен
  */
uint8_t dio_in_read(void)
{
	uint8_t din=0;
	if (HAL_GPIO_ReadPin(PRESENT_1RP)==P1_ON_ACTIV) din=0x01;
	if (HAL_GPIO_ReadPin(PRESENT_2RP)==P2_ON_ACTIV) din|=0x02;
	if (HAL_GPIO_ReadPin(PRESENT_3RP)==P3_ON_ACTIV) din|=0x04;
	if (HAL_GPIO_ReadPin(PRESENT_4RP)==P4_ON_ACTIV) din|=0x08;
	if (HAL_GPIO_ReadPin(PRESENT_5RP)==P5_ON_ACTIV) din|=0x10;
	return(din);
}

/**
  * @brief  Читает текущее состояние входов с функцией антидребезга
  *
  * @retval  состояние входов; битовые поля: 0 - выключен, 1 - включен
  */
uint8_t dio_in_filter(void)
{
	if (timers_get_time_left(dio_time)==0)
		{
		dio_time=timers_get_finish_time(1);
		if (dio_din!=dio_in_read())
			{
			dio_filter_st++;
			if (dio_filter_st>=dio_filter_set)
				{
				dio_filter_st=0;
				dio_din=dio_in_read();
				}
			}
		else
			dio_filter_st=0;
		}

	return(dio_din);
}

/**
  * @brief  Устанавливает величину фильтра антидребезга
  *
  * @param  filter_ms: время фильтра в мс.
  */
void dio_set_filter(uint16_t filter_ms)
{
	dio_filter_set=filter_ms;
}

/**
  * @brief  Возвращает сосотяния кнопок, которые нажимались и уже отпущены
  *
  * @retval  состояние кнопок; битовые поля: 0 - не нажималась, 1 - нажималась
  */
uint8_t dio_key_press(void)
{
	dio_in_filter();
	dio_key=(dio_key_tmp^dio_din)&dio_key_tmp; //установить в 1 только входы которые изменили состояние и оставить только те которые были раньше 1 (т.е. изменили сосотяние с 1 на 0)
	dio_key_tmp=dio_din;
	/*if (dio_din!=0)
		{
		dio_key_tmp=dio_din;
		}
	else
		{
		if (dio_key_tmp!=0)
			{
			dio_key|=dio_key_tmp;
			dio_key_tmp=0;
			}
		}*/

	return(dio_key);
}

/**
  * @brief  Очищает (сбрасывает) список нажатых кнопок
  *
  */
void dio_key_reset(void)
{
	dio_key=0;
}
