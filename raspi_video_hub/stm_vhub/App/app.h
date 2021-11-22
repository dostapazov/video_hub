/*
 * app.h
 *
 *  Created on: 6 сент. 2018 г.
 *      Author: user
 *
 *      v.1.1 - изменён протокол обмена, добавлено поле выбора активного канала
 */

#ifndef APP_APP_H_
#define APP_APP_H_

#include "hal_includes.h"
#include "types.h"


#define LED_BLINK_TIME 1000
#define LED_BLINK_FAST 300


#define VID			3
#define PID			0
#define SOFT_REV	1
#define SOFT_MOD	2
#define HARD_REV	1
#define HARD_MOD	0

//
//Public definitions
//

#define _RASPI_COUNT_				4			//число малинок
#define	_RESET_DURATION_			50			//длительность удержания RESET малинки (msec)
#define _RASPI_START_DURATION_		60000		//время запуска "малинки"
#define _RASPI_WD_INTERVAL_			10000 		//таймаут "моргалки" малинки
#define _MCU_RESET_DURATION_		3000		//интервал удержания кнопки "RESET" для перезапуска контроллера

#define _CAM_SWITCH_INTERVAL_		60000		//DEBUG

//PIN definitions
#define LED_RED			LED_RED_GPIO_Port, LED_RED_Pin
#define LED_GREEN		LED_GREEN_GPIO_Port, LED_GREEN_Pin
#define BUT_RST			BUT_RST_GPIO_Port, BUT_RST_Pin


//
//Public types definitions
//
typedef struct
{
	uint32_t		wait_time;
	uint32_t		rst_time;
	uint32_t		pulse_time;
}BTN_PIN_t;


//! Work mode
enum
{
	WM_STARTUP = 0,		//запуск
	WM_PART_READY,		//частично готов
	WM_WORKING,			//норм. работа
	WM_ERROR			//ошибка
};

//==================================================ETHERENET=====================================
#define PC_LINK_TX_INTERVAL 300 //максимально допустимый интервал между отправками пакетов
#define PC_LINK_TX_MIN_INTERVAL 10 //максимально допустимый интервал между отправками пакетов
#define PC_LINK_MAX_RX_PACK_TIME 400	//максимальное выделенное время на приём пакета (если в течении этого времени нет приёма пакета, то буфер очищается)

#define PC_LINK_VERSION 1
#define PC_LINK_TX_FLAG    0x5A
#define PC_LINK_RX_FLAG    0xA5
#define PC_LINK_SESSION_ID 0x1122

#define PC_BUF_SIZE 1536  //

#define MIN(a,b) ( (a)<(b) ? (a) :(b) )
#define MAX(a,b) ( (a)>(b) ? (a) :(b) )
#pragma pack(1)
//-----------------------------ПРОТОКОЛ ОБМЕНА С ПК--------------------

typedef union
{
	struct
		{
		uint8_t channel:2;
		uint8_t reserv:5;
		uint8_t active:1;
		}bit;
	uint8_t byte;
} channel_t;

typedef struct _header_t
{
	channel_t  mode;
	uint16_t session;
	uint16_t number;
	uint16_t version;
	uint8_t type;
	uint16_t size;
} header_t;

typedef struct _cam_data_t
{
	uint16_t err;
	uint8_t  out[4];
} cam_data_t;

typedef union
	{
	struct
		{
 		header_t    header;
 		cam_data_t  data;
 		uint32_t    crc32;
		}cam;
	struct
		{
		header_t  header;
		uint16_t  msec;
		uint8_t sec;
		uint8_t min;
		uint8_t h;
		uint8_t d;
		uint8_t m;
		uint8_t y;
		uint32_t    crc32;
		}time;
	} pc_link_pack_t;
#pragma pack()

#define DINF_SIGNATURE "{DEVINF}"
typedef struct
{
    uint8_t  signature[8]; //"{DEV_INFO}"
    uint8_t	vid;
    uint8_t	pid;
    uint8_t	hw_rev;
    uint8_t	hw_mod;
    uint8_t	sw_rev;
    uint8_t	sw_mod;
}DEV_INFO_t;
//
//Public declarations
//


void app_init();
void app_step();


#endif /* APP_APP_H_ */
