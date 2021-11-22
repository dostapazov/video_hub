/*
 * CRC.c
 *
 *  Created on: 01 сент. 2014 г.
 *      Author: Перчиц А.Н.
 */
#include "crc.h"
#ifdef CRC_SUGNATURE
DEV_INFO_t dev_info;
#endif

//Таблицы для расчета CRC
//---------------------------------------------------------------------
static const uint8_t auchCRCHi[] = {0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
                                   0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
                                   0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
                                   0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
                                   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
                                   0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
                                   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
                                   0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
                                   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
                                   0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
                                   0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
                                   0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
                                   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
                                   0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
                                   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
                                   0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
                                   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
                                   0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
                                   0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
                                   0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
                                   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
                                   0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
                                   0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
                                   0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
                                   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
                                   0x80, 0x41, 0x00, 0xC1, 0x81, 0x40};
static const uint8_t auchCRCLo[] = {0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
                                    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
                                    0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
                                    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
                                    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
                                    0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
                                    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
                                    0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
                                    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
                                    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
                                    0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
                                    0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
                                    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
                                    0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
                                    0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
                                    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
                                    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
                                    0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
                                    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
                                    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
                                    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
                                    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
                                    0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
                                    0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
                                    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
                                    0x43, 0x83, 0x41, 0x81, 0x80, 0x40};

extern CRC_HandleTypeDef hcrc;

static uint8_t crc8_old=0;


/**
  * @brief  Меняет местами (реверс) биты в двойном слове
  *
  * @param  data: исходные данные (число где налдо реверсировать биты)
  *
  * @retval результат реверса
  */
uint32_t reverse_32(uint32_t data)
{
//#ifndef CRC_REVERSE_EN
//	asm("rbit r0,r0");
//	asm("str r0, [r7, #4]"); //загрузить в data значение из r0
//#endif
//    return data;
	uint32_t res = 0,mask1 = 0x80000000;
	for(int i = 0; i <32;i++)
	{
	  if(data&1) res|=mask1;
	  data>>=1;
	  mask1>>=1;
	}
	return res;
};

/**
  * @brief  Рассчитывает 32-bit CRC используя предыдущий результат и новое значение
  *
  * @param  Data: новое значение для которого надо рассчитат CRC
  *
  * @retval 32-bit CRC
  */
uint32_t crc32(uint32_t Data)
{
	//CRC->DR = Data;
	hcrc.Instance->DR = Data;

	//return (CRC->DR);
	return(hcrc.Instance->DR);
}

#ifdef CRC_SUGNATURE
/**
  * @brief  Вычисляет 32-bit CRC для сегмента flash памяти
  *         CRC32 вычисляется по алгоритму аналогичному вычислению CRC в протоколе Ethernet.
  * @param  adr_start: стартовый адрес (с которого надо начать вычисление)
  * @param	adr_finish: конечный адрес
  * @retval 32-bit CRC
  */
uint32_t crc32_flash(uint32_t adr_start, uint32_t adr_finih)
{uint32_t crc=0, crc_reg=0, len, adr, dt;
uint8_t sig_cnt=0;

//#ifdef CRC_USE_HAL
//#else
	if (adr_start>adr_finih) return(0);
	CRC->CR = CRC_CR_RESET; //Делаем сброс CRC_ResetDR();

	len=adr_finih-adr_start;
	adr=adr_start;

	while(len >= 4)
		{
		//dt=flash_read(adr);
		//flash_read(&dt,adr,4);
		dt=(*(__IO uint32_t*) (adr));
		if (sig_cnt==3)
			{
			dev_info.dword[3]=dt;
			sig_cnt++;
			}
		if (sig_cnt==2)
			{
			dev_info.dword[2]=dt;
			sig_cnt++;
			}
		if (sig_cnt==1) //прочитали вторую часть сигнатуры
			{
			if	(dt==0x7D464E49) //если вторая часть "INF}"
				sig_cnt++;
			else sig_cnt=0;
			}
		if (dt==0x5645447B)
			sig_cnt=1; //если встретили имя "{DEV"


		crc_reg = crc32(reverse_32(dt));
		len -= 4;
		adr+=4;
		}
	crc = reverse_32(crc_reg);
	crc=~crc;
	return (crc);
//#endif
}
#endif

/**
  * @brief  Вычисляет 32-bit CRC для буфера данных
  *         CRC32 вычисляется по алгоритму аналогичному вычислению CRC в протокгое Ethernet.
  * @param  *buf: указатель на буфер с данныеми для которых надо произвести вычисление
  * @param	len: длина буфера для вычисления
  * @param  clear: 0 - учитываеть результат предыдущего вычисления (рассчёт с накоплением); 1 - не учитывыть (очистить) результат предыдущего вычисления (рассчёт без накопления)
  * @retval 32-bit CRC
  */
uint32_t crc32_ether(uint8_t *buf, int len, int clear)
{

#ifdef CRC_USE_HAL
	uint32_t res = HAL_CRC_Calculate(&hcrc, (uint32_t*)buf, len);
	return ~res;
#else
uint32_t *p = (uint32_t*) buf;
uint32_t crc, crc_reg=0;

	//if(clear) CRC->CR = CRC_CR_RESET; //Делаем сброс CRC_ResetDR();
	if(clear) __HAL_CRC_DR_RESET(&hcrc);

	while(len >= 4)
		{
		crc_reg = crc32(reverse_32(*p++));
		len -= 4;
		}
        crc = reverse_32(crc_reg);
        if(len) {
                crc32(crc_reg);
                switch(len) {
                        case 1:
                        crc_reg = crc32(reverse_32((*p & 0xFF) ^ crc) >> 24);
                        crc = ( crc >> 8 ) ^ reverse_32(crc_reg);
                        break;
                        case 2:
                        crc_reg = crc32(reverse_32((*p & 0xFFFF) ^ crc) >> 16);
                        crc = ( crc >> 16 ) ^ reverse_32(crc_reg);
                        break;
                        case 3:
                        crc_reg = crc32(reverse_32((*p & 0xFFFFFF) ^ crc) >> 8);
                        crc = ( crc >> 24 ) ^ reverse_32(crc_reg);
                        break;
                }
        }
        crc=~crc;
        return (crc);
#endif
}

/**
  * @brief  Вычисляет 8-bit CRC используя предыдущий результат и новое значение
  *         CRC8 вычисляется по алгоритму аналогичному вычислению CRC в протокгое 1-wire.
  * @param  data: байт новых данных для которого надо посчитать CRC
  * @param	crc: результат предыдущего вычисления
  * @retval 8-bit CRC
  */
unsigned char crc8(unsigned char data, unsigned char crc)
{unsigned char n; unsigned char cnt=8;
	do
		{
		n=(data^crc)&0x01;
		crc>>=1; data>>=1;
		if(n) crc^=0x8C;
		}
	while(--cnt);
	crc8_old=crc;
	return(crc);
}

/**
  * @brief  Вычисляет 8-bit CRC для буфера данных
  *         CRC32 вычисляется по алгоритму аналогичному вычислению CRC в протокгое 1-wire.
  * @param  *buf: указатель на буфер с данныеми для которых надо произвести вычисление
  * @param	len: длина буфера для вычисления
  * @param  clear: 0 - учитываеть результат предыдущего вычисления (рассчёт с накоплением); 1 - не учитывыть (очистить) результат предыдущего вычисления (рассчёт без накопления)
  * @retval 8-bit CRC
  */
uint8_t crc8_1wire(void *buf, int len, unsigned char clear)
{uint8_t crc;
uint16_t cnt;

	if (clear) crc=0;
	else crc=crc8_old;

	for (cnt=0; cnt<len; cnt++)
		crc=crc8(((uint8_t*)buf)[cnt], crc);

	return(crc);
}

/**
  * @brief  Функция подсчёта CRC для линии ModBus_RTU
  *
  * @param  *data: указатель на массив
  * 		len: размер данных
  *
  * @retval CRC16 для ModBusRTU
  */
uint16_t modbus_crc_rtu(uint8_t* data, uint16_t len)
{
	if (len > 256) return 0;

	uint8_t crc_hi = 0xFF;
	uint8_t crc_lo = 0xFF;
	uint32_t i;
	int n;
	for (n = 0; n < len; n++)
		{
	   i = crc_hi ^ *(data++);
	   crc_hi = crc_lo ^ auchCRCHi[i];
	   crc_lo = auchCRCLo[i];
		}
	//if (len%2) crc_hi^=0xf0;

	return ((crc_hi << 8) | crc_lo);
}
