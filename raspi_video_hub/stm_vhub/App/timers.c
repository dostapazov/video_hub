/*
 * timers.c
 *
 *  Created on: 4 ���. 2016 �.
 *      Author: ������ �.�.
 */
#include "timers.h"

/**
  * @brief  ��������� �������� ���������� ������� ��� �������� �������� �������� ������� ��������
  *
  * @param  delay: �������� ����� �������� � ��.
  *
  * @retval �������� ���������� ������� ��� ���������� �������� ��������
  */
uint32_t timers_get_finish_time(uint32_t delay)
{
	return(HAL_GetTick()+delay);
}
/**
  * @brief  ��������� ������� �������� ������� �� ���������� ��������� �������� ���������� �������
  *
  * @param  time: �������� ���������� �������, �������� �������� ���������� �������
  *
  * @retval �������� � ��, ������ �������� �� ���������� ��������� �������
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
  * @brief ���������� ������� ����� ����� ��������� ������� � ������ ������������
  *
  * @param  t2: ����� ����� ������� (����� �������)
  * 		t1: ������ ����� �������
  *
  * @retval �������� ����� ������
  */
uint32_t timers_diff_tick(uint32_t t2, uint32_t t1)
{
	if (t2<t1)
		return(0);
	else
		return(t2 - t1);
}
