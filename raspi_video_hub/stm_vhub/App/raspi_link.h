/*
 * raspi_link.h
 *
 *  Created on: 7 сент. 2018 г.
 *      Author: user
 */

#ifndef APP_RASPI_LINK_H_
#define APP_RASPI_LINK_H_
#include "hal_includes.h"
//
//
//
#define _CU_SIGNATURE_  (uint8_t)0x5a
#define _RP_SIGNATURE_  (uint8_t)0xa5

//
//
//
#pragma pack(1)
typedef struct
{
	uint8_t  signture;
	uint8_t  devId;
	uint8_t  pckType;
	uint8_t size;
}PCK_Header_t;


typedef struct
{
	uint8_t camId;
}PCK_CAM_t;

typedef struct
{
	uint8_t sec; //секунда
	uint8_t min; //минута
	uint8_t h; //час
	uint8_t d; //день
	uint8_t m; //месяц
	uint8_t y; //год
}PCK_TIME_t;


typedef struct
{
	uint8_t  camId;
	uint8_t  fanState;
	uint16_t  temper; //x1000 celsius degrees
}PCK_STATE_t;

#pragma pack()


enum PCK_Type
{
    PCT_SHUTDOWN = 0,
    PCT_CAM_SWITCH,
	PCT_SET_TIME,
    PCT_STATE
};


void raspi_link_init(UART_HandleTypeDef* init);
void raspi_link_set_cam(uint8_t raspi_id, uint8_t cam_id);
void raspi_link_set_time(uint8_t sec, uint8_t min, uint8_t h, uint8_t d, uint8_t m, uint8_t y);
void raspi_link_step(void);
void raspi_link_swith_cam(uint8_t raspi_id, uint8_t cam_id);

extern uint8_t cam_st_out[4];
extern uint8_t cam_set_out[4];

#endif /* APP_RASPI_LINK_H_ */
