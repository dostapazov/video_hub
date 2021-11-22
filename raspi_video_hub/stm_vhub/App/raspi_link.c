/*
 * raspi_link.c
 *
 *  Created on: 7 сент. 2018 г.
 *      Author: user
 */

#include "hal_includes.h"
#include "raspi_link.h"
#include "timers.h"
#include "crc.h"
//
//
//
static uint8_t txBuf[50];
static UART_HandleTypeDef* uart = 0;
uint32_t tx_time=0;
uint8_t raspi_id_tx=0;
//
//
//
uint8_t cam_st_out[4]={0,0,0,0};
uint8_t cam_set_out[4]={0,0,0,0};



void raspi_link_init(UART_HandleTypeDef* init)
{
	uart = init;
	tx_time=timers_get_finish_time(5000);
}

void raspi_link_swith_cam(uint8_t raspi_id, uint8_t cam_id)
{
	PCK_CAM_t pdu = {cam_id};
	PCK_Header_t header = {_CU_SIGNATURE_, raspi_id, PCT_CAM_SWITCH, sizeof(pdu)};
	memcpy(txBuf, &header, sizeof(header));
	memcpy(txBuf+sizeof(header), &pdu, sizeof(pdu));
	uint32_t crc = crc32_ether(txBuf, sizeof(header)+sizeof(pdu), 1);//0xffffffff;
	memcpy(txBuf+sizeof(header)+sizeof(pdu), &crc, sizeof(crc));

	HAL_UART_Transmit_DMA(uart, txBuf, sizeof(header)+sizeof(pdu)+sizeof(crc));
}

/**
  * @brief Установить время на малинках
  *
  * @param  time: sec - секунда
  * 			  min - минута
  * 			  h - час
  * 			  d - день
  * 			  m - месяц
  * 			  y - год
  *
  */
void raspi_link_set_time(uint8_t sec, uint8_t min, uint8_t h, uint8_t d, uint8_t m, uint8_t y)
{uint8_t i;
	PCK_TIME_t pdu = {sec, min, h, d, m, y};

	for (i=1; i<5; i++)
		{
		PCK_Header_t header = {_CU_SIGNATURE_, i, PCT_SET_TIME, sizeof(pdu)};
		memcpy(txBuf, &header, sizeof(header));
		memcpy(txBuf+sizeof(header), &pdu, sizeof(pdu));
		uint32_t crc = crc32_ether(txBuf, sizeof(header)+sizeof(pdu), 1);//0xffffffff;
		memcpy(txBuf+sizeof(header)+sizeof(pdu), &crc, sizeof(crc));

		HAL_UART_Transmit_DMA(uart, txBuf, sizeof(header)+sizeof(pdu)+sizeof(crc));
		HAL_Delay(10);
		}

}

/**
  * @brief  Изменить номер камеры отображаемый устйроством
  *
  * @param  time: raspi_id - номер малинки
  * 			  cam_id - номер камеры которую надо установить
  *
  * @retval значение в мс, которе осталось до достижения заданного времени
  */
void raspi_link_set_cam(uint8_t raspi_id, uint8_t cam_id)
{
	if ((raspi_id<5)&&(raspi_id>0))
		{
		raspi_id--;
		if(cam_set_out[raspi_id]!=cam_id)
		{
		cam_set_out[raspi_id]=cam_id; //сохранить значение камеры которое будет оправлено
		if (cam_set_out[raspi_id]!=cam_st_out[raspi_id])
			{
			tx_time=timers_get_finish_time(5);
			raspi_id_tx=raspi_id;
			}
		}
		}
}

/**
  * @brief  шаг обработки, необходимо добавить в основной цикл
  *
  */
void raspi_link_step(void)
{extern DMA_HandleTypeDef hdma_usart1_tx;

	if (__HAL_DMA_GET_COUNTER(&hdma_usart1_tx)==0)
		{
		if (__HAL_UART_GET_FLAG(uart, UART_FLAG_TC)!=RESET)
			{
			if (timers_get_time_left(tx_time)==0)
				{
				//if (cam_st_out[raspi_id_tx]!=cam_set_out[raspi_id_tx])
					{
					raspi_link_swith_cam(raspi_id_tx+1, cam_set_out[raspi_id_tx]);
					cam_st_out[raspi_id_tx]=cam_set_out[raspi_id_tx];
					}
				raspi_id_tx++;
				if (raspi_id_tx>3) raspi_id_tx=0;
				if (cam_set_out[raspi_id_tx]!=cam_st_out[raspi_id_tx])
					tx_time=timers_get_finish_time(50);
				else
					tx_time=timers_get_finish_time(500);
				}
			}
		}
}
