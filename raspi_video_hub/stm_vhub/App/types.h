/*
 * types.h
 *
 *  Created on: 18 марта 2015 г.
 *      Author: Перчиц А.Н.
 */

#ifndef TYPES_H_
#define TYPES_H_
#include "stm32f1xx_hal.h"

// Переопределения стандартных типов
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed long int32_t;
typedef unsigned long uint32_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;

typedef union
	{
	uint8_t byte[8];
	uint16_t word[4];
	uint32_t dword[2];
	uint64_t qword;
	}udata64_t;
typedef union
	{
	uint8_t byte[4];
	uint16_t word[2];
	uint32_t dword;
	}udata32_t;
typedef union
	{
	uint8_t byte[2];
	uint16_t word;
	}udata16_t;
typedef union
	{
	struct
		{
		uint8_t b1:1;
		uint8_t b2:1;
		uint8_t b3:1;
		uint8_t b4:1;
		uint8_t b5:1;
		uint8_t b6:1;
		uint8_t b7:1;
		uint8_t b8:1;
		}bit;
	uint8_t byte;
	}udata8_t;


#endif /* TYPES_H_ */
