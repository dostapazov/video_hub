/*
 * timers.c
 *
 *  Created on: 4 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "timers.h"

/**
  * @brief  Вычисляет значение системного таймера при значении которого истекает заданая задержка
  *
  * @param  delay: заданное время задержки в мс.
  *
  * @retval значение системного таймера при достижении заданной задержки
  */
uint32_t timers_get_finish_time(uint32_t delay)
{
	return(HAL_GetTick()+delay);
}
/**
  * @brief  Вычисляет сколько осталось времени до достижения заданного значения системного таймера
  *
  * @param  time: значение системного таймера, значение которого необходимо достичь
  *
  * @retval значение в мс, которе осталось до достижения заданного времени
  */
uint32_t timers_get_time_left(uint32_t time)
{uint32_t s_time;

 	s_time=HAL_GetTick();
 	if (time<s_time)
 		return(0);
 	else
 		return(time-s_time);
}

/**
  * @brief Вычисление разницы между двумя отсчетами времени с учетом переполнения
  *
  * @param  t2: ворая точка времени (более поздняя)
  * 		t1: первая точка времени
  *
  * @retval разнимца между тиками
  */
uint32_t timers_diff_tick(uint32_t t2, uint32_t t1)
{
	if (t2<t1)
		return(0);
	else
		return(t2 - t1);
}
