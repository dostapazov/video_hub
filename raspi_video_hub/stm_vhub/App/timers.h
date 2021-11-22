/*
 * timers.h
 *
 *  Created on: 4 ���. 2016 �.
 *      Author: ������ �.�.
 */

#ifndef APP_DRIVERS_TIMERS_H_
#define APP_DRIVERS_TIMERS_H_
#include "stm32f1xx_hal.h"

uint32_t timers_get_finish_time(uint32_t delay); //��������� �������� ���������� ������� ��� �������� �������� �������� ������� ��������
uint32_t timers_get_time_left(uint32_t time); //��������� ������� �������� ������� �� ���������� ��������� �������� ���������� �������
uint32_t timers_diff_tick(uint32_t t2, uint32_t t1);//���������� ������� ����� ����� ��������� ������� � ������ ������������

#endif /* APP_DRIVERS_TIMERS_H_ */
