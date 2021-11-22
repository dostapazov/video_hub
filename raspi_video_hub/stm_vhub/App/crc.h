/*
 * CRC.h
 *
 *  Created on: 01 сент. 2014 г.
 *      Author: Перчиц А.Н.
 *      Настройки HAL:
 *      Default Polynominal STate: Enable
 *      Default Init Value State: Enable
 *      Input Data Inersion Mode: Byte		!!!
 *      Output Data Inersion Mode: Enable	!!!
 *      Input Data Format: Bytes
 *
 *      v.2.0 переделано полностью н HAL
 *      v.3.0 комбинированный вариант с HAL и без HAL
 *      	- инверсия CRC переделана на Си
 *      	- добавлена идентификация ПО
 */

#ifndef CRC_H_
#define CRC_H_
#include "types.h"

//#define CRC_USE_HAL //объявить, если HAL поддерживает инверсию бит
//#definr CRC_SUGNATURE //объявить если используется поиск сигнатуры

//---------------------------Сигнатура для идентификации ПО
#ifdef CRC_SUGNATURE
#pragma pack(1)
#define DINF_SIGNATURE "{DEVINF}"
typedef union
	{
	struct
		{
    	uint8_t  signature[8]; //"{DEV_INFO}"
    	uint8_t	vid;
    	uint8_t	pid;
    	uint8_t	hw_rev;
    	uint8_t	hw_mod;
    	uint8_t	sw_rev;
    	uint8_t	sw_mod;
		}fld;
	uint32_t dword[4];
	}DEV_INFO_t;
#pragma pack()

extern DEV_INFO_t dev_info;
#endif


uint32_t reverse_32(uint32_t data);//Меняет местами (реверс) биты в двойном слове
uint32_t crc32(uint32_t Data);// Рассчитывает 32-bit CRC используя предыдущий результат и новое значение.
#ifdef CRC_SUGNATURE
uint32_t crc32_flash(uint32_t adr_start, uint32_t adr_finih);
#endif
uint32_t crc32_ether(uint8_t *buf, int len, int clear);//Вычисляет 32-bit CRC для буфера данных по алгоритму аналогичному вычислению CRC в протокгое Ethernet.
unsigned char crc8(unsigned char data, unsigned char crc);// Вычисляет 8-bit CRC используя предыдущий результат и новое значение.
uint8_t crc8_1wire(void *buf, int len, unsigned char clear);//Вычисляет 32-bit CRC для буфера данных по алгоритму аналогичному вычислению CRC в протокгое 1-wire.
uint16_t modbus_crc_rtu(uint8_t* data, uint16_t len);	//Функция подсчёта CRC для линии ModBus_RTU

#endif /* CRC_H_ */
