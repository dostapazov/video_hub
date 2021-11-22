/*
 * dio.h
 *
 *  Created on: 7 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */

#ifndef APP_DRIVERS_DIO_H_
#define APP_DRIVERS_DIO_H_
#include "hal_includes.h"
#include "main.h"

#define DIO_FILTER_SET 10//32 //антидребезг в мс.

//#define  POL1 K1_GPIO_Port, K1_Pin
//#define  POL2 K2_GPIO_Port, K2_Pin

#define PRESENT_1RP		GPIO3_5RP_GPIO_Port, GPIO3_5RP_Pin
#define PRESENT_2RP		GPIO3_1RP_GPIO_Port, GPIO3_1RP_Pin
#define PRESENT_3RP		GPIO3_3RP_GPIO_Port, GPIO3_3RP_Pin
#define PRESENT_4RP		GPIO3_4RP_GPIO_Port, GPIO3_4RP_Pin
#define PRESENT_5RP		GPIO3_2RP_GPIO_Port, GPIO3_2RP_Pin

#define  P1_ON_ACTIV GPIO_PIN_RESET	 //активный сигнал
#define  P2_ON_ACTIV GPIO_PIN_RESET
#define  P3_ON_ACTIV GPIO_PIN_RESET
#define  P4_ON_ACTIV GPIO_PIN_RESET
#define  P5_ON_ACTIV GPIO_PIN_RESET

#define DIO_OUTS_CNT 5
#define RST_1RP			RST_5RP_GPIO_Port, RST_5RP_Pin
#define RST_2RP			RST_1RP_GPIO_Port, RST_1RP_Pin
#define RST_3RP			RST_3RP_GPIO_Port, RST_3RP_Pin
#define RST_4RP			RST_4RP_GPIO_Port, RST_4RP_Pin
#define RST_5RP			RST_2RP_GPIO_Port, RST_2RP_Pin


typedef struct
{
	GPIO_TypeDef*	port;
	uint16_t		pin;
}dio_outs_t;

uint8_t dio_out_read(void);			//Читает текущее сосотяние выходов
uint8_t dio_out_bit_read(uint8_t number);//Читает текущее сосотяние одного выхода
void dio_out_write(uint8_t data);	//Установливает заданное значение дискретных выходов
void dio_out_bit_write(uint8_t number, uint8_t data); //Установливает заданное значение указанного выхода
uint8_t dio_in_read(void);			//Читает текущее сосотяние входов
uint8_t dio_in_filter(void);		//Читает текущее сосотяние входов с функцией андидребезга
void dio_set_filter(uint16_t filter_ms); //Устанавливает величину фильтра антидребезга
uint8_t dio_key_press(void); //Возвращает сосотяния кнопок, которые нажимались и уже отпущены
void dio_key_reset(void);	//Очищает (сбрасывает) список нажатых кнопок

#endif /* APP_DRIVERS_DIO_H_ */
