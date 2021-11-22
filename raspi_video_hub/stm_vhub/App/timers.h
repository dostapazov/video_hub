/*
 * timers.h
 *
 *  Created on: 4 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */

#ifndef APP_DRIVERS_TIMERS_H_
#define APP_DRIVERS_TIMERS_H_
#include "stm32f1xx_hal.h"

uint32_t timers_get_finish_time(uint32_t delay); //Вычисляет значение системного таймера при значении которого истекает заданая задержка
uint32_t timers_get_time_left(uint32_t time); //Вычисляет сколько осталось времени до достижения заданного значения системного таймера
uint32_t timers_diff_tick(uint32_t t2, uint32_t t1);//Вычисление разницы между двумя отсчетами времени с учетом переполнения

#endif /* APP_DRIVERS_TIMERS_H_ */
